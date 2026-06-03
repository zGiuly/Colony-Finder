#include "logging/Logger.h"
#include "logging/ConsoleLogSink.h"
#include "logging/FileLogSink.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <chrono>
#include <filesystem>
#include <vector>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

namespace fs = std::filesystem;

namespace
{
    void WaitForParentExit(const std::string& pidStr)
    {
#ifdef _WIN32
        DWORD pid = static_cast<DWORD>(std::stoul(pidStr));
        HANDLE h = OpenProcess(SYNCHRONIZE, FALSE, pid);
        if (!h)
        {
            std::this_thread::sleep_for(std::chrono::seconds(2));
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

    bool MovePath(const fs::path& from, const fs::path& to)
    {
        std::error_code ec;
        fs::rename(from, to, ec);
        if (!ec)
        {
            return true;
        }
        ec.clear();
        fs::copy_file(from, to, fs::copy_options::overwrite_existing, ec);
        if (ec)
        {
            return false;
        }
        std::error_code rmEc;
        fs::remove(from, rmEc);
        return true;
    }

    bool ReplaceBinary(const fs::path& src, const fs::path& dst)
    {
        fs::path backup = dst;
        backup += ".old";

        std::error_code ec;
        fs::remove(backup, ec);

        for (int attempt = 0; attempt < 60; ++attempt)
        {
            if (!MovePath(dst, backup))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                continue;
            }

            if (!MovePath(src, dst))
            {
                std::error_code rollbackEc;
                fs::rename(backup, dst, rollbackEc);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                continue;
            }

            std::error_code cleanupEc;
            fs::remove(backup, cleanupEc);
            return true;
        }
        return false;
    }

    void RelaunchApp(const fs::path& path)
    {
#ifdef _WIN32
        STARTUPINFOA si = {};
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi = {};
        std::string cmd = "\"" + path.string() + "\"";
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
    fs::path newBinary = argv[2];
    fs::path targetBinary = argv[3];

    Logger::Instance().SetLevel(LogLevel::Debug);
    Logger::Instance().AddSink(std::make_shared<ConsoleLogSink>());
    auto fileSink = std::make_shared<FileLogSink>((targetBinary.parent_path() / "colony_updater.log").string());
    if (fileSink->IsOpen())
    {
        Logger::Instance().AddSink(fileSink);
    }

    LOG_INFO("updater start: pid=%s new=%s target=%s", parentPid.c_str(), newBinary.string().c_str(), targetBinary.string().c_str());

    if (!fs::exists(newBinary))
    {
        LOG_ERROR("new binary missing: %s", newBinary.string().c_str());
        return 3;
    }

    WaitForParentExit(parentPid);
    LOG_DEBUG("parent exited, starting replace");

    if (!ReplaceBinary(newBinary, targetBinary))
    {
        LOG_ERROR("failed to replace binary after retries");
        return 2;
    }

    LOG_INFO("replace ok, relaunching %s", targetBinary.string().c_str());
    RelaunchApp(targetBinary);
    return 0;
}
