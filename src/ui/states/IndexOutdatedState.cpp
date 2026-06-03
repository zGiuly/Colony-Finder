#include "ui/states/IndexOutdatedState.h"
#include "ui/AppController.h"
#include "download/DatabaseService.h"
#include "ui/states/SelectDownloadState.h"
#include "imgui.h"

void IndexOutdatedState::Render(AppController* controller)
{
    ImGui::Spacing();
    ImGui::TextColored(controller->GetTheme().orangePrimary, ":: INDEX FORMAT OUTDATED");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(controller->GetTheme().textAlert, "[!] The existing index is no longer compatible with this build.");
    ImGui::Spacing();
    ImGui::TextWrapped("The application has been updated and the index file format has changed. The existing index must be regenerated from the source file before the database can be used.");
    ImGui::Spacing();
    ImGui::TextColored(controller->GetTheme().textNormal, "The old index will be deleted and a new one will be generated using the existing source file.");
    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::Button("Regenerate Index", ImVec2(controller->GetButtonWidthLarge(), controller->GetButtonHeightLarge())))
    {
        DatabaseService::GetInstance().ConfirmIndexRegeneration();
    }
    ImGui::Spacing();

    if (ImGui::Button("Back to Setup", ImVec2(controller->GetButtonWidthMedium(), controller->GetButtonHeightMedium())))
    {
        controller->TransitionTo(std::make_unique<SelectDownloadState>());
    }
}
