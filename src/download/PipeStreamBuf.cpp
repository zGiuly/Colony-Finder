#include "download/PipeStreamBuf.h"

PipeStreamBuf::PipeStreamBuf(size_t maxQueuedChunks)
    : m_capacity(maxQueuedChunks), m_closed(false)
{
    setg(nullptr, nullptr, nullptr);
}

bool PipeStreamBuf::Write(std::vector<unsigned char>&& chunk)
{
    if (chunk.empty())
    {
        return true;
    }
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cvNotFull.wait(lock, [&] { return m_queue.size() < m_capacity || m_closed; });
    if (m_closed)
    {
        return false;
    }
    m_queue.push(std::move(chunk));
    m_cvNotEmpty.notify_one();
    return true;
}

void PipeStreamBuf::Close()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_closed = true;
    m_cvNotEmpty.notify_all();
    m_cvNotFull.notify_all();
}

PipeStreamBuf::int_type PipeStreamBuf::underflow()
{
    if (gptr() && gptr() < egptr())
    {
        return traits_type::to_int_type(*gptr());
    }

    std::vector<unsigned char> next;
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cvNotEmpty.wait(lock, [&] { return !m_queue.empty() || m_closed; });
        if (m_queue.empty())
        {
            return traits_type::eof();
        }
        next = std::move(m_queue.front());
        m_queue.pop();
        m_cvNotFull.notify_one();
    }

    m_current = std::move(next);
    char* base = reinterpret_cast<char*>(m_current.data());
    setg(base, base, base + m_current.size());
    return traits_type::to_int_type(*gptr());
}
