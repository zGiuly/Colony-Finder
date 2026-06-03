#pragma once

#include <string>

class IUpdateObserver
{
public:
    virtual ~IUpdateObserver() = default;
    virtual void OnUpdateAvailable(const std::string& latestVersion, const std::string& downloadUrl) = 0;
    virtual void OnUpdateNotAvailable() = 0;
    virtual void OnUpdateCheckFailed(const std::string& error) = 0;
    virtual void OnUpdateDownloadProgress(double progress) = 0;
    virtual void OnUpdateReady(const std::string& newBinaryPath) = 0;
    virtual void OnUpdateFailed(const std::string& error) = 0;
};
