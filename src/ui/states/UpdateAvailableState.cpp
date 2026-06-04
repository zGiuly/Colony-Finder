#include "ui/states/UpdateAvailableState.h"
#include "ui/AppController.h"
#include "ui/UiStrings.h"
#include "ui/states/WelcomeState.h"
#include "update/UpdateService.h"
#include "Version.h"
#include "imgui.h"
#include <cstdlib>

UpdateAvailableState::UpdateAvailableState(const std::string& latestVersionVal, const std::string& downloadUrlVal)
    : latestVersion(latestVersionVal), downloadUrl(downloadUrlVal)
{
}

void UpdateAvailableState::Render(AppController* controller)
{
    ImGui::Spacing();
    ImGui::TextColored(controller->GetTheme().orangePrimary, "%s", UiStrings::UpdateAvailable::Title);
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text(UiStrings::UpdateAvailable::CurrentFmt, COLONYFINDER_VERSION_STRING);
    ImGui::Text(UiStrings::UpdateAvailable::LatestFmt, latestVersion.c_str());
    ImGui::Spacing();

    if (controller->IsUpdateReady())
    {
        ImGui::TextColored(controller->GetTheme().textSuccess, "%s", UiStrings::UpdateAvailable::Downloaded);
        ImGui::Spacing();
        if (ImGui::Button(UiStrings::UpdateAvailable::RestartNow, ImVec2(controller->GetButtonWidthLarge(), controller->GetButtonHeightLarge())))
        {
            UpdateService::GetInstance().RestartIntoUpdate();
            std::exit(0);
        }
        return;
    }

    if (downloadStarted)
    {
        ImGui::TextColored(controller->GetTheme().textAlert, "%s", UiStrings::UpdateAvailable::Downloading);
        ImGui::ProgressBar(static_cast<float>(controller->GetUpdateDownloadProgress()), ImVec2(-1.0f, 0.0f));
        return;
    }

    ImGui::TextWrapped("%s", UiStrings::UpdateAvailable::Prompt);
    ImGui::Spacing();
    if (ImGui::Button(UiStrings::UpdateAvailable::UpdateNow, ImVec2(controller->GetButtonWidthMedium(), controller->GetButtonHeightMedium())))
    {
        downloadStarted = true;
        UpdateService::GetInstance().DownloadAndApplyAsync(downloadUrl);
    }
    ImGui::SameLine();
    if (ImGui::Button(UiStrings::UpdateAvailable::Skip, ImVec2(controller->GetButtonWidthMedium(), controller->GetButtonHeightMedium())))
    {
        controller->TransitionTo(std::make_unique<WelcomeState>());
    }
}
