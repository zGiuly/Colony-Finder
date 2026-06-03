#include "download/StreamingProcessor.h"
#include "download/GzipDecompressor.h"
#include "download/LineSink.h"
#include "search/SimdjsonIndexer.h"
#include "search/SystemIndex.h"
#include <fstream>
#include <filesystem>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>
#include <algorithm>

namespace
{
    constexpr size_t MergeBufferSize = 4 * 1024 * 1024;
    constexpr size_t LineQueueDepth = 4096;
    constexpr unsigned int MaxWorkers = 8;

    template<typename T>
    class BoundedQueue
    {
    public:
        explicit BoundedQueue(size_t cap) : m_capacity(cap) {}

        bool Push(T item)
        {
            std::unique_lock<std::mutex> lk(m_mutex);
            m_cvNotFull.wait(lk, [&] { return m_items.size() < m_capacity || m_closed; });
            if (m_closed) return false;
            m_items.push(std::move(item));
            m_cvNotEmpty.notify_one();
            return true;
        }

        bool Pop(T& out)
        {
            std::unique_lock<std::mutex> lk(m_mutex);
            m_cvNotEmpty.wait(lk, [&] { return !m_items.empty() || m_closed; });
            if (m_items.empty()) return false;
            out = std::move(m_items.front());
            m_items.pop();
            m_cvNotFull.notify_one();
            return true;
        }

        void Close()
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            m_closed = true;
            m_cvNotEmpty.notify_all();
            m_cvNotFull.notify_all();
        }

    private:
        std::queue<T> m_items;
        size_t m_capacity;
        std::mutex m_mutex;
        std::condition_variable m_cvNotEmpty;
        std::condition_variable m_cvNotFull;
        bool m_closed = false;
    };

    struct WorkerCtx
    {
        std::filesystem::path recPath;
        std::filesystem::path strPath;
        std::vector<char> recBuf;
        std::vector<char> strBuf;
        std::ofstream recFile;
        std::ofstream strFile;
        uint64_t systemCount = 0;
        uint64_t stringOffset = 0;
        std::unique_ptr<SimdjsonIndexer> indexer;
        std::thread thread;
    };
}

StreamingProcessor::Result StreamingProcessor::Process(const std::string& gzPath,
                                                       const std::string& schemaPath,
                                                       const std::string& indexPath,
                                                       std::atomic<float>& decompressProgress,
                                                       std::atomic<float>& indexProgress,
                                                       std::atomic<bool>& cancelFlag,
                                                       int bufferSizeMb)
{
    (void)schemaPath;
    Result result;

    std::filesystem::path indexDir = std::filesystem::path(indexPath).parent_path();

    unsigned int hw = std::thread::hardware_concurrency();
    unsigned int workerCount = std::clamp(hw == 0 ? 4u : hw - 2u, 2u, MaxWorkers);

    std::vector<std::unique_ptr<WorkerCtx>> workers;
    workers.reserve(workerCount);

    auto cleanupWorkerFiles = [&] {
        std::error_code ec;
        for (auto& w : workers)
        {
            std::filesystem::remove(w->recPath, ec);
            std::filesystem::remove(w->strPath, ec);
        }
    };

    for (unsigned int i = 0; i < workerCount; ++i)
    {
        auto w = std::make_unique<WorkerCtx>();
        w->recPath = indexDir / ("records_" + std::to_string(i) + ".tmp");
        w->strPath = indexDir / ("strings_" + std::to_string(i) + ".tmp");
        w->recBuf.resize(4 * 1024 * 1024);
        w->strBuf.resize(2 * 1024 * 1024);
        w->recFile.rdbuf()->pubsetbuf(w->recBuf.data(), w->recBuf.size());
        w->strFile.rdbuf()->pubsetbuf(w->strBuf.data(), w->strBuf.size());
        w->recFile.open(w->recPath, std::ios::binary);
        w->strFile.open(w->strPath, std::ios::binary);
        if (!w->recFile.is_open() || !w->strFile.is_open())
        {
            for (auto& wp : workers)
            {
                if (wp->recFile.is_open()) wp->recFile.close();
                if (wp->strFile.is_open()) wp->strFile.close();
            }
            workers.push_back(std::move(w));
            cleanupWorkerFiles();
            result.errorMessage = "Failed to open worker tmp files.";
            return result;
        }
        w->indexer = std::make_unique<SimdjsonIndexer>(w->recFile, w->strFile, w->systemCount, w->stringOffset, cancelFlag);
        workers.push_back(std::move(w));
    }

    BoundedQueue<std::vector<char>> lineQueue(LineQueueDepth);
    std::atomic<bool> failed{false};

    for (auto& w : workers)
    {
        WorkerCtx* wp = w.get();
        wp->thread = std::thread([wp, &lineQueue, &failed, &cancelFlag] {
            std::vector<char> line;
            while (lineQueue.Pop(line))
            {
                if (cancelFlag.load()) { failed.store(true); break; }
                if (!wp->indexer->ProcessLine(line.data(), line.size()))
                {
                    failed.store(true);
                    break;
                }
            }
        });
    }

    LineSink lineSink([&](const char* data, size_t length) -> bool {
        if (cancelFlag.load() || failed.load()) return false;
        std::vector<char> line(data, data + length);
        return lineQueue.Push(std::move(line));
    });

    auto onDecompressProgress = [&](float p) {
        decompressProgress.store(p);
        indexProgress.store(p);
    };

    GzipDecompressor::Sink sink = [&lineSink](std::vector<unsigned char>&& data) -> bool {
        return lineSink.Consume(data.data(), data.size());
    };

    bool decompressOk = GzipDecompressor::DecompressToSink(gzPath, sink, onDecompressProgress, cancelFlag, bufferSizeMb);

    if (decompressOk)
    {
        lineSink.Flush();
    }

    lineQueue.Close();

    for (auto& w : workers)
    {
        if (w->thread.joinable()) w->thread.join();
        w->recFile.close();
        w->strFile.close();
    }

    if (cancelFlag.load())
    {
        result.cancelled = true;
        cleanupWorkerFiles();
        return result;
    }

    if (!decompressOk)
    {
        result.decompressionFailed = true;
        result.errorMessage = "Decompression failed. The downloaded file may be corrupted.";
        cleanupWorkerFiles();
        return result;
    }

    uint64_t totalSystems = 0;
    for (auto& w : workers) totalSystems += w->systemCount;

    if (totalSystems == 0)
    {
        result.indexingFailed = true;
        result.errorMessage = "Indexing produced no systems. The JSON format may be unexpected.";
        cleanupWorkerFiles();
        return result;
    }

    std::ofstream outFile(indexPath, std::ios::binary);
    if (!outFile.is_open())
    {
        result.indexingFailed = true;
        result.errorMessage = "Failed to open index output file.";
        cleanupWorkerFiles();
        return result;
    }

    SystemIndex::Header header{};
    header.magic[0] = 'C'; header.magic[1] = 'F'; header.magic[2] = 'I';
    header.magic[3] = 'D'; header.magic[4] = 'X'; header.magic[5] = '\0';
    header.version = SystemIndex::Version;
    header.systemCount = totalSystems;
    header.stringTableOffset = sizeof(SystemIndex::Header) + totalSystems * sizeof(SystemIndex::Record);
    header.reserved = 0;

    outFile.write(reinterpret_cast<const char*>(&header), sizeof(header));

    uint64_t stringBase = 0;
    std::vector<char> mergeBuf(MergeBufferSize);

    for (auto& w : workers)
    {
        std::ifstream recIn(w->recPath, std::ios::binary);
        if (!recIn.is_open())
        {
            outFile.close();
            std::error_code ec;
            std::filesystem::remove(indexPath, ec);
            cleanupWorkerFiles();
            result.indexingFailed = true;
            result.errorMessage = "Failed to read worker records.";
            return result;
        }
        constexpr size_t RecsPerBatch = MergeBufferSize / sizeof(SystemIndex::Record);
        std::vector<SystemIndex::Record> recs(RecsPerBatch);
        while (recIn)
        {
            recIn.read(reinterpret_cast<char*>(recs.data()), RecsPerBatch * sizeof(SystemIndex::Record));
            std::streamsize bytes = recIn.gcount();
            if (bytes <= 0) break;
            size_t count = static_cast<size_t>(bytes) / sizeof(SystemIndex::Record);
            for (size_t i = 0; i < count; ++i)
            {
                recs[i].nameOffset = static_cast<uint32_t>(static_cast<uint64_t>(recs[i].nameOffset) + stringBase);
            }
            outFile.write(reinterpret_cast<const char*>(recs.data()), static_cast<std::streamsize>(count * sizeof(SystemIndex::Record)));
        }
        recIn.close();
        stringBase += w->stringOffset;
    }

    for (auto& w : workers)
    {
        std::ifstream strIn(w->strPath, std::ios::binary);
        while (strIn)
        {
            strIn.read(mergeBuf.data(), mergeBuf.size());
            std::streamsize n = strIn.gcount();
            if (n > 0) outFile.write(mergeBuf.data(), n);
        }
        strIn.close();
    }

    outFile.close();
    cleanupWorkerFiles();

    indexProgress.store(1.0f);
    result.success = true;
    return result;
}
