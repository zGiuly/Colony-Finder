#include "ui/states/SelectDownloadState.h"
#include "ui/AppController.h"
#include "ui/states/WelcomeState.h"
#include "ui/FolderDialog.h"
#include "ui/states/MainAppState.h"
#include "ui/states/ErrorState.h"
#include "imgui.h"
#include <cstdio>
#include <filesystem>

void SelectDownloadState::Render(AppController* controller)
{
    ImGui::TextColored(controller->GetTheme().orangePrimary, ":: SPANSH DATABASE SETUP");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Columns(2, nullptr, false);

    ImGui::TextColored(controller->GetTheme().orangeActive, "[Download] Online Downloader");
    ImGui::Spacing();
    ImGui::TextWrapped("Download the data dump directly from Spansh servers.");
    ImGui::Spacing();

    ImGui::Text("Target Directory:");
    ImGui::TextColored(controller->GetTheme().textNormal, "%s", controller->GetDownloadDir().c_str());
    
    float availWidthCol1 = ImGui::GetContentRegionAvail().x;
    if (ImGui::Button("Browse Target Dir...", ImVec2(availWidthCol1, 30.0f)))
    {
        std::string path = SelectFolderDialog();
        if (!path.empty())
        {
            controller->SetDownloadDir(path);
        }
    }
    ImGui::Spacing();

    double size1MonthVal = controller->GetOnlineSize1Month();
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
        double sizeGb = size1MonthVal / (1024.0 * 1024.0 * 1024.0);
        std::snprintf(btnText1Month, sizeof(btnText1Month), "Download Galaxy 1 Month (%.2f GB)", sizeGb);
    }

    if (ImGui::Button(btnText1Month, ImVec2(availWidthCol1, controller->GetButtonHeightMedium())))
    {
        controller->StartDownload("https://downloads.spansh.co.uk/galaxy_1month.json.gz");
    }
    ImGui::Spacing();

    double sizeFullVal = controller->GetOnlineSizeFull();
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
        double sizeGb = sizeFullVal / (1024.0 * 1024.0 * 1024.0);
        std::snprintf(btnTextFull, sizeof(btnTextFull), "Download Full Galaxy (%.2f GB)", sizeGb);
    }

    if (ImGui::Button(btnTextFull, ImVec2(availWidthCol1, controller->GetButtonHeightMedium())))
    {
        controller->StartDownload("https://downloads.spansh.co.uk/galaxy.json.gz");
    }

    ImGui::NextColumn();

    ImGui::TextColored(controller->GetTheme().orangeActive, "[Local] Database Link");
    ImGui::Spacing();
    ImGui::TextWrapped("Search for a database file already stored in your storage.");
    ImGui::Spacing();

    ImGui::Text("Search Directory:");
    ImGui::TextColored(controller->GetTheme().textNormal, "%s", controller->GetSearchDir().c_str());
    
    float availWidthCol2 = ImGui::GetContentRegionAvail().x;
    if (ImGui::Button("Browse Search Dir...", ImVec2(availWidthCol2, 30.0f)))
    {
        std::string path = SelectFolderDialog();
        if (!path.empty())
        {
            controller->SetSearchDir(path);
        }
    }
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::Button("Scan & Verify Path", ImVec2(availWidthCol2, controller->GetButtonHeightMedium())))
    {
        controller->CheckLocalDump();
        if (controller->GetCurrentFilePath().empty())
        {
            controller->SetErrorMessage("No spansh database file (galaxy.json, galaxy_1month.json, galaxy.json.gz or galaxy_1month.json.gz) found in selected directory.");
            controller->TransitionTo(std::make_unique<ErrorState>());
        }
        else
        {
            std::filesystem::path path(controller->GetCurrentFilePath());
            if (path.extension() == ".gz")
            {
                controller->StartExtractionAndValidation();
            }
            else
            {
                controller->TransitionTo(std::make_unique<MainAppState>());
            }
        }
    }

    ImGui::Columns(1);
    ImGui::Spacing();
    ImGui::Separator();
    
    if (ImGui::Button("< Back", ImVec2(100.0f, 30.0f)))
    {
        controller->TransitionTo(std::make_unique<WelcomeState>());
    }
    ImGui::SameLine();
    if (ImGui::Button("Update Schema", ImVec2(140.0f, 30.0f)))
    {
        controller->StartSchemaUpdate();
    }
}
