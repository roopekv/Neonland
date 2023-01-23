#include "App.hpp"

using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::ApplicationModel::Core;
using namespace winrt::Windows::Graphics::Display;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Input;

App::App()
	: m_windowClosed{ false }
	, m_windowVisible{ true } {}

IFrameworkView App::CreateView() {
	return *this;
}

void App::Initialize(CoreApplicationView const& applicationView)
{
	applicationView.Activated({ this, &App::OnActivated });

	CoreApplication::Suspending({ this, &App::OnSuspending });

	CoreApplication::Resuming({ this, &App::OnResuming });

	m_deviceResources = std::make_shared<DeviceResources>();
}

void App::SetWindow(CoreWindow const& window)
{
	window.PointerCursor(CoreCursor(CoreCursorType::Arrow, 0));

	PointerVisualizationSettings visualizationSettings{ PointerVisualizationSettings::GetForCurrentView() };
	visualizationSettings.IsContactFeedbackEnabled(false);
	visualizationSettings.IsBarrelButtonFeedbackEnabled(false);

	m_deviceResources->SetWindow(window);

	window.Activated({ this, &App::OnWindowActivationChanged });

	window.SizeChanged({ this, &App::OnWindowSizeChanged });

	window.Closed({ this, &App::OnWindowClosed });

	window.VisibilityChanged({ this, &App::OnVisibilityChanged });

	DisplayInformation currentDisplayInformation{ DisplayInformation::GetForCurrentView() };

	currentDisplayInformation.DpiChanged({ this, &App::OnDpiChanged });

	currentDisplayInformation.OrientationChanged({ this, &App::OnOrientationChanged });

	DisplayInformation::DisplayContentsInvalidated({ this, &App::OnDisplayContentsInvalidated });
}

void App::Load(winrt::hstring const&)
{
	if (m_main == nullptr)
	{
		m_main = winrt::make_self<NeonMain>(m_deviceResources);
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
			PIXBeginEvent(commandQueue, 0, L"Update");
			{
				m_main->Update();
			}
			PIXEndEvent(commandQueue);

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

winrt::fire_and_forget App::OnSuspending(IInspectable const& /* sender */, SuspendingEventArgs const& args)
{
	auto lifetime = get_strong();

	// Save app state asynchronously after requesting a deferral. Holding a deferral
	// indicates that the application is busy performing suspending operations. Be
	// aware that a deferral may not be held indefinitely. After about five seconds,
	// the app will be forced to exit.
	SuspendingDeferral deferral = args.SuspendingOperation().GetDeferral();

	co_await winrt::resume_background();

	m_main->Suspend();
	deferral.Complete();
}

void App::OnResuming(IInspectable const& /* sender */, IInspectable const& /* args */)
{
	// Restore any data or state that was unloaded on suspend. By default, data
	// and state are persisted when resuming from suspend. Note that this event
	// does not occur if the app was previously terminated.
	m_main->Resume();
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

void App::OnWindowActivationChanged(CoreWindow const&, WindowActivatedEventArgs const& args)
{
	m_main->WindowActivationChanged(args.WindowActivationState());
}

void App::OnWindowClosed(CoreWindow const& /* sender */, CoreWindowEventArgs const& /* args */)
{
	m_windowClosed = true;
}

void App::OnDpiChanged(DisplayInformation const& sender, IInspectable const& /* args */)
{
	GetDeviceResources()->SetDpi(sender.LogicalDpi());
	m_main->OnWindowSizeChanged();
}

void App::OnOrientationChanged(DisplayInformation const& sender, IInspectable const& /* args */)
{
	m_deviceResources->SetCurrentOrientation(sender.CurrentOrientation());
	m_main->OnWindowSizeChanged();
}

void App::OnDisplayContentsInvalidated(DisplayInformation const& /* sender */, IInspectable const& /* args */)
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