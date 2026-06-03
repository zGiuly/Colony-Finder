#include "download/DownloadSubject.h"
#include <algorithm>

void DownloadSubject::AddObserver(IDownloadObserver* observer)
{
    if (!observer)
    {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex);
    observers.push_back(observer);
}

void DownloadSubject::RemoveObserver(IDownloadObserver* observer)
{
    if (!observer)
    {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex);
    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}

void DownloadSubject::NotifyDownloadStarted(double totalSize)
{
    std::lock_guard<std::mutex> lock(mutex);
    for (auto* observer : observers)
    {
        observer->OnDownloadStarted(totalSize);
    }
}

void DownloadSubject::NotifyDownloadProgress(double progress, double speed)
{
    std::lock_guard<std::mutex> lock(mutex);
    for (auto* observer : observers)
    {
        observer->OnDownloadProgress(progress, speed);
    }
}

void DownloadSubject::NotifyDownloadCompleted(const std::string& filePath)
{
    std::lock_guard<std::mutex> lock(mutex);
    for (auto* observer : observers)
    {
        observer->OnDownloadCompleted(filePath);
    }
}

void DownloadSubject::NotifyDownloadFailed(const std::string& error)
{
    std::lock_guard<std::mutex> lock(mutex);
    for (auto* observer : observers)
    {
        observer->OnDownloadFailed(error);
    }
}
