#pragma once

#include "search/SearchEngine.h"
#include "search/SearchObserver.h"
#include <thread>
#include <atomic>
#include <vector>
#include <cstddef>

class AsyncSearchService
{
public:
    explicit AsyncSearchService(SearchEngine& engine);
    ~AsyncSearchService();

    AsyncSearchService(const AsyncSearchService&) = delete;
    AsyncSearchService& operator=(const AsyncSearchService&) = delete;

    void AddObserver(ISearchObserver* observer);
    void RemoveObserver(ISearchObserver* observer);

    void Submit(const SearchFilters& filters, std::size_t maxResults);
    void Poll();
    void WaitForCompletion();

    bool IsRunning() const { return running.load(); }

private:
    void NotifyStarted();
    void NotifyCompleted(const std::vector<SearchResult>& results);

    SearchEngine& engine;
    std::vector<ISearchObserver*> observers;

    std::thread worker;
    std::atomic<bool> running;
    std::atomic<bool> done;
    std::vector<SearchResult> pendingResults;
};
