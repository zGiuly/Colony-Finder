#include "ui/states/DownloadingState.h"
#include "ui/AppController.h"
#include "imgui.h"
#include <cstdio>

void DownloadingState::Render(AppController* controller)
{
    ImGui::TextColored(controller->GetTheme().orangePrimary, "DOWNLOADING SPANSH DATABASE");
    ImGui::Separator();
    ImGui::Spacing();

    double sizeGb = controller->GetTotalFileSize() / (1024.0 * 1024.0 * 1024.0);
    ImGui::Text("Estimated File Size: %.2f GB", sizeGb);
    ImGui::Spacing();

    float progress = controller->GetDownloadProgress();
    char progressLabel[64];
    std::snprintf(progressLabel, sizeof(progressLabel), "%.1f%%", progress * 100.0f);
    ImGui::ProgressBar(progress, ImVec2(-1.0f, 30.0f), progressLabel);
    ImGui::Spacing();

    double speedBytes = controller->GetDownloadSpeed();
    double speedMb = speedBytes / (1024.0 * 1024.0);
    ImGui::Text("Download Speed: %.2f MB/s", speedMb);
    ImGui::Spacing();

    double totalBytes = controller->GetTotalFileSize();
    double remainingBytes = totalBytes * (1.0f - progress);
    char timeLabel[128];
    if (speedBytes <= 0.0)
    {
        std::snprintf(timeLabel, sizeof(timeLabel), "Estimated Time Remaining: Calculating...");
    }
    else
    {
        double remainingSeconds = remainingBytes / speedBytes;
        int hours = static_cast<int>(remainingSeconds) / 3600;
        int minutes = (static_cast<int>(remainingSeconds) % 3600) / 60;
        int seconds = static_cast<int>(remainingSeconds) % 60;
        if (hours > 0)
        {
            std::snprintf(timeLabel, sizeof(timeLabel), "Estimated Time Remaining: %dh %dm %ds", hours, minutes, seconds);
        }
        else if (minutes > 0)
        {
            std::snprintf(timeLabel, sizeof(timeLabel), "Estimated Time Remaining: %dm %ds", minutes, seconds);
        }
        else
        {
            std::snprintf(timeLabel, sizeof(timeLabel), "Estimated Time Remaining: %ds", seconds);
        }
    }
    ImGui::Text("%s", timeLabel);
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::TextColored(controller->GetTheme().orangeActive, "Please do not close the application during the download.");
    ImGui::TextWrapped("Depending on your connection speed, this process may take some time.");
    ImGui::Spacing();
    ImGui::Spacing();

    float availWidth = ImGui::GetContentRegionAvail().x;
    if (ImGui::Button("CANCEL DOWNLOAD [X]", ImVec2(availWidth, controller->GetButtonHeightMedium())))
    {
        controller->CancelDownload();
    }
}
