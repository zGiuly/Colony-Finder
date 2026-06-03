#include "logging/LogRecord.h"
#include <ctime>
#include <cstdio>

std::string LogTimestamp(std::chrono::system_clock::time_point tp)
{
    std::time_t raw = std::chrono::system_clock::to_time_t(tp);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &raw);
#else
    localtime_r(&raw, &local);
#endif
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", local.tm_hour, local.tm_min, local.tm_sec);
    return buffer;
}
