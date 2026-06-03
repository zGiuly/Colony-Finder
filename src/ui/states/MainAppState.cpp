#include "ui/states/MainAppState.h"
#include "ui/AppController.h"
#include "download/DatabaseService.h"
#include "ui/states/SelectDownloadState.h"
#include "imgui.h"
#include <filesystem>
#include <cstdio>
#include <cstring>

constexpr float FilterPanelWidth = 320.0f;
constexpr size_t MaxResultsToReturn = 100;
constexpr float MinDistanceLy = 0.0f;
constexpr float MaxDistanceLy = 100000.0f;
constexpr float DefaultMaxDistanceLy = 100.0f;
constexpr int MinBodiesDefault = 0;
constexpr int MaxBodiesDefault = 100;

static void FormatPopulation(uint64_t pop, char* buf, size_t buflen)
{
    if (pop == 0) { std::snprintf(buf, buflen, "Uninhabited"); return; }
    if (pop < 1000ULL) { std::snprintf(buf, buflen, "%llu", static_cast<unsigned long long>(pop)); return; }
    if (pop < 1000000ULL) { std::snprintf(buf, buflen, "%.1fK", pop / 1000.0); return; }
    if (pop < 1000000000ULL) { std::snprintf(buf, buflen, "%.2fM", pop / 1000000.0); return; }
    if (pop < 1000000000000ULL) { std::snprintf(buf, buflen, "%.2fB", pop / 1000000000.0); return; }
    std::snprintf(buf, buflen, "%.2fT", pop / 1000000000000.0);
}

MainAppState::MainAppState()
    : isEngineInitialized(false),
      searchTriggered(false),
      minPopBuf(0),
      maxPopBuf(100000000000ULL),
      starO(false),
      starB(false),
      starA(false),
      starF(false),
      starG(false),
      starK(false),
      starM(false),
      starLTY(false),
      starNeutron(false),
      starBlackHole(false),
      starWhiteDwarf(false),
      bodyELW(false),
      bodyWW(false),
      bodyAMW(false),
      bodyHMC(false),
      bodyMetalRich(false),
      bodyRocky(false),
      bodyIcy(false),
      bodyGasGiant(false),
      bodyLandable(false),
      bodyBio(false),
      bodyGeo(false)
{
    std::memset(systemQueryBuf, 0, sizeof(systemQueryBuf));
    std::memset(sourceSystemBuf, 0, sizeof(sourceSystemBuf));
    std::snprintf(sourceSystemBuf, sizeof(sourceSystemBuf), "Sol");

    filters.filterDistance = true;
    filters.maxDistanceLy = DefaultMaxDistanceLy;
}

void MainAppState::Render(AppController* controller)
{
    if (!isEngineInitialized)
    {
        std::filesystem::path jsonPath = DatabaseService::GetInstance().GetCurrentFilePath();
        std::filesystem::path indexPath = jsonPath;
        indexPath.replace_extension(".idx");
        searchEngine.Initialize(indexPath.string());
        isEngineInitialized = true;
    }

    ImGui::TextColored(controller->GetTheme().orangePrimary, ":: COMMANDER'S CONSOLE - SYSTEM SEARCH");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Columns(2, "MainLayout", true);
    
    static bool initColumnWidth = false;
    if (!initColumnWidth)
    {
        ImGui::SetColumnWidth(0, FilterPanelWidth);
        initColumnWidth = true;
    }

    ImGui::BeginChild("FilterPanel", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding);
    
    ImGui::TextColored(controller->GetTheme().orangeActive, "Location & Range");
    ImGui::Spacing();

    ImGui::Text("Start System:");
    ImGui::InputText("##SourceSystem", sourceSystemBuf, sizeof(sourceSystemBuf));
    ImGui::Spacing();

    ImGui::Checkbox("Filter Distance", &filters.filterDistance);
    if (filters.filterDistance)
    {
        ImGui::SliderFloat("Range (Ly)", &filters.maxDistanceLy, MinDistanceLy, MaxDistanceLy, "%.1f Ly");
    }
    ImGui::Spacing();

    ImGui::Text("Filter Name:");
    ImGui::InputText("##SystemQuery", systemQueryBuf, sizeof(systemQueryBuf));
    ImGui::Spacing();

    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Checkbox("Player Colonized Only", &filters.colonizedOnly);
    ImGui::Spacing();

    ImGui::Checkbox("Filter Population", &filters.filterPopulation);
    if (filters.filterPopulation)
    {
        ImGui::InputScalar("Min Pop", ImGuiDataType_U64, &minPopBuf);
        ImGui::InputScalar("Max Pop", ImGuiDataType_U64, &maxPopBuf);
        filters.minPopulation = minPopBuf;
        filters.maxPopulation = maxPopBuf;
    }
    ImGui::Spacing();

    ImGui::Checkbox("Filter Body Count", &filters.filterBodyCount);
    if (filters.filterBodyCount)
    {
        int minB = filters.minBodies;
        int maxB = filters.maxBodies;
        ImGui::DragInt("Min Bodies", &minB, 1.0f, MinBodiesDefault, MaxBodiesDefault);
        ImGui::DragInt("Max Bodies", &maxB, 1.0f, MinBodiesDefault, MaxBodiesDefault);
        filters.minBodies = static_cast<uint16_t>(minB);
        filters.maxBodies = static_cast<uint16_t>(maxB);
    }
    ImGui::Spacing();

    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::TreeNode("Star Types"))
    {
        ImGui::Checkbox("O Star", &starO);
        ImGui::Checkbox("B Star", &starB);
        ImGui::Checkbox("A Star", &starA);
        ImGui::Checkbox("F Star", &starF);
        ImGui::Checkbox("G Star", &starG);
        ImGui::Checkbox("K Star", &starK);
        ImGui::Checkbox("M Star", &starM);
        ImGui::Checkbox("L/T/Y Dwarf", &starLTY);
        ImGui::Checkbox("Neutron", &starNeutron);
        ImGui::Checkbox("Black Hole", &starBlackHole);
        ImGui::Checkbox("White Dwarf", &starWhiteDwarf);
        ImGui::TreePop();
    }
    ImGui::Spacing();

    if (ImGui::TreeNode("Planet Types"))
    {
        ImGui::Checkbox("Earth-like (ELW)", &bodyELW);
        ImGui::Checkbox("Water World (WW)", &bodyWW);
        ImGui::Checkbox("Ammonia World (AMW)", &bodyAMW);
        ImGui::Checkbox("High Metal (HMC)", &bodyHMC);
        ImGui::Checkbox("Metal-rich", &bodyMetalRich);
        ImGui::Checkbox("Rocky", &bodyRocky);
        ImGui::Checkbox("Icy", &bodyIcy);
        ImGui::Checkbox("Gas Giant", &bodyGasGiant);
        ImGui::Checkbox("Landable", &bodyLandable);
        ImGui::Checkbox("Bio Signals", &bodyBio);
        ImGui::Checkbox("Geo Signals", &bodyGeo);
        ImGui::TreePop();
    }
    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::Button("RUN QUERY", ImVec2(-1.0f, 40.0f)))
    {
        filters.systemNameQuery = systemQueryBuf;
        filters.sourceSystemName = sourceSystemBuf;

        filters.filterStarType = (starO || starB || starA || starF || starG || starK || starM || starLTY || starNeutron || starBlackHole || starWhiteDwarf);
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

        filters.filterBodyType = (bodyELW || bodyWW || bodyAMW || bodyHMC || bodyMetalRich || bodyRocky || bodyIcy || bodyGasGiant || bodyLandable || bodyBio || bodyGeo);
        filters.bodyTypesMask = 0;
        if (bodyELW) filters.bodyTypesMask |= SystemIndex::Body_ELW;
        if (bodyWW) filters.bodyTypesMask |= SystemIndex::Body_WW;
        if (bodyAMW) filters.bodyTypesMask |= SystemIndex::Body_AMW;
        if (bodyHMC) filters.bodyTypesMask |= SystemIndex::Body_HMC;
        if (bodyMetalRich) filters.bodyTypesMask |= SystemIndex::Body_MetalRich;
        if (bodyRocky) filters.bodyTypesMask |= SystemIndex::Body_Rocky;
        if (bodyIcy) filters.bodyTypesMask |= SystemIndex::Body_Icy;
        if (bodyGasGiant) filters.bodyTypesMask |= SystemIndex::Body_GasGiant;
        if (bodyLandable) filters.bodyTypesMask |= SystemIndex::Body_Landable;
        if (bodyBio) filters.bodyTypesMask |= SystemIndex::Body_BioSignals;
        if (bodyGeo) filters.bodyTypesMask |= SystemIndex::Body_GeoSignals;

        searchEngine.Search(filters, results, MaxResultsToReturn);
        searchTriggered = true;
    }
    ImGui::Spacing();

    if (ImGui::Button("Back to Setup", ImVec2(-1.0f, 30.0f)))
    {
        searchEngine.Shutdown();
        controller->TransitionTo(std::make_unique<SelectDownloadState>());
    }

    ImGui::EndChild();
    ImGui::NextColumn();

    ImGui::BeginChild("ResultPanel", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding);
    
    if (!searchTriggered)
    {
        ImGui::TextColored(controller->GetTheme().textNormal, "Configure the filters on the left panel and click 'RUN QUERY' to locate systems.");
    }
    else if (results.empty())
    {
        ImGui::TextColored(controller->GetTheme().textAlert, "No systems found matching the specified search criteria.");
    }
    else
    {
        ImGui::TextColored(controller->GetTheme().textSuccess, "Found %zu systems:", results.size());
        ImGui::Spacing();

        if (ImGui::BeginTable("ResultsTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp, ImVec2(0.0f, -1.0f)))
        {
            ImGui::TableSetupColumn("System Name", ImGuiTableColumnFlags_WidthStretch, 2.0f);
            ImGui::TableSetupColumn("Distance", ImGuiTableColumnFlags_WidthStretch, 1.0f);
            ImGui::TableSetupColumn("Bodies", ImGuiTableColumnFlags_WidthStretch, 0.8f);
            ImGui::TableSetupColumn("Population", ImGuiTableColumnFlags_WidthStretch, 1.5f);
            ImGui::TableHeadersRow();

            char popBuf[32];
            for (const auto& res : results)
            {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", res.name.c_str());

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%.2f Ly", res.distanceToSource);

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%d", res.bodyCount);

                ImGui::TableSetColumnIndex(3);
                FormatPopulation(res.population, popBuf, sizeof(popBuf));
                if (res.population > 0)
                {
                    ImGui::Text("%s", popBuf);
                }
                else
                {
                    ImGui::TextColored(controller->GetTheme().textMuted, "%s", popBuf);
                }
            }
            ImGui::EndTable();
        }
    }

    ImGui::EndChild();
    ImGui::Columns(1);
}
