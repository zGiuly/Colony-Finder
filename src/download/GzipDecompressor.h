#pragma once

#include <string>
#include <atomic>
#include <functional>
#include <vector>

class GzipDecompressor
{
public:
    using Sink = std::function<bool(std::vector<unsigned char>&&)>;

    static bool Decompress(const std::string& sourcePath, const std::string& destPath, std::function<void(float)> onProgress, std::atomic<bool>& cancelFlag, int bufferSizeMb = 4);
    static bool DecompressToSink(const std::string& sourcePath, Sink sink, std::function<void(float)> onProgress, std::atomic<bool>& cancelFlag, int bufferSizeMb = 4);
};
