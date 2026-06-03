#pragma once
#include "ui/states/IAppState.h"
#include "ui/components/FilterPanel.h"
#include "ui/components/ResultsPanel.h"
#include "search/SearchEngine.h"
#include "search/AsyncSearchService.h"
#include "search/SearchObserver.h"
#include <memory>

class MainAppState : public IAppState,
                     public ISearchObserver,
                     public IFilterPanelListener
{
public:
    MainAppState();
    ~MainAppState() override;

    void Render(AppController* controller) override;

    void OnSearchStarted() override;
    void OnSearchCompleted(const std::vector<SearchResult>& results) override;

    void OnRunQuery(const SearchFilters& filters) override;
    void OnBackRequested() override;

private:
    void EnsureEngineInitialized();

    SearchEngine searchEngine;
    std::unique_ptr<AsyncSearchService> asyncSearch;

    FilterPanel filterPanel;
    ResultsPanel resultsPanel;

    AppController* currentController;
    bool isEngineInitialized;
};
