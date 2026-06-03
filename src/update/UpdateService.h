#pragma once

#include "update/UpdateSubject.h"
#include "update/IUpdateChecker.h"
#include <memory>
#include <atomic>
#include <thread>
#include <string>

class UpdateService : public UpdateSubject
{
public:
    static UpdateService& GetInstance();

    void Configure(std::unique_ptr<IUpdateChecker> checker, const std::string& currentVersion);
    void CheckAsync();
    void DownloadAndApplyAsync(const std::string& downloadUrl);
    void RestartIntoUpdate();

    bool HasPendingUpdate() const { return pendingNewBinary.load() != nullptr; }

    void EmitDownloadProgress(double progress) { NotifyUpdateDownloadProgress(progress); }

private:
    UpdateService();
    ~UpdateService();
    UpdateService(const UpdateService&) = delete;
    UpdateService& operator=(const UpdateService&) = delete;

    void DoCheck();
    void DoDownloadAndApply(std::string downloadUrl);
    void JoinIfDone();

    std::unique_ptr<IUpdateChecker> checker;
    std::string currentVersion;

    std::thread worker;
    std::atomic<bool> working;
    std::atomic<std::string*> pendingNewBinary;
    std::string targetBinaryPath;
};
