#pragma once

#include <vector>
#include <functional>
#include <cstddef>

class LineSink
{
public:
    using LineCallback = std::function<bool(const char*, size_t)>;

    explicit LineSink(LineCallback cb);

    bool Consume(const unsigned char* data, size_t length);
    bool Flush();

private:
    std::vector<char> m_buffer;
    LineCallback m_callback;
};
