#pragma once

#include "pch.h"
#include "d3dx12.h"

constexpr uint32_t c_frameCount = 3;

interface IDeviceNotify
{
	virtual void OnDeviceLost() = 0;
	virtual void OnDeviceRestored() = 0;
};

class DeviceResources
{
public:
	DeviceResources();
	void SetWindow(winrt::Windows::UI::Core::CoreWindow const& window);
	void SetLogicalSize(winrt::Windows::Foundation::Size logicalSize);
	void SetCurrentOrientation(winrt::Windows::Graphics::Display::DisplayOrientations currentOrientation);
	void SetDpi(float dpi);
	void ValidateDevice();
	void Present();
	void WaitForGpu();

	// Device Accessors.
	winrt::Windows::Foundation::Size GetOutputSize() const { return m_outputSize; }
	winrt::Windows::Foundation::Size GetLogicalSize() const { return m_logicalSize; }
	float                            GetDpi() const { return m_dpi; }
	bool						IsDeviceRemoved() const { return m_deviceRemoved; }

	// D3D Accessors.
	ID3D12Device* GetD3DDevice() const { return m_d3dDevice.get(); }
	IDXGISwapChain3* GetSwapChain() const { return m_swapChain.get(); }
	ID3D12Resource* GetRenderTarget() const { return m_renderTargets[m_currentFrame].get(); }
	ID3D12Resource* GetDepthStencil() const { return m_depthStencil.get(); }
	ID3D12CommandQueue* GetCommandQueue() const { return m_commandQueue.get(); }
	ID3D12CommandAllocator* GetCommandAllocator() const { return m_commandAllocators[m_currentFrame].get(); }
	DXGI_FORMAT					GetBackBufferFormat() const { return m_backBufferFormat; }
	DXGI_FORMAT					GetDepthBufferFormat() const { return m_depthBufferFormat; }
	D3D12_VIEWPORT				GetScreenViewport() const { return m_screenViewport; }
	DirectX::XMFLOAT4X4			GetOrientationTransform3D() const { return m_orientationTransform3D; }
	uint32_t						GetCurrentFrameIndex() const { return m_currentFrame; }

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_currentFrame, m_rtvDescriptorSize);
	}
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	}
private:
	void CreateDeviceIndependentResources();
	void CreateDeviceResources();
	void CreateWindowSizeDependentResources();
	void UpdateRenderTargetSize();
	void MoveToNextFrame();
	DXGI_MODE_ROTATION ComputeDisplayRotation();

	// Direct3D objects.
	winrt::com_ptr<ID3D12Device>         m_d3dDevice;


	uint32_t											m_currentFrame;

	// Direct3D objects.
	winrt::com_ptr<ID3D12Device>			m_d3dDevice;
	winrt::com_ptr<IDXGIFactory4>			m_dxgiFactory;
	winrt::com_ptr<IDXGISwapChain3>			m_swapChain;
	winrt::com_ptr<ID3D12Resource>			m_renderTargets[c_frameCount];
	winrt::com_ptr<ID3D12Resource>			m_depthStencil;
	winrt::com_ptr<ID3D12DescriptorHeap>	m_rtvHeap;
	winrt::com_ptr<ID3D12DescriptorHeap>	m_dsvHeap;
	winrt::com_ptr<ID3D12CommandQueue>		m_commandQueue;
	winrt::com_ptr<ID3D12CommandAllocator>	m_commandAllocators[c_frameCount];
	DXGI_FORMAT										m_backBufferFormat;
	DXGI_FORMAT										m_depthBufferFormat;
	D3D12_VIEWPORT									m_screenViewport;
	uint32_t											m_rtvDescriptorSize;
	bool											m_deviceRemoved;

	// CPU/GPU Synchronization.
	winrt::com_ptr<ID3D12Fence>				m_fence;
	uint64_t											m_fenceValues[c_frameCount];
	HANDLE											m_fenceEvent;

	// Cached reference to the Window.
	Platform::Agile< winrt::Windows::UI::Core::CoreWindow>	m_window;

	// Cached device properties.
	winrt::Windows::Foundation::Size						m_d3dRenderTargetSize;
	winrt::Windows::Foundation::Size						m_outputSize;
	winrt::Windows::Foundation::Size						m_logicalSize;
	winrt::Windows::Graphics::Display::DisplayOrientations	m_nativeOrientation;
	winrt::Windows::Graphics::Display::DisplayOrientations	m_currentOrientation;
	float											m_dpi;

	// Transforms used for display orientation.
	DirectX::XMFLOAT4X4								m_orientationTransform3D;

};