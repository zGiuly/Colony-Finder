#include "ui/states/SettingsState.h"
#include "ui/AppController.h"
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
    ImGui::TextColored(controller->GetTheme().orangePrimary, ":: APPLICATION SETTINGS");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextColored(controller->GetTheme().orangeActive, "[Theme] Color Customization");
    ImGui::TextWrapped("Changes are applied immediately and saved automatically.");
    ImGui::Spacing();

    const ThemeColors& c = SettingsService::GetInstance().GetThemeColors();
    const ColorEntry entries[] = {
        { "Orange Primary",     SettingsService::ThemeColorId::OrangePrimary,   &c.orangePrimary },
        { "Orange Muted",       SettingsService::ThemeColorId::OrangeMuted,     &c.orangeMuted },
        { "Orange Active",      SettingsService::ThemeColorId::OrangeActive,    &c.orangeActive },
        { "Background Dark",    SettingsService::ThemeColorId::BgDark,          &c.bgDark },
        { "Background Panel",   SettingsService::ThemeColorId::BgPanel,         &c.bgPanel },
        { "Text Normal",        SettingsService::ThemeColorId::TextNormal,      &c.textNormal },
        { "Text Muted",         SettingsService::ThemeColorId::TextMuted,       &c.textMuted },
        { "Text Alert",         SettingsService::ThemeColorId::TextAlert,       &c.textAlert },
        { "Text Success",       SettingsService::ThemeColorId::TextSuccess,     &c.textSuccess },
        { "Border",             SettingsService::ThemeColorId::Border,          &c.border },
        { "Row Hover",          SettingsService::ThemeColorId::RowHover,        &c.rowHover },
        { "Row Hover Active",   SettingsService::ThemeColorId::RowHoverActive,  &c.rowHoverActive },
        { "Row Selected",       SettingsService::ThemeColorId::RowSelected,     &c.rowSelected },
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
    if (ImGui::Button("Reset Theme to Defaults", ImVec2(controller->GetButtonWidthMedium(), controller->GetTheme().buttonHeightSmall)))
    {
        SettingsService::GetInstance().ResetThemeToDefaults();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    if (!ImGui::Button("< Back", ImVec2(controller->GetTheme().buttonWidthSmall, controller->GetTheme().buttonHeightSmall)))
    {
        return;
    }
    if (!factory)
    {
        return;
    }
    controller->TransitionTo(factory());
}
