#pragma once

#include "search/SearchEngine.h"
#include <vector>

class ISearchObserver
{
public:
    virtual ~ISearchObserver() = default;
    virtual void OnSearchStarted() = 0;
    virtual void OnSearchCompleted(const std::vector<SearchResult>& results) = 0;
};
