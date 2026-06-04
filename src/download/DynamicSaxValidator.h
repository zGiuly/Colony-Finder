#pragma once

#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <atomic>
#include <algorithm>
#include <fstream>

/**
 * \todo PENDING IMPLEMENTATION: SAX validator built against the Spansh
 *       galaxy schema. Functional but not yet invoked — see
 *       JsonStreamValidator and StreamingProcessor::Process for the
 *       hook-up point.
 */
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
        if (depth == 2) MarkField(currentKey);
        return !cancel.load() && isValid;
    }

    bool number_unsigned(number_unsigned_t) override
    {
        if (depth == 2) MarkField(currentKey);
        return !cancel.load() && isValid;
    }

    bool number_float(number_float_t, const string_t&) override
    {
        if (depth == 2) MarkField(currentKey);
        else if (depth == 3 && inCoords) MarkCoordField(currentKey);
        return !cancel.load() && isValid;
    }

    bool string(string_t&) override
    {
        if (depth == 2) MarkField(currentKey);
        return !cancel.load() && isValid;
    }

    bool binary(binary_t&) override { return !cancel.load() && isValid; }

    bool start_object(std::size_t) override
    {
        if (cancel.load()) { isValid = false; return false; }
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
                if (!found) { isValid = false; break; }
            }
        }
        else if (depth == 3 && inCoords)
        {
            inCoords = false;
            for (bool found : foundCoordFields)
            {
                if (!found) { isValid = false; break; }
            }
        }
        depth--;
        return !cancel.load() && isValid;
    }

    bool start_array(std::size_t) override { depth++; return !cancel.load() && isValid; }
    bool end_array() override { depth--; return !cancel.load() && isValid; }

    bool parse_error(std::size_t, const std::string&, const nlohmann::detail::exception&) override
    {
        isValid = false;
        return false;
    }

    bool IsValid() const { return isValid && hasSystems && depth == 0; }

    static bool LoadSchema(const std::string& schemaPath, std::vector<std::string>& outRequired, std::vector<std::string>& outCoordRequired)
    {
        std::ifstream schemaFile(schemaPath);
        if (!schemaFile.is_open()) return false;
        nlohmann::json schema;
        try { schemaFile >> schema; } catch (...) { return false; }

        outRequired.clear();
        if (schema.contains("items") && schema["items"].contains("required"))
        {
            for (const auto& field : schema["items"]["required"])
            {
                outRequired.push_back(field.get<std::string>());
            }
        }
        else
        {
            outRequired = { "id64", "name", "coords" };
        }

        outCoordRequired.clear();
        if (schema.contains("items") && schema["items"].contains("properties") &&
            schema["items"]["properties"].contains("coords") &&
            schema["items"]["properties"]["coords"].contains("required"))
        {
            for (const auto& field : schema["items"]["properties"]["coords"]["required"])
            {
                outCoordRequired.push_back(field.get<std::string>());
            }
        }
        else
        {
            outCoordRequired = { "x", "y", "z" };
        }
        return true;
    }

private:
    void MarkField(const std::string& key)
    {
        for (size_t i = 0; i < requiredFields.size(); ++i)
        {
            if (requiredFields[i] == key) { foundFields[i] = true; break; }
        }
    }

    void MarkCoordField(const std::string& key)
    {
        for (size_t i = 0; i < requiredCoordFields.size(); ++i)
        {
            if (requiredCoordFields[i] == key) { foundCoordFields[i] = true; break; }
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
