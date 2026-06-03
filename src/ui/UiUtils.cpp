#include "ui/UiUtils.h"
#include <cmath>
#include <cstdio>

namespace UiUtils
{
    void DrawSpinner(float radius, float thickness, ImU32 color)
    {
        constexpr float Pi = 3.14159265358979323846f;
        constexpr float Pad = 4.0f;
        constexpr int Segments = 40;

        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImGui::Dummy(ImVec2(radius * 2.0f + Pad * 2.0f, radius * 2.0f + Pad * 2.0f));

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 center(cursor.x + radius + Pad, cursor.y + radius + Pad);

        float t = static_cast<float>(ImGui::GetTime());
        float startAngle = t * 5.0f;
        float arcLength = Pi * 1.5f;

        dl->PathClear();
        for (int i = 0; i <= Segments; ++i)
        {
            float a = startAngle + (arcLength * static_cast<float>(i) / static_cast<float>(Segments));
            dl->PathLineTo(ImVec2(center.x + std::cos(a) * radius, center.y + std::sin(a) * radius));
        }
        dl->PathStroke(color, ImDrawFlags_None, thickness);
    }

    void FormatPopulation(uint64_t pop, char* buf, size_t buflen)
    {
        if (pop == 0) { std::snprintf(buf, buflen, "Uninhabited"); return; }
        if (pop < 1000ULL) { std::snprintf(buf, buflen, "%llu", static_cast<unsigned long long>(pop)); return; }
        if (pop < 1000000ULL) { std::snprintf(buf, buflen, "%.1fK", pop / 1000.0); return; }
        if (pop < 1000000000ULL) { std::snprintf(buf, buflen, "%.2fM", pop / 1000000.0); return; }
        if (pop < 1000000000000ULL) { std::snprintf(buf, buflen, "%.2fB", pop / 1000000000.0); return; }
        std::snprintf(buf, buflen, "%.2fT", pop / 1000000000000.0);
    }
}
