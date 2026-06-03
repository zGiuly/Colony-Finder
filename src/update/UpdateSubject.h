#pragma once

#include "update/UpdateObserver.h"
#include <vector>
#include <mutex>
#include <string>

class UpdateSubject
{
public:
    virtual ~UpdateSubject() = default;
    void AddObserver(IUpdateObserver* observer);
    void RemoveObserver(IUpdateObserver* observer);

protected:
    void NotifyUpdateAvailable(const std::string& version, const std::string& url);
    void NotifyUpdateNotAvailable();
    void NotifyUpdateCheckFailed(const std::string& error);
    void NotifyUpdateDownloadProgress(double progress);
    void NotifyUpdateReady(const std::string& newBinaryPath);
    void NotifyUpdateFailed(const std::string& error);

private:
    std::vector<IUpdateObserver*> observers;
    std::mutex mutex;
};
