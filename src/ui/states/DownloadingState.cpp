#include "ui/states/DownloadingState.h"
#include "ui/AppController.h"
#include "download/DatabaseService.h"
#include "ui/states/SelectDownloadState.h"
#include "imgui.h"
#include <cstdio>

constexpr double BytesInMb = 1048576.0;

void DownloadingState::Render(AppController* controller)
{
    ImGui::Spacing();
    ImGui::TextColored(controller->GetTheme().orangePrimary, ":: DOWNLOADING DATABASE");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped("The database is currently downloading in the background. Do NOT close the application during this process, otherwise the downloaded files might get corrupted.");
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
        std::snprintf(statusText, sizeof(statusText), "Progress: %.1f MB / %.1f MB | Speed: %.2f MB/s | Est. Time Left: %dm %ds", currentMb, totalMb, speedMb, min, sec);
    }
    else
    {
        std::snprintf(statusText, sizeof(statusText), "Progress: %.1f MB / %.1f MB | Speed: 0.00 MB/s | Est. Time Left: Calculating...", currentMb, totalMb);
    }
    ImGui::TextColored(controller->GetTheme().textNormal, "%s", statusText);

    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::Button("Cancel Download", ImVec2(controller->GetButtonWidthMedium(), controller->GetButtonHeightMedium())))
    {
        db.CancelDownload();
        controller->TransitionTo(std::make_unique<SelectDownloadState>());
    }
}
