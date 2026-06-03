#include "update/UpdateSubject.h"
#include <algorithm>

void UpdateSubject::AddObserver(IUpdateObserver* observer)
{
    if (!observer)
    {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex);
    observers.push_back(observer);
}

void UpdateSubject::RemoveObserver(IUpdateObserver* observer)
{
    if (!observer)
    {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex);
    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}

void UpdateSubject::NotifyUpdateAvailable(const std::string& version, const std::string& url)
{
    std::lock_guard<std::mutex> lock(mutex);
    for (auto* o : observers) o->OnUpdateAvailable(version, url);
}

void UpdateSubject::NotifyUpdateNotAvailable()
{
    std::lock_guard<std::mutex> lock(mutex);
    for (auto* o : observers) o->OnUpdateNotAvailable();
}

void UpdateSubject::NotifyUpdateCheckFailed(const std::string& error)
{
    std::lock_guard<std::mutex> lock(mutex);
    for (auto* o : observers) o->OnUpdateCheckFailed(error);
}

void UpdateSubject::NotifyUpdateDownloadProgress(double progress)
{
    std::lock_guard<std::mutex> lock(mutex);
    for (auto* o : observers) o->OnUpdateDownloadProgress(progress);
}

void UpdateSubject::NotifyUpdateReady(const std::string& newBinaryPath)
{
    std::lock_guard<std::mutex> lock(mutex);
    for (auto* o : observers) o->OnUpdateReady(newBinaryPath);
}

void UpdateSubject::NotifyUpdateFailed(const std::string& error)
{
    std::lock_guard<std::mutex> lock(mutex);
    for (auto* o : observers) o->OnUpdateFailed(error);
}
