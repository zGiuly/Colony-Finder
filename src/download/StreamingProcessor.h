#pragma once

#include <string>
#include <atomic>

class StreamingProcessor
{
public:
    struct Result
    {
        bool success = false;
        bool cancelled = false;
        bool validationFailed = false;
        bool decompressionFailed = false;
        bool indexingFailed = false;
        std::string errorMessage;
    };

    static Result Process(const std::string& gzPath,
                          const std::string& schemaPath,
                          const std::string& indexPath,
                          std::atomic<float>& decompressProgress,
                          std::atomic<float>& indexProgress,
                          std::atomic<bool>& cancelFlag,
                          int bufferSizeMb);
};
