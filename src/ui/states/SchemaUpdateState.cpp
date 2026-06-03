#include "ui/states/SchemaUpdateState.h"
#include "ui/AppController.h"
#include "imgui.h"

void SchemaUpdateState::Render(AppController* controller)
{
    ImGui::TextColored(controller->GetTheme().orangePrimary, ":: SCHEMA UPDATE");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Downloading and updating the JSON schema...");
    ImGui::Spacing();

    float progress = controller->GetSchemaProgress();
    ImGui::ProgressBar(progress, ImVec2(-1.0f, 25.0f));
}
