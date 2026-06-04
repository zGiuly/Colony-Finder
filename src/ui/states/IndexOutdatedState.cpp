#include "ui/states/IndexOutdatedState.h"
#include "ui/AppController.h"
#include "ui/UiStrings.h"
#include "download/DatabaseService.h"
#include "ui/states/SelectDownloadState.h"
#include "imgui.h"

void IndexOutdatedState::Render(AppController* controller)
{
    ImGui::Spacing();
    ImGui::TextColored(controller->GetTheme().orangePrimary, "%s", UiStrings::IndexOutdated::Title);
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(controller->GetTheme().textAlert, "%s", UiStrings::IndexOutdated::Alert);
    ImGui::Spacing();
    ImGui::TextWrapped("%s", UiStrings::IndexOutdated::Description);
    ImGui::Spacing();
    ImGui::TextColored(controller->GetTheme().textNormal, "%s", UiStrings::IndexOutdated::Detail);
    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::Button(UiStrings::IndexOutdated::RegenerateButton, ImVec2(controller->GetButtonWidthLarge(), controller->GetButtonHeightLarge())))
    {
        DatabaseService::GetInstance().ConfirmIndexRegeneration();
    }
    ImGui::Spacing();

    if (ImGui::Button(UiStrings::Common::BackToSetup, ImVec2(controller->GetButtonWidthMedium(), controller->GetButtonHeightMedium())))
    {
        controller->TransitionTo(std::make_unique<SelectDownloadState>());
    }
}
