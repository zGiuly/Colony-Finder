#pragma once

#include "logging/LogLevel.h"
#include <string>
#include <chrono>

struct LogRecord
{
    LogLevel level;
    std::chrono::system_clock::time_point time;
    std::string message;
};

std::string LogTimestamp(std::chrono::system_clock::time_point tp);
