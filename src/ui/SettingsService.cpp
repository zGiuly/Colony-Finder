#include "ui/SettingsService.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <utility>

namespace
{
    nlohmann::json ColorToJson(const ThemeColor& c)
    {
        return nlohmann::json::array({ c.r, c.g, c.b, c.a });
    }

    bool ReadColor(const nlohmann::json& j, const char* key, ThemeColor& out)
    {
        if (!j.contains(key)) return false;
        const auto& arr = j[key];
        if (!arr.is_array() || arr.size() != 4) return false;
        out = ThemeColor{ arr[0].get<float>(), arr[1].get<float>(), arr[2].get<float>(), arr[3].get<float>() };
        return true;
    }
}

SettingsService::SettingsService()
    : downloadDir("."),
      searchDir("."),
      bufferSizeMb(16)
{
}

SettingsService& SettingsService::GetInstance()
{
    static SettingsService instance;
    return instance;
}

void SettingsService::Load()
{
    std::filesystem::path configPath = "config.json";
    if (!std::filesystem::exists(configPath))
    {
        NotifyThemeObservers();
        return;
    }

    std::ifstream f(configPath);
    if (!f.is_open())
    {
        NotifyThemeObservers();
        return;
    }

    try
    {
        nlohmann::json j;
        f >> j;
        if (j.contains("download_dir"))
        {
            downloadDir = j["download_dir"].get<std::string>();
        }
        if (j.contains("search_dir"))
        {
            searchDir = j["search_dir"].get<std::string>();
        }
        if (j.contains("buffer_size_mb"))
        {
            bufferSizeMb = j["buffer_size_mb"].get<int>();
        }
        if (j.contains("theme") && j["theme"].is_object())
        {
            const auto& t = j["theme"];
            ReadColor(t, "orange_primary", colors.orangePrimary);
            ReadColor(t, "orange_muted", colors.orangeMuted);
            ReadColor(t, "orange_active", colors.orangeActive);
            ReadColor(t, "bg_dark", colors.bgDark);
            ReadColor(t, "bg_panel", colors.bgPanel);
            ReadColor(t, "text_normal", colors.textNormal);
            ReadColor(t, "text_muted", colors.textMuted);
            ReadColor(t, "text_alert", colors.textAlert);
            ReadColor(t, "text_success", colors.textSuccess);
            ReadColor(t, "border", colors.border);
            ReadColor(t, "row_hover", colors.rowHover);
            ReadColor(t, "row_hover_active", colors.rowHoverActive);
            ReadColor(t, "row_selected", colors.rowSelected);
        }
        NotifyObservers();
        NotifyThemeObservers();
    }
    catch (...)
    {
    }
}

void SettingsService::Save()
{
    std::ofstream f("config.json");
    if (!f.is_open())
    {
        return;
    }

    try
    {
        nlohmann::json j;
        j["download_dir"] = downloadDir;
        j["search_dir"] = searchDir;
        j["buffer_size_mb"] = bufferSizeMb;
        nlohmann::json t;
        t["orange_primary"] = ColorToJson(colors.orangePrimary);
        t["orange_muted"] = ColorToJson(colors.orangeMuted);
        t["orange_active"] = ColorToJson(colors.orangeActive);
        t["bg_dark"] = ColorToJson(colors.bgDark);
        t["bg_panel"] = ColorToJson(colors.bgPanel);
        t["text_normal"] = ColorToJson(colors.textNormal);
        t["text_muted"] = ColorToJson(colors.textMuted);
        t["text_alert"] = ColorToJson(colors.textAlert);
        t["text_success"] = ColorToJson(colors.textSuccess);
        t["border"] = ColorToJson(colors.border);
        t["row_hover"] = ColorToJson(colors.rowHover);
        t["row_hover_active"] = ColorToJson(colors.rowHoverActive);
        t["row_selected"] = ColorToJson(colors.rowSelected);
        j["theme"] = std::move(t);
        f << j.dump(4);
    }
    catch (...)
    {
    }
}

void SettingsService::SetDownloadDir(const std::string& path)
{
    if (downloadDir == path) return;
    downloadDir = path;
    Save();
    NotifyObservers();
}

void SettingsService::SetSearchDir(const std::string& path)
{
    if (searchDir == path) return;
    searchDir = path;
    Save();
    NotifyObservers();
}

void SettingsService::SetBufferSizeMb(int val)
{
    if (bufferSizeMb == val) return;
    bufferSizeMb = val;
    Save();
    NotifyObservers();
}

ThemeColor* SettingsService::ResolveColor(ThemeColorId id)
{
    switch (id)
    {
    case ThemeColorId::OrangePrimary:   return &colors.orangePrimary;
    case ThemeColorId::OrangeMuted:     return &colors.orangeMuted;
    case ThemeColorId::OrangeActive:    return &colors.orangeActive;
    case ThemeColorId::BgDark:          return &colors.bgDark;
    case ThemeColorId::BgPanel:         return &colors.bgPanel;
    case ThemeColorId::TextNormal:      return &colors.textNormal;
    case ThemeColorId::TextMuted:       return &colors.textMuted;
    case ThemeColorId::TextAlert:       return &colors.textAlert;
    case ThemeColorId::TextSuccess:     return &colors.textSuccess;
    case ThemeColorId::Border:          return &colors.border;
    case ThemeColorId::RowHover:        return &colors.rowHover;
    case ThemeColorId::RowHoverActive:  return &colors.rowHoverActive;
    case ThemeColorId::RowSelected:     return &colors.rowSelected;
    }
    return nullptr;
}

void SettingsService::SetThemeColor(ThemeColorId id, const ThemeColor& color)
{
    ThemeColor* target = ResolveColor(id);
    if (!target) return;
    if (target->r == color.r && target->g == color.g && target->b == color.b && target->a == color.a) return;
    *target = color;
    Save();
    NotifyThemeObservers();
}

void SettingsService::ResetThemeToDefaults()
{
    colors = ThemeColors{};
    Save();
    NotifyThemeObservers();
}

void SettingsService::AddObserver(ISettingsObserver* observer)
{
    if (!observer) return;
    auto it = std::find(observers.begin(), observers.end(), observer);
    if (it != observers.end()) return;
    observers.push_back(observer);
}

void SettingsService::RemoveObserver(ISettingsObserver* observer)
{
    auto it = std::find(observers.begin(), observers.end(), observer);
    if (it == observers.end()) return;
    observers.erase(it);
}

void SettingsService::NotifyObservers()
{
    for (auto* observer : observers)
    {
        if (observer)
        {
            observer->OnSettingsChanged(downloadDir, searchDir);
        }
    }
}

void SettingsService::NotifyThemeObservers()
{
    for (auto* observer : observers)
    {
        if (observer)
        {
            observer->OnThemeChanged(colors);
        }
    }
}
