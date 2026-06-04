#include "CliSelfUpdater.h"
#include "update/GithubUpdateChecker.h"
#include "update/UpdaterLauncher.h"
#include "download/HttpDownloader.h"
#include "Version.h"
#include <iostream>
#include <filesystem>

bool CliSelfUpdater::Run()
{
#ifdef _WIN32
    std::string assetName = "colony_indexer-windows-x64.exe";
#else
    std::string assetName = "colony_indexer-linux-x64";
#endif

    std::cout << "Checking for updates on GitHub...\n";
    GithubUpdateChecker checker("zGiuly", "Colony-Finder", assetName);
    UpdateInfo info = checker.Check(COLONYFINDER_VERSION_STRING);

    if (!info.error.empty())
    {
        std::cerr << "Error checking for updates: " << info.error << "\n";
        return false;
    }
    if (!info.available)
    {
        std::cout << "ColonyIndexer is up to date (current version: " << COLONYFINDER_VERSION_STRING << ").\n";
        return true;
    }

    std::cout << "New version available: " << info.latestVersion << "\n";
    std::cout << "Downloading from: " << info.downloadUrl << "\n";

    std::filesystem::path currentExec = UpdaterLauncher::GetExecutablePath();
    if (currentExec.empty())
    {
        std::cerr << "Error: Cannot resolve current executable path.\n";
        return false;
    }

    std::filesystem::path tempNew = currentExec.parent_path() / "colony_indexer_new.tmp";
    HttpDownloader downloader;
    bool success = downloader.Download(info.downloadUrl, tempNew.string(), 1);
    if (!success)
    {
        std::cerr << "Error: Download failed.\n";
        std::filesystem::remove(tempNew);
        return false;
    }

#ifndef _WIN32
    std::filesystem::permissions(tempNew,
        std::filesystem::perms::owner_all | std::filesystem::perms::group_read |
        std::filesystem::perms::group_exec | std::filesystem::perms::others_read |
        std::filesystem::perms::others_exec,
        std::filesystem::perm_options::replace);
#endif

    std::cout << "Download completed. Launching updater...\n";
    if (!UpdaterLauncher::LaunchUpdater(tempNew.string(), currentExec.string()))
    {
        std::cerr << "Error: Failed to launch updater.\n";
        std::filesystem::remove(tempNew);
        return false;
    }

    std::exit(0);
}
