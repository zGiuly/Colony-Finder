#include "AppController.h"
#include "download/HttpDownloader.h"
#include "ui/states/IAppState.h"
#include "ui/states/WelcomeState.h"
#include "ui/states/SelectDownloadState.h"
#include "ui/states/DownloadingState.h"
#include "ui/states/MainAppState.h"
#include "ui/states/ErrorState.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <filesystem>
#include <thread>

constexpr int WindowWidth = 1280;
constexpr int WindowHeight = 720;
constexpr int ThreadCount = 4;
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
      isFetchingSizes(false)
{
    downloader->AddObserver(this);
}

AppController::~AppController()
{
    downloader->RemoveObserver(this);
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

    CheckLocalDump();

    return true;
}

void AppController::CheckLocalDump()
{
    std::filesystem::path p1 = std::filesystem::path(searchDir) / "galaxy.json.gz";
    std::filesystem::path p2 = std::filesystem::path(searchDir) / "galaxy_1month.json.gz";

    if (std::filesystem::exists(p1))
    {
        currentFilePath = p1.string();
        return;
    }

    if (std::filesystem::exists(p2))
    {
        currentFilePath = p2.string();
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
        TransitionTo(std::make_unique<MainAppState>());
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
        onlineSize1Month = size1;

        double size2 = HttpDownloader::GetContentLength("https://downloads.spansh.co.uk/galaxy.json.gz");
        onlineSizeFull = size2;

        isFetchingSizes = false;
    }).detach();
}
