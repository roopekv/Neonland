#include "pch.h"
#include "App.hpp"

#include "../Neonland.h"

using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::Graphics::Display;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Input;

App::App()
	: _windowClosed{ false }
	, _windowVisible{ true } {}

IFrameworkView App::CreateView() {
	return *this;
}

void App::Initialize(CoreApplicationView const& applicationView)
{
	applicationView.Activated({ this, &App::OnActivated });
}

void App::SetWindow(CoreWindow const& window)
{
	window.PointerCursor(CoreCursor(CoreCursorType::Arrow, 0));
	PointerVisualizationSettings visualizationSettings = PointerVisualizationSettings::GetForCurrentView();
	visualizationSettings.IsContactFeedbackEnabled(false);
	visualizationSettings.IsBarrelButtonFeedbackEnabled(false);

	window.SizeChanged({ this, &App::OnWindowSizeChanged });
	window.Closed({ this, &App::OnWindowClosed });
	window.VisibilityChanged({ this, &App::OnVisibilityChanged });

	DisplayInformation currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation.DpiChanged({ this, &App::OnDpiChanged });
	currentDisplayInformation.OrientationChanged({ this, &App::OnOrientationChanged });
	DisplayInformation::DisplayContentsInvalidated({ this, &App::OnDisplayContentsInvalidated });
}

void App::Load(winrt::hstring const&)
{
	if (_main == nullptr)
	{
		_main = std::make_unique<NeonMain>();
	}
}

void App::Run()
{
	while (!_windowClosed && !Neon_AppShouldQuit() && GetDeviceResources() != nullptr)
	{
		if (_windowVisible)
		{
			CoreWindow::GetForCurrentThread().Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
			if (_main->Render())
			{
				GetDeviceResources()->Present();
			}
		}
		else
		{
			CoreWindow::GetForCurrentThread().Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
}

void App::Uninitialize() {}

void App::OnActivated(CoreApplicationView const&, IActivatedEventArgs const&)
{
	CoreWindow window = CoreWindow::GetForCurrentThread();
	window.Activate();
}

void App::OnWindowSizeChanged(CoreWindow const&, WindowSizeChangedEventArgs const& args)
{
	_deviceResources->SetLogicalSize(args.Size());
	_main->OnWindowSizeChanged();
}

void App::OnVisibilityChanged(CoreWindow const&, VisibilityChangedEventArgs const& args)
{
	_windowVisible = args.Visible();
}

void App::OnWindowClosed(CoreWindow const&, CoreWindowEventArgs const&)
{
	_windowClosed = true;
}

void App::OnDpiChanged(DisplayInformation const& sender, IInspectable const&)
{
	GetDeviceResources()->SetDpi(sender.LogicalDpi());
	_main->OnWindowSizeChanged();
}

void App::OnOrientationChanged(DisplayInformation const& sender, IInspectable const&)
{
	_deviceResources->SetCurrentOrientation(sender.CurrentOrientation());
	_main->OnWindowSizeChanged();
}

void App::OnDisplayContentsInvalidated(DisplayInformation const&, IInspectable const&)
{
	GetDeviceResources()->ValidateDevice();
}

std::shared_ptr<DeviceResources> App::GetDeviceResources() {
	if (_deviceResources != nullptr && _deviceResources->IsDeviceRemoved())
	{
		_deviceResources = nullptr;
		_main->OnDeviceRemoved();
	}

	if (_deviceResources == nullptr)
	{
		_deviceResources = std::make_shared<DeviceResources>();
		_deviceResources->SetWindow(CoreWindow::GetForCurrentThread());
		_main->CreateRenderers(_deviceResources);
	}
	return _deviceResources;
}

void App::OnPointerPressed(IInspectable const&, PointerEventArgs const& args)
{

}

void App::OnPointerMoved(IInspectable const&, PointerEventArgs const& args)
{

}