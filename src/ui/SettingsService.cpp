#include "ui/SettingsService.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <algorithm>

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
        return;
    }

    std::ifstream f(configPath);
    if (!f.is_open())
    {
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
        NotifyObservers();
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
        f << j.dump(4);
    }
    catch (...)
    {
    }
}

void SettingsService::SetDownloadDir(const std::string& path)
{
    if (downloadDir == path)
    {
        return;
    }
    downloadDir = path;
    Save();
    NotifyObservers();
}

void SettingsService::SetSearchDir(const std::string& path)
{
    if (searchDir == path)
    {
        return;
    }
    searchDir = path;
    Save();
    NotifyObservers();
}

void SettingsService::SetBufferSizeMb(int val)
{
    if (bufferSizeMb == val)
    {
        return;
    }
    bufferSizeMb = val;
    Save();
    NotifyObservers();
}

void SettingsService::AddObserver(ISettingsObserver* observer)
{
    if (!observer)
    {
        return;
    }
    auto it = std::find(observers.begin(), observers.end(), observer);
    if (it != observers.end())
    {
        return;
    }
    observers.push_back(observer);
}

void SettingsService::RemoveObserver(ISettingsObserver* observer)
{
    auto it = std::find(observers.begin(), observers.end(), observer);
    if (it == observers.end())
    {
        return;
    }
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
