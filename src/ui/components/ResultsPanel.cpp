#include "ui/components/ResultsPanel.h"
#include "ui/AppController.h"
#include "ui/UiStrings.h"
#include "ui/UiUtils.h"
#include "imgui.h"
#include <algorithm>
#include <cstdio>

ResultsPanel::ResultsPanel()
    : mode(Mode::Idle),
      currentPage(0),
      resultsPerPage(50),
      maxResultsCap(5000)
{
}

void ResultsPanel::SetIdle()
{
    mode = Mode::Idle;
    results.clear();
    currentPage = 0;
}

void ResultsPanel::SetSearching()
{
    mode = Mode::Searching;
}

void ResultsPanel::SetResults(const std::vector<SearchResult>& res)
{
    results = res;
    mode = Mode::ShowResults;
    currentPage = 0;
}

void ResultsPanel::Render(AppController* controller)
{
    if (mode == Mode::Searching) { RenderSearching(controller); return; }
    if (mode == Mode::Idle) { RenderIdle(controller); return; }
    if (results.empty()) { RenderEmpty(controller); return; }
    RenderResults(controller);
}

void ResultsPanel::RenderIdle(AppController* controller)
{
    ImGui::TextColored(controller->GetTheme().textNormal, "%s", UiStrings::Results::HintRunQuery);
}

void ResultsPanel::RenderSearching(AppController* controller)
{
    ImGui::TextColored(controller->GetTheme().orangeActive, "%s", UiStrings::Results::Searching);
    ImGui::Spacing();
    UiUtils::DrawSpinner(18.0f, 3.5f, ImGui::GetColorU32(controller->GetTheme().orangePrimary));
}

void ResultsPanel::RenderEmpty(AppController* controller)
{
    ImGui::TextColored(controller->GetTheme().textAlert, "%s", UiStrings::Results::NoneFound);
}

void ResultsPanel::RenderResults(AppController* controller)
{
    int totalPages = static_cast<int>((results.size() + resultsPerPage - 1) / resultsPerPage);
    if (currentPage >= totalPages) currentPage = totalPages - 1;
    if (currentPage < 0) currentPage = 0;

    size_t startIdx = static_cast<size_t>(currentPage) * resultsPerPage;
    size_t endIdx = std::min(startIdx + resultsPerPage, results.size());

    if (results.size() >= maxResultsCap)
    {
        ImGui::TextColored(controller->GetTheme().textSuccess, UiStrings::Results::FoundCappedFmt, results.size());
    }
    else
    {
        ImGui::TextColored(controller->GetTheme().textSuccess, UiStrings::Results::FoundFmt, results.size());
    }
    ImGui::Spacing();

    RenderPager(controller, totalPages, startIdx, endIdx);
    ImGui::Spacing();
    RenderTable(controller, startIdx, endIdx);
}

void ResultsPanel::RenderPager(AppController* controller, int totalPages, size_t startIdx, size_t endIdx)
{
    ImGui::BeginDisabled(currentPage <= 0);
    if (ImGui::Button(UiStrings::Results::PrevPage, ImVec2(controller->GetTheme().buttonWidthPager, 0.0f))) currentPage--;
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::Text(UiStrings::Results::PageInfoFmt, currentPage + 1, totalPages, startIdx + 1, endIdx);
    ImGui::SameLine();
    ImGui::BeginDisabled(currentPage >= totalPages - 1);
    if (ImGui::Button(UiStrings::Results::NextPage, ImVec2(controller->GetTheme().buttonWidthPager, 0.0f))) currentPage++;
    ImGui::EndDisabled();
}

void ResultsPanel::RenderTable(AppController* controller, size_t startIdx, size_t endIdx)
{
    constexpr ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg
        | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp;

    if (!ImGui::BeginTable("ResultsTable", 4, flags, ImVec2(0.0f, -1.0f))) return;

    ImGui::TableSetupColumn(UiStrings::Results::ColSystemName, ImGuiTableColumnFlags_WidthStretch, 2.0f);
    ImGui::TableSetupColumn(UiStrings::Results::ColDistance, ImGuiTableColumnFlags_WidthStretch, 1.0f);
    ImGui::TableSetupColumn(UiStrings::Results::ColBodies, ImGuiTableColumnFlags_WidthStretch, 0.8f);
    ImGui::TableSetupColumn(UiStrings::Results::ColPopulation, ImGuiTableColumnFlags_WidthStretch, 1.5f);
    ImGui::TableHeadersRow();

    char popBuf[32];
    char popupId[32];
    for (size_t i = startIdx; i < endIdx; ++i)
    {
        const auto& res = results[i];
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::PushStyleColor(ImGuiCol_Header, controller->GetTheme().rowHover);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, controller->GetTheme().rowHoverActive);
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, controller->GetTheme().rowSelected);
        ImGui::Selectable(res.name.c_str(), false,
            ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap);
        ImGui::PopStyleColor(3);

        std::snprintf(popupId, sizeof(popupId), "row_ctx_%zu", i);
        if (ImGui::BeginPopupContextItem(popupId, ImGuiPopupFlags_MouseButtonRight))
        {
            if (ImGui::MenuItem(UiStrings::Results::CopySystemName))
            {
                ImGui::SetClipboardText(res.name.c_str());
            }
            ImGui::EndPopup();
        }

        ImGui::TableSetColumnIndex(1);
        ImGui::Text(UiStrings::Results::DistanceLyFmt, res.distanceToSource);

        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%d", res.bodyCount);

        ImGui::TableSetColumnIndex(3);
        UiUtils::FormatPopulation(res.population, popBuf, sizeof(popBuf));
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
