#pragma once

#include "ui/states/IAppState.h"

class DownloadingState : public IAppState
{
public:
    ~DownloadingState() override = default;
    void Render(AppController* controller) override;
};
