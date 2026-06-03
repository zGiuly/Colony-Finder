#pragma once
#include "search/SearchEngine.h"
#include <cstdint>

class AppController;

class IFilterPanelListener
{
public:
    virtual ~IFilterPanelListener() = default;
    virtual void OnRunQuery(const SearchFilters& filters) = 0;
    virtual void OnBackRequested() = 0;
};

class FilterPanel
{
public:
    FilterPanel();

    void SetListener(IFilterPanelListener* listener) { this->listener = listener; }
    void SetBusy(bool busy) { this->busy = busy; }

    void Render(AppController* controller);

private:
    void RenderLocationSection();
    void RenderFiltersSection();
    void RenderStarTypesSection();
    void RenderPlanetTypesSection();
    void RenderActions(AppController* controller);
    void BuildFiltersFromUi();

    IFilterPanelListener* listener;
    bool busy;

    SearchFilters filters;

    char systemQueryBuf[128];
    char sourceSystemBuf[128];

    uint64_t minPopBuf;
    uint64_t maxPopBuf;

    bool starO, starB, starA, starF, starG, starK, starM;
    bool starLTY, starNeutron, starBlackHole, starWhiteDwarf;

    bool bodyEnable[8];
    int bodyMin[8];
    int bodyMax[8];
    bool bodyLandable;
};
