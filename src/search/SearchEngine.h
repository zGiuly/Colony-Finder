#pragma once
#include <string>
#include <vector>
#include "search/SystemIndex.h"

struct SearchFilters
{
    bool filterColonized = false;
    bool colonizedOnly = false;

    bool filterBodyCount = false;
    uint16_t minBodies = 0;
    uint16_t maxBodies = 0;

    bool filterStarType = false;
    uint16_t starTypesMask = 0;

    bool filterBodyType = false;
    uint32_t bodyTypesMask = 0;

    bool filterDistance = false;
    float maxDistanceLy = 0.0f;

    bool filterPopulation = false;
    uint64_t minPopulation = 0;
    uint64_t maxPopulation = 0;

    std::string systemNameQuery;
    std::string sourceSystemName;
};

struct SearchResult
{
    std::string name;
    uint64_t id64 = 0;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float distanceToSource = 0.0f;
    uint64_t population = 0;
    uint16_t bodyCount = 0;
};

class SearchEngine
{
public:
    SearchEngine();
    ~SearchEngine();

    bool Initialize(const std::string& indexPath);
    void Shutdown();

    bool Search(const SearchFilters& filters, std::vector<SearchResult>& results, size_t maxResults = 100);

    bool IsLoaded() const { return mappedData != nullptr; }

private:
    bool FindSystemCoords(const std::string& name, float& x, float& y, float& z) const;

    void* fileHandle;
    void* mappingHandle;
    void* mappedData;
    uint64_t fileSize;

    const SystemIndex::Header* header;
    const SystemIndex::Record* records;
    const char* stringTable;
};
