#pragma once

#include "download/DownloadSubject.h"
#include <string>
#include <atomic>

class HttpDownloader : public DownloadSubject
{
public:
    HttpDownloader();
    ~HttpDownloader() override = default;

    bool Download(const std::string& url, const std::string& destinationPath, int threadCount = 4);
    void Cancel();
    bool IsCancelled() const;

    static double GetContentLength(const std::string& url);

private:
    struct DownloadChunk
    {
        size_t id;
        std::string url;
        std::string path;
        size_t start;
        size_t end;
        size_t downloaded;
        std::atomic<bool>* cancelFlag;
        HttpDownloader* downloader;
        void* fileHandle;
    };

    std::atomic<bool> isCancelled;
    std::atomic<size_t> totalDownloadedBytes;
    size_t totalFileLength;

    static size_t HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata);
    static size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata);
    
    void DownloadChunkFunc(DownloadChunk chunk);
};
