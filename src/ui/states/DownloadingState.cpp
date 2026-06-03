#include "ui/states/DownloadingState.h"
#include "ui/AppController.h"
#include "imgui.h"
#include <cstdio>

void DownloadingState::Render(AppController* controller)
{
    ImGui::TextColored(controller->GetTheme().orangePrimary, "DOWNLOADING SPANSH DATABASE");
    ImGui::Separator();
    ImGui::Spacing();

    double sizeMb = controller->GetTotalFileSize() / (1024.0 * 1024.0);
    ImGui::Text("Estimated File Size: %.2f MB", sizeMb);
    ImGui::Spacing();

    float progress = controller->GetDownloadProgress();
    char progressLabel[64];
    sprintf(progressLabel, "%.1f%%", progress * 100.0f);
    ImGui::ProgressBar(progress, ImVec2(-1.0f, 30.0f), progressLabel);
    ImGui::Spacing();

    double speedMb = controller->GetDownloadSpeed() / (1024.0 * 1024.0);
    ImGui::Text("Download Speed: %.2f MB/s", speedMb);
    ImGui::Spacing();
    ImGui::Spacing();

    float availWidth = ImGui::GetContentRegionAvail().x;
    if (ImGui::Button("CANCEL DOWNLOAD [X]", ImVec2(availWidth, controller->GetButtonHeightMedium())))
    {
        controller->CancelDownload();
    }
}
