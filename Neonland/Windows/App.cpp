#include "pch.h"
#include "App.hpp"

#include <iostream>

#include "../Neonland.h"

using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::Graphics::Display;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Input;
using namespace winrt::Windows::UI::ViewManagement;
using namespace winrt::Windows::System;

App::App()
	: _windowClosed{ false }
	, _windowVisible{ true }
	, forwardHeld{ false }
	, backHeld{ false }
	, rightHeld{ false }
	, leftHeld{ false } {}

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

	window.PointerPressed({ this, &App::OnPointerPressed });
	window.PointerReleased({ this, &App::OnPointerReleased });
	window.PointerMoved({ this, &App::OnPointerMoved });

	window.KeyDown({ this, &App::OnKeyDown });
	window.KeyUp({ this, &App::OnKeyUp });

	window.PointerCursor(nullptr);
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
	window.PointerCursor(nullptr);
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
	Neon_UpdateMouseDown(true);
}

void App::OnPointerReleased(IInspectable const&, PointerEventArgs const& args)
{
	Neon_UpdateMouseDown(false);
}

void App::OnPointerMoved(IInspectable const&, PointerEventArgs const& args)
{
	auto bounds = CoreWindow::GetForCurrentThread().Bounds();
	float x = (args.CurrentPoint().RawPosition().X / bounds.Width) * 2.0 - 1.0;
	float y = -((args.CurrentPoint().RawPosition().Y / bounds.Height) * 2.0 - 1.0);

	Neon_UpdateCursorPosition(x, y);
}

void App::OnKeyUp(IInspectable const&, winrt::Windows::UI::Core::KeyEventArgs const& event)
{
	switch (event.VirtualKey())
	{
	case VirtualKey::W:
	case VirtualKey::Up:
		forwardHeld = false;
		break;
	case VirtualKey::S:
	case VirtualKey::Down:
		backHeld = false;
		break;
	case VirtualKey::A:
	case VirtualKey::Left:
		leftHeld = false;
		break;
	case VirtualKey::D:
	case VirtualKey::Right:
		rightHeld = false;
		break;
	}

	OnInputDirChanged();
}

void App::OnKeyDown(IInspectable const&, winrt::Windows::UI::Core::KeyEventArgs const& event)
{
	switch (event.VirtualKey())
	{
	case VirtualKey::Escape:
		Neon_EscapePressed();
		break;
	case VirtualKey::Number1:
		Neon_UpdateNumberKeyPressed(1);
		break;
	case VirtualKey::Number2:
		Neon_UpdateNumberKeyPressed(2);
		break;
	case VirtualKey::Number3:
		Neon_UpdateNumberKeyPressed(3);
		break;
	case VirtualKey::F11:
		auto view = ApplicationView::GetForCurrentView();
		if (view.IsFullScreenMode()) 
		{
			view.ExitFullScreenMode();
			ApplicationView::PreferredLaunchWindowingMode(ApplicationViewWindowingMode::Auto);
		}
		else if (view.TryEnterFullScreenMode())
		{
			ApplicationView::PreferredLaunchWindowingMode(ApplicationViewWindowingMode::FullScreen);
		}
		break;
	}

	switch (event.VirtualKey())
	{
	case VirtualKey::W:
	case VirtualKey::Up:
		forwardHeld = true;
		break;
	case VirtualKey::S:
	case VirtualKey::Down:
		backHeld = true;
		break;
	case VirtualKey::A:
	case VirtualKey::Left:
		leftHeld = true;
		break;
	case VirtualKey::D:
	case VirtualKey::Right:
		rightHeld = true;
		break;
	}

	OnInputDirChanged();
}

void App::OnInputDirChanged()
{
	float x = 0;
	float y = 0;

	if (forwardHeld) {
		y += 1;
	}

	if (backHeld) {
		y -= 1;
	}

	if (rightHeld) {
		x += 1;
	}

	if (leftHeld) {
		x -= 1;
	}

	Neon_UpdateDirectionalInput(x, y);
}