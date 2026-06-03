#pragma once

#include <string>

struct UpdateInfo
{
    bool available = false;
    std::string latestVersion;
    std::string downloadUrl;
    std::string error;
};

class IUpdateChecker
{
public:
    virtual ~IUpdateChecker() = default;
    virtual UpdateInfo Check(const std::string& currentVersion) = 0;
};
