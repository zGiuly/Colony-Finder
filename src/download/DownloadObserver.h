#pragma once

#include <string>

class IDownloadObserver
{
public:
    virtual ~IDownloadObserver() = default;
    virtual void OnDownloadStarted(double totalSize) = 0;
    virtual void OnDownloadProgress(double progress, double speed) = 0;
    virtual void OnDownloadCompleted(const std::string& filePath) = 0;
    virtual void OnDownloadFailed(const std::string& error) = 0;
};
