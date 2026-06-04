#pragma once
#include "ui/SettingsObserver.h"
#include <string>
#include <vector>

struct ThemeColor
{
    float r;
    float g;
    float b;
    float a;
};

struct ThemeColors
{
    ThemeColor orangePrimary   { 1.0f, 0.498f, 0.0f, 1.0f };
    ThemeColor orangeMuted     { 0.7f, 0.3f, 0.0f, 1.0f };
    ThemeColor orangeActive    { 1.0f, 0.65f, 0.0f, 1.0f };
    ThemeColor bgDark          { 0.04f, 0.04f, 0.05f, 0.95f };
    ThemeColor bgPanel         { 0.08f, 0.08f, 0.10f, 1.0f };
    ThemeColor textNormal      { 0.9f, 0.9f, 0.9f, 1.0f };
    ThemeColor textMuted       { 0.6f, 0.6f, 0.6f, 1.0f };
    ThemeColor textAlert       { 1.0f, 0.2f, 0.2f, 1.0f };
    ThemeColor textSuccess     { 0.2f, 1.0f, 0.2f, 1.0f };
    ThemeColor border          { 0.6f, 0.25f, 0.0f, 0.5f };
    ThemeColor rowHover        { 1.0f, 0.498f, 0.0f, 0.18f };
    ThemeColor rowHoverActive  { 1.0f, 0.498f, 0.0f, 0.22f };
    ThemeColor rowSelected     { 1.0f, 0.498f, 0.0f, 0.32f };
};

class SettingsService
{
public:
    enum class ThemeColorId
    {
        OrangePrimary,
        OrangeMuted,
        OrangeActive,
        BgDark,
        BgPanel,
        TextNormal,
        TextMuted,
        TextAlert,
        TextSuccess,
        Border,
        RowHover,
        RowHoverActive,
        RowSelected
    };

    static SettingsService& GetInstance();

    void Load();
    void Save();

    const std::string& GetDownloadDir() const { return downloadDir; }
    void SetDownloadDir(const std::string& path);

    const std::string& GetSearchDir() const { return searchDir; }
    void SetSearchDir(const std::string& path);

    int GetBufferSizeMb() const { return bufferSizeMb; }
    void SetBufferSizeMb(int val);

    const ThemeColors& GetThemeColors() const { return colors; }
    void SetThemeColor(ThemeColorId id, const ThemeColor& color);
    void ResetThemeToDefaults();

    void AddObserver(ISettingsObserver* observer);
    void RemoveObserver(ISettingsObserver* observer);

private:
    SettingsService();
    ~SettingsService() = default;

    SettingsService(const SettingsService&) = delete;
    SettingsService& operator=(const SettingsService&) = delete;

    void NotifyObservers();
    void NotifyThemeObservers();
    ThemeColor* ResolveColor(ThemeColorId id);

    std::string downloadDir;
    std::string searchDir;
    int bufferSizeMb;
    ThemeColors colors;
    std::vector<ISettingsObserver*> observers;
};
