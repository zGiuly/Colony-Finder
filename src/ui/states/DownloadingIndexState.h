#pragma once
#include "ui/states/IAppState.h"

class DownloadingIndexState : public IAppState
{
public:
    void Render(AppController* controller) override;
};
