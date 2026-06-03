#pragma once
#include <stdint.h>

namespace SystemIndex
{
    constexpr uint32_t Version = 2;

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
        uint32_t bodyTypesMask;
        uint32_t nameOffset;
        uint32_t flags;
        uint32_t reserved;
    };

    enum SystemFlag : uint32_t
    {
        System_PlayerColonized = 1 << 0
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

    enum BodyType : uint32_t
    {
        Body_ELW = 1 << 0,
        Body_WW = 1 << 1,
        Body_AMW = 1 << 2,
        Body_HMC = 1 << 3,
        Body_MetalRich = 1 << 4,
        Body_Rocky = 1 << 5,
        Body_Icy = 1 << 6,
        Body_GasGiant = 1 << 7,
        Body_Landable = 1 << 8,
        Body_BioSignals = 1 << 9,
        Body_GeoSignals = 1 << 10
    };
}
