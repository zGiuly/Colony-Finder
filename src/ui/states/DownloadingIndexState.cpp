#include "ui/states/DownloadingIndexState.h"
#include "ui/AppController.h"
#include "ui/UiStrings.h"
#include "download/DatabaseService.h"
#include "ui/states/DownloadIndexState.h"
#include "imgui.h"
#include <cstdio>
#include <memory>

constexpr double BytesInMb = 1048576.0;

void DownloadingIndexState::Render(AppController* controller)
{
    ImGui::Spacing();
    ImGui::TextColored(controller->GetTheme().orangePrimary, "%s", UiStrings::PrebuiltDownload::Title);
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped("%s", UiStrings::PrebuiltDownload::Notice);
    ImGui::Spacing();

    auto& db = DatabaseService::GetInstance();
    float progress = db.GetDownloadProgress();
    double speed = db.GetDownloadSpeed();
    double totalSize = db.GetTotalFileSize();

    ImGui::ProgressBar(progress, ImVec2(-1.0f, controller->GetTheme().progressBarHeight));
    ImGui::Spacing();

    double totalMb = totalSize / BytesInMb;
    double currentMb = progress * totalMb;
    double speedMb = speed / BytesInMb;

    char statusText[256];
    if (speedMb > 0.0)
    {
        double remainingSeconds = (totalMb - currentMb) / speedMb;
        int min = static_cast<int>(remainingSeconds) / 60;
        int sec = static_cast<int>(remainingSeconds) % 60;
        std::snprintf(statusText, sizeof(statusText), UiStrings::DownloadDump::StatusFmt, currentMb, totalMb, speedMb, min, sec);
    }
    else
    {
        std::snprintf(statusText, sizeof(statusText), UiStrings::DownloadDump::StatusCalcFmt, currentMb, totalMb);
    }
    ImGui::TextColored(controller->GetTheme().textNormal, "%s", statusText);

    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::Button(UiStrings::Common::CancelDownload, ImVec2(controller->GetButtonWidthMedium(), controller->GetButtonHeightMedium())))
    {
        db.CancelDownload();
        controller->TransitionTo(std::make_unique<DownloadIndexState>());
    }
}
