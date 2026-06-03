#include "ui/states/MainAppState.h"
#include "ui/AppController.h"
#include "imgui.h"

void MainAppState::Render(AppController* controller)
{
    ImGui::TextColored(controller->GetTheme().orangePrimary, "DATABASE READY - MAIN APP DASHBOARD");
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("Active database: %s", controller->GetCurrentFilePath().c_str());
    ImGui::Spacing();
    ImGui::TextColored(controller->GetTheme().textSuccess, "All tools are loaded successfully and Commander's console is online.");
}
