#pragma once

#include "pch.h"
#include "DeviceResources.hpp"

class Renderer
{
public:
	Renderer(const std::shared_ptr<DeviceResources>& deviceResources);
	~Renderer();

	void CreateDeviceDependentResources();
	void CreateWindowSizeDependentResources();
	void Update(StepTimer const& timer);
	bool Render();
	void SaveState();

private:
	// Constant buffers must be 256-byte aligned.
	static const uint32_t c_alignedConstantBufferSize = (sizeof(ModelViewProjectionConstantBuffer) + 255) & ~255;

	// Cached pointer to device resources.
	std::shared_ptr<DeviceResources> m_deviceResources;

	bool m_loadingComplete;

	// Direct3D resources for cube geometry.
	winrt::com_ptr<ID3D12GraphicsCommandList>	m_commandList;
	winrt::com_ptr<ID3D12RootSignature>			m_rootSignature;
	winrt::com_ptr<ID3D12PipelineState>			m_pipelineState;
	winrt::com_ptr<ID3D12DescriptorHeap>		m_cbvHeap;
	winrt::com_ptr<ID3D12Resource>				m_vertexBuffer;
	winrt::com_ptr<ID3D12Resource>				m_indexBuffer;
	winrt::com_ptr<ID3D12Resource>				m_constantBuffer;
	ModelViewProjectionConstantBuffer					m_constantBufferData;
	uint8_t* m_mappedConstantBuffer;
	uint32_t												m_cbvDescriptorSize;
	D3D12_RECT											m_scissorRect;
	std::vector<byte>									m_vertexShader;
	std::vector<byte>									m_pixelShader;
	D3D12_VERTEX_BUFFER_VIEW							m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW								m_indexBufferView;
};
