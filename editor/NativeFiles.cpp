#include "NativeFiles.hpp"
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#endif
std::vector<std::filesystem::path> chooseAssetFiles(GLFWwindow* owner) {
    std::vector<std::filesystem::path> result;
#ifdef _WIN32
    std::vector<wchar_t> buffer(65536,0);OPENFILENAMEW dialog{};dialog.lStructSize=sizeof(dialog);dialog.hwndOwner=glfwGetWin32Window(owner);
    dialog.lpstrFilter=L"Supported assets\0*.obj;*.png;*.jpg;*.jpeg;*.lua\0All files\0*.*\0";
    dialog.lpstrFile=buffer.data();dialog.nMaxFile=static_cast<DWORD>(buffer.size());dialog.lpstrTitle=L"Import assets into this project";
    dialog.Flags=OFN_EXPLORER|OFN_ALLOWMULTISELECT|OFN_FILEMUSTEXIST|OFN_NOCHANGEDIR;
    if(GetOpenFileNameW(&dialog)) {
        std::filesystem::path first(buffer.data());auto* next=buffer.data()+first.native().size()+1;
        if(!*next)result.push_back(first);
        else while(*next){std::filesystem::path name(next);result.push_back(first/name);next+=name.native().size()+1;}
    }
#else
    (void)owner;
#endif
    return result;
}
