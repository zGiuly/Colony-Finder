#pragma once
#include <string>

struct GLFWwindow;
class AppController;

class WindowManager
{
public:
    WindowManager();
    ~WindowManager();

    bool Initialize(const std::string& title, int width, int height);
    void Run(AppController* controller);
    void Shutdown();

    GLFWwindow* GetWindow() const { return window; }

private:
    GLFWwindow* window;
};
