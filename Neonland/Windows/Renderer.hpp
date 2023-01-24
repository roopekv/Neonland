#pragma once

#include <memory>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <pix.h>
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
	void Update();
	bool Render();

private:
	// Constant buffers must be 256-byte aligned.
	static const uint32_t c_alignedConstantBufferSize = (sizeof(GlobalUniforms) + 255) & ~255;

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
	GlobalUniforms								m_constantBufferData;
	uint8_t* m_mappedConstantBuffer;
	uint32_t									m_cbvDescriptorSize;
	D3D12_RECT									m_scissorRect;
	std::vector<char>							m_vertexShader;
	std::vector<char>							m_fragmentShader;
	D3D12_VERTEX_BUFFER_VIEW					m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW						m_indexBufferView;
};
