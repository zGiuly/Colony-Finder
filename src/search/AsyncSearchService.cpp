#include "search/AsyncSearchService.h"
#include <algorithm>

AsyncSearchService::AsyncSearchService(SearchEngine& eng)
    : engine(eng), running(false), done(false)
{
}

AsyncSearchService::~AsyncSearchService()
{
    WaitForCompletion();
}

void AsyncSearchService::AddObserver(ISearchObserver* observer)
{
    if (std::find(observers.begin(), observers.end(), observer) == observers.end())
    {
        observers.push_back(observer);
    }
}

void AsyncSearchService::RemoveObserver(ISearchObserver* observer)
{
    auto it = std::find(observers.begin(), observers.end(), observer);
    if (it != observers.end()) observers.erase(it);
}

void AsyncSearchService::Submit(const SearchFilters& filters, std::size_t maxResults)
{
    WaitForCompletion();

    pendingResults.clear();
    done.store(false);
    running.store(true);
    NotifyStarted();

    SearchFilters snapshot = filters;
    worker = std::thread([this, snapshot, maxResults]() {
        engine.Search(snapshot, pendingResults, maxResults);
        running.store(false);
        done.store(true);
    });
}

void AsyncSearchService::Poll()
{
    if (!done.load()) return;
    if (worker.joinable()) worker.join();
    done.store(false);
    NotifyCompleted(pendingResults);
    pendingResults.clear();
}

void AsyncSearchService::WaitForCompletion()
{
    if (worker.joinable()) worker.join();
    done.store(false);
    running.store(false);
}

void AsyncSearchService::NotifyStarted()
{
    for (auto* o : observers) o->OnSearchStarted();
}

void AsyncSearchService::NotifyCompleted(const std::vector<SearchResult>& results)
{
    for (auto* o : observers) o->OnSearchCompleted(results);
}
