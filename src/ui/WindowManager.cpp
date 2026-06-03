#include "ui/WindowManager.h"
#include "ui/AppController.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

constexpr const char* GlslVersion = "#version 130";
constexpr int GlContextVersionMajor = 3;
constexpr int GlContextVersionMinor = 0;
constexpr int GlSwapInterval = 1;

WindowManager::WindowManager()
    : window(nullptr)
{
}

WindowManager::~WindowManager()
{
    Shutdown();
}

bool WindowManager::Initialize(const std::string& title, int width, int height)
{
    if (!glfwInit())
    {
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GlContextVersionMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GlContextVersionMinor);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(GlSwapInterval);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(GlslVersion);

    return true;
}

void WindowManager::Run(AppController* controller)
{
    if (!window)
    {
        return;
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        controller->RenderUI();

        ImGui::Render();
        int display_w = 0;
        int display_h = 0;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        ImVec4 clearColor = controller->GetTheme().bgDark;
        glClearColor(clearColor.x * clearColor.w, clearColor.y * clearColor.w, clearColor.z * clearColor.w, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
}

void WindowManager::Shutdown()
{
    if (!window)
    {
        return;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    window = nullptr;
    glfwTerminate();
}
