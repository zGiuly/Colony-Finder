#include "ui/states/WelcomeState.h"
#include "ui/AppController.h"
#include "ui/UiStrings.h"
#include "download/DatabaseService.h"
#include "ui/states/SelectDownloadState.h"
#include "ui/states/SettingsState.h"
#include "imgui.h"

void WelcomeState::Render(AppController* controller)
{
    ImGui::Spacing();
    ImGui::TextColored(controller->GetTheme().orangePrimary, "%s", UiStrings::Welcome::Title);
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("%s", UiStrings::Welcome::Greeting);
    ImGui::Spacing();

    const std::string& currentPath = DatabaseService::GetInstance().GetCurrentFilePath();
    if (!currentPath.empty())
    {
        ImGui::TextColored(controller->GetTheme().textSuccess, UiStrings::Welcome::DumpDetectedFmt, currentPath.c_str());
        ImGui::Spacing();
        ImGui::Spacing();
        if (ImGui::Button(UiStrings::Welcome::EnterApplication, ImVec2(controller->GetButtonWidthLarge(), controller->GetButtonHeightLarge())))
        {
            DatabaseService::GetInstance().EnterApplicationFlow();
        }
        ImGui::SameLine();
        if (ImGui::Button(UiStrings::Common::Settings, ImVec2(controller->GetTheme().buttonWidthMedium, controller->GetButtonHeightLarge())))
        {
            controller->TransitionTo(std::make_unique<SettingsState>());
        }
        return;
    }

    ImGui::TextColored(controller->GetTheme().textAlert, "%s", UiStrings::Welcome::NoDumpDetected);
    ImGui::Spacing();
    ImGui::Spacing();
    if (ImGui::Button(UiStrings::Welcome::ConfigureAndDl, ImVec2(controller->GetButtonWidthLarge(), controller->GetButtonHeightLarge())))
    {
        DatabaseService::GetInstance().FetchOnlineSizes();
        controller->TransitionTo(std::make_unique<SelectDownloadState>());
    }
    ImGui::SameLine();
    if (ImGui::Button(UiStrings::Common::Settings, ImVec2(controller->GetTheme().buttonWidthMedium, controller->GetButtonHeightLarge())))
    {
        controller->TransitionTo(std::make_unique<SettingsState>());
    }
}
