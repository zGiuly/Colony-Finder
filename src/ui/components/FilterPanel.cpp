#include "ui/components/FilterPanel.h"
#include "ui/AppController.h"
#include "ui/UiStrings.h"
#include "imgui.h"
#include <cstring>
#include <cstdio>

constexpr float MinDistanceLy = 0.0f;
constexpr float MaxDistanceLy = 100000.0f;
constexpr float DefaultMaxDistanceLy = 100.0f;
constexpr int MinBodiesDefault = 0;
constexpr int MaxBodiesDefault = 100;

FilterPanel::FilterPanel()
    : listener(nullptr),
      busy(false),
      minPopBuf(0),
      maxPopBuf(100000000000ULL),
      starO(false), starB(false), starA(false), starF(false),
      starG(false), starK(false), starM(false), starLTY(false),
      starNeutron(false), starBlackHole(false), starWhiteDwarf(false),
      bodyLandable(false)
{
    for (int i = 0; i < 8; ++i) { bodyEnable[i] = false; bodyMin[i] = 1; bodyMax[i] = 100; }
    std::memset(systemQueryBuf, 0, sizeof(systemQueryBuf));
    std::memset(sourceSystemBuf, 0, sizeof(sourceSystemBuf));
    std::snprintf(sourceSystemBuf, sizeof(sourceSystemBuf), "Sol");

    filters.filterDistance = true;
    filters.maxDistanceLy = DefaultMaxDistanceLy;
}

void FilterPanel::Render(AppController* controller)
{
    ImGui::TextColored(controller->GetTheme().orangeActive, "%s", UiStrings::Filters::LocationSection);
    ImGui::Spacing();

    RenderLocationSection();
    ImGui::Separator();
    ImGui::Spacing();

    RenderFiltersSection();
    ImGui::Separator();
    ImGui::Spacing();

    RenderStarTypesSection();
    ImGui::Spacing();

    RenderPlanetTypesSection();
    ImGui::Spacing();
    ImGui::Spacing();

    RenderActions(controller);
}

void FilterPanel::RenderLocationSection()
{
    ImGui::Text("%s", UiStrings::Filters::StartSystem);
    ImGui::InputText("##SourceSystem", sourceSystemBuf, sizeof(sourceSystemBuf));
    ImGui::Spacing();

    ImGui::Checkbox(UiStrings::Filters::FilterDistance, &filters.filterDistance);
    if (filters.filterDistance)
    {
        ImGui::SliderFloat(UiStrings::Filters::RangeLy, &filters.maxDistanceLy, MinDistanceLy, MaxDistanceLy, UiStrings::Filters::RangeLyFormat);
    }
    ImGui::Spacing();

    ImGui::Text("%s", UiStrings::Filters::FilterName);
    ImGui::InputText("##SystemQuery", systemQueryBuf, sizeof(systemQueryBuf));
    ImGui::Spacing();
}

void FilterPanel::RenderFiltersSection()
{
    ImGui::Checkbox(UiStrings::Filters::ColonizedOnly, &filters.colonizedOnly);
    ImGui::Spacing();

    ImGui::Checkbox(UiStrings::Filters::FilterPopulation, &filters.filterPopulation);
    if (filters.filterPopulation)
    {
        ImGui::InputScalar(UiStrings::Filters::MinPop, ImGuiDataType_U64, &minPopBuf);
        ImGui::InputScalar(UiStrings::Filters::MaxPop, ImGuiDataType_U64, &maxPopBuf);
        filters.minPopulation = minPopBuf;
        filters.maxPopulation = maxPopBuf;
    }
    ImGui::Spacing();

    ImGui::Checkbox(UiStrings::Filters::FilterBodyCount, &filters.filterBodyCount);
    if (filters.filterBodyCount)
    {
        int minB = filters.minBodies;
        int maxB = filters.maxBodies;
        ImGui::DragInt(UiStrings::Filters::MinBodies, &minB, 1.0f, MinBodiesDefault, MaxBodiesDefault);
        ImGui::DragInt(UiStrings::Filters::MaxBodies, &maxB, 1.0f, MinBodiesDefault, MaxBodiesDefault);
        filters.minBodies = static_cast<uint16_t>(minB);
        filters.maxBodies = static_cast<uint16_t>(maxB);
    }
    ImGui::Spacing();
}

void FilterPanel::RenderStarTypesSection()
{
    if (!ImGui::TreeNode(UiStrings::Filters::StarTypesNode)) return;

    ImGui::Checkbox(UiStrings::Filters::StarO, &starO);
    ImGui::Checkbox(UiStrings::Filters::StarB, &starB);
    ImGui::Checkbox(UiStrings::Filters::StarA, &starA);
    ImGui::Checkbox(UiStrings::Filters::StarF, &starF);
    ImGui::Checkbox(UiStrings::Filters::StarG, &starG);
    ImGui::Checkbox(UiStrings::Filters::StarK, &starK);
    ImGui::Checkbox(UiStrings::Filters::StarM, &starM);
    ImGui::Checkbox(UiStrings::Filters::StarLTY, &starLTY);
    ImGui::Checkbox(UiStrings::Filters::StarNeutron, &starNeutron);
    ImGui::Checkbox(UiStrings::Filters::StarBlackHole, &starBlackHole);
    ImGui::Checkbox(UiStrings::Filters::StarWhiteDwarf, &starWhiteDwarf);
    ImGui::TreePop();
}

void FilterPanel::RenderPlanetTypesSection()
{
    if (!ImGui::TreeNode(UiStrings::Filters::PlanetTypesNode)) return;

    static const char* labels[8] = {
        UiStrings::Filters::PlanetEarthLike, UiStrings::Filters::PlanetWaterWorld,
        UiStrings::Filters::PlanetAmmonia,   UiStrings::Filters::PlanetHighMetal,
        UiStrings::Filters::PlanetMetalRich, UiStrings::Filters::PlanetRocky,
        UiStrings::Filters::PlanetIcy,       UiStrings::Filters::PlanetGasGiant
    };

    for (int i = 0; i < 8; ++i)
    {
        ImGui::PushID(i);
        ImGui::Checkbox(labels[i], &bodyEnable[i]);
        if (bodyEnable[i])
        {
            ImGui::Indent();
            ImGui::PushItemWidth(60.0f);
            ImGui::InputInt(UiStrings::Filters::Min, &bodyMin[i], 0, 0);
            ImGui::SameLine();
            ImGui::InputInt(UiStrings::Filters::Max, &bodyMax[i], 0, 0);
            ImGui::PopItemWidth();
            if (bodyMin[i] < 0) bodyMin[i] = 0;
            if (bodyMax[i] > 255) bodyMax[i] = 255;
            if (bodyMax[i] < bodyMin[i]) bodyMax[i] = bodyMin[i];
            ImGui::Unindent();
        }
        ImGui::PopID();
    }
    ImGui::Checkbox(UiStrings::Filters::LandableRequired, &bodyLandable);
    ImGui::TreePop();
}

void FilterPanel::RenderActions(AppController* controller)
{
    ImGui::BeginDisabled(busy);
    if (ImGui::Button(UiStrings::Filters::RunQuery, ImVec2(-1.0f, controller->GetTheme().buttonHeightMedium)))
    {
        BuildFiltersFromUi();
        if (listener) listener->OnRunQuery(filters);
    }
    ImGui::EndDisabled();
    ImGui::Spacing();

    if (ImGui::Button(UiStrings::Common::BackToSetup, ImVec2(-1.0f, controller->GetTheme().buttonHeightSmall)))
    {
        if (listener) listener->OnBackRequested();
    }
}

void FilterPanel::BuildFiltersFromUi()
{
    filters.systemNameQuery = systemQueryBuf;
    filters.sourceSystemName = sourceSystemBuf;

    filters.filterStarType = (starO || starB || starA || starF || starG || starK || starM
                              || starLTY || starNeutron || starBlackHole || starWhiteDwarf);
    filters.starTypesMask = 0;
    if (starO) filters.starTypesMask |= SystemIndex::Star_O;
    if (starB) filters.starTypesMask |= SystemIndex::Star_B;
    if (starA) filters.starTypesMask |= SystemIndex::Star_A;
    if (starF) filters.starTypesMask |= SystemIndex::Star_F;
    if (starG) filters.starTypesMask |= SystemIndex::Star_G;
    if (starK) filters.starTypesMask |= SystemIndex::Star_K;
    if (starM) filters.starTypesMask |= SystemIndex::Star_M;
    if (starLTY) filters.starTypesMask |= SystemIndex::Star_LTY;
    if (starNeutron) filters.starTypesMask |= SystemIndex::Star_Neutron;
    if (starBlackHole) filters.starTypesMask |= SystemIndex::Star_BlackHole;
    if (starWhiteDwarf) filters.starTypesMask |= SystemIndex::Star_WhiteDwarf;

    for (int i = 0; i < 8; ++i)
    {
        filters.bodyCountEnabled[i] = bodyEnable[i];
        filters.minBodyTypeCount[i] = static_cast<uint8_t>(bodyMin[i]);
        filters.maxBodyTypeCount[i] = static_cast<uint8_t>(bodyMax[i]);
    }
    filters.filterLandable = bodyLandable;
}
