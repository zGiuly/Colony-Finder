#include "ui/states/ErrorState.h"
#include "ui/AppController.h"
#include "ui/UiStrings.h"
#include "ui/states/SelectDownloadState.h"
#include "download/DatabaseService.h"
#include "imgui.h"
#include <utility>

ErrorState::ErrorState()
    : buttonLabel(UiStrings::Error::DefaultBackLabel),
      factory([]() -> std::unique_ptr<IAppState> {
          DatabaseService::GetInstance().FetchOnlineSizes();
          return std::make_unique<SelectDownloadState>();
      })
{
}

ErrorState::ErrorState(std::string buttonLabelVal, ReturnFactory factoryVal)
    : buttonLabel(std::move(buttonLabelVal)),
      factory(std::move(factoryVal))
{
}

void ErrorState::Render(AppController* controller)
{
    ImGui::TextColored(controller->GetTheme().textAlert, "%s", UiStrings::Error::Title);
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::TextWrapped("%s", controller->GetErrorMessage().c_str());
    ImGui::Spacing();
    ImGui::Spacing();

    if (!ImGui::Button(buttonLabel.c_str(), ImVec2(controller->GetButtonWidthLarge(), controller->GetButtonHeightMedium())))
    {
        return;
    }
    if (!factory)
    {
        return;
    }
    controller->TransitionTo(factory());
}
