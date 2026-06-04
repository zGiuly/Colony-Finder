#pragma once

#include "download/DatabaseObserver.h"
#include "ui/SettingsObserver.h"
#include "update/UpdateObserver.h"
#include "ui/AppTheme.h"
#include <string>
#include <memory>
#include <atomic>

class IAppState;

class AppController : public IDatabaseObserver, public ISettingsObserver, public IUpdateObserver
{
    friend class WindowManager;

public:
    AppController();
    ~AppController() override;

    bool Initialize();
    void Run();

    void OnDownloadStarted(double totalSize) override;
    void OnDownloadProgress(double progress, double speed) override;
    void OnDownloadCompleted() override;
    void OnDownloadFailed(const std::string& error) override;

    void OnSchemaUpdateProgress(float progress) override;
    void OnSchemaUpdateCompleted() override;
    void OnSchemaUpdateFailed(const std::string& error) override;

    void OnExtractionProgress(float progress) override;
    void OnValidationProgress(float progress) override;
    void OnDatabaseReady(const std::string& dbPath) override;
    void OnDatabaseFailed(const std::string& error) override;

    void OnIndexingProgress(float progress) override;
    void OnIndexingCompleted() override;
    void OnIndexingFailed(const std::string& error) override;
    void OnIndexOutdated() override;

    void OnSettingsChanged(const std::string& downloadDir, const std::string& searchDir) override;
    void OnThemeChanged(const ThemeColors& colors) override;

    void OnUpdateAvailable(const std::string& latestVersion, const std::string& downloadUrl) override;
    void OnUpdateNotAvailable() override;
    void OnUpdateCheckFailed(const std::string& error) override;
    void OnUpdateDownloadProgress(double progress) override;
    void OnUpdateReady(const std::string& newBinaryPath) override;
    void OnUpdateFailed(const std::string& error) override;

    bool IsUpdateReady() const { return updateReady.load(); }
    double GetUpdateDownloadProgress() const { return updateProgress.load(); }

    void TransitionTo(std::unique_ptr<IAppState> nextState);

    const AppTheme& GetTheme() const { return theme; }

    float GetButtonWidthLarge() const { return theme.buttonWidthLarge; }
    float GetButtonHeightLarge() const { return theme.buttonHeightLarge; }
    float GetButtonWidthMedium() const { return theme.buttonWidthMedium; }
    float GetButtonHeightMedium() const { return theme.buttonHeightMedium; }

    const std::string& GetErrorMessage() const { return errorMessage; }
    void SetErrorMessage(const std::string& error) { errorMessage = error; }

private:
    void RenderUI();

    std::unique_ptr<IAppState> currentState;
    AppTheme theme;
    std::string errorMessage;
    std::unique_ptr<class WindowManager> windowManager;

    std::atomic<bool> updateReady{false};
    std::atomic<double> updateProgress{0.0};
};
