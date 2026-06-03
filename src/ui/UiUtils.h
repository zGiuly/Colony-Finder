#pragma once
#include "imgui.h"
#include <cstdint>
#include <cstddef>

namespace UiUtils
{
    void DrawSpinner(float radius, float thickness, ImU32 color);
    void FormatPopulation(uint64_t pop, char* buf, size_t buflen);
}
