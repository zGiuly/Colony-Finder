#pragma once

#include <cstdint>

class SystemMemory
{
public:
    static uint64_t GetTotalRAM();
    static uint64_t GetAvailableRAM();
};
