#include "download/HttpDownloader.h"
#include <curl/curl.h>
#include <fstream>
#include <thread>
#include <vector>
#include <chrono>

HttpDownloader::HttpDownloader() : isCancelled(false), totalDownloadedBytes(0), totalFileLength(0)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

size_t HttpDownloader::HeaderCallback(char*, size_t size, size_t nitems, void*)
{
    return size * nitems;
}

size_t HttpDownloader::WriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    DownloadChunk* chunk = static_cast<DownloadChunk*>(userdata);
    if (!chunk || chunk->cancelFlag->load())
    {
        return 0;
    }

    FILE* fp = static_cast<FILE*>(chunk->fileHandle);
    if (!fp)
    {
        return 0;
    }

    size_t written = fwrite(ptr, size, nmemb, fp);
    chunk->downloaded += written;
    chunk->downloader->totalDownloadedBytes += written;

    return written;
}

double HttpDownloader::GetContentLength(const std::string& url)
{
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        return -1.0;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 8L);

    double contentLength = -1.0;
    if (curl_easy_perform(curl) == CURLE_OK)
    {
        curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentLength);
    }

    curl_easy_cleanup(curl);
    return contentLength;
}

void HttpDownloader::DownloadChunkFunc(DownloadChunk chunk)
{
    FILE* fp = nullptr;
    const char* mode = (chunk.downloader->totalFileLength == (chunk.end + 1 - chunk.start)) ? "wb" : "rb+";
#ifdef _WIN32
    if (fopen_s(&fp, chunk.path.c_str(), mode) != 0)
    {
        fp = nullptr;
    }
#else
    fp = fopen(chunk.path.c_str(), mode);
#endif
    if (!fp)
    {
        return;
    }

    if (mode == std::string("rb+"))
    {
#ifdef _WIN32
        _fseeki64(fp, static_cast<__int64>(chunk.start + chunk.downloaded), SEEK_SET);
#else
        fseeko(fp, static_cast<off_t>(chunk.start + chunk.downloaded), SEEK_SET);
#endif
    }
    chunk.fileHandle = fp;

    CURL* curl = curl_easy_init();
    if (!curl)
    {
        fclose(fp);
        return;
    }

    std::string rangeHeader = std::to_string(chunk.start) + "-" + std::to_string(chunk.end);

    curl_easy_setopt(curl, CURLOPT_URL, chunk.url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
    curl_easy_setopt(curl, CURLOPT_RANGE, rangeHeader.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    fclose(fp);
}

bool HttpDownloader::Download(const std::string& url, const std::string& destinationPath, int threadCount)
{
    isCancelled = false;
    totalDownloadedBytes = 0;

    double contentLength = GetContentLength(url);
    if (contentLength <= 0.0)
    {
        NotifyDownloadFailed("Failed to retrieve file size.");
        return false;
    }

    size_t totalSize = static_cast<size_t>(contentLength);
    totalFileLength = totalSize;
    NotifyDownloadStarted(static_cast<double>(totalSize));

    if (threadCount > 1)
    {
        std::ofstream out(destinationPath, std::ios::binary | std::ios::out);
        if (!out.is_open())
        {
            NotifyDownloadFailed("Failed to create destination file.");
            return false;
        }
        out.seekp(totalSize - 1);
        out.write("", 1);
        out.close();
    }

    size_t chunkSize = totalSize / threadCount;
    std::vector<std::thread> threads;
    std::vector<DownloadChunk> chunks;

    for (int i = 0; i < threadCount; ++i)
    {
        size_t start = i * chunkSize;
        size_t end = (i == threadCount - 1) ? (totalSize - 1) : (start + chunkSize - 1);
        chunks.push_back({static_cast<size_t>(i), url, destinationPath, start, end, 0, &isCancelled, this, nullptr});
    }

    auto startTime = std::chrono::steady_clock::now();
    auto lastTime = startTime;
    size_t lastDownloaded = 0;

    for (int i = 0; i < threadCount; ++i)
    {
        threads.emplace_back(&HttpDownloader::DownloadChunkFunc, this, chunks[i]);
    }

    while (totalDownloadedBytes < totalSize)
    {
        if (isCancelled.load())
        {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        auto currentTime = std::chrono::steady_clock::now();
        double elapsedSeconds = std::chrono::duration<double>(currentTime - lastTime).count();
        size_t currentDownloaded = totalDownloadedBytes.load();
        double speed = 0.0;
        if (elapsedSeconds > 0.0)
        {
            speed = static_cast<double>(currentDownloaded - lastDownloaded) / elapsedSeconds;
        }

        lastTime = currentTime;
        lastDownloaded = currentDownloaded;

        double progress = static_cast<double>(currentDownloaded) / static_cast<double>(totalSize);

        NotifyDownloadProgress(progress, speed);
    }

    for (auto& t : threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }

    if (isCancelled.load())
    {
        NotifyDownloadFailed("Download cancelled.");
        return false;
    }

    if (totalDownloadedBytes < totalSize)
    {
        NotifyDownloadFailed("Download incomplete.");
        return false;
    }

    NotifyDownloadCompleted(destinationPath);
    return true;
}

void HttpDownloader::Cancel()
{
    isCancelled = true;
}

bool HttpDownloader::IsCancelled() const
{
    return isCancelled.load();
}
