#include "logging/LogLevel.h"
#include <cstring>

const char* LogLevelToString(LogLevel level)
{
    switch (level)
    {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warn: return "WARN";
        case LogLevel::Error: return "ERROR";
        default: return "NONE";
    }
}

LogLevel LogLevelFromString(const char* text)
{
    if (!text)
    {
        return LogLevel::None;
    }
    if (std::strcmp(text, "DEBUG") == 0) return LogLevel::Debug;
    if (std::strcmp(text, "INFO") == 0) return LogLevel::Info;
    if (std::strcmp(text, "WARN") == 0) return LogLevel::Warn;
    if (std::strcmp(text, "ERROR") == 0) return LogLevel::Error;
    return LogLevel::None;
}

const char* LogLevelToColor(LogLevel level)
{
    switch (level)
    {
        case LogLevel::Debug: return "\x1b[35m";
        case LogLevel::Info: return "\x1b[32m";
        case LogLevel::Warn: return "\x1b[33m";
        case LogLevel::Error: return "\x1b[1;31m";
        default: return "\x1b[0m";
    }
}
