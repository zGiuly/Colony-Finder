#include "JsonStreamValidator.h"
#include "download/DynamicSaxValidator.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <vector>

class ProgressStreamBuf : public std::streambuf
{
public:
    ProgressStreamBuf(std::streambuf* buf, uint64_t total, std::atomic<float>& progress)
        : sourceBuf(buf), totalBytes(total), progressRef(progress), processedBytes(0)
    {
        buffer.resize(256 * 1024);
        setg(nullptr, nullptr, nullptr);
    }

protected:
    int_type underflow() override
    {
        if (gptr() < egptr())
        {
            return traits_type::to_int_type(*gptr());
        }

        std::streamsize bytesRead = sourceBuf->sgetn(reinterpret_cast<char*>(buffer.data()), buffer.size());
        if (bytesRead <= 0)
        {
            return traits_type::eof();
        }

        processedBytes += bytesRead;
        progressRef.store(static_cast<float>(processedBytes) / totalBytes);

        setg(reinterpret_cast<char*>(buffer.data()), reinterpret_cast<char*>(buffer.data()), reinterpret_cast<char*>(buffer.data()) + bytesRead);
        return traits_type::to_int_type(*gptr());
    }

private:
    std::streambuf* sourceBuf;
    uint64_t totalBytes;
    std::atomic<float>& progressRef;
    uint64_t processedBytes;
    std::vector<unsigned char> buffer;
};

bool JsonStreamValidator::Validate(const std::string& filePath, const std::string& schemaPath, std::atomic<float>& progress, std::atomic<bool>& cancelFlag)
{
    std::vector<std::string> requiredFields;
    std::vector<std::string> requiredCoordFields;
    if (!DynamicSaxValidator::LoadSchema(schemaPath, requiredFields, requiredCoordFields))
    {
        return false;
    }

    std::ifstream jsonFile(filePath, std::ios::binary);
    if (!jsonFile.is_open())
    {
        return false;
    }

    uint64_t totalBytes = std::filesystem::file_size(filePath);
    if (totalBytes == 0)
    {
        return false;
    }

    ProgressStreamBuf customBuf(jsonFile.rdbuf(), totalBytes, progress);
    std::istream progressStream(&customBuf);

    DynamicSaxValidator validator(requiredFields, requiredCoordFields, cancelFlag);
    bool parsed = nlohmann::json::sax_parse(progressStream, &validator);

    return parsed && validator.IsValid() && !cancelFlag.load();
}
