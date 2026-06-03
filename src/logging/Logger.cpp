#include "logging/Logger.h"
#include <algorithm>
#include <cstdarg>
#include <cstdio>

Logger& Logger::Instance()
{
    static Logger instance;
    return instance;
}

Logger::Logger() : minLevel(LogLevel::Info)
{
}

void Logger::AddSink(std::shared_ptr<ILogSink> sink)
{
    if (!sink)
    {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex);
    sinks.push_back(std::move(sink));
}

void Logger::RemoveSink(const std::shared_ptr<ILogSink>& sink)
{
    std::lock_guard<std::mutex> lock(mutex);
    sinks.erase(std::remove(sinks.begin(), sinks.end(), sink), sinks.end());
}

void Logger::ClearSinks()
{
    std::lock_guard<std::mutex> lock(mutex);
    sinks.clear();
}

void Logger::SetLevel(LogLevel level)
{
    minLevel.store(level);
}

LogLevel Logger::GetLevel() const
{
    return minLevel.load();
}

void Logger::Log(LogLevel level, const char* fmt, ...)
{
    if (static_cast<int>(level) < static_cast<int>(minLevel.load()))
    {
        return;
    }
    if (!fmt)
    {
        return;
    }

    char buffer[2048];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    Dispatch(LogRecord{level, std::chrono::system_clock::now(), buffer});
}

void Logger::Dispatch(const LogRecord& record)
{
    std::lock_guard<std::mutex> lock(mutex);
    for (auto& sink : sinks)
    {
        sink->Write(record);
    }
}
