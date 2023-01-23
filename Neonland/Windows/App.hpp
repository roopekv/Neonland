#pragma once

#include "pch.h"
#include "DeviceResources.hpp"
#include "NeonMain.hpp"

class App : winrt::implements<App, winrt::Windows::ApplicationModel::Core::IFrameworkViewSource, winrt::Windows::ApplicationModel::Core::IFrameworkView>
{
public:
    App();
    winrt::Windows::ApplicationModel::Core::IFrameworkView CreateView();

    void Initialize(winrt::Windows::ApplicationModel::Core::CoreApplicationView const& applicationView);
    void SetWindow(winrt::Windows::UI::Core::CoreWindow const& window);
    void Load(winrt::hstring const&);
    void Run();
    void Uninitialize();
protected:
    void OnActivated(winrt::Windows::ApplicationModel::Core::CoreApplicationView const&, winrt::Windows::ApplicationModel::Activation::IActivatedEventArgs const&);
    winrt::fire_and_forget OnSuspending(IInspectable const&, SuspendingEventArgs const& args);
    void OnResuming(IInspectable const&, IInspectable const&);


    void OnWindowSizeChanged(winrt::Windows::UI::Core::CoreWindow const&, winrt::Windows::UI::Core::WindowSizeChangedEventArgs const& args);
    void OnVisibilityChanged(winrt::Windows::UI::Core::CoreWindow const&, winrt::Windows::UI::Core::VisibilityChangedEventArgs const& args);
    void OnWindowClosed(winrt::Windows::UI::Core::CoreWindow const&, winrt::Windows::UI::Core::CoreWindowEventArgs const&);

    void OnWindowActivationChanged(winrt::Windows::UI::Core::CoreWindow const&, winrt::Windows::UI::Core::WindowActivatedEventArgs const& args);

    void OnDpiChanged(winrt::Windows::Graphics::Display::DisplayInformation const& sender, IInspectable const&);
    void OnOrientationChanged(winrt::Windows::Graphics::Display::DisplayInformation const& sender, IInspectable const&);
    void OnDisplayContentsInvalidated(winrt::Windows::Graphics::Display::DisplayInformation const&, IInspectable const&);

private:
    std::shared_ptr<DeviceResources> GetDeviceResources();

    std::shared_ptr<DeviceResources> m_deviceResources;
    winrt::com_ptr<NeonMain> m_main;

    bool m_windowClosed;
    bool m_windowVisible;
};