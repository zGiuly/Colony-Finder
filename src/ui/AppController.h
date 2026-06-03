#pragma once

#include "download/DownloadObserver.h"
#include "ui/AppTheme.h"
#include <string>
#include <atomic>
#include <memory>

class HttpDownloader;
class IAppState;
struct GLFWwindow;

class AppController : public IDownloadObserver
{
public:
    AppController();
    ~AppController() override;

    bool Initialize();
    void Run();

    void OnDownloadStarted(double totalSize) override;
    void OnDownloadProgress(double progress, double speed) override;
    void OnDownloadCompleted(const std::string& filePath) override;
    void OnDownloadFailed(const std::string& error) override;

    void CheckLocalDump();
    void StartDownload(const std::string& url);
    void FetchOnlineSizes();
    void TransitionTo(std::unique_ptr<IAppState> nextState);

    const AppTheme& GetTheme() const { return theme; }
    const std::string& GetCurrentFilePath() const { return currentFilePath; }
    void SetCurrentFilePath(const std::string& path) { currentFilePath = path; }

    float GetButtonWidthLarge() const { return 240.0f; }
    float GetButtonHeightLarge() const { return 50.0f; }
    float GetButtonWidthMedium() const { return 200.0f; }
    float GetButtonHeightMedium() const { return 40.0f; }

    const std::string& GetDownloadDir() const { return downloadDir; }
    void SetDownloadDir(const std::string& path) { downloadDir = path; }

    const std::string& GetSearchDir() const { return searchDir; }
    void SetSearchDir(const std::string& path) { searchDir = path; }

    double GetOnlineSize1Month() const { return onlineSize1Month.load(); }
    double GetOnlineSizeFull() const { return onlineSizeFull.load(); }
    
    double GetTotalFileSize() const { return totalFileSize.load(); }
    float GetDownloadProgress() const { return downloadProgress.load(); }
    double GetDownloadSpeed() const { return downloadSpeed.load(); }
    void CancelDownload();

    const std::string& GetErrorMessage() const { return errorMessage; }
    void SetErrorMessage(const std::string& error) { errorMessage = error; }

private:
    void RenderUI();

    GLFWwindow* window;
    std::unique_ptr<IAppState> currentState;
    AppTheme theme;
    std::shared_ptr<HttpDownloader> downloader;

    std::atomic<float> downloadProgress;
    std::atomic<double> downloadSpeed;
    std::atomic<double> totalFileSize;
    std::string currentFilePath;
    std::string errorMessage;

    std::string downloadDir;
    std::string searchDir;
    std::atomic<bool> isBusy;
    std::atomic<double> onlineSize1Month;
    std::atomic<double> onlineSizeFull;
    std::atomic<bool> isFetchingSizes;
};
