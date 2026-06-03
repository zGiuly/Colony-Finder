#pragma once

#include "ui/states/IAppState.h"

class IndexOutdatedState : public IAppState
{
public:
    ~IndexOutdatedState() override = default;
    void Render(AppController* controller) override;
};
