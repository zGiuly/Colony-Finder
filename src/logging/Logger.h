#pragma once

#include "logging/ILogSink.h"
#include "logging/LogLevel.h"
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

class Logger
{
public:
    static Logger& Instance();

    void AddSink(std::shared_ptr<ILogSink> sink);
    void RemoveSink(const std::shared_ptr<ILogSink>& sink);
    void ClearSinks();

    void SetLevel(LogLevel level);
    LogLevel GetLevel() const;

    void Log(LogLevel level, const char* fmt, ...);

private:
    Logger();
    ~Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void Dispatch(const LogRecord& record);

    std::vector<std::shared_ptr<ILogSink>> sinks;
    std::atomic<LogLevel> minLevel;
    std::mutex mutex;
};

#define LOG_DEBUG(...) Logger::Instance().Log(LogLevel::Debug, __VA_ARGS__)
#define LOG_INFO(...) Logger::Instance().Log(LogLevel::Info, __VA_ARGS__)
#define LOG_WARN(...) Logger::Instance().Log(LogLevel::Warn, __VA_ARGS__)
#define LOG_ERROR(...) Logger::Instance().Log(LogLevel::Error, __VA_ARGS__)
