#include "ui/states/ExtractionState.h"
#include "ui/AppController.h"
#include "ui/UiStrings.h"
#include "download/DatabaseService.h"
#include "ui/states/SelectDownloadState.h"
#include "imgui.h"

void ExtractionState::Render(AppController* controller)
{
    ImGui::Spacing();
    ImGui::TextColored(controller->GetTheme().orangePrimary, "%s", UiStrings::Extraction::Title);
    ImGui::Separator();
    ImGui::Spacing();

    auto& db = DatabaseService::GetInstance();

    ImGui::Text("%s", UiStrings::Extraction::Notice);
    ImGui::ProgressBar(db.GetExtractionProgress(), ImVec2(-1.0f, controller->GetTheme().progressBarHeight));

    double remaining = db.GetExtractionTimeRemaining();
    if (remaining >= 0.0)
    {
        int minutes = static_cast<int>(remaining) / 60;
        int seconds = static_cast<int>(remaining) % 60;
        if (minutes > 0)
        {
            ImGui::Text(UiStrings::Extraction::EtaMinSecFmt, minutes, seconds);
        }
        else
        {
            ImGui::Text(UiStrings::Extraction::EtaSecFmt, seconds);
        }
    }
    else
    {
        ImGui::Text("%s", UiStrings::Extraction::EtaCalculating);
    }

    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::Button(UiStrings::Common::Cancel, ImVec2(controller->GetButtonWidthMedium(), controller->GetButtonHeightMedium())))
    {
        db.CancelExtraction();
        db.CancelIndexing();
        controller->TransitionTo(std::make_unique<SelectDownloadState>());
    }
}
