#pragma once

#include "ui/states/IAppState.h"

class SelectDownloadState : public IAppState
{
public:
    ~SelectDownloadState() override = default;
    void Render(AppController* controller) override;
};
