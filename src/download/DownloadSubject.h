#pragma once

#include <vector>
#include <mutex>
#include <string>
#include "DownloadObserver.h"

class DownloadSubject
{
public:
    virtual ~DownloadSubject() = default;
    void AddObserver(IDownloadObserver* observer);
    void RemoveObserver(IDownloadObserver* observer);

protected:
    void NotifyDownloadStarted(double totalSize);
    void NotifyDownloadProgress(double progress, double speed);
    void NotifyDownloadCompleted(const std::string& filePath);
    void NotifyDownloadFailed(const std::string& error);

private:
    std::vector<IDownloadObserver*> observers;
    std::mutex mutex;
};
