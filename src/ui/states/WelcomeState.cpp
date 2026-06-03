#include "ui/states/WelcomeState.h"
#include "ui/AppController.h"
#include "ui/states/SelectDownloadState.h"
#include "ui/states/MainAppState.h"
#include "imgui.h"

void WelcomeState::Render(AppController* controller)
{
    ImGui::Spacing();
    ImGui::TextColored(controller->GetTheme().orangePrimary, ">> COLONY FINDER - ELITE DANGEROUS DATABASE");
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Welcome, Commander.");
    ImGui::Spacing();

    if (!controller->GetCurrentFilePath().empty())
    {
        ImGui::TextColored(controller->GetTheme().textSuccess, "[+] Spansh dump file detected: %s", controller->GetCurrentFilePath().c_str());
        ImGui::Spacing();
        ImGui::Spacing();
        if (ImGui::Button("ENTER APPLICATION >", ImVec2(controller->GetButtonWidthLarge(), controller->GetButtonHeightLarge())))
        {
            controller->TransitionTo(std::make_unique<MainAppState>());
        }
        return;
    }

    ImGui::TextColored(controller->GetTheme().textAlert, "[!] No Spansh database file detected in default search path.");
    ImGui::Spacing();
    ImGui::Spacing();
    if (ImGui::Button("CONFIGURE & DOWNLOAD >", ImVec2(controller->GetButtonWidthLarge(), controller->GetButtonHeightLarge())))
    {
        controller->FetchOnlineSizes();
        controller->TransitionTo(std::make_unique<SelectDownloadState>());
    }
}
