#pragma once

#include "NeonMain.hpp"
#include "DeviceResources.hpp"
#include "Renderer.hpp"

class NeonMain {
public:
	NeonMain();
	void CreateRenderers(const std::shared_ptr<DeviceResources>& deviceResources);
	bool Render();

	void OnWindowSizeChanged();
	void OnDeviceRemoved();
private:
	std::unique_ptr<Renderer> _sceneRenderer;
};