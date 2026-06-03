#pragma once

#include "ui/states/IAppState.h"
#include <memory>

class SchemaUpdateState : public IAppState
{
public:
    ~SchemaUpdateState() override = default;
    void Render(AppController* controller) override;
};
