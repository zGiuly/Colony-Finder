#include "download/DatabaseService.h"
#include "download/HttpDownloader.h"
#include "download/GzipDecompressor.h"
#include "download/JsonStreamValidator.h"
#include "download/StreamingProcessor.h"
#include "search/IndexBuilder.h"
#include "search/SystemIndex.h"
#include "ui/SettingsService.h"
#include <filesystem>
#include <fstream>
#include <thread>
#include <algorithm>
#include <chrono>

constexpr int ThreadCount = 1;
constexpr double ErrorSizeCode = -2.0;
constexpr double DefaultInitSize = -1.0;
constexpr const char* OfficialIndexUrl = "https://github.com/zGiuly/Colony-Finder/releases/latest/download/galaxy_1month.idx";
constexpr const char* PrebuiltIndexFileName = "galaxy_1month.idx";
constexpr const char* PrebuiltIndexCompanionJson = "galaxy_1month.json";

static bool IsIndexCurrent(const std::filesystem::path& idxPath)
{
    std::ifstream in(idxPath, std::ios::binary);
    if (!in.is_open()) return false;
    SystemIndex::Header header{};
    in.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (in.gcount() != static_cast<std::streamsize>(sizeof(header))) return false;
    if (std::string(header.magic) != "CFIDX") return false;
    return header.version == SystemIndex::Version;
}

DatabaseService::DatabaseService()
    : downloader(std::make_shared<HttpDownloader>()),
      isBusy(false),
      onlineSize1Month(DefaultInitSize),
      onlineSizeFull(DefaultInitSize),
      isFetchingSizes(false),
      downloadProgress(0.0f),
      downloadSpeed(0.0),
      totalFileSize(0.0),
      schemaProgress(0.0f),
      isUpdatingSchema(false),
      extractionProgress(0.0f),
      validationProgress(0.0f),
      isExtracting(false),
      isValidating(false),
      cancelExtractionFlag(false),
      indexingProgress(0.0f),
      isIndexing(false),
      cancelIndexingFlag(false)
{
    downloader->AddObserver(this);
}

DatabaseService::~DatabaseService()
{
    downloader->RemoveObserver(this);
}

DatabaseService& DatabaseService::GetInstance()
{
    static DatabaseService instance;
    return instance;
}

void DatabaseService::AddObserver(IDatabaseObserver* observer)
{
    if (std::find(observers.begin(), observers.end(), observer) == observers.end())
    {
        observers.push_back(observer);
    }
}

void DatabaseService::RemoveObserver(IDatabaseObserver* observer)
{
    auto it = std::find(observers.begin(), observers.end(), observer);
    if (it != observers.end())
    {
        observers.erase(it);
    }
}

void DatabaseService::Initialize(const std::string& downloadDirVal, const std::string& searchDirVal)
{
    downloadDir = downloadDirVal;
    searchDir = searchDirVal;
    CheckLocalDump();
}

void DatabaseService::SetPaths(const std::string& downloadDirVal, const std::string& searchDirVal)
{
    downloadDir = downloadDirVal;
    searchDir = searchDirVal;
    CheckLocalDump();
}

void DatabaseService::CheckLocalDump()
{
    std::filesystem::path p1_json = std::filesystem::path(searchDir) / "galaxy.json";
    std::filesystem::path p2_json = std::filesystem::path(searchDir) / "galaxy_1month.json";
    std::filesystem::path p1_gz = std::filesystem::path(searchDir) / "galaxy.json.gz";
    std::filesystem::path p2_gz = std::filesystem::path(searchDir) / "galaxy_1month.json.gz";

    if (std::filesystem::exists(p1_json))
    {
        currentFilePath = p1_json.string();
        return;
    }

    if (std::filesystem::exists(p2_json))
    {
        currentFilePath = p2_json.string();
        return;
    }

    if (std::filesystem::exists(p1_gz))
    {
        currentFilePath = p1_gz.string();
        return;
    }

    if (std::filesystem::exists(p2_gz))
    {
        currentFilePath = p2_gz.string();
        return;
    }

    if (searchDir != downloadDir)
    {
        std::filesystem::path d1_json = std::filesystem::path(downloadDir) / "galaxy.json";
        std::filesystem::path d2_json = std::filesystem::path(downloadDir) / "galaxy_1month.json";
        std::filesystem::path d1_gz = std::filesystem::path(downloadDir) / "galaxy.json.gz";
        std::filesystem::path d2_gz = std::filesystem::path(downloadDir) / "galaxy_1month.json.gz";

        if (std::filesystem::exists(d1_json))
        {
            currentFilePath = d1_json.string();
            return;
        }

        if (std::filesystem::exists(d2_json))
        {
            currentFilePath = d2_json.string();
            return;
        }

        if (std::filesystem::exists(d1_gz))
        {
            currentFilePath = d1_gz.string();
            return;
        }

        if (std::filesystem::exists(d2_gz))
        {
            currentFilePath = d2_gz.string();
            return;
        }
    }

    currentFilePath.clear();
}

void DatabaseService::StartDownload(const std::string& url)
{
    if (isBusy.load())
    {
        return;
    }

    isBusy = true;
    downloadProgress = 0.0f;
    downloadSpeed = 0.0;

    std::thread([this, url]() {
        std::string fileName = url.substr(url.find_last_of('/') + 1);
        std::filesystem::path destPath = std::filesystem::path(downloadDir) / fileName;

        bool success = downloader->Download(url, destPath.string(), ThreadCount);
        isBusy = false;

        if (!success)
        {
            std::error_code ec;
            std::filesystem::remove(destPath, ec);
            return;
        }

        currentFilePath = destPath.string();
        NotifyDownloadCompleted();
        StartExtractionAndValidation();
    }).detach();
}

const char* DatabaseService::GetOfficialIndexUrl()
{
    return OfficialIndexUrl;
}

void DatabaseService::StartPrebuiltIndexDownload(const std::string& url)
{
    if (isBusy.load())
    {
        return;
    }
    if (url.empty())
    {
        NotifyDownloadFailed("Index URL is empty.");
        return;
    }
    isBusy = true;
    downloadProgress = 0.0f;
    downloadSpeed = 0.0;

    std::thread([this, url]() {
        std::filesystem::path destPath = std::filesystem::path(searchDir) / PrebuiltIndexFileName;
        bool success = downloader->Download(url, destPath.string(), ThreadCount);
        isBusy = false;

        if (!success)
        {
            std::error_code ec;
            std::filesystem::remove(destPath, ec);
            return;
        }

        if (!IsIndexCurrent(destPath))
        {
            std::error_code ec;
            std::filesystem::remove(destPath, ec);
            NotifyDownloadFailed("Downloaded file is not a valid or compatible index.");
            return;
        }

        currentFilePath = (std::filesystem::path(searchDir) / PrebuiltIndexCompanionJson).string();
        NotifyDownloadCompleted();
        NotifyDatabaseReady(currentFilePath);
    }).detach();
}

void DatabaseService::CancelDownload()
{
    downloader->Cancel();
    downloadProgress = 0.0f;
}

void DatabaseService::FetchOnlineSizes()
{
    if (onlineSize1Month.load() != DefaultInitSize && onlineSizeFull.load() != DefaultInitSize)
    {
        return;
    }

    if (isFetchingSizes.load())
    {
        return;
    }

    isFetchingSizes = true;

    if (onlineSize1Month.load() == DefaultInitSize)
    {
        std::thread([this]() {
            double size1 = HttpDownloader::GetContentLength("https://downloads.spansh.co.uk/galaxy_1month.json.gz");
            onlineSize1Month = (size1 <= 0.0) ? ErrorSizeCode : size1;
            if (onlineSizeFull.load() != DefaultInitSize)
            {
                isFetchingSizes = false;
            }
        }).detach();
    }

    if (onlineSizeFull.load() == DefaultInitSize)
    {
        std::thread([this]() {
            double size2 = HttpDownloader::GetContentLength("https://downloads.spansh.co.uk/galaxy.json.gz");
            onlineSizeFull = (size2 <= 0.0) ? ErrorSizeCode : size2;
            if (onlineSize1Month.load() != DefaultInitSize)
            {
                isFetchingSizes = false;
            }
        }).detach();
    }
}

void DatabaseService::StartSchemaUpdate()
{
    if (isUpdatingSchema.load())
    {
        return;
    }

    isUpdatingSchema = true;
    schemaProgress = 0.0f;

    std::thread([this]() {
        std::filesystem::path destPath = std::filesystem::path(downloadDir) / "galaxy.schema.json";
        auto schemaDownloader = std::make_shared<HttpDownloader>();
        bool success = schemaDownloader->Download("https://docs.spansh.co.uk/galaxy.schema.json", destPath.string(), 1);

        isUpdatingSchema = false;
        if (!success)
        {
            NotifySchemaUpdateFailed("Failed to download schema.");
            return;
        }

        NotifySchemaUpdateCompleted();
    }).detach();
}

void DatabaseService::StartExtractionAndValidation()
{
    if (isExtracting.load() || isValidating.load())
    {
        return;
    }

    isExtracting = true;
    isValidating = false;
    extractionProgress = 0.0f;
    validationProgress = 0.0f;
    cancelExtractionFlag = false;

    std::thread([this]() {
        std::filesystem::path gzPath(currentFilePath);
        std::filesystem::path jsonPath = gzPath;
        jsonPath.replace_extension("");
        std::filesystem::path indexPath = jsonPath;
        indexPath.replace_extension(".idx");
        std::filesystem::path schemaPath = std::filesystem::path(downloadDir) / "galaxy.schema.json";

        extractionStartTime = std::chrono::steady_clock::now();

        int bufSize = SettingsService::GetInstance().GetBufferSizeMb();

        isIndexing = true;
        indexingProgress = 0.0f;
        cancelIndexingFlag = false;

        std::thread notifier([this]() {
            float lastExtract = -1.0f;
            float lastIndex = -1.0f;
            while (isExtracting.load())
            {
                float e = extractionProgress.load();
                float i = indexingProgress.load();
                if (e != lastExtract)
                {
                    lastExtract = e;
                    NotifyExtractionProgress(e);
                }
                if (i != lastIndex)
                {
                    lastIndex = i;
                    NotifyIndexingProgress(i);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });

        StreamingProcessor::Result res = StreamingProcessor::Process(
            gzPath.string(), schemaPath.string(), indexPath.string(),
            extractionProgress, indexingProgress, cancelExtractionFlag, bufSize);

        isExtracting = false;
        isIndexing = false;
        if (notifier.joinable()) notifier.join();

        if (res.cancelled)
        {
            std::error_code ec;
            std::filesystem::remove(indexPath, ec);
            return;
        }

        if (!res.success)
        {
            std::error_code ec;
            std::filesystem::remove(indexPath, ec);
            NotifyDatabaseFailed(res.errorMessage);
            return;
        }

        currentFilePath = jsonPath.string();
        NotifyIndexingCompleted();
        NotifyDatabaseReady(currentFilePath);
    }).detach();
}

void DatabaseService::CancelExtraction()
{
    cancelExtractionFlag = true;
    extractionProgress = 0.0f;
    validationProgress = 0.0f;
}

void DatabaseService::StartIndexing()
{
    if (isIndexing.load())
    {
        return;
    }

    isIndexing = true;
    indexingProgress = 0.0f;
    cancelIndexingFlag = false;

    std::thread([this]() {
        std::filesystem::path jsonPath(currentFilePath);
        std::filesystem::path indexPath = jsonPath;
        indexPath.replace_extension(".idx");

        bool success = IndexBuilder::Build(jsonPath.string(), indexPath.string(), indexingProgress, cancelIndexingFlag);
        isIndexing = false;

        if (cancelIndexingFlag.load() || !success)
        {
            std::error_code ec;
            std::filesystem::remove(indexPath, ec);
            if (!cancelIndexingFlag.load())
            {
                NotifyIndexingFailed("Indexing failed. The JSON file could not be parsed.");
            }
            return;
        }

        currentFilePath = jsonPath.string();
        NotifyIndexingCompleted();
        NotifyDatabaseReady(currentFilePath);
    }).detach();
}

void DatabaseService::CancelIndexing()
{
    cancelIndexingFlag = true;
    indexingProgress = 0.0f;
}

void DatabaseService::EnterApplicationFlow()
{
    if (currentFilePath.empty())
    {
        return;
    }

    std::filesystem::path path(currentFilePath);
    if (path.extension() == ".gz")
    {
        std::filesystem::path jsonPath = path;
        jsonPath.replace_extension("");
        std::filesystem::path idxPath = jsonPath;
        idxPath.replace_extension(".idx");
        if (std::filesystem::exists(idxPath) && IsIndexCurrent(idxPath))
        {
            currentFilePath = jsonPath.string();
            NotifyDatabaseReady(currentFilePath);
        }
        else if (std::filesystem::exists(idxPath))
        {
            pendingRegen = PendingRegen::Extraction;
            NotifyIndexOutdated();
        }
        else
        {
            StartExtractionAndValidation();
        }
    }
    else
    {
        std::filesystem::path idxPath = path;
        idxPath.replace_extension(".idx");
        if (std::filesystem::exists(idxPath) && IsIndexCurrent(idxPath))
        {
            NotifyDatabaseReady(currentFilePath);
        }
        else if (std::filesystem::exists(idxPath))
        {
            pendingRegen = PendingRegen::Indexing;
            NotifyIndexOutdated();
        }
        else
        {
            StartIndexing();
        }
    }
}

void DatabaseService::ConfirmIndexRegeneration()
{
    std::filesystem::path path(currentFilePath);
    std::filesystem::path idxPath = path;
    if (path.extension() == ".gz")
    {
        idxPath.replace_extension("");
        idxPath.replace_extension(".idx");
    }
    else
    {
        idxPath.replace_extension(".idx");
    }
    std::error_code ec;
    std::filesystem::remove(idxPath, ec);

    PendingRegen action = pendingRegen;
    pendingRegen = PendingRegen::None;
    if (action == PendingRegen::Extraction)
    {
        StartExtractionAndValidation();
    }
    else if (action == PendingRegen::Indexing)
    {
        StartIndexing();
    }
}

void DatabaseService::OnDownloadStarted(double totalSize)
{
    totalFileSize = totalSize;
    NotifyDownloadStarted(totalSize);
}

void DatabaseService::OnDownloadProgress(double progress, double speed)
{
    downloadProgress = static_cast<float>(progress);
    downloadSpeed = speed;
    NotifyDownloadProgress(progress, speed);
}

void DatabaseService::OnDownloadCompleted(const std::string&)
{
}

void DatabaseService::OnDownloadFailed(const std::string& error)
{
    NotifyDownloadFailed(error);
}

void DatabaseService::NotifyDownloadStarted(double totalSize)
{
    for (auto observer : observers)
    {
        observer->OnDownloadStarted(totalSize);
    }
}

void DatabaseService::NotifyDownloadProgress(double progress, double speed)
{
    for (auto observer : observers)
    {
        observer->OnDownloadProgress(progress, speed);
    }
}

void DatabaseService::NotifyDownloadCompleted()
{
    for (auto observer : observers)
    {
        observer->OnDownloadCompleted();
    }
}

void DatabaseService::NotifyDownloadFailed(const std::string& error)
{
    for (auto observer : observers)
    {
        observer->OnDownloadFailed(error);
    }
}

void DatabaseService::NotifySchemaUpdateProgress(float progress)
{
    for (auto observer : observers)
    {
        observer->OnSchemaUpdateProgress(progress);
    }
}

void DatabaseService::NotifySchemaUpdateCompleted()
{
    for (auto observer : observers)
    {
        observer->OnSchemaUpdateCompleted();
    }
}

void DatabaseService::NotifySchemaUpdateFailed(const std::string& error)
{
    for (auto observer : observers)
    {
        observer->OnSchemaUpdateFailed(error);
    }
}

void DatabaseService::NotifyExtractionProgress(float progress)
{
    if (cancelExtractionFlag.load())
    {
        return;
    }
    for (auto observer : observers)
    {
        observer->OnExtractionProgress(progress);
    }
}

void DatabaseService::NotifyValidationProgress(float progress)
{
    if (cancelExtractionFlag.load())
    {
        return;
    }
    for (auto observer : observers)
    {
        observer->OnValidationProgress(progress);
    }
}

void DatabaseService::NotifyDatabaseReady(const std::string& dbPath)
{
    for (auto observer : observers)
    {
        observer->OnDatabaseReady(dbPath);
    }
}

void DatabaseService::NotifyDatabaseFailed(const std::string& error)
{
    for (auto observer : observers)
    {
        observer->OnDatabaseFailed(error);
    }
}

void DatabaseService::NotifyIndexingProgress(float progress)
{
    if (cancelIndexingFlag.load())
    {
        return;
    }
    for (auto observer : observers)
    {
        observer->OnIndexingProgress(progress);
    }
}

void DatabaseService::NotifyIndexingCompleted()
{
    for (auto observer : observers)
    {
        observer->OnIndexingCompleted();
    }
}

void DatabaseService::NotifyIndexingFailed(const std::string& error)
{
    for (auto observer : observers)
    {
        observer->OnIndexingFailed(error);
    }
}

void DatabaseService::NotifyIndexOutdated()
{
    for (auto observer : observers)
    {
        observer->OnIndexOutdated();
    }
}

double DatabaseService::GetExtractionTimeRemaining() const
{
    float progress = extractionProgress.load();
    if (progress <= 0.001f)
    {
        return -1.0;
    }

    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - extractionStartTime).count() / 1000.0;
    if (elapsed <= 0.1)
    {
        return -1.0;
    }

    double totalTime = elapsed / progress;
    double remaining = totalTime - elapsed;
    return (remaining < 0.0) ? 0.0 : remaining;
}
