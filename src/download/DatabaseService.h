#pragma once
#include "download/DatabaseObserver.h"
#include "download/DownloadObserver.h"
#include <string>
#include <vector>
#include <atomic>
#include <memory>
#include <chrono>

class HttpDownloader;

class DatabaseService : public IDownloadObserver
{
public:
    static DatabaseService& GetInstance();

    void AddObserver(IDatabaseObserver* observer);
    void RemoveObserver(IDatabaseObserver* observer);

    void Initialize(const std::string& downloadDir, const std::string& searchDir);
    void SetPaths(const std::string& downloadDir, const std::string& searchDir);

    void CheckLocalDump();
    const std::string& GetCurrentFilePath() const { return currentFilePath; }
    void SetCurrentFilePath(const std::string& path) { currentFilePath = path; }

    void StartDownload(const std::string& url);
    void StartPrebuiltIndexDownload(const std::string& url);
    void CancelDownload();

    static const char* GetOfficialIndexUrl();

    void StartSchemaUpdate();
    void StartExtractionAndValidation();
    void CancelExtraction();

    void StartIndexing();
    void CancelIndexing();

    void EnterApplicationFlow();
    void ConfirmIndexRegeneration();

    bool IsBusy() const { return isBusy.load(); }
    double GetOnlineSize1Month() const { return onlineSize1Month.load(); }
    double GetOnlineSizeFull() const { return onlineSizeFull.load(); }
    void FetchOnlineSizes();

    float GetDownloadProgress() const { return downloadProgress.load(); }
    double GetDownloadSpeed() const { return downloadSpeed.load(); }
    double GetTotalFileSize() const { return totalFileSize.load(); }

    float GetSchemaProgress() const { return schemaProgress.load(); }
    bool IsSchemaUpdating() const { return isUpdatingSchema.load(); }

    float GetExtractionProgress() const { return extractionProgress.load(); }
    double GetExtractionTimeRemaining() const;
    float GetValidationProgress() const { return validationProgress.load(); }
    bool IsExtracting() const { return isExtracting.load(); }
    bool IsValidating() const { return isValidating.load(); }

    float GetIndexingProgress() const { return indexingProgress.load(); }
    bool IsIndexing() const { return isIndexing.load(); }

    void OnDownloadStarted(double totalSize) override;
    void OnDownloadProgress(double progress, double speed) override;
    void OnDownloadCompleted(const std::string& filePath) override;
    void OnDownloadFailed(const std::string& error) override;

private:
    DatabaseService();
    ~DatabaseService() override;

    DatabaseService(const DatabaseService&) = delete;
    DatabaseService& operator=(const DatabaseService&) = delete;

    void NotifyDownloadStarted(double totalSize);
    void NotifyDownloadProgress(double progress, double speed);
    void NotifyDownloadCompleted();
    void NotifyDownloadFailed(const std::string& error);

    void NotifySchemaUpdateProgress(float progress);
    void NotifySchemaUpdateCompleted();
    void NotifySchemaUpdateFailed(const std::string& error);

    void NotifyExtractionProgress(float progress);
    void NotifyValidationProgress(float progress);
    void NotifyDatabaseReady(const std::string& dbPath);
    void NotifyDatabaseFailed(const std::string& error);

    void NotifyIndexingProgress(float progress);
    void NotifyIndexingCompleted();
    void NotifyIndexingFailed(const std::string& error);
    void NotifyIndexOutdated();

    enum class PendingRegen { None, Extraction, Indexing };
    PendingRegen pendingRegen = PendingRegen::None;

    std::vector<IDatabaseObserver*> observers;
    std::shared_ptr<HttpDownloader> downloader;

    std::string downloadDir;
    std::string searchDir;
    std::string currentFilePath;

    std::atomic<bool> isBusy;
    std::atomic<double> onlineSize1Month;
    std::atomic<double> onlineSizeFull;
    std::atomic<bool> isFetchingSizes;

    std::atomic<float> downloadProgress;
    std::atomic<double> downloadSpeed;
    std::atomic<double> totalFileSize;

    std::atomic<float> schemaProgress;
    std::atomic<bool> isUpdatingSchema;

    std::atomic<float> extractionProgress;
    std::atomic<float> validationProgress;
    std::atomic<bool> isExtracting;
    std::atomic<bool> isValidating;
    std::atomic<bool> cancelExtractionFlag;
    std::chrono::steady_clock::time_point extractionStartTime;

    std::atomic<float> indexingProgress;
    std::atomic<bool> isIndexing;
    std::atomic<bool> cancelIndexingFlag;
};
