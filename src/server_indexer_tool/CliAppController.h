#pragma once
#include "download/DatabaseObserver.h"
#include "ArgumentParser.h"
#include <string>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <memory>

class CliAppController : public IDatabaseObserver
{
public:
    CliAppController();
    ~CliAppController() override;

    int Run(const CliConfig& config);

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

private:
    void RunInteractive();
    void ShowCurrentSettings();
    void ConfigureDownloadDir();
    void ConfigureSearchDir();
    void ConfigureBufferSize();

    void StartDownloadAction(const std::string& url);
    void StartSchemaAction();
    void StartLocalIndexAction(const std::string& path);

    void WaitForCompletion();
    void Reset();
    void WakeUp(bool success);
    void PrintProgressBar(float progress, double speed, const std::string& phase);
    void PrintDbPrepProgress();

    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> isDone;
    std::atomic<bool> isFailed;
    std::string currentPhase;

    float lastExtractionProgress;
    float lastIndexingProgress;
    bool downloadStarted;
    bool dbPrepStarted;
    bool schemaStarted;

    std::chrono::steady_clock::time_point downloadStartTime;
    std::chrono::steady_clock::time_point dbPrepStartTime;
    std::chrono::steady_clock::time_point schemaStartTime;
};
