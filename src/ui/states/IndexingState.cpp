#include "ui/states/IndexingState.h"
#include "ui/AppController.h"
#include "download/DatabaseService.h"
#include "ui/states/SelectDownloadState.h"
#include "imgui.h"

void IndexingState::Render(AppController* controller)
{
    ImGui::Spacing();
    ImGui::TextColored(controller->GetTheme().orangePrimary, ":: PRE-ANALYZING DATABASE");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped("Compiling database into the ultra-fast compact binary index 'galaxy.idx'. This might take a few minutes depending on your storage speed.");
    ImGui::Spacing();

    auto& db = DatabaseService::GetInstance();
    ImGui::ProgressBar(db.GetIndexingProgress(), ImVec2(-1.0f, 30.0f));
    ImGui::Spacing();

    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::Button("Cancel Indexing", ImVec2(controller->GetButtonWidthMedium(), controller->GetButtonHeightMedium())))
    {
        db.CancelIndexing();
        controller->TransitionTo(std::make_unique<SelectDownloadState>());
    }
}
