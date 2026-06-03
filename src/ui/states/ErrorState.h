#pragma once

#include "ui/states/IAppState.h"

class ErrorState : public IAppState
{
public:
    ~ErrorState() override = default;
    void Render(AppController* controller) override;
};
