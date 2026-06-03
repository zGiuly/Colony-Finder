#include "logging/ConsoleLogSink.h"
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#endif

ConsoleLogSink::ConsoleLogSink()
{
#ifdef _WIN32
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (handle == INVALID_HANDLE_VALUE || !GetConsoleMode(handle, &mode))
    {
        return;
    }
    SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
}

void ConsoleLogSink::Write(const LogRecord& record)
{
    std::printf("%s (%s%s%s): %s\n",
        LogTimestamp(record.time).c_str(),
        LogLevelToColor(record.level),
        LogLevelToString(record.level),
        AnsiReset,
        record.message.c_str());
    std::fflush(stdout);
}
