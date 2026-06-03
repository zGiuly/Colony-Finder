#include "ui/states/SchemaUpdateState.h"
#include "ui/AppController.h"
#include "download/DatabaseService.h"
#include "imgui.h"

void SchemaUpdateState::Render(AppController* controller)
{
    ImGui::Spacing();
    ImGui::TextColored(controller->GetTheme().orangePrimary, ":: UPDATING DATABASE SCHEMA");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped("Downloading Spansh schema file...");
    ImGui::Spacing();

    auto& db = DatabaseService::GetInstance();
    ImGui::ProgressBar(db.GetSchemaProgress(), ImVec2(-1.0f, 30.0f));
}
