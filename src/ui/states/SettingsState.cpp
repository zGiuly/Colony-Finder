#include "ui/states/SettingsState.h"
#include "ui/AppController.h"
#include "ui/UiStrings.h"
#include "ui/SettingsService.h"
#include "ui/states/WelcomeState.h"
#include "imgui.h"
#include <memory>
#include <utility>

namespace
{
    struct ColorEntry
    {
        const char* label;
        SettingsService::ThemeColorId id;
        const ThemeColor* value;
    };
}

SettingsState::SettingsState()
    : factory([]() -> std::unique_ptr<IAppState> { return std::make_unique<WelcomeState>(); })
{
}

SettingsState::SettingsState(ReturnFactory factoryVal)
    : factory(std::move(factoryVal))
{
}

void SettingsState::Render(AppController* controller)
{
    ImGui::Spacing();
    ImGui::TextColored(controller->GetTheme().orangePrimary, "%s", UiStrings::SettingsScreen::Title);
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(controller->GetTheme().orangeActive, "%s", UiStrings::SettingsScreen::ThemeSection);
    ImGui::TextWrapped("%s", UiStrings::SettingsScreen::ThemeNotice);
    ImGui::Spacing();

    const ThemeColors& c = SettingsService::GetInstance().GetThemeColors();
    const ColorEntry entries[] = {
        { UiStrings::SettingsScreen::ColorOrangePrimary,   SettingsService::ThemeColorId::OrangePrimary,   &c.orangePrimary },
        { UiStrings::SettingsScreen::ColorOrangeMuted,     SettingsService::ThemeColorId::OrangeMuted,     &c.orangeMuted },
        { UiStrings::SettingsScreen::ColorOrangeActive,    SettingsService::ThemeColorId::OrangeActive,    &c.orangeActive },
        { UiStrings::SettingsScreen::ColorBgDark,          SettingsService::ThemeColorId::BgDark,          &c.bgDark },
        { UiStrings::SettingsScreen::ColorBgPanel,         SettingsService::ThemeColorId::BgPanel,         &c.bgPanel },
        { UiStrings::SettingsScreen::ColorTextNormal,      SettingsService::ThemeColorId::TextNormal,      &c.textNormal },
        { UiStrings::SettingsScreen::ColorTextMuted,       SettingsService::ThemeColorId::TextMuted,       &c.textMuted },
        { UiStrings::SettingsScreen::ColorTextAlert,       SettingsService::ThemeColorId::TextAlert,       &c.textAlert },
        { UiStrings::SettingsScreen::ColorTextSuccess,     SettingsService::ThemeColorId::TextSuccess,     &c.textSuccess },
        { UiStrings::SettingsScreen::ColorBorder,          SettingsService::ThemeColorId::Border,          &c.border },
        { UiStrings::SettingsScreen::ColorRowHover,        SettingsService::ThemeColorId::RowHover,        &c.rowHover },
        { UiStrings::SettingsScreen::ColorRowHoverActive,  SettingsService::ThemeColorId::RowHoverActive,  &c.rowHoverActive },
        { UiStrings::SettingsScreen::ColorRowSelected,     SettingsService::ThemeColorId::RowSelected,     &c.rowSelected },
    };
    for (const auto& entry : entries)
    {
        float buffer[4] = { entry.value->r, entry.value->g, entry.value->b, entry.value->a };
        if (ImGui::ColorEdit4(entry.label, buffer, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar))
        {
            SettingsService::GetInstance().SetThemeColor(entry.id, ThemeColor{ buffer[0], buffer[1], buffer[2], buffer[3] });
        }
    }
    ImGui::Spacing();
    if (ImGui::Button(UiStrings::SettingsScreen::ResetTheme, ImVec2(controller->GetButtonWidthMedium(), controller->GetTheme().buttonHeightSmall)))
    {
        SettingsService::GetInstance().ResetThemeToDefaults();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    if (!ImGui::Button(UiStrings::Common::Back, ImVec2(controller->GetTheme().buttonWidthSmall, controller->GetTheme().buttonHeightSmall)))
    {
        return;
    }
    if (!factory)
    {
        return;
    }
    controller->TransitionTo(factory());
}
