#include "ui/states/SchemaUpdateState.h"
#include "ui/AppController.h"
#include "ui/UiStrings.h"
#include "download/DatabaseService.h"
#include "imgui.h"

void SchemaUpdateState::Render(AppController* controller)
{
    ImGui::Spacing();
    ImGui::TextColored(controller->GetTheme().orangePrimary, "%s", UiStrings::SchemaUpdate::Title);
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped("%s", UiStrings::SchemaUpdate::Notice);
    ImGui::Spacing();

    auto& db = DatabaseService::GetInstance();
    ImGui::ProgressBar(db.GetSchemaProgress(), ImVec2(-1.0f, controller->GetTheme().progressBarHeight));
}
