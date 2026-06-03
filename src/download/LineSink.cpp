#include "download/LineSink.h"

LineSink::LineSink(LineCallback cb) : m_callback(std::move(cb)) {}

bool LineSink::Consume(const unsigned char* data, size_t length)
{
    const char* p = reinterpret_cast<const char*>(data);
    size_t start = 0;
    for (size_t i = 0; i < length; ++i)
    {
        if (p[i] == '\n')
        {
            if (m_buffer.empty())
            {
                if (!m_callback(p + start, i - start)) return false;
            }
            else
            {
                m_buffer.insert(m_buffer.end(), p + start, p + i);
                if (!m_callback(m_buffer.data(), m_buffer.size())) return false;
                m_buffer.clear();
            }
            start = i + 1;
        }
    }
    if (start < length)
    {
        m_buffer.insert(m_buffer.end(), p + start, p + length);
    }
    return true;
}

bool LineSink::Flush()
{
    if (!m_buffer.empty())
    {
        bool ok = m_callback(m_buffer.data(), m_buffer.size());
        m_buffer.clear();
        return ok;
    }
    return true;
}
