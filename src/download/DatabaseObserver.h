#pragma once
#include <string>

class IDatabaseObserver
{
public:
    virtual ~IDatabaseObserver() = default;

    virtual void OnDownloadStarted(double totalSize) = 0;
    virtual void OnDownloadProgress(double progress, double speed) = 0;
    virtual void OnDownloadCompleted() = 0;
    virtual void OnDownloadFailed(const std::string& error) = 0;

    virtual void OnSchemaUpdateProgress(float progress) = 0;
    virtual void OnSchemaUpdateCompleted() = 0;
    virtual void OnSchemaUpdateFailed(const std::string& error) = 0;

    virtual void OnExtractionProgress(float progress) = 0;
    virtual void OnValidationProgress(float progress) = 0;
    virtual void OnDatabaseReady(const std::string& dbPath) = 0;
    virtual void OnDatabaseFailed(const std::string& error) = 0;

    virtual void OnIndexingProgress(float progress) = 0;
    virtual void OnIndexingCompleted() = 0;
    virtual void OnIndexingFailed(const std::string& error) = 0;
};
