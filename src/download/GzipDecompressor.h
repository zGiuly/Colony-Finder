#pragma once

#include <string>
#include <atomic>

class GzipDecompressor
{
public:
    static bool Decompress(const std::string& sourcePath, const std::string& destPath, std::atomic<float>& progress, std::atomic<bool>& cancelFlag);
};
