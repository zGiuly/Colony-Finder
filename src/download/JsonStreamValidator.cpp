#include "JsonStreamValidator.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <vector>
#include <algorithm>

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

class DynamicSaxValidator : public nlohmann::json_sax<nlohmann::json>
{
public:
    DynamicSaxValidator(const std::vector<std::string>& required, const std::vector<std::string>& requiredCoords, std::atomic<bool>& cancelFlag)
        : requiredFields(required),
          requiredCoordFields(requiredCoords),
          cancel(cancelFlag),
          isValid(true),
          depth(0),
          hasSystems(false),
          inCoords(false)
    {
        foundFields.resize(requiredFields.size(), false);
        foundCoordFields.resize(requiredCoordFields.size(), false);
    }

    bool null() override { return !cancel.load() && isValid; }
    bool boolean(bool) override { return !cancel.load() && isValid; }
    
    bool number_integer(number_integer_t) override 
    {
        if (depth == 2) 
        {
            MarkField(currentKey);
        }
        return !cancel.load() && isValid;
    }

    bool number_unsigned(number_unsigned_t) override 
    {
        if (depth == 2) 
        {
            MarkField(currentKey);
        }
        return !cancel.load() && isValid;
    }

    bool number_float(number_float_t, const string_t&) override 
    {
        if (depth == 2) 
        {
            MarkField(currentKey);
        }
        else if (depth == 3 && inCoords) 
        {
            MarkCoordField(currentKey);
        }
        return !cancel.load() && isValid;
    }

    bool string(string_t&) override 
    {
        if (depth == 2) 
        {
            MarkField(currentKey);
        }
        return !cancel.load() && isValid;
    }

    bool binary(binary_t&) override { return !cancel.load() && isValid; }

    bool start_object(std::size_t) override 
    {
        if (cancel.load()) 
        {
            isValid = false;
            return false;
        }
        depth++;
        if (depth == 2) 
        {
            std::fill(foundFields.begin(), foundFields.end(), false);
        }
        else if (depth == 3 && currentKey == "coords") 
        {
            inCoords = true;
            std::fill(foundCoordFields.begin(), foundCoordFields.end(), false);
        }
        return isValid;
    }

    bool key(string_t& val) override 
    {
        currentKey = val;
        return !cancel.load() && isValid;
    }

    bool end_object() override 
    {
        if (depth == 2) 
        {
            hasSystems = true;
            for (bool found : foundFields) 
            {
                if (!found) 
                {
                    isValid = false;
                    break;
                }
            }
        }
        else if (depth == 3 && inCoords) 
        {
            inCoords = false;
            for (bool found : foundCoordFields) 
            {
                if (!found) 
                {
                    isValid = false;
                    break;
                }
            }
        }
        depth--;
        return !cancel.load() && isValid;
    }

    bool start_array(std::size_t) override 
    {
        depth++;
        return !cancel.load() && isValid;
    }

    bool end_array() override 
    {
        depth--;
        return !cancel.load() && isValid;
    }

    bool parse_error(std::size_t, const std::string&, const nlohmann::detail::exception&) override 
    {
        isValid = false;
        return false;
    }

    bool IsValid() const 
    {
        return isValid && hasSystems && depth == 0;
    }

private:
    void MarkField(const std::string& key)
    {
        for (size_t i = 0; i < requiredFields.size(); ++i)
        {
            if (requiredFields[i] == key)
            {
                foundFields[i] = true;
                break;
            }
        }
    }

    void MarkCoordField(const std::string& key)
    {
        for (size_t i = 0; i < requiredCoordFields.size(); ++i)
        {
            if (requiredCoordFields[i] == key)
            {
                foundCoordFields[i] = true;
                break;
            }
        }
    }

    std::vector<std::string> requiredFields;
    std::vector<std::string> requiredCoordFields;
    std::atomic<bool>& cancel;
    bool isValid;
    int depth;
    bool hasSystems;
    std::string currentKey;
    bool inCoords;
    std::vector<bool> foundFields;
    std::vector<bool> foundCoordFields;
};

bool JsonStreamValidator::Validate(const std::string& filePath, const std::string& schemaPath, std::atomic<float>& progress, std::atomic<bool>& cancelFlag)
{
    std::ifstream schemaFile(schemaPath);
    if (!schemaFile.is_open())
    {
        return false;
    }

    nlohmann::json schema;
    try
    {
        schemaFile >> schema;
    }
    catch (...)
    {
        return false;
    }

    std::vector<std::string> requiredFields;
    if (schema.contains("items") && schema["items"].contains("required"))
    {
        for (const auto& field : schema["items"]["required"])
        {
            requiredFields.push_back(field.get<std::string>());
        }
    }
    else
    {
        requiredFields = { "id64", "name", "coords" };
    }

    std::vector<std::string> requiredCoordFields;
    if (schema.contains("items") && schema["items"].contains("properties") &&
        schema["items"]["properties"].contains("coords") &&
        schema["items"]["properties"]["coords"].contains("required"))
    {
        for (const auto& field : schema["items"]["properties"]["coords"]["required"])
        {
            requiredCoordFields.push_back(field.get<std::string>());
        }
    }
    else
    {
        requiredCoordFields = { "x", "y", "z" };
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
