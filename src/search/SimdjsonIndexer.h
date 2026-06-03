#pragma once

#include <fstream>
#include <atomic>
#include <cstdint>
#include <simdjson.h>
#include "search/SystemIndex.h"

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
    struct ParsedSystem
    {
        SystemIndex::Record rec{};
        std::string_view name;
        bool hasId = false;
        bool hasName = false;
        bool hasCoords = false;
    };

    static bool TrimAndSkip(const char*& data, size_t& length);
    bool ParseDocument(const char* data, size_t length, simdjson::dom::object& obj);

    void HandleField(std::string_view key, simdjson::dom::element value, ParsedSystem& out);
    void HandleId(simdjson::dom::element value, ParsedSystem& out);
    void HandleName(simdjson::dom::element value, ParsedSystem& out);
    void HandleCoords(simdjson::dom::element value, ParsedSystem& out);
    void HandlePopulation(simdjson::dom::element value, ParsedSystem& out);
    void HandleBodyCount(simdjson::dom::element value, ParsedSystem& out);
    void HandleControllingFaction(simdjson::dom::element value, ParsedSystem& out);
    void HandleBodies(simdjson::dom::element value, ParsedSystem& out);
    void HandleBody(simdjson::dom::object body, ParsedSystem& out);

    void WriteRecord(const ParsedSystem& parsed);

    void ParseStarType(std::string_view subType, uint16_t& mask);
    void ParseBodyType(std::string_view subType, uint8_t (&counts)[8]);

    std::ofstream& m_recordsFile;
    std::ofstream& m_stringsFile;
    uint64_t& m_totalSystems;
    uint64_t& m_stringTableOffset;
    std::atomic<bool>& m_cancel;
    simdjson::dom::parser m_parser;
};
