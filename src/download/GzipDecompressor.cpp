#include "GzipDecompressor.h"
#include <zlib.h>
#include <fstream>
#include <filesystem>
#include <vector>

bool GzipDecompressor::Decompress(const std::string& sourcePath, const std::string& destPath, std::atomic<float>& progress, std::atomic<bool>& cancelFlag)
{
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    int ret = inflateInit2(&strm, 16 + MAX_WBITS);
    if (ret != Z_OK)
    {
        return false;
    }

    std::ifstream in(sourcePath, std::ios::binary);
    if (!in.is_open())
    {
        inflateEnd(&strm);
        return false;
    }

    std::ofstream out(destPath, std::ios::binary);
    if (!out.is_open())
    {
        inflateEnd(&strm);
        return false;
    }

    uint64_t totalBytes = std::filesystem::file_size(sourcePath);
    if (totalBytes == 0)
    {
        inflateEnd(&strm);
        return false;
    }

    uint64_t processedBytes = 0;
    constexpr size_t ChunkSize = 256 * 1024;
    std::vector<unsigned char> inBuffer(ChunkSize);
    std::vector<unsigned char> outBuffer(ChunkSize);

    bool success = true;

    while (ret != Z_STREAM_END)
    {
        if (cancelFlag.load())
        {
            success = false;
            break;
        }

        in.read(reinterpret_cast<char*>(inBuffer.data()), ChunkSize);
        std::streamsize bytesRead = in.gcount();
        if (bytesRead <= 0)
        {
            break;
        }

        processedBytes += bytesRead;
        progress.store(static_cast<float>(processedBytes) / totalBytes);

        strm.next_in = inBuffer.data();
        strm.avail_in = static_cast<uInt>(bytesRead);

        while (strm.avail_in > 0)
        {
            strm.next_out = outBuffer.data();
            strm.avail_out = ChunkSize;

            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_NEED_DICT || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR)
            {
                success = false;
                break;
            }

            std::streamsize bytesToWrite = ChunkSize - strm.avail_out;
            if (bytesToWrite > 0)
            {
                out.write(reinterpret_cast<char*>(outBuffer.data()), bytesToWrite);
            }

            if (ret == Z_STREAM_END)
            {
                break;
            }
        }

        if (!success)
        {
            break;
        }
    }

    inflateEnd(&strm);
    return success && (ret == Z_STREAM_END);
}
