#pragma once

#include <streambuf>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <cstddef>

class PipeStreamBuf : public std::streambuf
{
public:
    explicit PipeStreamBuf(size_t maxQueuedChunks = 8);

    bool Write(std::vector<unsigned char>&& chunk);
    void Close();

protected:
    int_type underflow() override;

private:
    std::queue<std::vector<unsigned char>> m_queue;
    std::vector<unsigned char> m_current;
    size_t m_capacity;
    std::mutex m_mutex;
    std::condition_variable m_cvNotEmpty;
    std::condition_variable m_cvNotFull;
    bool m_closed;
};
