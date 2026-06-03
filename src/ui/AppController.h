#pragma once

#include "download/DatabaseObserver.h"
#include "ui/SettingsObserver.h"
#include "ui/AppTheme.h"
#include <string>
#include <memory>

class IAppState;

class AppController : public IDatabaseObserver, public ISettingsObserver
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

    void OnSettingsChanged(const std::string& downloadDir, const std::string& searchDir) override;

    void TransitionTo(std::unique_ptr<IAppState> nextState);

    const AppTheme& GetTheme() const { return theme; }

    float GetButtonWidthLarge() const { return 240.0f; }
    float GetButtonHeightLarge() const { return 50.0f; }
    float GetButtonWidthMedium() const { return 200.0f; }
    float GetButtonHeightMedium() const { return 40.0f; }

    const std::string& GetErrorMessage() const { return errorMessage; }
    void SetErrorMessage(const std::string& error) { errorMessage = error; }

private:
    void RenderUI();

    std::unique_ptr<IAppState> currentState;
    AppTheme theme;
    std::string errorMessage;
    std::unique_ptr<class WindowManager> windowManager;
};
