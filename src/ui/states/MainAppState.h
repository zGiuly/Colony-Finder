#pragma once
#include "ui/states/IAppState.h"
#include "search/SearchEngine.h"
#include <vector>

class MainAppState : public IAppState
{
public:
    MainAppState();
    ~MainAppState() override = default;
    void Render(AppController* controller) override;

private:
    SearchEngine searchEngine;
    SearchFilters filters;
    std::vector<SearchResult> results;
    bool isEngineInitialized;
    bool searchTriggered;

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

    bool bodyELW;
    bool bodyWW;
    bool bodyAMW;
    bool bodyHMC;
    bool bodyMetalRich;
    bool bodyRocky;
    bool bodyIcy;
    bool bodyGasGiant;
    bool bodyLandable;
    bool bodyBio;
    bool bodyGeo;
};
