#include "CliAppController.h"
#include "ArgumentParser.h"
#include "download/DatabaseService.h"
#include "ui/SettingsService.h"
#include "CliSelfUpdater.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <filesystem>

#ifdef _WIN32
#include <conio.h>
bool KeyPressed()
{
    return _kbhit() != 0;
}
int GetChar()
{
    return _getch();
}
#else
#include <sys/select.h>
#include <unistd.h>
bool KeyPressed()
{
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}
int GetChar()
{
    char ch = 0;
    if (read(STDIN_FILENO, &ch, 1) > 0)
    {
        return ch;
    }
    return 0;
}
#endif



CliAppController::CliAppController()
    : isDone(false),
      isFailed(false),
      lastExtractionProgress(0.0f),
      lastIndexingProgress(0.0f),
      downloadStarted(false),
      dbPrepStarted(false),
      schemaStarted(false)
{
    DatabaseService::GetInstance().AddObserver(this);
}

CliAppController::~CliAppController()
{
    DatabaseService::GetInstance().RemoveObserver(this);
}

int CliAppController::Run(const CliConfig& config)
{
    if (!config.downloadDir.empty())
    {
        SettingsService::GetInstance().SetDownloadDir(config.downloadDir);
    }
    if (!config.searchDir.empty())
    {
        SettingsService::GetInstance().SetSearchDir(config.searchDir);
    }
    if (config.bufferSizeMb > 0)
    {
        SettingsService::GetInstance().SetBufferSizeMb(config.bufferSizeMb);
    }

    DatabaseService::GetInstance().Initialize(
        SettingsService::GetInstance().GetDownloadDir(),
        SettingsService::GetInstance().GetSearchDir()
    );

    if (config.interactiveMode || (!config.updateSchema && config.downloadUrl.empty() && config.localFilePath.empty() && !config.selfUpdate))
    {
        RunInteractive();
        return 0;
    }

    if (config.updateSchema)
    {
        StartSchemaAction();
        return isFailed.load() ? 1 : 0;
    }

    if (!config.downloadUrl.empty())
    {
        StartDownloadAction(config.downloadUrl);
        return isFailed.load() ? 1 : 0;
    }

    if (!config.localFilePath.empty())
    {
        StartLocalIndexAction(config.localFilePath);
        return isFailed.load() ? 1 : 0;
    }

    return 0;
}

void CliAppController::Reset()
{
    std::lock_guard<std::mutex> lock(mtx);
    isDone = false;
    isFailed = false;
    lastExtractionProgress = 0.0f;
    lastIndexingProgress = 0.0f;
    downloadStarted = false;
    dbPrepStarted = false;
    schemaStarted = false;
}

void CliAppController::WaitForCompletion()
{
    while (!isDone.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        if (KeyPressed())
        {
            int ch = GetChar();
            if (ch == 27 || ch == 'q' || ch == 'Q')
            {
                std::cout << "\nCancelling operation...\n";
                DatabaseService::GetInstance().CancelDownload();
                DatabaseService::GetInstance().CancelExtraction();
                DatabaseService::GetInstance().CancelIndexing();

                if (!DatabaseService::GetInstance().IsBusy())
                {
                    while (DatabaseService::GetInstance().IsExtracting() || DatabaseService::GetInstance().IsIndexing())
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                    WakeUp(false);
                }
            }
        }
    }
}

void CliAppController::WakeUp(bool success)
{
    isDone = true;
    isFailed = !success;
    cv.notify_all();
}

void CliAppController::PrintProgressBar(float progress, double speed, const std::string& phase)
{
    auto now = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point startTime = now;

    if (phase == "Downloading")
    {
        if (!downloadStarted)
        {
            downloadStartTime = now;
            downloadStarted = true;
        }
        startTime = downloadStartTime;
    }
    if (phase == "Updating Schema")
    {
        if (!schemaStarted)
        {
            schemaStartTime = now;
            schemaStarted = true;
        }
        startTime = schemaStartTime;
    }

    if (progress < 0.0f)
    {
        progress = 0.0f;
    }
    if (progress > 1.0f)
    {
        progress = 1.0f;
    }

    int width = 40;
    int pos = static_cast<int>(width * progress);

    std::cout << "\r" << phase << " [";
    for (int i = 0; i < width; ++i)
    {
        if (i < pos)
        {
            std::cout << "=";
            continue;
        }
        if (i == pos)
        {
            std::cout << ">";
            continue;
        }
        std::cout << " ";
    }
    std::cout << "] " << std::fixed << std::setprecision(1) << (progress * 100.0f) << "%";

    if (speed > 0.0)
    {
        double speedMb = speed / (1024.0 * 1024.0);
        std::cout << " (" << std::fixed << std::setprecision(2) << speedMb << " MB/s)";
    }

    double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count() / 1000.0;
    bool hasTime = (progress > 0.001f && elapsed > 0.5);
    double remaining = hasTime ? ((elapsed / progress) - elapsed) : 0.0;
    int totalSecs = static_cast<int>(remaining < 0.0 ? 0.0 : remaining);
    int hours = totalSecs / 3600;
    int mins = (totalSecs % 3600) / 60;
    int secs = totalSecs % 60;

    char buf[64];
    std::snprintf(buf, sizeof(buf), "--:--");
    if (hasTime && hours > 0)
    {
        std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", hours, mins, secs);
    }
    if (hasTime && hours == 0)
    {
        std::snprintf(buf, sizeof(buf), "%02d:%02d", mins, secs);
    }
    std::string timeStr = buf;

    std::cout << " | ETA: " << timeStr << " (Press ESC or Q to cancel)                       " << std::flush;
}

void CliAppController::PrintDbPrepProgress()
{
    auto now = std::chrono::steady_clock::now();
    if (!dbPrepStarted)
    {
        dbPrepStartTime = now;
        dbPrepStarted = true;
    }

    float progress = lastIndexingProgress;
    if (progress < 0.0f)
    {
        progress = 0.0f;
    }
    if (progress > 1.0f)
    {
        progress = 1.0f;
    }

    int width = 40;
    int pos = static_cast<int>(width * progress);

    std::cout << "\rDatabase Prep [";
    for (int i = 0; i < width; ++i)
    {
        if (i < pos)
        {
            std::cout << "=";
            continue;
        }
        if (i == pos)
        {
            std::cout << ">";
            continue;
        }
        std::cout << " ";
    }
    std::cout << "] Extract: " << std::fixed << std::setprecision(1) << (lastExtractionProgress * 100.0f) << "%"
              << " | Index: " << std::fixed << std::setprecision(1) << (lastIndexingProgress * 100.0f) << "%";

    double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - dbPrepStartTime).count() / 1000.0;
    bool hasTime = (progress > 0.001f && elapsed > 0.5);
    double remaining = hasTime ? ((elapsed / progress) - elapsed) : 0.0;
    int totalSecs = static_cast<int>(remaining < 0.0 ? 0.0 : remaining);
    int hours = totalSecs / 3600;
    int mins = (totalSecs % 3600) / 60;
    int secs = totalSecs % 60;

    char buf[64];
    std::snprintf(buf, sizeof(buf), "--:--");
    if (hasTime && hours > 0)
    {
        std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", hours, mins, secs);
    }
    if (hasTime && hours == 0)
    {
        std::snprintf(buf, sizeof(buf), "%02d:%02d", mins, secs);
    }
    std::string timeStr = buf;

    std::cout << " | ETA: " << timeStr << " (Press ESC or Q to cancel)                       " << std::flush;
}

void CliAppController::OnDownloadStarted(double totalSize)
{
    double sizeMb = totalSize / (1024.0 * 1024.0);
    std::cout << "Download started. Total size: " << std::fixed << std::setprecision(2) << sizeMb << " MB\n";
}

void CliAppController::OnDownloadProgress(double progress, double speed)
{
    PrintProgressBar(static_cast<float>(progress), speed, "Downloading");
}

void CliAppController::OnDownloadCompleted()
{
    std::cout << "\nDownload completed successfully.\n";
}

void CliAppController::OnDownloadFailed(const std::string& error)
{
    std::cout << "\nDownload failed: " << error << "\n";
    WakeUp(false);
}

void CliAppController::OnSchemaUpdateProgress(float progress)
{
    PrintProgressBar(progress, 0.0, "Updating Schema");
}

void CliAppController::OnSchemaUpdateCompleted()
{
    std::cout << "\nSchema updated successfully.\n";
    WakeUp(true);
}

void CliAppController::OnSchemaUpdateFailed(const std::string& error)
{
    std::cout << "\nSchema update failed: " << error << "\n";
    WakeUp(false);
}

void CliAppController::OnExtractionProgress(float progress)
{
    lastExtractionProgress = progress;
    PrintDbPrepProgress();
}

void CliAppController::OnValidationProgress(float)
{
}

void CliAppController::OnDatabaseReady(const std::string& dbPath)
{
    std::cout << "\nDatabase is ready at: " << dbPath << "\n";
    WakeUp(true);
}

void CliAppController::OnDatabaseFailed(const std::string& error)
{
    std::cout << "\nDatabase initialization failed: " << error << "\n";
    WakeUp(false);
}

void CliAppController::OnIndexingProgress(float progress)
{
    lastIndexingProgress = progress;
    PrintDbPrepProgress();
}

void CliAppController::OnIndexingCompleted()
{
    std::cout << "\nIndexing completed successfully.\n";
}

void CliAppController::OnIndexingFailed(const std::string& error)
{
    std::cout << "\nIndexing failed: " << error << "\n";
    WakeUp(false);
}

void CliAppController::OnIndexOutdated()
{
    std::cout << "Local index is outdated. Regenerating...\n";
    DatabaseService::GetInstance().ConfirmIndexRegeneration();
}

void CliAppController::StartDownloadAction(const std::string& url)
{
    Reset();
    DatabaseService::GetInstance().StartDownload(url);
    WaitForCompletion();
}

void CliAppController::StartSchemaAction()
{
    Reset();
    DatabaseService::GetInstance().StartSchemaUpdate();
    WaitForCompletion();
}

void CliAppController::StartLocalIndexAction(const std::string& path)
{
    if (!std::filesystem::exists(path))
    {
        std::cerr << "Error: Local file does not exist: " << path << "\n";
        isFailed = true;
        return;
    }

    std::filesystem::path filePath(path);
    std::filesystem::path idxPath = filePath;
    if (filePath.extension() == ".gz")
    {
        std::filesystem::path jsonPath = filePath;
        jsonPath.replace_extension("");
        idxPath = jsonPath;
        idxPath.replace_extension(".idx");
    }
    else
    {
        idxPath.replace_extension(".idx");
    }

    if (std::filesystem::exists(idxPath))
    {
        std::cout << "An index already exists for this file: " << idxPath.string() << "\n";
        std::cout << "Do you want to regenerate it? (y/n) [n]: ";
        std::string response;
        if (std::getline(std::cin, response))
        {
            if (response == "y" || response == "Y")
            {
                std::error_code ec;
                std::filesystem::remove(idxPath, ec);
            }
        }
    }

    Reset();
    DatabaseService::GetInstance().SetCurrentFilePath(path);
    DatabaseService::GetInstance().EnterApplicationFlow();
    WaitForCompletion();
}

void CliAppController::ShowCurrentSettings()
{
    std::cout << "\nCurrent Settings:\n"
              << "  Download Directory: " << SettingsService::GetInstance().GetDownloadDir() << "\n"
              << "  Search/Index Directory: " << SettingsService::GetInstance().GetSearchDir() << "\n"
              << "  Buffer Size (MB): " << SettingsService::GetInstance().GetBufferSizeMb() << "\n\n";
}

void CliAppController::ConfigureDownloadDir()
{
    std::cout << "Enter new download directory path: ";
    std::string path;
    if (!std::getline(std::cin, path) || path.empty())
    {
        std::cout << "Error: Path cannot be empty.\n";
        return;
    }
    SettingsService::GetInstance().SetDownloadDir(path);
    DatabaseService::GetInstance().SetPaths(
        SettingsService::GetInstance().GetDownloadDir(),
        SettingsService::GetInstance().GetSearchDir()
    );
    std::cout << "Download directory set to " << path << "\n";
}

void CliAppController::ConfigureSearchDir()
{
    std::cout << "Enter new search/index directory path: ";
    std::string path;
    if (!std::getline(std::cin, path) || path.empty())
    {
        std::cout << "Error: Path cannot be empty.\n";
        return;
    }
    SettingsService::GetInstance().SetSearchDir(path);
    DatabaseService::GetInstance().SetPaths(
        SettingsService::GetInstance().GetDownloadDir(),
        SettingsService::GetInstance().GetSearchDir()
    );
    std::cout << "Search/index directory set to " << path << "\n";
}

void CliAppController::ConfigureBufferSize()
{
    std::cout << "Enter new buffer size in MB (1-64): ";
    std::string sizeStr;
    if (!std::getline(std::cin, sizeStr))
    {
        return;
    }
    int val = ArgumentParser::ParseBufferSize(sizeStr);
    if (val == 0)
    {
        std::cout << "Error: Buffer size must be between 1 and 64.\n";
        return;
    }
    SettingsService::GetInstance().SetBufferSizeMb(val);
    std::cout << "Buffer size set to " << val << " MB.\n";
}

void CliAppController::RunInteractive()
{
    while (true)
    {
        std::cout << "=== ColonyFinder Server Indexer (Interactive) ===\n"
                  << "1. Download & Index Full Galaxy Database\n"
                  << "2. Download & Index 1 Month Galaxy Database\n"
                  << "3. Update Schema\n"
                  << "4. Index Local File\n"
                  << "5. Configure Decompression Buffer Size\n"
                  << "6. Configure Download Directory\n"
                  << "7. Configure Search/Index Directory\n"
                  << "8. Show Current Settings\n"
                  << "9. Check & Update CLI Tool\n"
                  << "10. Exit\n"
                  << "Select option (1-10): ";

        std::string choice;
        if (!std::getline(std::cin, choice) || choice == "10")
        {
            break;
        }
        if (choice.empty())
        {
            continue;
        }

        int opt = 0;
        try
        {
            opt = std::stoi(choice);
        }
        catch (...)
        {
            std::cout << "Invalid option. Please try again.\n";
            continue;
        }

        switch (opt)
        {
            case 1:
                StartDownloadAction("https://downloads.spansh.co.uk/galaxy.json.gz");
                break;
            case 2:
                StartDownloadAction("https://downloads.spansh.co.uk/galaxy_1month.json.gz");
                break;
            case 3:
                StartSchemaAction();
                break;
            case 4:
            {
                std::cout << "Enter path to local JSON or JSON.GZ file: ";
                std::string path;
                if (!std::getline(std::cin, path))
                {
                    break;
                }
                StartLocalIndexAction(path);
                break;
            }
            case 5:
                ConfigureBufferSize();
                break;
            case 6:
                ConfigureDownloadDir();
                break;
            case 7:
                ConfigureSearchDir();
                break;
            case 8:
                ShowCurrentSettings();
                break;
            case 9:
                CliSelfUpdater::Run();
                break;
            default:
                std::cout << "Invalid option. Please try again.\n";
                break;
        }
    }
}
