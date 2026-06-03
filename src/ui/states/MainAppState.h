#pragma once
#include "ui/states/IAppState.h"
#include "search/SearchEngine.h"
#include "search/AsyncSearchService.h"
#include "search/SearchObserver.h"
#include <vector>
#include <memory>

class MainAppState : public IAppState, public ISearchObserver
{
public:
    MainAppState();
    ~MainAppState() override;
    void Render(AppController* controller) override;

    void OnSearchStarted() override;
    void OnSearchCompleted(const std::vector<SearchResult>& results) override;

private:
    SearchEngine searchEngine;
    std::unique_ptr<AsyncSearchService> asyncSearch;
    SearchFilters filters;
    std::vector<SearchResult> results;
    bool isEngineInitialized;
    bool searchTriggered;
    bool searchRunning;
    int currentPage;

    char systemQueryBuf[128];
    char sourceSystemBuf[128];

    uint64_t minPopBuf;
    uint64_t maxPopBuf;

    bool starO;
    bool starB;
    bool starA;
    bool starF;
    bool starG;
    bool starK;
    bool starM;
    bool starLTY;
    bool starNeutron;
    bool starBlackHole;
    bool starWhiteDwarf;

    bool bodyEnable[8];
    int bodyMin[8];
    int bodyMax[8];
    bool bodyLandable;
};
