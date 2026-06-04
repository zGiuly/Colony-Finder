#pragma once

#include "ui/states/IAppState.h"
#include <functional>
#include <memory>
#include <string>

class ErrorState : public IAppState
{
public:
    using ReturnFactory = std::function<std::unique_ptr<IAppState>()>;

    ErrorState();
    ErrorState(std::string buttonLabel, ReturnFactory factory);

    ~ErrorState() override = default;
    void Render(AppController* controller) override;

private:
    std::string buttonLabel;
    ReturnFactory factory;
};
