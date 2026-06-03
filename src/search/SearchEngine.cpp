#include "search/SearchEngine.h"
#include <cmath>
#include <queue>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

struct HeapItem
{
    SearchResult result;
    bool operator<(const HeapItem& other) const
    {
        return result.distanceToSource < other.result.distanceToSource;
    }
};

static bool MatchSystemName(const char* name, const std::string& query)
{
    if (query.empty())
    {
        return true;
    }
    size_t qLen = query.size();
    for (const char* p = name; *p != '\0'; ++p)
    {
        bool match = true;
        for (size_t i = 0; i < qLen; ++i)
        {
            if (p[i] == '\0' || std::tolower(static_cast<unsigned char>(p[i])) != std::tolower(static_cast<unsigned char>(query[i])))
            {
                match = false;
                break;
            }
        }
        if (match)
        {
            return true;
        }
    }
    return false;
}

SearchEngine::SearchEngine()
    : fileHandle(nullptr),
      mappingHandle(nullptr),
      mappedData(nullptr),
      fileSize(0),
      header(nullptr),
      records(nullptr),
      stringTable(nullptr)
{
}

SearchEngine::~SearchEngine()
{
    Shutdown();
}

bool SearchEngine::Initialize(const std::string& indexPath)
{
    Shutdown();

#ifdef _WIN32
    HANDLE hFile = CreateFileA(indexPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size))
    {
        CloseHandle(hFile);
        return false;
    }
    fileSize = size.QuadPart;

    if (fileSize < sizeof(SystemIndex::Header))
    {
        CloseHandle(hFile);
        return false;
    }

    HANDLE hMap = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (!hMap)
    {
        CloseHandle(hFile);
        return false;
    }

    void* data = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
    if (!data)
    {
        CloseHandle(hMap);
        CloseHandle(hFile);
        return false;
    }

    fileHandle = hFile;
    mappingHandle = hMap;
    mappedData = data;
#else
    int fd = open(indexPath.c_str(), O_RDONLY);
    if (fd == -1)
    {
        return false;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1)
    {
        close(fd);
        return false;
    }
    fileSize = sb.st_size;

    if (fileSize < sizeof(SystemIndex::Header))
    {
        close(fd);
        return false;
    }

    void* data = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED)
    {
        close(fd);
        return false;
    }

    fileHandle = reinterpret_cast<void*>(static_cast<uintptr_t>(fd));
    mappingHandle = nullptr;
    mappedData = data;
#endif

    header = reinterpret_cast<const SystemIndex::Header*>(mappedData);
    if (std::string(header->magic) != "CFIDX" || header->version != SystemIndex::Version)
    {
        Shutdown();
        return false;
    }

    records = reinterpret_cast<const SystemIndex::Record*>(static_cast<const char*>(mappedData) + sizeof(SystemIndex::Header));
    stringTable = static_cast<const char*>(mappedData) + header->stringTableOffset;

    return true;
}

void SearchEngine::Shutdown()
{
    if (!mappedData)
    {
        return;
    }

#ifdef _WIN32
    UnmapViewOfFile(mappedData);
    CloseHandle(reinterpret_cast<HANDLE>(mappingHandle));
    CloseHandle(reinterpret_cast<HANDLE>(fileHandle));
#else
    munmap(mappedData, fileSize);
    close(static_cast<int>(reinterpret_cast<uintptr_t>(fileHandle)));
#endif

    fileHandle = nullptr;
    mappingHandle = nullptr;
    mappedData = nullptr;
    fileSize = 0;
    header = nullptr;
    records = nullptr;
    stringTable = nullptr;
}

bool SearchEngine::FindSystemCoords(const std::string& name, float& x, float& y, float& z) const
{
    if (!mappedData || name.empty())
    {
        return false;
    }

    for (uint64_t i = 0; i < header->systemCount; ++i)
    {
        const auto& rec = records[i];
        const char* namePtr = stringTable + rec.nameOffset;
        bool match = true;
        for (size_t j = 0; j < name.size(); ++j)
        {
            if (namePtr[j] == '\0' || std::tolower(static_cast<unsigned char>(namePtr[j])) != std::tolower(static_cast<unsigned char>(name[j])))
            {
                match = false;
                break;
            }
        }
        if (match && namePtr[name.size()] == '\0')
        {
            x = rec.x;
            y = rec.y;
            z = rec.z;
            return true;
        }
    }
    return false;
}

bool SearchEngine::Search(const SearchFilters& filters, std::vector<SearchResult>& results, size_t maxResults)
{
    if (!mappedData)
    {
        return false;
    }

    float sourceX = 0.0f;
    float sourceY = 0.0f;
    float sourceZ = 0.0f;

    if (!filters.sourceSystemName.empty())
    {
        FindSystemCoords(filters.sourceSystemName, sourceX, sourceY, sourceZ);
    }

    std::priority_queue<HeapItem> minDistanceHeap;

    for (uint64_t i = 0; i < header->systemCount; ++i)
    {
        const auto& rec = records[i];

        if (filters.colonizedOnly && rec.population == 0)
        {
            continue;
        }

        if (filters.filterPopulation && (rec.population < filters.minPopulation || rec.population > filters.maxPopulation))
        {
            continue;
        }

        if (filters.filterBodyCount && (rec.bodyCount < filters.minBodies || rec.bodyCount > filters.maxBodies))
        {
            continue;
        }

        if (filters.filterStarType && (rec.starTypesMask & filters.starTypesMask) == 0)
        {
            continue;
        }

        bool countsOk = true;
        for (int t = 0; t < SystemIndex::BTI_Count; ++t)
        {
            if (!filters.bodyCountEnabled[t]) continue;
            uint8_t c = rec.bodyTypeCounts[t];
            if (c < filters.minBodyTypeCount[t] || c > filters.maxBodyTypeCount[t])
            {
                countsOk = false;
                break;
            }
        }
        if (!countsOk) continue;

        if (filters.filterLandable && (rec.flags & SystemIndex::System_HasLandable) == 0)
        {
            continue;
        }

        float dx = rec.x - sourceX;
        float dy = rec.y - sourceY;
        float dz = rec.z - sourceZ;
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

        if (filters.filterDistance && dist > filters.maxDistanceLy)
        {
            continue;
        }

        const char* namePtr = stringTable + rec.nameOffset;
        if (!filters.systemNameQuery.empty() && !MatchSystemName(namePtr, filters.systemNameQuery))
        {
            continue;
        }

        HeapItem item;
        item.result.name = namePtr;
        item.result.id64 = rec.id64;
        item.result.x = rec.x;
        item.result.y = rec.y;
        item.result.z = rec.z;
        item.result.distanceToSource = dist;
        item.result.population = rec.population;
        item.result.bodyCount = rec.bodyCount;

        if (minDistanceHeap.size() < maxResults)
        {
            minDistanceHeap.push(item);
        }
        else if (dist < minDistanceHeap.top().result.distanceToSource)
        {
            minDistanceHeap.pop();
            minDistanceHeap.push(item);
        }
    }

    results.clear();
    results.resize(minDistanceHeap.size());
    for (int i = static_cast<int>(minDistanceHeap.size()) - 1; i >= 0; --i)
    {
        results[i] = minDistanceHeap.top().result;
        minDistanceHeap.pop();
    }

    return true;
}
