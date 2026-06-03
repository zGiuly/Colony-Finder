#include "AppController.h"
#include "download/HttpDownloader.h"
#include "download/GzipDecompressor.h"
#include "download/JsonStreamValidator.h"
#include <nlohmann/json.hpp>
#include "ui/states/IAppState.h"
#include "ui/states/WelcomeState.h"
#include "ui/states/SelectDownloadState.h"
#include "ui/states/DownloadingState.h"
#include "ui/states/ExtractionState.h"
#include "ui/states/SchemaUpdateState.h"
#include "ui/states/MainAppState.h"
#include "ui/states/ErrorState.h"
#include "ui/SettingsService.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <filesystem>
#include <thread>

constexpr int WindowWidth = 1280;
constexpr int WindowHeight = 720;
constexpr int ThreadCount = 1;
constexpr float CardWidth = 760.0f;
constexpr float CardHeight = 460.0f;

AppController::AppController()
    : window(nullptr),
      currentState(std::make_unique<WelcomeState>()),
      downloader(std::make_shared<HttpDownloader>()),
      downloadProgress(0.0f),
      downloadSpeed(0.0f),
      totalFileSize(0.0f),
      downloadDir("."),
      searchDir("."),
      isBusy(false),
      onlineSize1Month(-1.0),
      onlineSizeFull(-1.0),
      isFetchingSizes(false),
      schemaProgress(0.0f),
      isUpdatingSchema(false),
      extractionProgress(0.0f),
      validationProgress(0.0f),
      isExtracting(false),
      isValidating(false),
      cancelExtractionFlag(false)
{
    downloader->AddObserver(this);
    SettingsService::GetInstance().AddObserver(this);
}

AppController::~AppController()
{
    downloader->RemoveObserver(this);
    SettingsService::GetInstance().RemoveObserver(this);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    if (window)
    {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

bool AppController::Initialize()
{
    if (!glfwInit())
    {
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(WindowWidth, WindowHeight, "ColonyFinder - Elite Dangerous Tool", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    AppTheme::Apply(theme);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    SettingsService::GetInstance().Load();
    CheckLocalDump();

    std::filesystem::path schemaPath = std::filesystem::path(downloadDir) / "galaxy.schema.json";
    if (!std::filesystem::exists(schemaPath))
    {
        StartSchemaUpdate();
    }

    return true;
}

void AppController::CheckLocalDump()
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

    currentFilePath.clear();
}

void AppController::StartDownload(const std::string& url)
{
    if (isBusy.load())
    {
        return;
    }

    isBusy = true;
    downloadProgress = 0.0f;
    downloadSpeed = 0.0f;
    TransitionTo(std::make_unique<DownloadingState>());

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
        StartExtractionAndValidation();
    }).detach();
}

void AppController::OnDownloadStarted(double totalSize)
{
    totalFileSize = totalSize;
}

void AppController::OnDownloadProgress(double progress, double speed)
{
    downloadProgress = static_cast<float>(progress);
    downloadSpeed = speed;
}

void AppController::OnDownloadCompleted(const std::string&)
{
}

void AppController::OnDownloadFailed(const std::string& error)
{
    if (downloader->IsCancelled())
    {
        TransitionTo(std::make_unique<SelectDownloadState>());
        return;
    }
    errorMessage = error;
    TransitionTo(std::make_unique<ErrorState>());
}

void AppController::CancelDownload()
{
    downloader->Cancel();
}

void AppController::TransitionTo(std::unique_ptr<IAppState> nextState)
{
    currentState = std::move(nextState);
}

void AppController::Run()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        RenderUI();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        
        ImVec4 clearColor = theme.bgDark;
        glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
}

void AppController::RenderUI()
{
    ImVec2 viewportSize = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2((viewportSize.x - CardWidth) * 0.5f, (viewportSize.y - CardHeight) * 0.5f));
    ImGui::SetNextWindowSize(ImVec2(CardWidth, CardHeight));
    
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
    ImGui::Begin("CardWindow", nullptr, windowFlags);

    if (currentState)
    {
        currentState->Render(this);
    }

    ImGui::End();
}

void AppController::FetchOnlineSizes()
{
    if (isFetchingSizes.load())
    {
        return;
    }

    isFetchingSizes = true;
    onlineSize1Month = -1.0;
    onlineSizeFull = -1.0;

    std::thread([this]() {
        double size1 = HttpDownloader::GetContentLength("https://downloads.spansh.co.uk/galaxy_1month.json.gz");
        onlineSize1Month = (size1 <= 0.0) ? -2.0 : size1;

        double size2 = HttpDownloader::GetContentLength("https://downloads.spansh.co.uk/galaxy.json.gz");
        onlineSizeFull = (size2 <= 0.0) ? -2.0 : size2;

        isFetchingSizes = false;
    }).detach();
}

void AppController::StartSchemaUpdate()
{
    if (isUpdatingSchema.load())
    {
        return;
    }

    isUpdatingSchema = true;
    schemaProgress = 0.0f;
    TransitionTo(std::make_unique<SchemaUpdateState>());

    std::thread([this]() {
        std::filesystem::path destPath = std::filesystem::path(downloadDir) / "galaxy.schema.json";
        auto schemaDownloader = std::make_shared<HttpDownloader>();
        bool success = schemaDownloader->Download("https://docs.spansh.co.uk/galaxy.schema.json", destPath.string(), 1);
        
        isUpdatingSchema = false;
        if (!success)
        {
            errorMessage = "Failed to download schema from https://docs.spansh.co.uk/galaxy.schema.json";
            TransitionTo(std::make_unique<ErrorState>());
            return;
        }

        TransitionTo(std::make_unique<SelectDownloadState>());
    }).detach();
}

void AppController::StartExtractionAndValidation()
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

    TransitionTo(std::make_unique<ExtractionState>());

    std::thread([this]() {
        std::filesystem::path gzPath(currentFilePath);
        std::filesystem::path jsonPath = gzPath;
        jsonPath.replace_extension("");

        bool success = GzipDecompressor::Decompress(gzPath.string(), jsonPath.string(), extractionProgress, cancelExtractionFlag);
        isExtracting = false;

        if (cancelExtractionFlag.load())
        {
            std::error_code ec;
            std::filesystem::remove(jsonPath, ec);
            return;
        }

        if (!success)
        {
            std::error_code ec;
            std::filesystem::remove(jsonPath, ec);
            errorMessage = "Decompression failed. The downloaded file may be corrupted.";
            TransitionTo(std::make_unique<ErrorState>());
            return;
        }

        isValidating = true;
        std::filesystem::path schemaPath = std::filesystem::path(downloadDir) / "galaxy.schema.json";

        bool valid = JsonStreamValidator::Validate(jsonPath.string(), schemaPath.string(), validationProgress, cancelExtractionFlag);
        isValidating = false;

        if (cancelExtractionFlag.load() || !valid)
        {
            std::error_code ec;
            std::filesystem::remove(jsonPath, ec);
            if (cancelExtractionFlag.load())
            {
                return;
            }
            errorMessage = "Schema validation failed. The JSON structure is invalid or outdated.";
            TransitionTo(std::make_unique<ErrorState>());
            return;
        }

        currentFilePath = jsonPath.string();
        TransitionTo(std::make_unique<MainAppState>());
    }).detach();
}

void AppController::CancelExtraction()
{
    cancelExtractionFlag = true;
}

void AppController::SetDownloadDir(const std::string& path)
{
    SettingsService::GetInstance().SetDownloadDir(path);
}

void AppController::SetSearchDir(const std::string& path)
{
    SettingsService::GetInstance().SetSearchDir(path);
}

void AppController::OnSettingsChanged(const std::string& downloadDirVal, const std::string& searchDirVal)
{
    downloadDir = downloadDirVal;
    searchDir = searchDirVal;
}
