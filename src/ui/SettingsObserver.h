#pragma once
#include <string>

struct ThemeColors;

class ISettingsObserver
{
public:
    virtual ~ISettingsObserver() = default;
    virtual void OnSettingsChanged(const std::string& downloadDir, const std::string& searchDir) = 0;
    virtual void OnThemeChanged(const ThemeColors& colors) { (void)colors; }
};
