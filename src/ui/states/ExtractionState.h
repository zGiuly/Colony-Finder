#pragma once

#include "ui/states/IAppState.h"

class ExtractionState : public IAppState
{
public:
    ~ExtractionState() override = default;
    void Render(AppController* controller) override;
};
