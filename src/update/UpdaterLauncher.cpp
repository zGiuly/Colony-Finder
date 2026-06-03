#include "update/UpdaterLauncher.h"
#include <filesystem>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#endif

std::string UpdaterLauncher::GetExecutablePath()
{
#ifdef _WIN32
    wchar_t buffer[MAX_PATH];
    DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (len == 0 || len == MAX_PATH)
    {
        return {};
    }
    int size = WideCharToMultiByte(CP_UTF8, 0, buffer, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0)
    {
        return {};
    }
    std::string out(size - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, buffer, -1, out.data(), size, nullptr, nullptr);
    return out;
#else
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, PATH_MAX - 1);
    if (len <= 0)
    {
        return {};
    }
    buffer[len] = '\0';
    return std::string(buffer);
#endif
}

std::string UpdaterLauncher::GetUpdaterPath()
{
    std::string exe = GetExecutablePath();
    if (exe.empty())
    {
        return {};
    }
    std::filesystem::path dir = std::filesystem::path(exe).parent_path();
#ifdef _WIN32
    return (dir / "colony_updater.exe").string();
#else
    return (dir / "colony_updater").string();
#endif
}

bool UpdaterLauncher::LaunchUpdater(const std::string& newBinaryPath, const std::string& targetBinaryPath)
{
    std::string updater = GetUpdaterPath();
    if (updater.empty() || !std::filesystem::exists(updater))
    {
        return false;
    }

#ifdef _WIN32
    DWORD pid = GetCurrentProcessId();
    std::string cmdline = "\"" + updater + "\" " + std::to_string(pid)
        + " \"" + newBinaryPath + "\" \"" + targetBinaryPath + "\"";

    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};
    std::vector<char> mutableCmd(cmdline.begin(), cmdline.end());
    mutableCmd.push_back('\0');

    BOOL ok = CreateProcessA(nullptr, mutableCmd.data(), nullptr, nullptr, FALSE,
                             CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi);
    if (!ok)
    {
        return false;
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
#else
    pid_t pid = fork();
    if (pid < 0)
    {
        return false;
    }
    if (pid == 0)
    {
        std::string ppid = std::to_string(getppid());
        execl(updater.c_str(), updater.c_str(), ppid.c_str(),
              newBinaryPath.c_str(), targetBinaryPath.c_str(), nullptr);
        _exit(127);
    }
    return true;
#endif
}
