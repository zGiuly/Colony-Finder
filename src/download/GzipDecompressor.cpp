#include "GzipDecompressor.h"
#include <zlib.h>
#include <fstream>
#include <filesystem>
#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <algorithm>

namespace
{
    struct Chunk
    {
        std::vector<unsigned char> data;
        size_t size = 0;
    };

    using ChunkPtr = std::shared_ptr<Chunk>;

    class BoundedQueue
    {
    public:
        explicit BoundedQueue(size_t capacity) : m_capacity(capacity) {}

        bool Push(ChunkPtr item)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cvNotFull.wait(lock, [&] { return m_items.size() < m_capacity || m_closed; });
            if (m_closed)
            {
                return false;
            }
            m_items.push(std::move(item));
            m_cvNotEmpty.notify_one();
            return true;
        }

        bool Pop(ChunkPtr& out)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cvNotEmpty.wait(lock, [&] { return !m_items.empty() || m_closed; });
            if (m_items.empty())
            {
                return false;
            }
            out = std::move(m_items.front());
            m_items.pop();
            m_cvNotFull.notify_one();
            return true;
        }

        void Close()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_closed = true;
            m_cvNotEmpty.notify_all();
            m_cvNotFull.notify_all();
        }

    private:
        std::queue<ChunkPtr> m_items;
        size_t m_capacity;
        std::mutex m_mutex;
        std::condition_variable m_cvNotEmpty;
        std::condition_variable m_cvNotFull;
        bool m_closed = false;
    };
}

bool GzipDecompressor::Decompress(const std::string& sourcePath, const std::string& destPath, std::function<void(float)> onProgress, std::atomic<bool>& cancelFlag, int bufferSizeMb)
{
    std::vector<char> outStreamBuffer(8 * 1024 * 1024);
    std::ofstream out;
    out.rdbuf()->pubsetbuf(outStreamBuffer.data(), outStreamBuffer.size());
    out.open(destPath, std::ios::binary);
    if (!out.is_open())
    {
        return false;
    }

    Sink sink = [&out](std::vector<unsigned char>&& data) -> bool {
        out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
        return static_cast<bool>(out);
    };

    return DecompressToSink(sourcePath, sink, onProgress, cancelFlag, bufferSizeMb);
}

bool GzipDecompressor::DecompressToSink(const std::string& sourcePath, Sink sink, std::function<void(float)> onProgress, std::atomic<bool>& cancelFlag, int bufferSizeMb)
{
    int safeBufferSizeMb = std::clamp(bufferSizeMb, 1, 16);
    const size_t chunkSize = static_cast<size_t>(safeBufferSizeMb) * 1024 * 1024;
    const size_t ioBufferSize = 8 * 1024 * 1024;
    const size_t queueDepth = 4;

    std::vector<char> inStreamBuffer(ioBufferSize);
    std::ifstream in;
    in.rdbuf()->pubsetbuf(inStreamBuffer.data(), inStreamBuffer.size());
    in.open(sourcePath, std::ios::binary);
    if (!in.is_open())
    {
        return false;
    }

    uint64_t totalBytes = std::filesystem::file_size(sourcePath);
    if (totalBytes == 0)
    {
        return false;
    }

    z_stream strm{};
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    if (inflateInit2(&strm, 16 + MAX_WBITS) != Z_OK)
    {
        return false;
    }

    BoundedQueue inQueue(queueDepth);
    BoundedQueue outQueue(queueDepth);
    std::atomic<bool> failed{false};
    std::atomic<uint64_t> readBytes{0};

    std::thread readerThread([&] {
        while (!cancelFlag.load() && !failed.load())
        {
            auto chunk = std::make_shared<Chunk>();
            chunk->data.resize(chunkSize);
            in.read(reinterpret_cast<char*>(chunk->data.data()), chunkSize);
            std::streamsize n = in.gcount();
            if (n <= 0)
            {
                break;
            }
            chunk->size = static_cast<size_t>(n);
            readBytes.fetch_add(static_cast<uint64_t>(n), std::memory_order_relaxed);
            if (!inQueue.Push(std::move(chunk)))
            {
                break;
            }
        }
        inQueue.Close();
    });

    std::thread writerThread([&] {
        ChunkPtr chunk;
        while (outQueue.Pop(chunk))
        {
            if (cancelFlag.load())
            {
                failed.store(true);
                break;
            }
            chunk->data.resize(chunk->size);
            if (!sink(std::move(chunk->data)))
            {
                failed.store(true);
                break;
            }
        }
    });

    int ret = Z_OK;
    bool success = true;
    float lastNotifiedProgress = -1.0f;
    ChunkPtr inChunk;

    while (inQueue.Pop(inChunk))
    {
        if (cancelFlag.load() || failed.load())
        {
            success = false;
            break;
        }

        float currentProgress = static_cast<float>(readBytes.load(std::memory_order_relaxed)) / static_cast<float>(totalBytes);
        if (currentProgress - lastNotifiedProgress >= 0.005f)
        {
            lastNotifiedProgress = currentProgress;
            if (onProgress)
            {
                onProgress(currentProgress);
            }
        }

        strm.next_in = inChunk->data.data();
        strm.avail_in = static_cast<uInt>(inChunk->size);

        while (strm.avail_in > 0)
        {
            auto outChunk = std::make_shared<Chunk>();
            outChunk->data.resize(chunkSize);
            strm.next_out = outChunk->data.data();
            strm.avail_out = static_cast<uInt>(chunkSize);

            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_NEED_DICT || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR)
            {
                success = false;
                break;
            }

            outChunk->size = chunkSize - strm.avail_out;
            if (outChunk->size > 0)
            {
                if (!outQueue.Push(std::move(outChunk)))
                {
                    success = false;
                    break;
                }
            }

            if (ret == Z_STREAM_END)
            {
                break;
            }
        }

        if (!success || ret == Z_STREAM_END)
        {
            break;
        }
    }

    inQueue.Close();
    outQueue.Close();

    if (readerThread.joinable())
    {
        readerThread.join();
    }
    if (writerThread.joinable())
    {
        writerThread.join();
    }

    inflateEnd(&strm);

    bool ok = success && !failed.load() && (ret == Z_STREAM_END);
    if (ok && onProgress)
    {
        onProgress(1.0f);
    }
    return ok;
}
