#pragma once
#include "ui/states/IAppState.h"
#include <functional>
#include <memory>

class SettingsState : public IAppState
{
public:
    using ReturnFactory = std::function<std::unique_ptr<IAppState>()>;

    SettingsState();
    explicit SettingsState(ReturnFactory factory);

    void Render(AppController* controller) override;

private:
    ReturnFactory factory;
};
