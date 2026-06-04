#include "ui/states/SelectDownloadState.h"
#include "ui/AppController.h"
#include "ui/UiStrings.h"
#include "download/DatabaseService.h"
#include "ui/SettingsService.h"
#include "ui/states/WelcomeState.h"
#include "ui/FolderDialog.h"
#include "ui/states/ErrorState.h"
#include "ui/states/DownloadingState.h"
#include "ui/states/DownloadIndexState.h"
#include "ui/SystemMemory.h"
#include "imgui.h"
#include <cstdio>
#include <memory>

constexpr double BytesInGb = 1073741824.0;

void SelectDownloadState::Render(AppController* controller)
{
    DatabaseService::GetInstance().FetchOnlineSizes();
    ImGui::TextColored(controller->GetTheme().orangePrimary, "%s", UiStrings::Setup::Title);
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Columns(2, nullptr, false);

    ImGui::TextColored(controller->GetTheme().orangeActive, "%s", UiStrings::Setup::OnlineSection);
    ImGui::Spacing();
    ImGui::TextWrapped("%s", UiStrings::Setup::OnlineDescription);
    ImGui::Spacing();

    ImGui::Text("%s", UiStrings::Common::TargetDirectory);
    ImGui::TextColored(controller->GetTheme().textNormal, "%s", SettingsService::GetInstance().GetDownloadDir().c_str());

    float availWidthCol1 = ImGui::GetContentRegionAvail().x;
    if (ImGui::Button(UiStrings::Common::BrowseTargetDir, ImVec2(availWidthCol1, controller->GetTheme().buttonHeightSmall)))
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
        std::snprintf(btnText1Month, sizeof(btnText1Month), "%s", UiStrings::Setup::DownloadGalaxy1MonthBusy);
    }
    else if (size1MonthVal == -2.0)
    {
        std::snprintf(btnText1Month, sizeof(btnText1Month), "%s", UiStrings::Setup::DownloadGalaxy1MonthOff);
    }
    else
    {
        double sizeGb = size1MonthVal / BytesInGb;
        std::snprintf(btnText1Month, sizeof(btnText1Month), UiStrings::Setup::DownloadGalaxy1MonthFmt, sizeGb);
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
        std::snprintf(btnTextFull, sizeof(btnTextFull), "%s", UiStrings::Setup::DownloadGalaxyFullBusy);
    }
    else if (sizeFullVal == -2.0)
    {
        std::snprintf(btnTextFull, sizeof(btnTextFull), "%s", UiStrings::Setup::DownloadGalaxyFullOff);
    }
    else
    {
        double sizeGb = sizeFullVal / BytesInGb;
        std::snprintf(btnTextFull, sizeof(btnTextFull), UiStrings::Setup::DownloadGalaxyFullFmt, sizeGb);
    }

    if (ImGui::Button(btnTextFull, ImVec2(availWidthCol1, controller->GetButtonHeightMedium())))
    {
        DatabaseService::GetInstance().StartDownload("https://downloads.spansh.co.uk/galaxy.json.gz");
        controller->TransitionTo(std::make_unique<DownloadingState>());
    }

    ImGui::NextColumn();

    ImGui::TextColored(controller->GetTheme().orangeActive, "%s", UiStrings::Setup::LocalSection);
    ImGui::Spacing();
    ImGui::TextWrapped("%s", UiStrings::Setup::LocalDescription);
    ImGui::Spacing();

    ImGui::Text("%s", UiStrings::Common::SearchDirectory);
    ImGui::TextColored(controller->GetTheme().textNormal, "%s", SettingsService::GetInstance().GetSearchDir().c_str());

    float availWidthCol2 = ImGui::GetContentRegionAvail().x;
    if (ImGui::Button(UiStrings::Common::BrowseSearchDir, ImVec2(availWidthCol2, controller->GetTheme().buttonHeightSmall)))
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
        ImGui::TextColored(controller->GetTheme().textSuccess, UiStrings::Setup::DetectedFmt, detected.c_str());
        ImGui::Spacing();
        if (ImGui::Button(UiStrings::Setup::UseThisDatabase, ImVec2(availWidthCol2, controller->GetButtonHeightMedium())))
        {
            DatabaseService::GetInstance().EnterApplicationFlow();
        }
    }
    else
    {
        ImGui::TextColored(controller->GetTheme().textMuted, "%s", UiStrings::Setup::NoLocalDump);
    }

    ImGui::Columns(1);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(controller->GetTheme().orangeActive, "%s", UiStrings::Setup::PrebuiltSection);
    ImGui::TextWrapped("%s", UiStrings::Setup::PrebuiltDescription);
    ImGui::Spacing();
    if (ImGui::Button(UiStrings::Setup::PrebuiltOpenButton, ImVec2(controller->GetButtonWidthMedium(), controller->GetButtonHeightMedium())))
    {
        controller->TransitionTo(std::make_unique<DownloadIndexState>());
        return;
    }
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

    ImGui::TextColored(controller->GetTheme().orangeActive, "%s", UiStrings::Setup::MemorySection);
    ImGui::Spacing();
    ImGui::Text(UiStrings::Setup::SystemRamFmt, totalRamGb, availRamGb);
    ImGui::Text(UiStrings::Setup::RecommendedFmt, recommendedMb);

    if (ImGui::SliderInt(UiStrings::Setup::BufferSliderLabel, &bufferSize, 1, 64))
    {
        SettingsService::GetInstance().SetBufferSizeMb(bufferSize);
    }

    if (bufferSize > warningThresholdMb)
    {
        ImGui::TextColored(controller->GetTheme().orangePrimary, "%s", UiStrings::Setup::HighAllocWarning);
    }

    ImGui::Spacing();
    ImGui::Separator();

    if (ImGui::Button(UiStrings::Common::Back, ImVec2(controller->GetTheme().buttonWidthSmall, controller->GetTheme().buttonHeightSmall)))
    {
        controller->TransitionTo(std::make_unique<WelcomeState>());
    }
    ImGui::SameLine();
    if (ImGui::Button(UiStrings::Setup::UpdateSchema, ImVec2(controller->GetTheme().buttonWidthSmall * 1.4f, controller->GetTheme().buttonHeightSmall)))
    {
        DatabaseService::GetInstance().StartSchemaUpdate();
    }
}
