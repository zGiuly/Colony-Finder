#include "ui/AppController.h"
#include "ui/WindowManager.h"
#include "download/DatabaseService.h"
#include "ui/SettingsService.h"
#include "ui/states/IAppState.h"
#include "ui/states/WelcomeState.h"
#include "ui/states/SelectDownloadState.h"
#include "ui/states/DownloadingState.h"
#include "ui/states/DownloadIndexState.h"
#include "ui/states/DownloadingIndexState.h"
#include "ui/UiStrings.h"
#include "ui/states/ExtractionState.h"
#include "ui/states/IndexOutdatedState.h"
#include "ui/states/SchemaUpdateState.h"
#include "ui/states/MainAppState.h"
#include "ui/states/ErrorState.h"
#include "ui/states/UpdateAvailableState.h"
#include "update/UpdateService.h"
#include "update/GithubUpdateChecker.h"
#include "Version.h"
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
    UpdateService::GetInstance().AddObserver(this);
}

AppController::~AppController()
{
    DatabaseService::GetInstance().RemoveObserver(this);
    SettingsService::GetInstance().RemoveObserver(this);
    UpdateService::GetInstance().RemoveObserver(this);
}

bool AppController::Initialize()
{
    SettingsService::GetInstance().Load();
    
    DatabaseService::GetInstance().Initialize(
        SettingsService::GetInstance().GetDownloadDir(),
        SettingsService::GetInstance().GetSearchDir()
    );

    if (!windowManager->Initialize(UiStrings::App::WindowTitle, WindowWidth, WindowHeight))
    {
        return false;
    }

    AppTheme::Apply(theme);

#ifdef _WIN32
    const std::string updateAsset = "ColonyFinder-windows-x64.exe";
#else
    const std::string updateAsset = "ColonyFinder-linux-x64";
#endif
    UpdateService::GetInstance().Configure(
        std::make_unique<GithubUpdateChecker>("zGiuly", "Colony-Finder", updateAsset),
        COLONYFINDER_VERSION_STRING);
    UpdateService::GetInstance().CheckAsync();

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
    bool fromPrebuiltIndex = dynamic_cast<DownloadingIndexState*>(currentState.get()) != nullptr
                          || dynamic_cast<DownloadIndexState*>(currentState.get()) != nullptr;
    if (!fromPrebuiltIndex)
    {
        TransitionTo(std::make_unique<ErrorState>());
        return;
    }
    TransitionTo(std::make_unique<ErrorState>(UiStrings::Error::IndexBackLabel, []() -> std::unique_ptr<IAppState> {
        return std::make_unique<DownloadIndexState>();
    }));
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

void AppController::OnThemeChanged(const ThemeColors& colors)
{
    theme.orangePrimary   = ImVec4(colors.orangePrimary.r,   colors.orangePrimary.g,   colors.orangePrimary.b,   colors.orangePrimary.a);
    theme.orangeMuted     = ImVec4(colors.orangeMuted.r,     colors.orangeMuted.g,     colors.orangeMuted.b,     colors.orangeMuted.a);
    theme.orangeActive    = ImVec4(colors.orangeActive.r,    colors.orangeActive.g,    colors.orangeActive.b,    colors.orangeActive.a);
    theme.bgDark          = ImVec4(colors.bgDark.r,          colors.bgDark.g,          colors.bgDark.b,          colors.bgDark.a);
    theme.bgPanel         = ImVec4(colors.bgPanel.r,         colors.bgPanel.g,         colors.bgPanel.b,         colors.bgPanel.a);
    theme.textNormal      = ImVec4(colors.textNormal.r,      colors.textNormal.g,      colors.textNormal.b,      colors.textNormal.a);
    theme.textMuted       = ImVec4(colors.textMuted.r,       colors.textMuted.g,       colors.textMuted.b,       colors.textMuted.a);
    theme.textAlert       = ImVec4(colors.textAlert.r,       colors.textAlert.g,       colors.textAlert.b,       colors.textAlert.a);
    theme.textSuccess     = ImVec4(colors.textSuccess.r,     colors.textSuccess.g,     colors.textSuccess.b,     colors.textSuccess.a);
    theme.border          = ImVec4(colors.border.r,          colors.border.g,          colors.border.b,          colors.border.a);
    theme.rowHover        = ImVec4(colors.rowHover.r,        colors.rowHover.g,        colors.rowHover.b,        colors.rowHover.a);
    theme.rowHoverActive  = ImVec4(colors.rowHoverActive.r,  colors.rowHoverActive.g,  colors.rowHoverActive.b,  colors.rowHoverActive.a);
    theme.rowSelected     = ImVec4(colors.rowSelected.r,     colors.rowSelected.g,     colors.rowSelected.b,     colors.rowSelected.a);
    if (!ImGui::GetCurrentContext())
    {
        return;
    }
    AppTheme::Apply(theme);
}

void AppController::OnUpdateAvailable(const std::string& latestVersion, const std::string& downloadUrl)
{
    if (!dynamic_cast<WelcomeState*>(currentState.get()))
    {
        return;
    }
    updateReady = false;
    updateProgress = 0.0;
    TransitionTo(std::make_unique<UpdateAvailableState>(latestVersion, downloadUrl));
}

void AppController::OnUpdateNotAvailable()
{
}

void AppController::OnUpdateCheckFailed(const std::string&)
{
}

void AppController::OnUpdateDownloadProgress(double progress)
{
    updateProgress = progress;
}

void AppController::OnUpdateReady(const std::string&)
{
    updateProgress = 1.0;
    updateReady = true;
}

void AppController::OnUpdateFailed(const std::string& error)
{
    errorMessage = error;
    TransitionTo(std::make_unique<ErrorState>());
}
