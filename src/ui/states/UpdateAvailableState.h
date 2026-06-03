#pragma once

#include "ui/states/IAppState.h"
#include <string>

class UpdateAvailableState : public IAppState
{
public:
    UpdateAvailableState(const std::string& latestVersion, const std::string& downloadUrl);
    void Render(AppController* controller) override;

private:
    std::string latestVersion;
    std::string downloadUrl;
    bool downloadStarted = false;
};
