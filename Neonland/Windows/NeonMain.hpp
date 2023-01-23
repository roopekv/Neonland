#pragma once

#include "NeonMain.hpp"
#include "DeviceResources.hpp"
#include "Renderer.hpp"

class NeonMain {
public:
	NeonMain();
	void CreateRenderers(const std::shared_ptr<DeviceResources>& deviceResources);
	void Update();
	bool Render();

	void OnWindowSizeChanged();
	void OnSuspending();
	void OnResuming();
	void OnDeviceRemoved();

private:
	// TODO: Replace with your own content renderers.
	std::unique_ptr<Renderer> m_sceneRenderer;

	// Rendering loop timer.
	StepTimer m_timer;
};