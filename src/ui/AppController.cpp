#include "ui/AppController.h"
#include "ui/WindowManager.h"
#include "download/DatabaseService.h"
#include "ui/SettingsService.h"
#include "ui/states/IAppState.h"
#include "ui/states/WelcomeState.h"
#include "ui/states/SelectDownloadState.h"
#include "ui/states/DownloadingState.h"
#include "ui/states/ExtractionState.h"
#include "ui/states/IndexOutdatedState.h"
#include "ui/states/SchemaUpdateState.h"
#include "ui/states/MainAppState.h"
#include "ui/states/ErrorState.h"
#include "imgui.h"
#include <filesystem>

constexpr int WindowWidth = 1600;
constexpr int WindowHeight = 900;
constexpr float CardWidth = 1520.0f;
constexpr float CardHeight = 820.0f;

AppController::AppController()
    : currentState(std::make_unique<WelcomeState>()),
      windowManager(std::make_unique<WindowManager>())
{
    DatabaseService::GetInstance().AddObserver(this);
    SettingsService::GetInstance().AddObserver(this);
}

AppController::~AppController()
{
    DatabaseService::GetInstance().RemoveObserver(this);
    SettingsService::GetInstance().RemoveObserver(this);
}

bool AppController::Initialize()
{
    SettingsService::GetInstance().Load();
    
    DatabaseService::GetInstance().Initialize(
        SettingsService::GetInstance().GetDownloadDir(),
        SettingsService::GetInstance().GetSearchDir()
    );

    if (!windowManager->Initialize("ColonyFinder - Elite Dangerous Tool", WindowWidth, WindowHeight))
    {
        return false;
    }

    AppTheme::Apply(theme);

    std::filesystem::path schemaPath = std::filesystem::path(SettingsService::GetInstance().GetDownloadDir()) / "galaxy.schema.json";
    if (!std::filesystem::exists(schemaPath))
    {
        DatabaseService::GetInstance().StartSchemaUpdate();
        TransitionTo(std::make_unique<SchemaUpdateState>());
    }

    return true;
}

void AppController::Run()
{
    windowManager->Run(this);
}

void AppController::TransitionTo(std::unique_ptr<IAppState> nextState)
{
    currentState = std::move(nextState);
}

void AppController::RenderUI()
{
    ImVec2 viewportSize = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2((viewportSize.x - CardWidth) * 0.5f, (viewportSize.y - CardHeight) * 0.5f));
    ImGui::SetNextWindowSize(ImVec2(CardWidth, CardHeight));

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    ImGui::Begin("CardWindow", nullptr, windowFlags);

    if (currentState)
    {
        currentState->Render(this);
    }

    ImGui::End();
}

void AppController::OnDownloadStarted(double)
{
}

void AppController::OnDownloadProgress(double, double)
{
}

void AppController::OnDownloadCompleted()
{
}

void AppController::OnDownloadFailed(const std::string& error)
{
    errorMessage = error;
    TransitionTo(std::make_unique<ErrorState>());
}

void AppController::OnSchemaUpdateProgress(float)
{
}

void AppController::OnSchemaUpdateCompleted()
{
    TransitionTo(std::make_unique<SelectDownloadState>());
}

void AppController::OnSchemaUpdateFailed(const std::string& error)
{
    errorMessage = error;
    TransitionTo(std::make_unique<ErrorState>());
}

void AppController::OnExtractionProgress(float)
{
    if (!dynamic_cast<ExtractionState*>(currentState.get()))
    {
        TransitionTo(std::make_unique<ExtractionState>());
    }
}

void AppController::OnValidationProgress(float)
{
    if (!dynamic_cast<ExtractionState*>(currentState.get()))
    {
        TransitionTo(std::make_unique<ExtractionState>());
    }
}

void AppController::OnDatabaseReady(const std::string&)
{
    TransitionTo(std::make_unique<MainAppState>());
}

void AppController::OnDatabaseFailed(const std::string& error)
{
    errorMessage = error;
    TransitionTo(std::make_unique<ErrorState>());
}

void AppController::OnIndexingProgress(float)
{
    if (!dynamic_cast<ExtractionState*>(currentState.get()))
    {
        TransitionTo(std::make_unique<ExtractionState>());
    }
}

void AppController::OnIndexingCompleted()
{
}

void AppController::OnIndexingFailed(const std::string& error)
{
    errorMessage = error;
    TransitionTo(std::make_unique<ErrorState>());
}

void AppController::OnIndexOutdated()
{
    TransitionTo(std::make_unique<IndexOutdatedState>());
}

void AppController::OnSettingsChanged(const std::string& downloadDirVal, const std::string& searchDirVal)
{
    DatabaseService::GetInstance().SetPaths(downloadDirVal, searchDirVal);
}
