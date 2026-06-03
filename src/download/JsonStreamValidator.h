#pragma once

#include <string>
#include <atomic>

class JsonStreamValidator
{
public:
    static bool Validate(const std::string& filePath, const std::string& schemaPath, std::atomic<float>& progress, std::atomic<bool>& cancelFlag);
};
