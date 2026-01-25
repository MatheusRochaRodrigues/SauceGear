#include "FileDialog.h"

#include <windows.h>
#include <shobjidl.h> // IFileOpenDialog
#include <filesystem>

std::string FileDialog::Open(const char* filter) {
    std::string result;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
        return result;

    IFileOpenDialog* dialog = nullptr;
    hr = CoCreateInstance(
        CLSID_FileOpenDialog,
        nullptr,
        CLSCTX_ALL,
        IID_PPV_ARGS(&dialog)
    );

    if (FAILED(hr)) {
        CoUninitialize();
        return result;
    }

    // Converte "*.png;*.jpg" → filtro COMDLG
    COMDLG_FILTERSPEC spec[1];
    spec[0].pszName = L"Image Files";
    spec[0].pszSpec = L"*.png;*.jpg;*.jpeg;*.tga";

    dialog->SetFileTypes(1, spec);
    dialog->SetOptions(FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST);

    hr = dialog->Show(nullptr);
    if (SUCCEEDED(hr)) {
        IShellItem* item = nullptr;
        if (SUCCEEDED(dialog->GetResult(&item))) {
            PWSTR path = nullptr;
            if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                result = std::filesystem::path(path).string();
                CoTaskMemFree(path);
            }
            item->Release();
        }
    }

    dialog->Release();
    CoUninitialize();

    return result;
}
