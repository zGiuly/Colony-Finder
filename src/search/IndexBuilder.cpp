#include "search/IndexBuilder.h"
#include "search/IndexProgressStreamBuf.h"
#include "search/IndexBuilderSax.h"
#include "search/SystemIndex.h"
#include <fstream>
#include <filesystem>
#include <vector>

constexpr size_t MergeBufferSize = 1048576;

bool IndexBuilder::Build(const std::string& jsonPath, const std::string& indexPath, std::atomic<float>& progress, std::atomic<bool>& cancelFlag)
{
    std::ifstream jsonFile(jsonPath, std::ios::binary);
    if (!jsonFile.is_open())
    {
        return false;
    }

    uint64_t totalBytes = std::filesystem::file_size(jsonPath);
    if (totalBytes == 0)
    {
        return false;
    }

    std::filesystem::path recTmp = std::filesystem::path(indexPath).parent_path() / "records.tmp";
    std::filesystem::path strTmp = std::filesystem::path(indexPath).parent_path() / "strings.tmp";

    std::ofstream recFile(recTmp, std::ios::binary);
    if (!recFile.is_open())
    {
        return false;
    }

    std::ofstream strFile(strTmp, std::ios::binary);
    if (!strFile.is_open())
    {
        recFile.close();
        std::filesystem::remove(recTmp);
        return false;
    }

    uint64_t totalSystems = 0;
    uint64_t stringTableOffset = 0;

    IndexProgressStreamBuf customBuf(jsonFile.rdbuf(), totalBytes, progress);
    std::istream progressStream(&customBuf);

    IndexBuilderSax saxHandler(recFile, strFile, totalSystems, stringTableOffset, cancelFlag);
    bool parsed = nlohmann::json::sax_parse(progressStream, &saxHandler);

    recFile.close();
    strFile.close();

    if (!parsed || cancelFlag.load() || totalSystems == 0)
    {
        std::filesystem::remove(recTmp);
        std::filesystem::remove(strTmp);
        return false;
    }

    std::ofstream outFile(indexPath, std::ios::binary);
    if (!outFile.is_open())
    {
        std::filesystem::remove(recTmp);
        std::filesystem::remove(strTmp);
        return false;
    }

    SystemIndex::Header header;
    header.magic[0] = 'C';
    header.magic[1] = 'F';
    header.magic[2] = 'I';
    header.magic[3] = 'D';
    header.magic[4] = 'X';
    header.magic[5] = '\0';
    header.version = SystemIndex::Version;
    header.systemCount = totalSystems;
    header.stringTableOffset = sizeof(SystemIndex::Header) + totalSystems * sizeof(SystemIndex::Record);
    header.reserved = 0;

    outFile.write(reinterpret_cast<const char*>(&header), sizeof(header));

    std::ifstream recIn(recTmp, std::ios::binary);
    std::ifstream strIn(strTmp, std::ios::binary);

    std::vector<char> buffer(MergeBufferSize);
    while (recIn)
    {
        recIn.read(buffer.data(), buffer.size());
        std::streamsize read = recIn.gcount();
        if (read > 0)
        {
            outFile.write(buffer.data(), read);
        }
    }
    recIn.close();

    while (strIn)
    {
        strIn.read(buffer.data(), buffer.size());
        std::streamsize read = strIn.gcount();
        if (read > 0)
        {
            outFile.write(buffer.data(), read);
        }
    }
    strIn.close();

    outFile.close();
    std::filesystem::remove(recTmp);
    std::filesystem::remove(strTmp);

    return true;
}
