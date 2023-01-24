#include "pch.h"
#include "NeonMain.hpp"

#include <winrt/Windows.System.Threading.h>

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::System::Threading;

NeonMain::NeonMain() {}

void NeonMain::CreateRenderers(const std::shared_ptr<DeviceResources>& deviceResources)
{
	m_sceneRenderer = std::unique_ptr<Renderer>(new Renderer(deviceResources));

	OnWindowSizeChanged();
}

bool NeonMain::Render() {
	// Render the scene objects.
	// TODO: Replace this with your app's content rendering functions.
	return m_sceneRenderer->Render();
	return false;
}

// Updates application state when the window's size changes (e.g. device orientation change)
void NeonMain::OnWindowSizeChanged()
{
	// TODO: Replace this with the size-dependent initialization of your app's content.
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

// Notifies renderers that device resources need to be released.
void NeonMain::OnDeviceRemoved()
{
	// TODO: Save any necessary application or renderer state and release the renderer
	// and its resources which are no longer valid.
	m_sceneRenderer = nullptr;
}

