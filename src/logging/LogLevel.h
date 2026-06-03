#pragma once

enum class LogLevel
{
    Debug = 0,
    Info = 1,
    Warn = 2,
    Error = 3,
    None = 4
};

const char* LogLevelToString(LogLevel level);
LogLevel LogLevelFromString(const char* text);
const char* LogLevelToColor(LogLevel level);

inline constexpr const char* AnsiReset = "\x1b[0m";
