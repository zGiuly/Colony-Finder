#pragma once
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>
#include <atomic>
#include <stdint.h>
#include "search/SystemIndex.h"

struct CurrentSystem
{
    uint64_t id64 = 0;
    std::string name;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    uint64_t population = 0;
    uint16_t bodyCount = 0;
    uint16_t starTypesMask = 0;
    uint32_t bodyTypesMask = 0;
};

class IndexBuilderSax : public nlohmann::json_sax<nlohmann::json>
{
public:
    IndexBuilderSax(std::ofstream& recFile, std::ofstream& strFile, uint64_t& systemCount, uint64_t& strOffset, std::atomic<bool>& cancelFlag);

    bool null() override;
    bool boolean(bool val) override;
    bool number_integer(number_integer_t val) override;
    bool number_unsigned(number_unsigned_t val) override;
    bool number_float(number_float_t val, const string_t& s) override;
    bool string(string_t& val) override;
    bool binary(binary_t& val) override;
    bool start_object(std::size_t elements) override;
    bool key(string_t& val) override;
    bool end_object() override;
    bool start_array(std::size_t elements) override;
    bool end_array() override;
    bool parse_error(std::size_t position, const std::string& last_token, const nlohmann::detail::exception& ex) override;

private:
    void ParseStarType(const std::string& subType, uint16_t& mask);
    void ParseBodyType(const std::string& subType, uint32_t& mask);

    std::ofstream& recordsFile;
    std::ofstream& stringsFile;
    uint64_t& totalSystems;
    uint64_t& stringTableOffset;
    std::atomic<bool>& cancel;

    int depth;
    bool inCoords;
    bool inBodies;
    bool inBody;
    std::string currentKey;
    CurrentSystem currentSystem;
    std::string currentBodyType;
    std::string currentBodySubType;
    bool currentBodyIsLandable;
};
