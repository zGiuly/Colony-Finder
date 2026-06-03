#pragma once
#include "ui/states/IAppState.h"

class IndexingState : public IAppState
{
public:
    ~IndexingState() override = default;
    void Render(AppController* controller) override;
};
