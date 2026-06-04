#include "ui/states/DownloadIndexState.h"
#include "ui/AppController.h"
#include "ui/UiStrings.h"
#include "download/DatabaseService.h"
#include "ui/SettingsService.h"
#include "ui/FolderDialog.h"
#include "ui/states/SelectDownloadState.h"
#include "ui/states/DownloadingIndexState.h"
#include "imgui.h"
#include <cstring>
#include <memory>

constexpr size_t UrlBufferCapacity = 1024;

DownloadIndexState::DownloadIndexState()
    : sourceKind(SourceKind::Official),
      urlBuffer(DatabaseService::GetOfficialIndexUrl())
{
}

void DownloadIndexState::ApplySourceKind(SourceKind kind)
{
    sourceKind = kind;
    switch (kind)
    {
    case SourceKind::Official:
        urlBuffer = DatabaseService::GetOfficialIndexUrl();
        return;
    case SourceKind::Custom:
        urlBuffer.clear();
        return;
    }
}

void DownloadIndexState::RenderSourceSelector(AppController* controller)
{
    ImGui::TextColored(controller->GetTheme().orangeActive, "%s", UiStrings::PrebuiltIndex::SourceSection);
    ImGui::Spacing();

    bool isOfficial = (sourceKind == SourceKind::Official);
    bool isCustom = (sourceKind == SourceKind::Custom);

    if (ImGui::RadioButton(UiStrings::PrebuiltIndex::OfficialOption, isOfficial))
    {
        ApplySourceKind(SourceKind::Official);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton(UiStrings::PrebuiltIndex::CustomOption, isCustom))
    {
        ApplySourceKind(SourceKind::Custom);
    }
    ImGui::Spacing();

    switch (sourceKind)
    {
    case SourceKind::Official:
        ImGui::Text("%s", UiStrings::PrebuiltIndex::UrlLabel);
        ImGui::TextColored(controller->GetTheme().textNormal, "%s", urlBuffer.c_str());
        return;
    case SourceKind::Custom:
        char editBuffer[UrlBufferCapacity];
        std::strncpy(editBuffer, urlBuffer.c_str(), UrlBufferCapacity - 1);
        editBuffer[UrlBufferCapacity - 1] = '\0';
        if (ImGui::InputText(UiStrings::PrebuiltIndex::UrlInputLabel, editBuffer, UrlBufferCapacity))
        {
            urlBuffer = editBuffer;
        }
        return;
    }
}

void DownloadIndexState::RenderDownloadControls(AppController* controller)
{
    ImGui::Spacing();
    ImGui::Text("%s", UiStrings::Common::TargetDirectory);
    ImGui::TextColored(controller->GetTheme().textNormal, "%s", SettingsService::GetInstance().GetSearchDir().c_str());

    float availWidth = ImGui::GetContentRegionAvail().x;
    if (ImGui::Button(UiStrings::Common::BrowseTargetDir, ImVec2(availWidth, controller->GetTheme().buttonHeightSmall)))
    {
        std::string path = SelectFolderDialog();
        if (!path.empty())
        {
            SettingsService::GetInstance().SetSearchDir(path);
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(controller->GetTheme().orangePrimary, "%s", UiStrings::PrebuiltIndex::WarningHeader);
    ImGui::TextWrapped("%s", UiStrings::PrebuiltIndex::WarningBody);
    ImGui::Spacing();

    bool urlEmpty = urlBuffer.empty();
    bool busy = DatabaseService::GetInstance().IsBusy();
    bool disabled = urlEmpty || busy;

    if (disabled)
    {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button(UiStrings::PrebuiltIndex::StartDownload, ImVec2(controller->GetButtonWidthMedium(), controller->GetButtonHeightMedium())))
    {
        DatabaseService::GetInstance().StartPrebuiltIndexDownload(urlBuffer);
        controller->TransitionTo(std::make_unique<DownloadingIndexState>());
    }
    if (disabled)
    {
        ImGui::EndDisabled();
    }
}

void DownloadIndexState::Render(AppController* controller)
{
    ImGui::Spacing();
    ImGui::TextColored(controller->GetTheme().orangePrimary, "%s", UiStrings::PrebuiltIndex::Title);
    ImGui::Separator();
    ImGui::Spacing();

    RenderSourceSelector(controller);
    RenderDownloadControls(controller);

    ImGui::Spacing();
    ImGui::Separator();
    if (ImGui::Button(UiStrings::Common::Back, ImVec2(controller->GetTheme().buttonWidthSmall, controller->GetTheme().buttonHeightSmall)))
    {
        if (DatabaseService::GetInstance().IsBusy())
        {
            DatabaseService::GetInstance().CancelDownload();
        }
        controller->TransitionTo(std::make_unique<SelectDownloadState>());
    }
}
