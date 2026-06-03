#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <chrono>
#include <filesystem>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

namespace
{
    void WaitForParentExit(const std::string& pidStr)
    {
#ifdef _WIN32
        DWORD pid = static_cast<DWORD>(std::stoul(pidStr));
        HANDLE h = OpenProcess(SYNCHRONIZE, FALSE, pid);
        if (!h)
        {
            return;
        }
        WaitForSingleObject(h, 30000);
        CloseHandle(h);
#else
        pid_t pid = static_cast<pid_t>(std::stoi(pidStr));
        for (int i = 0; i < 300; ++i)
        {
            if (kill(pid, 0) != 0)
            {
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
#endif
    }

    bool ReplaceBinary(const std::string& src, const std::string& dst)
    {
        std::error_code ec;
        for (int attempt = 0; attempt < 20; ++attempt)
        {
            std::filesystem::remove(dst, ec);
            std::filesystem::rename(src, dst, ec);
            if (!ec)
            {
                return true;
            }
            std::filesystem::copy_file(src, dst,
                std::filesystem::copy_options::overwrite_existing, ec);
            if (!ec)
            {
                std::error_code rmEc;
            std::filesystem::remove(src, rmEc);
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
        return false;
    }

    void RelaunchApp(const std::string& path)
    {
#ifdef _WIN32
        STARTUPINFOA si = {};
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi = {};
        std::string cmd = "\"" + path + "\"";
        std::vector<char> buf(cmd.begin(), cmd.end());
        buf.push_back('\0');
        if (CreateProcessA(nullptr, buf.data(), nullptr, nullptr, FALSE,
                           DETACHED_PROCESS, nullptr, nullptr, &si, &pi))
        {
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
#else
        chmod(path.c_str(), 0755);
        pid_t pid = fork();
        if (pid == 0)
        {
            execl(path.c_str(), path.c_str(), nullptr);
            _exit(127);
        }
#endif
    }
}

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        std::fprintf(stderr, "usage: colony_updater <parent_pid> <new_binary> <target_binary>\n");
        return 1;
    }

    std::string parentPid = argv[1];
    std::string newBinary = argv[2];
    std::string targetBinary = argv[3];

    WaitForParentExit(parentPid);

    if (!ReplaceBinary(newBinary, targetBinary))
    {
        std::fprintf(stderr, "failed to replace binary\n");
        return 2;
    }

    RelaunchApp(targetBinary);
    return 0;
}
