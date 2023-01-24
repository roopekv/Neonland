#include "pch.h"
#include "App.hpp"

using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::Graphics::Display;
using namespace winrt::Windows::UI::Core;

App::App()
	: m_windowClosed{ false }
	, m_windowVisible{ true } {}

IFrameworkView App::CreateView() {
	return *this;
}

void App::Initialize(CoreApplicationView const& applicationView)
{
	applicationView.Activated({ this, &App::OnActivated });

	m_deviceResources = std::make_shared<DeviceResources>();
}

void App::SetWindow(CoreWindow const& window)
{
	window.PointerCursor(CoreCursor(CoreCursorType::Arrow, 0));

	m_deviceResources->SetWindow(window);

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
	if (m_main == nullptr)
	{
		m_main = std::make_unique<NeonMain>();
	}
}

void App::Run()
{
	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			CoreWindow::GetForCurrentThread().Dispatcher().ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

			auto commandQueue = GetDeviceResources()->GetCommandQueue();
			PIXBeginEvent(commandQueue, 0, L"Render");
			{
				if (m_main->Render())
				{
					GetDeviceResources()->Present();
				}
			}
			PIXEndEvent(commandQueue);
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
	m_deviceResources->SetLogicalSize(args.Size());
	m_main->OnWindowSizeChanged();
}

void App::OnVisibilityChanged(CoreWindow const&, VisibilityChangedEventArgs const& args)
{
	m_windowVisible = args.Visible();
}

void App::OnWindowClosed(CoreWindow const&, CoreWindowEventArgs const&)
{
	m_windowClosed = true;
}

void App::OnDpiChanged(DisplayInformation const& sender, IInspectable const&)
{
	GetDeviceResources()->SetDpi(sender.LogicalDpi());
	m_main->OnWindowSizeChanged();
}

void App::OnOrientationChanged(DisplayInformation const& sender, IInspectable const&)
{
	m_deviceResources->SetCurrentOrientation(sender.CurrentOrientation());
	m_main->OnWindowSizeChanged();
}

void App::OnDisplayContentsInvalidated(DisplayInformation const&, IInspectable const&)
{
	GetDeviceResources()->ValidateDevice();
}

std::shared_ptr<DeviceResources> App::GetDeviceResources() {
	if (m_deviceResources != nullptr && m_deviceResources->IsDeviceRemoved())
	{
		m_deviceResources = nullptr;
		m_main->OnDeviceRemoved();
	}

	if (m_deviceResources == nullptr)
	{
		m_deviceResources = std::make_shared<DeviceResources>();
		m_deviceResources->SetWindow(CoreWindow::GetForCurrentThread());
		m_main->CreateRenderers(m_deviceResources);
	}
	return m_deviceResources;
}

void App::OnPointerPressed(IInspectable const&, PointerEventArgs const& args)
{

}

void App::OnPointerMoved(IInspectable const&, PointerEventArgs const& args)
{

}