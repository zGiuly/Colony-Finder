#include "ui/states/MainAppState.h"
#include "ui/AppController.h"
#include "download/DatabaseService.h"
#include "ui/states/SelectDownloadState.h"
#include "imgui.h"
#include <filesystem>

constexpr float FilterPanelWidth = 320.0f;
constexpr size_t MaxResultsToReturn = 5000;
constexpr int ResultsPerPage = 50;

MainAppState::MainAppState()
    : currentController(nullptr),
      isEngineInitialized(false)
{
    filterPanel.SetListener(this);
    resultsPanel.SetResultsPerPage(ResultsPerPage);
    resultsPanel.SetMaxResults(MaxResultsToReturn);
}

MainAppState::~MainAppState()
{
    if (asyncSearch) asyncSearch->RemoveObserver(this);
}

void MainAppState::EnsureEngineInitialized()
{
    if (isEngineInitialized) return;

    std::filesystem::path jsonPath = DatabaseService::GetInstance().GetCurrentFilePath();
    std::filesystem::path indexPath = jsonPath;
    indexPath.replace_extension(".idx");
    searchEngine.Initialize(indexPath.string());

    asyncSearch = std::make_unique<AsyncSearchService>(searchEngine);
    asyncSearch->AddObserver(this);
    isEngineInitialized = true;
}

void MainAppState::OnSearchStarted()
{
    resultsPanel.SetSearching();
    filterPanel.SetBusy(true);
}

void MainAppState::OnSearchCompleted(const std::vector<SearchResult>& res)
{
    resultsPanel.SetResults(res);
    filterPanel.SetBusy(false);
}

void MainAppState::OnRunQuery(const SearchFilters& filters)
{
    if (!asyncSearch) return;
    asyncSearch->Submit(filters, MaxResultsToReturn);
}

void MainAppState::OnBackRequested()
{
    if (!currentController) return;
    asyncSearch.reset();
    searchEngine.Shutdown();
    currentController->TransitionTo(std::make_unique<SelectDownloadState>());
}

void MainAppState::Render(AppController* controller)
{
    currentController = controller;
    EnsureEngineInitialized();

    if (asyncSearch) asyncSearch->Poll();

    ImGui::TextColored(controller->GetTheme().orangePrimary, ":: COLONY FINDER - SYSTEM SEARCH");
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
    filterPanel.Render(controller);
    ImGui::EndChild();

    ImGui::NextColumn();

    ImGui::BeginChild("ResultPanel", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding);
    resultsPanel.Render(controller);
    ImGui::EndChild();

    ImGui::Columns(1);
}
