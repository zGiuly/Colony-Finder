#include "ui/FolderDialog.h"

#ifdef _WIN32
#include <windows.h>
#include <shobjidl.h>

std::string SelectFolderDialog()
{
    std::string path;
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (!SUCCEEDED(hr))
    {
        return path;
    }

    IFileOpenDialog* pDlg = nullptr;
    hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pDlg));
    if (SUCCEEDED(hr))
    {
        DWORD dwOptions;
        if (SUCCEEDED(pDlg->GetOptions(&dwOptions)))
        {
            pDlg->SetOptions(dwOptions | FOS_PICKFOLDERS);
        }

        if (SUCCEEDED(pDlg->Show(nullptr)))
        {
            IShellItem* pItem = nullptr;
            if (SUCCEEDED(pDlg->GetResult(&pItem)))
            {
                PWSTR pszPath = nullptr;
                if (SUCCEEDED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath)))
                {
                    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, pszPath, -1, nullptr, 0, nullptr, nullptr);
                    if (sizeNeeded > 1)
                    {
                        path.resize(sizeNeeded - 1);
                        WideCharToMultiByte(CP_UTF8, 0, pszPath, -1, &path[0], sizeNeeded, nullptr, nullptr);
                    }
                    CoTaskMemFree(pszPath);
                }
                pItem->Release();
            }
        }
        pDlg->Release();
    }
    CoUninitialize();
    return path;
}
#else
#include <cstdio>
#include <memory>
#include <array>

std::string SelectFolderDialog()
{
    std::string path;
    std::array<char, 512> buffer;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("zenity --file-selection --directory 2>/dev/null", "r"), pclose);
    if (pipe)
    {
        if (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            path = buffer.data();
            if (!path.empty() && path.back() == '\n')
            {
                path.pop_back();
            }
        }
    }
    return path;
}
#endif
