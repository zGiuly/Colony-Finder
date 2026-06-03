#include "logging/FileLogSink.h"

FileLogSink::FileLogSink(const std::string& path) : fp(nullptr)
{
#ifdef _WIN32
    fopen_s(&fp, path.c_str(), "a");
#else
    fp = std::fopen(path.c_str(), "a");
#endif
}

FileLogSink::~FileLogSink()
{
    if (fp)
    {
        std::fclose(fp);
    }
}

bool FileLogSink::IsOpen() const
{
    return fp != nullptr;
}

void FileLogSink::Write(const LogRecord& record)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (!fp)
    {
        return;
    }
    std::fprintf(fp, "%s (%s): %s\n",
        LogTimestamp(record.time).c_str(),
        LogLevelToString(record.level),
        record.message.c_str());
    std::fflush(fp);
}
