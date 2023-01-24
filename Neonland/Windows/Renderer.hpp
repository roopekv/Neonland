#pragma once

#include <memory>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <winrt/base.h>

#include "DeviceResources.hpp"

#include "../Engine/ShaderTypes.h"

class Renderer
{
public:
	Renderer(const std::shared_ptr<DeviceResources>& deviceResources);
	~Renderer();

	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();
	bool Render();

private:
	// Constant buffers must be 256-byte aligned.
	static constexpr uint32_t AlignedConstantBufferSize = (sizeof(GlobalUniforms) + 255) & ~255;

	// Cached pointer to device resources.
	std::shared_ptr<DeviceResources> _deviceResources;

	bool _loadingComplete;

	// Direct3D resources for cube geometry.
	winrt::com_ptr<ID3D12GraphicsCommandList>	_commandList;
	winrt::com_ptr<ID3D12RootSignature>			_rootSignature;
	winrt::com_ptr<ID3D12PipelineState>			_pipelineState;
	winrt::com_ptr<ID3D12DescriptorHeap>		_cbvHeap;
	winrt::com_ptr<ID3D12Resource>				_vertexBuffer;
	winrt::com_ptr<ID3D12Resource>				_indexBuffer;
	winrt::com_ptr<ID3D12Resource>				_globalUniformsBuffer;
	uint8_t*									_mappedGlobalUniformsBuffer;
	uint32_t									_cbvDescriptorSize;
	D3D12_RECT									_scissorRect;
	std::vector<char>							_vertexShader;
	std::vector<char>							_fragmentShader;
	D3D12_VERTEX_BUFFER_VIEW					_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW						_indexBufferView;
};
