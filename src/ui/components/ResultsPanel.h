#pragma once
#include "search/SearchEngine.h"
#include <vector>
#include <cstddef>

class AppController;

class ResultsPanel
{
public:
    ResultsPanel();

    void SetResultsPerPage(int perPage) { resultsPerPage = perPage; }
    void SetMaxResults(size_t maxResults) { maxResultsCap = maxResults; }

    void SetIdle();
    void SetSearching();
    void SetResults(const std::vector<SearchResult>& results);

    void Render(AppController* controller);

private:
    void RenderIdle(AppController* controller);
    void RenderSearching(AppController* controller);
    void RenderEmpty(AppController* controller);
    void RenderResults(AppController* controller);
    void RenderPager(AppController* controller, int totalPages, size_t startIdx, size_t endIdx);
    void RenderTable(AppController* controller, size_t startIdx, size_t endIdx);

    enum class Mode { Idle, Searching, ShowResults };

    Mode mode;
    std::vector<SearchResult> results;
    int currentPage;
    int resultsPerPage;
    size_t maxResultsCap;
};
