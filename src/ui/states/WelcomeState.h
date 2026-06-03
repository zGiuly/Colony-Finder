#pragma once

#include "ui/states/IAppState.h"

class WelcomeState : public IAppState
{
public:
    ~WelcomeState() override = default;
    void Render(AppController* controller) override;
};
