#pragma once
#include "ui/SettingsObserver.h"
#include <string>
#include <vector>

class SettingsService
{
public:
    static SettingsService& GetInstance();

    void Load();
    void Save();

    const std::string& GetDownloadDir() const { return downloadDir; }
    void SetDownloadDir(const std::string& path);

    const std::string& GetSearchDir() const { return searchDir; }
    void SetSearchDir(const std::string& path);

    void AddObserver(ISettingsObserver* observer);
    void RemoveObserver(ISettingsObserver* observer);

private:
    SettingsService();
    ~SettingsService() = default;

    SettingsService(const SettingsService&) = delete;
    SettingsService& operator=(const SettingsService&) = delete;

    void NotifyObservers();

    std::string downloadDir;
    std::string searchDir;
    std::vector<ISettingsObserver*> observers;
};
