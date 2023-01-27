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

	void LoadMesh(MeshType type, ID3D12Resource* vertexUploadBuffer, ID3D12Resource* indexUploadBuffer);
	void LoadTexture(TextureType type, ID3D12Resource* uploadBuffer);

private:
	static constexpr uint32_t AlignedGlobalUnformsBufferSize = (sizeof(GlobalUniforms) + 255) & ~255;
	static const uint32_t AlignedInstanceBufferSize;

	std::shared_ptr<DeviceResources> _deviceResources;

	bool _loadingComplete;

	winrt::com_ptr<ID3D12GraphicsCommandList>	_commandList;
	winrt::com_ptr<ID3D12RootSignature>			_rootSignature;
	D3D12_RECT									_scissorRect;

	std::array<winrt::com_ptr<ID3D12PipelineState>, ShaderTypeCount> _pipelineStates;

	winrt::com_ptr<ID3D12DescriptorHeap> _descHeap;

	uint32_t _CBV_SRV_UAV_ViewDescriptorSize;

	winrt::com_ptr<ID3D12Resource> _globalUniformsBuffer;
	uint8_t* _mappedGlobalUniformsBuffer;

	winrt::com_ptr<ID3D12Resource> _instanceBuffer;
	uint8_t* _mappedInstanceBuffer;

	std::array<D3D12_VERTEX_BUFFER_VIEW, MeshTypeCount> _vertexBufferViews;
	std::array<winrt::com_ptr<ID3D12Resource>, MeshTypeCount> _vertexBuffers;

	std::array<D3D12_INDEX_BUFFER_VIEW, MeshTypeCount> _indexBufferViews;
	std::array<winrt::com_ptr<ID3D12Resource>, MeshTypeCount> _indexBuffers;

	std::array<winrt::com_ptr<ID3D12Resource>, TextureTypeCount> _textureBuffers;
};
