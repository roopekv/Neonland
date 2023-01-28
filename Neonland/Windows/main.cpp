#include "pch.h"
#include "App.hpp"

using namespace winrt::Windows::ApplicationModel::Core;

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
//#if defined(DEBUG) || defined(_DEBUG)
//    AllocConsole();
//    FILE* debugConsole;
//    freopen_s(&debugConsole, "CONOUT$", "w", stdout);
//    freopen_s(&debugConsole, "CONOUT$", "w", stderr);
//    freopen_s(&debugConsole, "CONIN$", "r", stdin);
//    std::cout.clear();
//    std::clog.clear();
//    std::cerr.clear();
//    std::cin.clear();
//#endif
    CoreApplication::Run(winrt::make<App>());
}