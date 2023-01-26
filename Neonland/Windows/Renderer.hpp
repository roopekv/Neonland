#pragma once

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

	void LoadMesh(MeshType type, uint32_t& vertexCount, uint32_t& indexCount, ID3D12Resource* vertexUploadBuffer, ID3D12Resource* indexUploadBuffer);
	void LoadTexture();

	winrt::com_ptr<ID3D12Resource> m_vertexBuffer;
	winrt::com_ptr<ID3D12Resource> m_indexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

private:
	// Constant buffers must be 256-byte aligned.
	static constexpr uint32_t AlignedGlobalUnformsBufferSize = (sizeof(GlobalUniforms) + 255) & ~255;
	static const uint32_t AlignedInstanceBufferSize;

	// Cached pointer to device resources.
	std::shared_ptr<DeviceResources> _deviceResources;

	bool _loadingComplete;

	// Direct3D resources for cube geometry.
	winrt::com_ptr<ID3D12GraphicsCommandList>	_commandList;
	winrt::com_ptr<ID3D12RootSignature>			_rootSignature;
	winrt::com_ptr<ID3D12PipelineState>			_pipelineState;
	D3D12_RECT									_scissorRect;

	winrt::com_ptr<ID3D12DescriptorHeap>		_globalUniformsVHeap;
	uint32_t									_globalUniformsVDescriptorSize;
	winrt::com_ptr<ID3D12Resource> _globalUniformsBuffer;
	uint8_t* _mappedGlobalUniformsBuffer;

	winrt::com_ptr<ID3D12Resource> _instanceBuffer;
	uint8_t* _mappedInstanceBuffer;

	std::array<D3D12_VERTEX_BUFFER_VIEW, MeshTypeCount> _vertexBufferViews;
	std::array<winrt::com_ptr<ID3D12Resource>, MeshTypeCount> _vertexBuffers;

	std::array<D3D12_INDEX_BUFFER_VIEW, MeshTypeCount> _indexBufferViews;
	std::array<winrt::com_ptr<ID3D12Resource>, MeshTypeCount> _indexBuffers;
};
