#include "ui/states/ExtractionState.h"
#include "ui/AppController.h"
#include "download/DatabaseService.h"
#include "ui/states/SelectDownloadState.h"
#include "imgui.h"

void ExtractionState::Render(AppController* controller)
{
    ImGui::Spacing();
    ImGui::TextColored(controller->GetTheme().orangePrimary, ":: PROCESS DATABASE FILES");
    ImGui::Separator();
    ImGui::Spacing();

    auto& db = DatabaseService::GetInstance();

    ImGui::Text("Decompressing & indexing Spansh dump (streaming)...");
    ImGui::ProgressBar(db.GetExtractionProgress(), ImVec2(-1.0f, controller->GetTheme().progressBarHeight));

    double remaining = db.GetExtractionTimeRemaining();
    if (remaining >= 0.0)
    {
        int minutes = static_cast<int>(remaining) / 60;
        int seconds = static_cast<int>(remaining) % 60;
        if (minutes > 0)
        {
            ImGui::Text("Estimated time remaining: %dm %ds", minutes, seconds);
        }
        else
        {
            ImGui::Text("Estimated time remaining: %ds", seconds);
        }
    }
    else
    {
        ImGui::Text("Estimated time remaining: Calculating...");
    }

    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::Button("Cancel", ImVec2(controller->GetButtonWidthMedium(), controller->GetButtonHeightMedium())))
    {
        db.CancelExtraction();
        db.CancelIndexing();
        controller->TransitionTo(std::make_unique<SelectDownloadState>());
    }
}
