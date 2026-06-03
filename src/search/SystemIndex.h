#pragma once
#include <stdint.h>

namespace SystemIndex
{
    constexpr uint32_t Version = 3;

    enum BodyTypeIndex : uint8_t
    {
        BTI_ELW = 0,
        BTI_WW,
        BTI_AMW,
        BTI_HMC,
        BTI_MetalRich,
        BTI_Rocky,
        BTI_Icy,
        BTI_GasGiant,
        BTI_Count
    };

    struct Header
    {
        char magic[6];
        uint32_t version;
        uint64_t systemCount;
        uint64_t stringTableOffset;
        uint32_t reserved;
    };

    struct Record
    {
        uint64_t id64;
        float x, y, z;
        uint64_t population;
        uint16_t bodyCount;
        uint16_t starTypesMask;
        uint8_t bodyTypeCounts[BTI_Count];
        uint32_t nameOffset;
        uint32_t flags;
    };

    enum StarType : uint16_t
    {
        Star_O = 1 << 0,
        Star_B = 1 << 1,
        Star_A = 1 << 2,
        Star_F = 1 << 3,
        Star_G = 1 << 4,
        Star_K = 1 << 5,
        Star_M = 1 << 6,
        Star_LTY = 1 << 7,
        Star_Neutron = 1 << 8,
        Star_BlackHole = 1 << 9,
        Star_WhiteDwarf = 1 << 10,
        Star_Other = 1 << 11
    };

    enum SystemFlag : uint32_t
    {
        System_PlayerColonized = 1 << 0,
        System_HasLandable = 1 << 1
    };
}
