#include "ui/states/ErrorState.h"
#include "ui/AppController.h"
#include "ui/states/SelectDownloadState.h"
#include "imgui.h"

void ErrorState::Render(AppController* controller)
{
    ImGui::TextColored(controller->GetTheme().textAlert, "[!] SYSTEM CRITICAL ERROR");
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::TextWrapped("%s", controller->GetErrorMessage().c_str());
    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::Button("< RETURN TO SETUP", ImVec2(controller->GetButtonWidthLarge(), controller->GetButtonHeightMedium())))
    {
        controller->FetchOnlineSizes();
        controller->TransitionTo(std::make_unique<SelectDownloadState>());
    }
}
