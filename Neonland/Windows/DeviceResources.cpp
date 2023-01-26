#include "pch.h"
#include "DeviceResources.hpp"

#include <algorithm>

using namespace DirectX;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Graphics::Display;
using namespace winrt::Windows::UI::Core;

// Constants used to calculate screen rotations.
namespace ScreenRotation
{
	// 0-degree Z-rotation
	static const XMFLOAT4X4 Rotation0(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// 90-degree Z-rotation
	static const XMFLOAT4X4 Rotation90(
		0.0f, 1.0f, 0.0f, 0.0f,
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// 180-degree Z-rotation
	static const XMFLOAT4X4 Rotation180(
		-1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// 270-degree Z-rotation
	static const XMFLOAT4X4 Rotation270(
		0.0f, -1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
};

// Constructor for DeviceResources.
DeviceResources::DeviceResources() :
	m_currentFrame(0),
	m_screenViewport(),
	m_rtvDescriptorSize(0),
	m_fenceEvent(0),
	m_backBufferFormat(DXGI_FORMAT_B8G8R8A8_UNORM),
	m_depthBufferFormat(DXGI_FORMAT_D32_FLOAT),
	m_fenceValues{},
	m_d3dRenderTargetSize(),
	m_outputSize(),
	m_logicalSize(),
	m_nativeOrientation(DisplayOrientations::None),
	m_currentOrientation(DisplayOrientations::None),
	m_dpi(-1.0f),
	m_deviceRemoved(false)
{
	CreateDeviceIndependentResources();
	CreateDeviceResources();
}

// Configures resources that don't depend on the Direct3D device.
void DeviceResources::CreateDeviceIndependentResources()
{
}

// Configures the Direct3D device, and stores handles to it and the device context.
void DeviceResources::CreateDeviceResources()
{

#if defined(_DEBUG)
	// If the project is in a debug build, enable debugging via SDK Layers.
	{
		winrt::com_ptr<ID3D12Debug6> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
	winrt::check_hresult(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_dxgiFactory)));
#else
	winrt::check_hresult(CreateDXGIFactory2(0, IID_PPV_ARGS(&m_dxgiFactory)));
#endif

	winrt::com_ptr<IDXGIAdapter4> adapter;
	for (uint32_t adapterIndex = 0; DXGI_ERROR_NOT_FOUND != m_dxgiFactory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(adapter.put())); adapterIndex++)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			continue;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter.get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	// Create the Direct3D 12 API device object
	winrt::check_hresult(D3D12CreateDevice(
		adapter.get(),					// The hardware adapter.
		D3D_FEATURE_LEVEL_11_0,			// Minimum feature level this app can support.
		IID_PPV_ARGS(&m_d3dDevice)		// Returns the Direct3D device created.
	));

	// Create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	winrt::check_hresult(m_d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	// Create descriptor heaps for render target views and depth stencil views.
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = MaxFramesInFlight;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	winrt::check_hresult(m_d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

	m_rtvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	winrt::check_hresult(m_d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));

	for (uint32_t n = 0; n < MaxFramesInFlight; n++)
	{
		winrt::check_hresult(
			m_d3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[n]))
		);
	}

	// Create synchronization objects.
	winrt::check_hresult(m_d3dDevice->CreateFence(m_fenceValues[m_currentFrame], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceValues[m_currentFrame]++;

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		winrt::check_hresult(HRESULT_FROM_WIN32(GetLastError()));
	}
}

// These resources need to be recreated every time the window size is changed.
void DeviceResources::CreateWindowSizeDependentResources()
{
	// Wait until all previous GPU work is complete.
	WaitForGpu();

	// Clear the previous window size specific content and update the tracked fence values.
	for (uint32_t n = 0; n < MaxFramesInFlight; n++)
	{
		m_renderTargets[n] = nullptr;
		m_fenceValues[n] = m_fenceValues[m_currentFrame];
	}

	UpdateRenderTargetSize();

	// The width and height of the swap chain must be based on the window's
	// natively-oriented width and height. If the window is not in the native
	// orientation, the dimensions must be reversed.
	DXGI_MODE_ROTATION displayRotation = ComputeDisplayRotation();

	bool swapDimensions = displayRotation == DXGI_MODE_ROTATION_ROTATE90 || displayRotation == DXGI_MODE_ROTATION_ROTATE270;
	m_d3dRenderTargetSize.Width = swapDimensions ? m_outputSize.Height : m_outputSize.Width;
	m_d3dRenderTargetSize.Height = swapDimensions ? m_outputSize.Width : m_outputSize.Height;

	uint32_t backBufferWidth = lround(m_d3dRenderTargetSize.Width);
	uint32_t backBufferHeight = lround(m_d3dRenderTargetSize.Height);

	if (m_swapChain != nullptr)
	{
		// If the swap chain already exists, resize it.
		HRESULT hr = m_swapChain->ResizeBuffers(MaxFramesInFlight, backBufferWidth, backBufferHeight, m_backBufferFormat, 0);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// If the device was removed for any reason, a new device and swap chain will need to be created.
			m_deviceRemoved = true;

			// Do not continue execution of this method. DeviceResources will be destroyed and re-created.
			return;
		}
		else
		{
			winrt::check_hresult(hr);
		}
	}
	else
	{
		// Otherwise, create a new one using the same adapter as the existing Direct3D device.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

		swapChainDesc.Width = backBufferWidth;						// Match the size of the window.
		swapChainDesc.Height = backBufferHeight;
		swapChainDesc.Format = m_backBufferFormat;
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;							// Don't use multi-sampling.
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = MaxFramesInFlight;					// Use triple-buffering to minimize latency.
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	// All Windows Universal apps must use _FLIP_ SwapEffects.
		swapChainDesc.Flags = 0;
		swapChainDesc.Scaling = DXGI_SCALING_NONE;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		winrt::com_ptr<IDXGISwapChain1> swapChain;
		winrt::check_hresult(
			m_dxgiFactory->CreateSwapChainForCoreWindow(
				m_commandQueue.get(),								// Swap chains need a reference to the command queue in DirectX 12.
				winrt::get_unknown(m_window.get()),
				&swapChainDesc,
				nullptr,
				swapChain.put()
			)
		);

		winrt::check_hresult(swapChain.try_as(m_swapChain));
	}

	// Set the proper orientation for the swap chain, and generate
	// 3D matrix transformations for rendering to the rotated swap chain.
	// The 3D matrix is specified explicitly to avoid rounding errors.

	switch (displayRotation)
	{
	case DXGI_MODE_ROTATION_IDENTITY:
		m_orientationTransform3D = ScreenRotation::Rotation0;
		break;

	case DXGI_MODE_ROTATION_ROTATE90:
		m_orientationTransform3D = ScreenRotation::Rotation270;
		break;

	case DXGI_MODE_ROTATION_ROTATE180:
		m_orientationTransform3D = ScreenRotation::Rotation180;
		break;

	case DXGI_MODE_ROTATION_ROTATE270:
		m_orientationTransform3D = ScreenRotation::Rotation90;
		break;

	default:
		winrt::throw_hresult(E_FAIL);
	}

	winrt::check_hresult(
		m_swapChain->SetRotation(displayRotation)
	);

	// Create render target views of the swap chain back buffer.
	{
		m_currentFrame = m_swapChain->GetCurrentBackBufferIndex();
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescriptor(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (uint32_t n = 0; n < MaxFramesInFlight; n++)
		{
			winrt::check_hresult(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
			m_d3dDevice->CreateRenderTargetView(m_renderTargets[n].get(), nullptr, rtvDescriptor);
			rtvDescriptor.Offset(m_rtvDescriptorSize);
		}
	}

	// Create a depth stencil and view.
	{
		D3D12_HEAP_PROPERTIES depthHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

		D3D12_RESOURCE_DESC depthResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(m_depthBufferFormat, backBufferWidth, backBufferHeight, 1, 1);
		depthResourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		CD3DX12_CLEAR_VALUE depthOptimizedClearValue(m_depthBufferFormat, 1.0f, 0);

		winrt::check_hresult(m_d3dDevice->CreateCommittedResource(
			&depthHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&depthResourceDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&m_depthStencil)
		));

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = m_depthBufferFormat;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		m_d3dDevice->CreateDepthStencilView(m_depthStencil.get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	// Set the 3D rendering viewport to target the entire window.
	m_screenViewport = { 0.0f, 0.0f, m_d3dRenderTargetSize.Width, m_d3dRenderTargetSize.Height, 0.0f, 1.0f };
}

// Determine the dimensions of the render target and whether it will be scaled down.
void DeviceResources::UpdateRenderTargetSize()
{
	// Calculate the necessary render target size in pixels.
	static constexpr float dipsPerInch = 96.0f;

	m_outputSize.Width = std::floor(m_logicalSize.Width * m_dpi / dipsPerInch + 0.5f);
	m_outputSize.Height = std::floor(m_logicalSize.Height * m_dpi / dipsPerInch + 0.5f);
	// Prevent zero size DirectX content from being created.
	m_outputSize.Width = std::max(m_outputSize.Width, 1.0f);
	m_outputSize.Height = std::max(m_outputSize.Height, 1.0f);
}

// This method is called when the CoreWindow is created (or re-created).
void DeviceResources::SetWindow(CoreWindow const& window)
{
	DisplayInformation currentDisplayInformation = DisplayInformation::GetForCurrentView();

	m_window = window;
	m_logicalSize = winrt::Windows::Foundation::Size(window.Bounds().Width, window.Bounds().Height);
	m_nativeOrientation = currentDisplayInformation.NativeOrientation();
	m_currentOrientation = currentDisplayInformation.CurrentOrientation();
	m_dpi = currentDisplayInformation.LogicalDpi();

	CreateWindowSizeDependentResources();
}

// This method is called in the event handler for the SizeChanged event.
void DeviceResources::SetLogicalSize(winrt::Windows::Foundation::Size logicalSize)
{
	if (m_logicalSize != logicalSize)
	{
		m_logicalSize = logicalSize;
		CreateWindowSizeDependentResources();
	}
}

// This method is called in the event handler for the DpiChanged event.
void DeviceResources::SetDpi(float dpi)
{
	if (dpi != m_dpi)
	{
		m_dpi = dpi;

		// When the display DPI changes, the logical size of the window (measured in Dips) also changes and needs to be updated.
		m_logicalSize = winrt::Windows::Foundation::Size(m_window.get().Bounds().Width, m_window.get().Bounds().Height);

		CreateWindowSizeDependentResources();
	}
}

// This method is called in the event handler for the OrientationChanged event.
void DeviceResources::SetCurrentOrientation(DisplayOrientations currentOrientation)
{
	if (m_currentOrientation != currentOrientation)
	{
		m_currentOrientation = currentOrientation;
		CreateWindowSizeDependentResources();
	}
}

// This method is called in the event handler for the DisplayContentsInvalidated event.
void DeviceResources::ValidateDevice()
{
	// The D3D Device is no longer valid if the default adapter changed since the device
	// was created or if the device has been removed.

	// First, get the LUID for the default adapter from when the device was created.

	DXGI_ADAPTER_DESC previousDesc;
	{
		winrt::com_ptr<IDXGIAdapter1> previousDefaultAdapter;
		winrt::check_hresult(m_dxgiFactory->EnumAdapters1(0, previousDefaultAdapter.put()));

		winrt::check_hresult(previousDefaultAdapter->GetDesc(&previousDesc));
	}

	// Next, get the information for the current default adapter.

	DXGI_ADAPTER_DESC currentDesc;
	{
		winrt::com_ptr<IDXGIFactory4> currentDxgiFactory;
		winrt::check_hresult(CreateDXGIFactory1(IID_PPV_ARGS(&currentDxgiFactory)));

		winrt::com_ptr<IDXGIAdapter1> currentDefaultAdapter;
		winrt::check_hresult(currentDxgiFactory->EnumAdapters1(0, currentDefaultAdapter.put()));

		winrt::check_hresult(currentDefaultAdapter->GetDesc(&currentDesc));
	}

	// If the adapter LUIDs don't match, or if the device reports that it has been removed,
	// a new D3D device must be created.

	if (previousDesc.AdapterLuid.LowPart != currentDesc.AdapterLuid.LowPart ||
		previousDesc.AdapterLuid.HighPart != currentDesc.AdapterLuid.HighPart ||
		FAILED(m_d3dDevice->GetDeviceRemovedReason()))
	{
		m_deviceRemoved = true;
	}
}

// Present the contents of the swap chain to the screen.
void DeviceResources::Present()
{
	// The first argument instructs DXGI to block until VSync, putting the application
	// to sleep until the next VSync. This ensures we don't waste any cycles rendering
	// frames that will never be displayed to the screen.
	HRESULT hr = m_swapChain->Present(1, 0);

	// If the device was removed either by a disconnection or a driver upgrade, we 
	// must recreate all device resources.
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		m_deviceRemoved = true;
	}
	else
	{
		winrt::check_hresult(hr);

		MoveToNextFrame();
	}
}

// Wait for pending GPU work to complete.
void DeviceResources::WaitForGpu()
{
	// Schedule a Signal command in the queue.
	winrt::check_hresult(m_commandQueue->Signal(m_fence.get(), m_fenceValues[m_currentFrame]));

	// Wait until the fence has been crossed.
	winrt::check_hresult(m_fence->SetEventOnCompletion(m_fenceValues[m_currentFrame], m_fenceEvent));
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	// Increment the fence value for the current frame.
	m_fenceValues[m_currentFrame]++;
}

// Prepare to render the next frame.
void DeviceResources::MoveToNextFrame()
{
	// Schedule a Signal command in the queue.
	const uint64_t currentFenceValue = m_fenceValues[m_currentFrame];
	winrt::check_hresult(m_commandQueue->Signal(m_fence.get(), currentFenceValue));

	// Advance the frame index.
	m_currentFrame = m_swapChain->GetCurrentBackBufferIndex();

	// Check to see if the next frame is ready to start.
	if (m_fence->GetCompletedValue() < m_fenceValues[m_currentFrame])
	{
		winrt::check_hresult(m_fence->SetEventOnCompletion(m_fenceValues[m_currentFrame], m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	// Set the fence value for the next frame.
	m_fenceValues[m_currentFrame] = currentFenceValue + 1;
}

// This method determines the rotation between the display device's native Orientation and the
// current display orientation.
DXGI_MODE_ROTATION DeviceResources::ComputeDisplayRotation()
{
	DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;

	// Note: NativeOrientation can only be Landscape or Portrait even though
	// the DisplayOrientations enum has other values.
	switch (m_nativeOrientation)
	{
	case DisplayOrientations::Landscape:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;

		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;

		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;

		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;
		}
		break;

	case DisplayOrientations::Portrait:
		switch (m_currentOrientation)
		{
		case DisplayOrientations::Landscape:
			rotation = DXGI_MODE_ROTATION_ROTATE90;
			break;

		case DisplayOrientations::Portrait:
			rotation = DXGI_MODE_ROTATION_IDENTITY;
			break;

		case DisplayOrientations::LandscapeFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE270;
			break;

		case DisplayOrientations::PortraitFlipped:
			rotation = DXGI_MODE_ROTATION_ROTATE180;
			break;
		}
		break;
	}
	return rotation;
}
