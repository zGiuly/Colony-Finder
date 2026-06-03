#pragma once

#include "ui/states/IAppState.h"

class MainAppState : public IAppState
{
public:
    ~MainAppState() override = default;
    void Render(AppController* controller) override;
};
