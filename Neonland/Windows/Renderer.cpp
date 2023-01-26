#include "pch.h"

#include "Renderer.hpp"

#include "../Neonland.h"
#include "../Engine/MathUtils.hpp"

using namespace DirectX;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;

const uint32_t Renderer::AlignedInstanceBufferSize = (sizeof(Instance) * MAX_INSTANCE_COUNT + 255) & ~255;


struct VertexPositionColor
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 color;
};

Renderer::Renderer(const std::shared_ptr<DeviceResources>& deviceResources) :
	_mappedGlobalUniformsBuffer(nullptr),
	_mappedInstanceBuffer(nullptr),
	_loadingComplete{ false },
	_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

Renderer::~Renderer()
{
	_globalUniformsBuffer->Unmap(0, nullptr);
	_instanceBuffer->Unmap(0, nullptr);
	_mappedGlobalUniformsBuffer = nullptr;
	_mappedInstanceBuffer = nullptr;
}

struct Vertex {
	float3 position;
	float3 normal;
	float2 texCoords;
};

void Renderer::LoadMesh(MeshType type, uint32_t& vertexCount, uint32_t& indexCount, ID3D12Resource* vertexUploadBuffer, ID3D12Resource* indexUploadBuffer) {

	static const std::map<MeshType, std::wstring> meshIdxToName = {
		{PLANE_MESH, L"plane"},
		{CROSSHAIR_MESH, L"crosshair"},
		{SPREAD_MESH, L"spread_circle"},
		{CUBE_MESH, L"cube"},
		{SHARD_MESH, L"shard"},
		{SPHERE_MESH, L"sphere"}
	};

	std::wstring filename = L"" + meshIdxToName.at(type) + L".obj";

	WaveFrontReader<uint16_t> wfReader;
	HRESULT hr = wfReader.Load(filename.c_str());
	if (FAILED(hr)) {
		std::abort();
	}

	vertexCount = wfReader.vertices.size();
	indexCount = wfReader.indices.size();

	auto device = _deviceResources->GetD3DDevice();

	CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(Vertex) * vertexCount);
	winrt::check_hresult(device->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(_vertexBuffers[type].put())));

	CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	winrt::check_hresult(device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexUploadBuffer)));

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = wfReader.vertices.data();
	vertexData.RowPitch = sizeof(Vertex) * vertexCount;
	vertexData.SlicePitch = vertexData.RowPitch;

	UpdateSubresources(_commandList.get(), _vertexBuffers[type].get(), vertexUploadBuffer, 0, 0, 1, &vertexData);

	CD3DX12_RESOURCE_BARRIER vertexBufferResourceBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(_vertexBuffers[type].get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	_commandList->ResourceBarrier(1, &vertexBufferResourceBarrier);

	CD3DX12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexCount * sizeof(uint16_t));
	winrt::check_hresult(device->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(_indexBuffers[type].put())));

	winrt::check_hresult(device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexUploadBuffer)));

	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = wfReader.indices.data();
	indexData.RowPitch = indexCount * sizeof(uint16_t);
	indexData.SlicePitch = indexData.RowPitch;

	UpdateSubresources(_commandList.get(), _indexBuffers[type].get(), indexUploadBuffer, 0, 0, 1, &indexData);

	CD3DX12_RESOURCE_BARRIER indexBufferResourceBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(_indexBuffers[type].get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	_commandList->ResourceBarrier(1, &indexBufferResourceBarrier);
}

void Renderer::LoadTexture() {

}

void Renderer::CreateDeviceDependentResources()
{
	auto device = _deviceResources->GetD3DDevice();

	CD3DX12_DESCRIPTOR_RANGE range;
	CD3DX12_ROOT_PARAMETER parameter;

	range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	parameter.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_VERTEX);

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
	descRootSignature.Init(1, &parameter, 0, nullptr, rootSignatureFlags);

	winrt::com_ptr<ID3DBlob> pSignature;
	winrt::com_ptr<ID3DBlob> pError;
	winrt::check_hresult(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.put(), pError.put()));
	winrt::check_hresult(device->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&_rootSignature)));

	auto loadFile = [](const std::string& filename) -> std::vector<char>
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		size_t fileSize = file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	};

	std::vector<char> vertexShader = loadFile("VertexShader.cso");
	std::vector<char> fragmentShader = loadFile("FragmentShader.cso");

	static const D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "INSTANCETF",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "INSTANCETF",  1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "INSTANCETF",  2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "INSTANCETF",  3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		{ "INSTANCECOLOR",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
	state.InputLayout = { inputLayout, _countof(inputLayout) };
	state.pRootSignature = _rootSignature.get();
	state.VS = CD3DX12_SHADER_BYTECODE(&vertexShader[0], vertexShader.size());
	state.PS = CD3DX12_SHADER_BYTECODE(&fragmentShader[0], fragmentShader.size());
	state.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	state.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	state.SampleMask = UINT_MAX;
	state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	state.NumRenderTargets = 1;
	state.RTVFormats[0] = _deviceResources->GetBackBufferFormat();
	state.DSVFormat = _deviceResources->GetDepthBufferFormat();
	state.SampleDesc.Count = 1;

	winrt::check_hresult(_deviceResources->GetD3DDevice()->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&_pipelineState)));

	winrt::check_hresult(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _deviceResources->GetCommandAllocator(), _pipelineState.get(), IID_PPV_ARGS(&_commandList)));

	const std::map<TextureType, std::wstring> textureIdxToName = {
		{NO_TEX, L"blank"},
		{ENEMIES_REMAINING_TEX, L"enemies_remaining"},
		{HP_TEX, L"hp"},
		{WAVE_TEX, L"wave"},
		{NEONLAND_TEX, L"neonland"},
		{NUM_KEYS_TEX, L"num_keys"},
		{GROUND1_TEX, L"ground1"},
		{GROUND2_TEX, L"ground2"},
		{GROUND3_TEX, L"ground3"},
		{LEVEL1_BT_TEX, L"level1_bt"},
		{LEVEL2_BT_TEX, L"level2_bt"},
		{LEVEL3_BT_TEX, L"level3_bt"},
		{PAUSED_TEX, L"paused"},
		{RESUME_BT_TEX, L"resume_bt"},
		{EXIT_BT_TEX, L"exit_bt"},
		{QUIT_BT_TEX, L"quit_bt"},
		{GAME_OVER_TEX, L"game_over"},
		{LEVEL_CLEARED_TEX, L"level_cleared"},
		{SPHERE_HEART_TEX, L"sphere_heart"},
		{SPHERE_360_SHOTS_TEX, L"sphere_360_shots"},
		{LOCK_TEX, L"lock"},
		{BY_TEX, L"by"}
	};

	std::array<winrt::com_ptr<ID3D12Resource>, MeshTypeCount> vertexUploadBuffers;
	std::array<winrt::com_ptr<ID3D12Resource>, MeshTypeCount> indexUploadBuffers;

	for (uint32_t i = 0; i < MeshTypeCount; i++)
	{
		uint32_t vertexCount;
		uint32_t indexCount;

		LoadMesh(static_cast<MeshType>(i), vertexCount, indexCount, vertexUploadBuffers[i].get(), indexUploadBuffers[i].get());

		_vertexBufferViews[i].BufferLocation = _vertexBuffers[i]->GetGPUVirtualAddress();
		_vertexBufferViews[i].StrideInBytes = sizeof(Vertex);
		_vertexBufferViews[i].SizeInBytes = sizeof(Vertex) * vertexCount;

		_indexBufferViews[i].BufferLocation = _indexBuffers[i]->GetGPUVirtualAddress();
		_indexBufferViews[i].SizeInBytes = sizeof(uint16_t) * indexCount;
		_indexBufferViews[i].Format = DXGI_FORMAT_R16_UINT;
	}

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = MaxFramesInFlight;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	winrt::check_hresult(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_globalUniformsVHeap)));

	CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);

	CD3DX12_RESOURCE_DESC globalUniformsBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(MaxFramesInFlight * AlignedGlobalUnformsBufferSize);
	winrt::check_hresult(device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&globalUniformsBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_globalUniformsBuffer)));

	D3D12_GPU_VIRTUAL_ADDRESS globalUniformsBufferVGpuAddress = _globalUniformsBuffer->GetGPUVirtualAddress();
	CD3DX12_CPU_DESCRIPTOR_HANDLE globalUniformsVCpuHandle(_globalUniformsVHeap->GetCPUDescriptorHandleForHeapStart());
	_globalUniformsVDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (int n = 0; n < MaxFramesInFlight; n++)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
		desc.BufferLocation = globalUniformsBufferVGpuAddress;
		desc.SizeInBytes = AlignedGlobalUnformsBufferSize;
		device->CreateConstantBufferView(&desc, globalUniformsVCpuHandle);

		globalUniformsBufferVGpuAddress += desc.SizeInBytes;
		globalUniformsVCpuHandle.Offset(_globalUniformsVDescriptorSize);
	}

	CD3DX12_RANGE readRange(0, 0);
	winrt::check_hresult(_globalUniformsBuffer->Map(0, &readRange, reinterpret_cast<void**>(&_mappedGlobalUniformsBuffer)));
	ZeroMemory(_mappedGlobalUniformsBuffer, MaxFramesInFlight * AlignedGlobalUnformsBufferSize);

	CD3DX12_RESOURCE_DESC instanceBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(MaxFramesInFlight * AlignedInstanceBufferSize);
	winrt::check_hresult(device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&instanceBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_instanceBuffer)));

	winrt::check_hresult(_instanceBuffer->Map(0, &readRange, reinterpret_cast<void**>(&_mappedInstanceBuffer)));
	ZeroMemory(_mappedInstanceBuffer, MaxFramesInFlight * AlignedInstanceBufferSize);

	// TEMP


	// Cube vertices. Each vertex has a position and a color.
	VertexPositionColor cubeVertices[] =
	{
		{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f) },
		{ XMFLOAT3(0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f) },
		{ XMFLOAT3(0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
	};

	const UINT vertexBufferSize = sizeof(cubeVertices);

	// Create the vertex buffer resource in the GPU's default heap and copy vertex data into it using the upload heap.
	// The upload resource must not be released until after the GPU has finished using it.
	winrt::com_ptr<ID3D12Resource> vertexBufferUpload;

	CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
	winrt::check_hresult(device->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer)));

	winrt::check_hresult(device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBufferUpload)));

	// Upload the vertex buffer to the GPU.
	{
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = reinterpret_cast<BYTE*>(cubeVertices);
		vertexData.RowPitch = vertexBufferSize;
		vertexData.SlicePitch = vertexData.RowPitch;

		UpdateSubresources(_commandList.get(), m_vertexBuffer.get(), vertexBufferUpload.get(), 0, 0, 1, &vertexData);

		CD3DX12_RESOURCE_BARRIER vertexBufferResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		_commandList->ResourceBarrier(1, &vertexBufferResourceBarrier);
	}

	// Load mesh indices. Each trio of indices represents a triangle to be rendered on the screen.
	// For example: 0,2,1 means that the vertices with indexes 0, 2 and 1 from the vertex buffer compose the
	// first triangle of this mesh.
	unsigned short cubeIndices[] =
	{
		0, 2, 1, // -x
		1, 2, 3,

		4, 5, 6, // +x
		5, 7, 6,

		0, 1, 5, // -y
		0, 5, 4,

		2, 6, 7, // +y
		2, 7, 3,

		0, 4, 6, // -z
		0, 6, 2,

		1, 3, 7, // +z
		1, 7, 5,
	};

	const UINT indexBufferSize = sizeof(cubeIndices);

	// Create the index buffer resource in the GPU's default heap and copy index data into it using the upload heap.
	// The upload resource must not be released until after the GPU has finished using it.
	winrt::com_ptr<ID3D12Resource> indexBufferUpload;

	CD3DX12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexBufferSize);
	winrt::check_hresult(device->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_indexBuffer)));

	winrt::check_hresult(device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBufferUpload)));

	// Upload the index buffer to the GPU.
	{
		D3D12_SUBRESOURCE_DATA indexData = {};
		indexData.pData = reinterpret_cast<BYTE*>(cubeIndices);
		indexData.RowPitch = indexBufferSize;
		indexData.SlicePitch = indexData.RowPitch;

		UpdateSubresources(_commandList.get(), m_indexBuffer.get(), indexBufferUpload.get(), 0, 0, 1, &indexData);

		CD3DX12_RESOURCE_BARRIER indexBufferResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
		_commandList->ResourceBarrier(1, &indexBufferResourceBarrier);
	}

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(VertexPositionColor);
	m_vertexBufferView.SizeInBytes = sizeof(cubeVertices);

	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = sizeof(cubeIndices);
	m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;

	// TEMP


	winrt::check_hresult(_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { _commandList.get() };
	_deviceResources->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	_deviceResources->WaitForGpu();

	_loadingComplete = true;
	Neon_Start();
}

void Renderer::CreateWindowSizeDependentResources()
{
	D3D12_VIEWPORT viewport = _deviceResources->GetScreenViewport();
	_scissorRect = { 0, 0, static_cast<int32_t>(viewport.Width), static_cast<int32_t>(viewport.Height) };
}

bool Renderer::Render()
{
	if (!_loadingComplete)
	{
		return false;
	}

	Size outputSize = _deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	FrameData frameData = Neon_Render(aspectRatio);

	uint8_t* currentGlobalUniforms = _mappedGlobalUniformsBuffer + (_deviceResources->GetCurrentFrameIndex() * AlignedGlobalUnformsBufferSize);
	memcpy(currentGlobalUniforms, &frameData.globalUniforms, sizeof(frameData.globalUniforms));

	uint8_t* currentInstances = _mappedInstanceBuffer + (_deviceResources->GetCurrentFrameIndex() * AlignedInstanceBufferSize);
	memcpy(currentInstances, frameData.instances, frameData.instanceCount * sizeof(Instance));

	winrt::check_hresult(_deviceResources->GetCommandAllocator()->Reset());

	winrt::check_hresult(_commandList->Reset(_deviceResources->GetCommandAllocator(), _pipelineState.get()));

	_commandList->SetGraphicsRootSignature(_rootSignature.get());
	ID3D12DescriptorHeap* ppHeaps[] = { _globalUniformsVHeap.get() };
	_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(_globalUniformsVHeap->GetGPUDescriptorHandleForHeapStart(), _deviceResources->GetCurrentFrameIndex(), _globalUniformsVDescriptorSize);
	_commandList->SetGraphicsRootDescriptorTable(0, gpuHandle);

	D3D12_VIEWPORT viewport = _deviceResources->GetScreenViewport();
	_commandList->RSSetViewports(1, &viewport);
	_commandList->RSSetScissorRects(1, &_scissorRect);

	CD3DX12_RESOURCE_BARRIER renderTargetResourceBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(_deviceResources->GetRenderTarget(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	_commandList->ResourceBarrier(1, &renderTargetResourceBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView = _deviceResources->GetRenderTargetView();
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = _deviceResources->GetDepthStencilView();

	XMVECTORF32 clearColor = { frameData.clearColor.x, frameData.clearColor.y, frameData.clearColor.z, 1 };
	_commandList->ClearRenderTargetView(renderTargetView, clearColor, 0, nullptr);
	_commandList->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	_commandList->OMSetRenderTargets(1, &renderTargetView, false, &depthStencilView);

	uint32_t startOffset = 0;
	uint32_t prevTexIdx = TextureTypeCount;
	uint32_t prevShaderIdx = ShaderTypeCount;
	uint32_t prevMeshIdx = MeshTypeCount;

	D3D12_VERTEX_BUFFER_VIEW instanceBufferView;
	instanceBufferView.BufferLocation = _instanceBuffer->GetGPUVirtualAddress() + AlignedInstanceBufferSize * _deviceResources->GetCurrentFrameIndex();
	instanceBufferView.StrideInBytes = sizeof(Instance);
	instanceBufferView.SizeInBytes = AlignedInstanceBufferSize;

	_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (size_t groupIdx = 0; groupIdx < frameData.groupCount; groupIdx++)
	{
		uint32_t instanceCount = frameData.groupSizes[groupIdx];
		uint32_t meshIdx = frameData.groupMeshes[groupIdx];

		if (meshIdx != prevMeshIdx) {
			_commandList->IASetVertexBuffers(0, 1, &_vertexBufferViews[meshIdx]);
			_commandList->IASetVertexBuffers(1, 1, &instanceBufferView);
			_commandList->IASetIndexBuffer(&_indexBufferViews[meshIdx]);
			prevMeshIdx = meshIdx;
		}

		uint32_t indexCount = _indexBufferViews[meshIdx].SizeInBytes / sizeof(uint16_t);
		_commandList->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, startOffset);
		startOffset += instanceCount;
	}

	_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	_commandList->IASetIndexBuffer(&m_indexBufferView);
	_commandList->DrawIndexedInstanced(36, 1, 0, 0, 0);

	CD3DX12_RESOURCE_BARRIER presentResourceBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(_deviceResources->GetRenderTarget(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	_commandList->ResourceBarrier(1, &presentResourceBarrier);

	winrt::check_hresult(_commandList->Close());

	ID3D12CommandList* ppCommandLists[] = { _commandList.get() };
	_deviceResources->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	return true;
}