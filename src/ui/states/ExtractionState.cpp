#include "ui/states/ExtractionState.h"
#include "ui/AppController.h"
#include "ui/states/SelectDownloadState.h"
#include "imgui.h"

void ExtractionState::Render(AppController* controller)
{
    ImGui::TextColored(controller->GetTheme().orangePrimary, ":: DATABASE PROCESSING");
    ImGui::Separator();
    ImGui::Spacing();

    if (controller->IsExtracting())
    {
        ImGui::Text("Extracting database file (Decompressing Gzip)...");
        ImGui::Spacing();
        float progress = controller->GetExtractionProgress();
        ImGui::ProgressBar(progress, ImVec2(-1.0f, 25.0f));
    }
    else if (controller->IsValidating())
    {
        ImGui::Text("Validating JSON format and Spansh schema...");
        ImGui::Spacing();
        float progress = controller->GetValidationProgress();
        ImGui::ProgressBar(progress, ImVec2(-1.0f, 25.0f));
    }
    else
    {
        ImGui::Text("Preparing processing...");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Cancel", ImVec2(controller->GetButtonWidthMedium(), controller->GetButtonHeightMedium())))
    {
        controller->CancelExtraction();
        controller->TransitionTo(std::make_unique<SelectDownloadState>());
    }
}
