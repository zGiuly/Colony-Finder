#include "update/UpdateService.h"
#include "update/UpdaterLauncher.h"
#include <curl/curl.h>
#include <filesystem>
#include <cstdio>

namespace
{
    struct DownloadCtx
    {
        FILE* fp = nullptr;
        UpdateService* svc = nullptr;
        double totalBytes = 0.0;
        double receivedBytes = 0.0;
    };

    size_t WriteFileCb(void* ptr, size_t size, size_t nmemb, void* userdata)
    {
        DownloadCtx* ctx = static_cast<DownloadCtx*>(userdata);
        if (!ctx || !ctx->fp)
        {
            return 0;
        }
        return fwrite(ptr, size, nmemb, ctx->fp);
    }

    int ProgressCb(void* clientp, double dltotal, double dlnow, double, double)
    {
        DownloadCtx* ctx = static_cast<DownloadCtx*>(clientp);
        if (!ctx)
        {
            return 0;
        }
        ctx->totalBytes = dltotal;
        ctx->receivedBytes = dlnow;
        return 0;
    }
}

UpdateService& UpdateService::GetInstance()
{
    static UpdateService instance;
    return instance;
}

UpdateService::UpdateService() : working(false), pendingNewBinary(nullptr)
{
}

UpdateService::~UpdateService()
{
    if (worker.joinable())
    {
        worker.join();
    }
    std::string* pending = pendingNewBinary.exchange(nullptr);
    delete pending;
}

void UpdateService::Configure(std::unique_ptr<IUpdateChecker> checkerVal, const std::string& currentVersionVal)
{
    checker = std::move(checkerVal);
    currentVersion = currentVersionVal;
    targetBinaryPath = UpdaterLauncher::GetExecutablePath();
}

void UpdateService::JoinIfDone()
{
    if (worker.joinable() && !working.load())
    {
        worker.join();
    }
}

void UpdateService::CheckAsync()
{
    JoinIfDone();
    if (working.load() || !checker)
    {
        return;
    }
    working = true;
    worker = std::thread(&UpdateService::DoCheck, this);
}

void UpdateService::DoCheck()
{
    UpdateInfo info = checker->Check(currentVersion);
    if (!info.error.empty())
    {
        NotifyUpdateCheckFailed(info.error);
        working = false;
        return;
    }
    if (!info.available)
    {
        NotifyUpdateNotAvailable();
        working = false;
        return;
    }
    NotifyUpdateAvailable(info.latestVersion, info.downloadUrl);
    working = false;
}

void UpdateService::DownloadAndApplyAsync(const std::string& downloadUrl)
{
    JoinIfDone();
    if (working.load())
    {
        return;
    }
    working = true;
    worker = std::thread(&UpdateService::DoDownloadAndApply, this, downloadUrl);
}

void UpdateService::DoDownloadAndApply(std::string downloadUrl)
{
    if (targetBinaryPath.empty())
    {
        NotifyUpdateFailed("cannot resolve current executable path");
        working = false;
        return;
    }

    std::filesystem::path tmpDir = std::filesystem::temp_directory_path();
    std::filesystem::path tmpFile = tmpDir / "colonyfinder_update.bin";

    FILE* fp = nullptr;
#ifdef _WIN32
    fopen_s(&fp, tmpFile.string().c_str(), "wb");
#else
    fp = fopen(tmpFile.string().c_str(), "wb");
#endif
    if (!fp)
    {
        NotifyUpdateFailed("cannot open temp file");
        working = false;
        return;
    }

    CURL* curl = curl_easy_init();
    if (!curl)
    {
        fclose(fp);
        NotifyUpdateFailed("curl init failed");
        working = false;
        return;
    }

    DownloadCtx ctx;
    ctx.fp = fp;
    ctx.svc = this;

    curl_easy_setopt(curl, CURLOPT_URL, downloadUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, ProgressCb);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "ColonyFinder-Updater");

    CURLcode rc = curl_easy_perform(curl);

    if (ctx.totalBytes > 0.0)
    {
        NotifyUpdateDownloadProgress(ctx.receivedBytes / ctx.totalBytes);
    }

    curl_easy_cleanup(curl);
    fclose(fp);

    if (rc != CURLE_OK)
    {
        NotifyUpdateFailed(std::string("download failed: ") + curl_easy_strerror(rc));
        working = false;
        return;
    }

#ifndef _WIN32
    std::filesystem::permissions(tmpFile,
        std::filesystem::perms::owner_all | std::filesystem::perms::group_read |
        std::filesystem::perms::group_exec | std::filesystem::perms::others_read |
        std::filesystem::perms::others_exec,
        std::filesystem::perm_options::replace);
#endif

    std::string* prev = pendingNewBinary.exchange(new std::string(tmpFile.string()));
    delete prev;

    NotifyUpdateReady(tmpFile.string());
    working = false;
}

void UpdateService::RestartIntoUpdate()
{
    std::string* pending = pendingNewBinary.load();
    if (!pending || targetBinaryPath.empty())
    {
        return;
    }
    UpdaterLauncher::LaunchUpdater(*pending, targetBinaryPath);
}
