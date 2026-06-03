#pragma once
#include <string>
#include <atomic>

class IndexBuilder
{
public:
    static bool Build(const std::string& jsonPath, const std::string& indexPath, std::atomic<float>& progress, std::atomic<bool>& cancelFlag);
};
