#pragma once

#include <string>

class UpdaterLauncher
{
public:
    static std::string GetExecutablePath();
    static std::string GetUpdaterPath();
    static bool LaunchUpdater(const std::string& newBinaryPath, const std::string& targetBinaryPath);
};
