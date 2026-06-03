#pragma once
#include <streambuf>
#include <vector>
#include <atomic>
#include <stdint.h>

class IndexProgressStreamBuf : public std::streambuf
{
public:
    static constexpr size_t BufferSize = 262144;

    IndexProgressStreamBuf(std::streambuf* buf, uint64_t total, std::atomic<float>& progress)
        : sourceBuf(buf), totalBytes(total), progressRef(progress), processedBytes(0)
    {
        buffer.resize(BufferSize);
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
