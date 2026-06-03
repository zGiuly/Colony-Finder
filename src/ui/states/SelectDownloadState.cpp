#include "ui/states/SelectDownloadState.h"
#include "ui/AppController.h"
#include "download/DatabaseService.h"
#include "ui/SettingsService.h"
#include "ui/states/WelcomeState.h"
#include "ui/FolderDialog.h"
#include "ui/states/ErrorState.h"
#include "ui/states/DownloadingState.h"
#include "ui/SystemMemory.h"
#include "imgui.h"
#include <cstdio>
#include <memory>

constexpr double BytesInGb = 1073741824.0;

void SelectDownloadState::Render(AppController* controller)
{
    DatabaseService::GetInstance().FetchOnlineSizes();
    ImGui::TextColored(controller->GetTheme().orangePrimary, ":: SPANSH DATABASE SETUP");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Columns(2, nullptr, false);

    ImGui::TextColored(controller->GetTheme().orangeActive, "[Download] Online Downloader");
    ImGui::Spacing();
    ImGui::TextWrapped("Download the data dump directly from Spansh servers.");
    ImGui::Spacing();

    ImGui::Text("Target Directory:");
    ImGui::TextColored(controller->GetTheme().textNormal, "%s", SettingsService::GetInstance().GetDownloadDir().c_str());

    float availWidthCol1 = ImGui::GetContentRegionAvail().x;
    if (ImGui::Button("Browse Target Dir...", ImVec2(availWidthCol1, controller->GetTheme().buttonHeightSmall)))
    {
        std::string path = SelectFolderDialog();
        if (!path.empty())
        {
            SettingsService::GetInstance().SetDownloadDir(path);
        }
    }
    ImGui::Spacing();

    double size1MonthVal = DatabaseService::GetInstance().GetOnlineSize1Month();
    char btnText1Month[128];
    if (size1MonthVal == -1.0)
    {
        std::snprintf(btnText1Month, sizeof(btnText1Month), "Download Galaxy 1 Month (Querying...)");
    }
    else if (size1MonthVal == -2.0)
    {
        std::snprintf(btnText1Month, sizeof(btnText1Month), "Download Galaxy 1 Month (~16.0 GB) (Offline)");
    }
    else
    {
        double sizeGb = size1MonthVal / BytesInGb;
        std::snprintf(btnText1Month, sizeof(btnText1Month), "Download Galaxy 1 Month (%.2f GB)", sizeGb);
    }

    if (ImGui::Button(btnText1Month, ImVec2(availWidthCol1, controller->GetButtonHeightMedium())))
    {
        DatabaseService::GetInstance().StartDownload("https://downloads.spansh.co.uk/galaxy_1month.json.gz");
        controller->TransitionTo(std::make_unique<DownloadingState>());
    }
    ImGui::Spacing();

    double sizeFullVal = DatabaseService::GetInstance().GetOnlineSizeFull();
    char btnTextFull[128];
    if (sizeFullVal == -1.0)
    {
        std::snprintf(btnTextFull, sizeof(btnTextFull), "Download Full Galaxy (Querying...)");
    }
    else if (sizeFullVal == -2.0)
    {
        std::snprintf(btnTextFull, sizeof(btnTextFull), "Download Full Galaxy (~100.0 GB) (Offline)");
    }
    else
    {
        double sizeGb = sizeFullVal / BytesInGb;
        std::snprintf(btnTextFull, sizeof(btnTextFull), "Download Full Galaxy (%.2f GB)", sizeGb);
    }

    if (ImGui::Button(btnTextFull, ImVec2(availWidthCol1, controller->GetButtonHeightMedium())))
    {
        DatabaseService::GetInstance().StartDownload("https://downloads.spansh.co.uk/galaxy.json.gz");
        controller->TransitionTo(std::make_unique<DownloadingState>());
    }

    ImGui::NextColumn();

    ImGui::TextColored(controller->GetTheme().orangeActive, "[Local] Database Link");
    ImGui::Spacing();
    ImGui::TextWrapped("Search for a database file already stored in your storage.");
    ImGui::Spacing();

    ImGui::Text("Search Directory:");
    ImGui::TextColored(controller->GetTheme().textNormal, "%s", SettingsService::GetInstance().GetSearchDir().c_str());

    float availWidthCol2 = ImGui::GetContentRegionAvail().x;
    if (ImGui::Button("Browse Search Dir...", ImVec2(availWidthCol2, controller->GetTheme().buttonHeightSmall)))
    {
        std::string path = SelectFolderDialog();
        if (!path.empty())
        {
            SettingsService::GetInstance().SetSearchDir(path);
            DatabaseService::GetInstance().CheckLocalDump();
            if (!DatabaseService::GetInstance().GetCurrentFilePath().empty())
            {
                DatabaseService::GetInstance().EnterApplicationFlow();
            }
        }
    }
    ImGui::Spacing();

    const std::string& detected = DatabaseService::GetInstance().GetCurrentFilePath();
    if (!detected.empty())
    {
        ImGui::TextColored(controller->GetTheme().textSuccess, "[+] Detected: %s", detected.c_str());
        ImGui::Spacing();
        if (ImGui::Button("Use This Database", ImVec2(availWidthCol2, controller->GetButtonHeightMedium())))
        {
            DatabaseService::GetInstance().EnterApplicationFlow();
        }
    }
    else
    {
        ImGui::TextColored(controller->GetTheme().textMuted, "No Spansh dump detected in this directory.");
    }

    ImGui::Columns(1);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    uint64_t totalRam = SystemMemory::GetTotalRAM();
    uint64_t availRam = SystemMemory::GetAvailableRAM();
    double totalRamGb = static_cast<double>(totalRam) / (1024.0 * 1024.0 * 1024.0);
    double availRamGb = static_cast<double>(availRam) / (1024.0 * 1024.0 * 1024.0);

    int recommendedMb = static_cast<int>(availRam / (1024ULL * 1024ULL * 128ULL));
    if (recommendedMb < 8)
    {
        recommendedMb = 8;
    }
    if (recommendedMb > 64)
    {
        recommendedMb = 64;
    }

    int warningThresholdMb = static_cast<int>(availRam / (1024ULL * 1024ULL * 32ULL));
    if (warningThresholdMb < 16)
    {
        warningThresholdMb = 16;
    }

    int bufferSize = SettingsService::GetInstance().GetBufferSizeMb();

    ImGui::TextColored(controller->GetTheme().orangeActive, "[Settings] Decompression Memory Allocation");
    ImGui::Spacing();
    ImGui::Text("System RAM: %.1f GB Total, %.1f GB Available", totalRamGb, availRamGb);
    ImGui::Text("Recommended allocation: %d MB", recommendedMb);

    if (ImGui::SliderInt("Decompression Buffer Size (MB)", &bufferSize, 1, 64))
    {
        SettingsService::GetInstance().SetBufferSizeMb(bufferSize);
    }

    if (bufferSize > warningThresholdMb)
    {
        ImGui::TextColored(controller->GetTheme().orangePrimary, "Warning: High allocation may cause system slowdown or swapping on low memory.");
    }

    ImGui::Spacing();
    ImGui::Separator();

    if (ImGui::Button("< Back", ImVec2(controller->GetTheme().buttonWidthSmall, controller->GetTheme().buttonHeightSmall)))
    {
        controller->TransitionTo(std::make_unique<WelcomeState>());
    }
    ImGui::SameLine();
    if (ImGui::Button("Update Schema", ImVec2(controller->GetTheme().buttonWidthSmall * 1.4f, controller->GetTheme().buttonHeightSmall)))
    {
        DatabaseService::GetInstance().StartSchemaUpdate();
    }
}
