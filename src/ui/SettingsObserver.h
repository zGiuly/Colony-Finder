#pragma once
#include <string>

class ISettingsObserver
{
public:
    virtual ~ISettingsObserver() = default;
    virtual void OnSettingsChanged(const std::string& downloadDir, const std::string& searchDir) = 0;
};
