#include "ui/SystemMemory.h"
#include <fstream>
#include <string>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/sysinfo.h>
#endif

uint64_t SystemMemory::GetTotalRAM()
{
#ifdef _WIN32
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (!GlobalMemoryStatusEx(&statex))
    {
        return 0;
    }
    return statex.ullTotalPhys;
#else
    struct sysinfo info;
    if (sysinfo(&info) != 0)
    {
        return 0;
    }
    return static_cast<uint64_t>(info.totalram) * info.mem_unit;
#endif
}

uint64_t SystemMemory::GetAvailableRAM()
{
#ifdef _WIN32
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (!GlobalMemoryStatusEx(&statex))
    {
        return 0;
    }
    return statex.ullAvailPhys;
#else
    std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open())
    {
        std::string line;
        while (std::getline(meminfo, line))
        {
            if (line.rfind("MemAvailable:", 0) == 0)
            {
                uint64_t val = 0;
                for (char c : line)
                {
                    if (std::isdigit(c))
                    {
                        val = val * 10 + (c - '0');
                    }
                }
                return val * 1024;
            }
        }
    }
    struct sysinfo info;
    if (sysinfo(&info) != 0)
    {
        return 0;
    }
    return static_cast<uint64_t>(info.freeram) * info.mem_unit;
#endif
}
