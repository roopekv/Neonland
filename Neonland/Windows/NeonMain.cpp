#include "pch.h"
#include "NeonMain.hpp"

#include <winrt/Windows.System.Threading.h>

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::System::Threading;

NeonMain::NeonMain() {}

void NeonMain::CreateRenderers(const std::shared_ptr<DeviceResources>& deviceResources) {
	_sceneRenderer = std::make_unique<Renderer>(deviceResources);
	OnWindowSizeChanged();
}

bool NeonMain::Render() {
	return _sceneRenderer->Render();
}

void NeonMain::OnWindowSizeChanged() {
	_sceneRenderer->CreateWindowSizeDependentResources();
}

void NeonMain::OnDeviceRemoved() {
	_sceneRenderer = nullptr;
}