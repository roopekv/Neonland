#include "pch.h"
#include "App.hpp"

using namespace winrt::Windows::ApplicationModel::Core;

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    CoreApplication::Run(winrt::make<App>());
}