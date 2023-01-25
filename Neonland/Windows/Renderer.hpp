#pragma once

#include <memory>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <winrt/base.h>
#include <array>
#include <map>
#include <string>

#include "DeviceResources.hpp"

#include "../Engine/ShaderTypes.h"
#include "../NeonConstants.h"

class Renderer
{
public:
	Renderer(const std::shared_ptr<DeviceResources>& deviceResources);
	~Renderer();

	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();
	bool Render();

	void LoadMesh(char* meshData, ID3D12Resource** vertexBuffer, ID3D12Resource** indexBuffer, uint32_t& vertexCount, uint32_t& indexCount);
	void LoadTexture();

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
	winrt::com_ptr<ID3D12Resource>				_globalUniformsBuffer;
	uint32_t									_cbvDescriptorSize;
	D3D12_RECT									_scissorRect;

	uint8_t* _mappedGlobalUniformsBuffer;

	std::array<D3D12_VERTEX_BUFFER_VIEW, MeshTypeCount> _vertexBufferViews;
	std::array<winrt::com_ptr<ID3D12Resource>, MeshTypeCount> _vertexBuffers;

	std::array<D3D12_INDEX_BUFFER_VIEW, MeshTypeCount> _indexBufferViews;
	std::array<winrt::com_ptr<ID3D12Resource>, MeshTypeCount> _indexBuffers;
};
