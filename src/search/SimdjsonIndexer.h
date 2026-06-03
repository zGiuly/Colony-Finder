#pragma once

#include <fstream>
#include <atomic>
#include <cstdint>
#include <simdjson.h>

class SimdjsonIndexer
{
public:
    SimdjsonIndexer(std::ofstream& recFile,
                    std::ofstream& strFile,
                    uint64_t& totalSystems,
                    uint64_t& stringTableOffset,
                    std::atomic<bool>& cancelFlag);

    bool ProcessLine(const char* data, size_t length);

private:
    void ParseStarType(std::string_view subType, uint16_t& mask);
    void ParseBodyType(std::string_view subType, uint32_t& mask);

    std::ofstream& m_recordsFile;
    std::ofstream& m_stringsFile;
    uint64_t& m_totalSystems;
    uint64_t& m_stringTableOffset;
    std::atomic<bool>& m_cancel;
    simdjson::dom::parser m_parser;
};
