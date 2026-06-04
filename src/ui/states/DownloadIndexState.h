#pragma once
#include "ui/states/IAppState.h"
#include <string>

class DownloadIndexState : public IAppState
{
public:
    enum class SourceKind
    {
        Official,
        Custom
    };

    DownloadIndexState();
    void Render(AppController* controller) override;

private:
    void RenderSourceSelector(AppController* controller);
    void RenderDownloadControls(AppController* controller);
    void ApplySourceKind(SourceKind kind);

    SourceKind sourceKind;
    std::string urlBuffer;
};
