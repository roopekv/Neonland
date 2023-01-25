#include "pch.h"

#include "Renderer.hpp"

#include <fstream>

#include "../Neonland.h"
#include "../Engine/MathUtils.hpp"

using namespace DirectX;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;

Renderer::Renderer(const std::shared_ptr<DeviceResources>& deviceResources) :
	_mappedGlobalUniformsBuffer(nullptr),
	_loadingComplete{ false },
	_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

Renderer::~Renderer()
{
	_globalUniformsBuffer->Unmap(0, nullptr);
	_mappedGlobalUniformsBuffer = nullptr;
}

struct Vertex {
	float3 position;
	float3 normal;
	float2 texCoords;
};

void Renderer::LoadMesh(char* meshData, ID3D12Resource** vertexBuffer, ID3D12Resource** indexBuffer, uint32_t& vertexCount, uint32_t& indexCount) {
	// The first 4 bytes of the BasicMesh format define the number of vertices in the mesh.
	vertexCount = *reinterpret_cast<uint32_t*>(meshData);

	// The following 4 bytes define the number of indices in the mesh.
	indexCount = *reinterpret_cast<uint32_t*>(meshData + sizeof(uint32_t));

	// The next segment of the BasicMesh format contains the vertices of the mesh.
	Vertex* vertices = reinterpret_cast<Vertex*>(meshData + sizeof(uint32_t) * 2);

	// The last segment of the BasicMesh format contains the indices of the mesh.
	uint16_t* indices = reinterpret_cast<uint16_t*>(meshData + sizeof(uint32_t) * 2 + sizeof(Vertex) * vertexCount);

	// Create the vertex and index buffers with the mesh data.

	auto d3dDevice = _deviceResources->GetD3DDevice();

	// Create the vertex buffer resource in the GPU's default heap and copy vertex data into it using the upload heap.
	// The upload resource must not be released until after the GPU has finished using it.
	winrt::com_ptr<ID3D12Resource> vertexBufferUpload;

	CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC vertexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(Vertex) * vertexCount);
	winrt::check_hresult(d3dDevice->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(vertexBuffer)));

	CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	winrt::check_hresult(d3dDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vertexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBufferUpload)));

	// Upload the vertex buffer to the GPU.
	{
		D3D12_SUBRESOURCE_DATA vertexData = {};
		vertexData.pData = vertices;
		vertexData.RowPitch = sizeof(Vertex) * vertexCount;
		vertexData.SlicePitch = vertexData.RowPitch;

		UpdateSubresources(_commandList.get(), *vertexBuffer, vertexBufferUpload.get(), 0, 0, 1, &vertexData);

		CD3DX12_RESOURCE_BARRIER vertexBufferResourceBarrier =
			CD3DX12_RESOURCE_BARRIER::Transition(*vertexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		_commandList->ResourceBarrier(1, &vertexBufferResourceBarrier);
	}

	// Create the index buffer resource in the GPU's default heap and copy index data into it using the upload heap.
	// The upload resource must not be released until after the GPU has finished using it.
	winrt::com_ptr<ID3D12Resource> indexBufferUpload;

	CD3DX12_RESOURCE_DESC indexBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(indexCount * sizeof(uint16_t));
	winrt::check_hresult(d3dDevice->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(indexBuffer)));

	winrt::check_hresult(d3dDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&indexBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBufferUpload)));

	// Upload the index buffer to the GPU.
	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = indices;
	indexData.RowPitch = indexCount * sizeof(uint16_t);
	indexData.SlicePitch = indexData.RowPitch;

	UpdateSubresources(_commandList.get(), *indexBuffer, indexBufferUpload.get(), 0, 0, 1, &indexData);

	CD3DX12_RESOURCE_BARRIER indexBufferResourceBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(*indexBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
	_commandList->ResourceBarrier(1, &indexBufferResourceBarrier);
}

void Renderer::LoadTexture() {

}

void Renderer::CreateDeviceDependentResources()
{
	auto d3dDevice = _deviceResources->GetD3DDevice();

	// Create a root signature with a single constant buffer slot.
	CD3DX12_DESCRIPTOR_RANGE range;
	CD3DX12_ROOT_PARAMETER parameter;

	range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	parameter.InitAsDescriptorTable(1, &range, D3D12_SHADER_VISIBILITY_VERTEX);

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Only the input assembler stage needs access to the constant buffer.
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
	descRootSignature.Init(1, &parameter, 0, nullptr, rootSignatureFlags);

	winrt::com_ptr<ID3DBlob> pSignature;

	winrt::com_ptr<ID3DBlob> pError;
	winrt::check_hresult(D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, pSignature.put(), pError.put()));
	winrt::check_hresult(d3dDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&_rootSignature)));

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
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEX_COORDS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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

	// Create and upload cube geometry resources to the GPU.
	// Create a command list.
	winrt::check_hresult(d3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _deviceResources->GetCommandAllocator(), _pipelineState.get(), IID_PPV_ARGS(&_commandList)));

	// Create a descriptor heap for the constant buffers.
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = MaxFramesInFlight;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	// This flag indicates that this descriptor heap can be bound to the pipeline and that descriptors contained in it can be referenced by a root table.
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	winrt::check_hresult(d3dDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_cbvHeap)));

	CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC constantBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(MaxFramesInFlight * AlignedConstantBufferSize);
	winrt::check_hresult(d3dDevice->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&constantBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_globalUniformsBuffer)));

	// Create constant buffer views to access the upload buffer.
	D3D12_GPU_VIRTUAL_ADDRESS cbvGpuAddress = _globalUniformsBuffer->GetGPUVirtualAddress();
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvCpuHandle(_cbvHeap->GetCPUDescriptorHandleForHeapStart());
	_cbvDescriptorSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	for (int n = 0; n < MaxFramesInFlight; n++)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
		desc.BufferLocation = cbvGpuAddress;
		desc.SizeInBytes = AlignedConstantBufferSize;
		d3dDevice->CreateConstantBufferView(&desc, cbvCpuHandle);

		cbvGpuAddress += desc.SizeInBytes;
		cbvCpuHandle.Offset(_cbvDescriptorSize);
	}

	// Map the constant buffers.
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	winrt::check_hresult(_globalUniformsBuffer->Map(0, &readRange, reinterpret_cast<void**>(&_mappedGlobalUniformsBuffer)));
	ZeroMemory(_mappedGlobalUniformsBuffer, MaxFramesInFlight * AlignedConstantBufferSize);
	// We don't unmap this until the app closes. Keeping things mapped for the lifetime of the resource is okay.

	// Close the command list and execute it to begin the vertex/index buffer copy into the GPU's default heap.
	winrt::check_hresult(_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { _commandList.get() };
	_deviceResources->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Create vertex/index buffer views.

	const std::map<MeshType, std::string> meshIdxToName = {
		{PLANE_MESH, "plane"},
		{CROSSHAIR_MESH, "crosshair"},
		{SPREAD_MESH, "spread_circle"},
		{CUBE_MESH, "cube"},
		{SHARD_MESH, "shard"},
		{SPHERE_MESH, "sphere"}
	};

	const std::map<TextureType, std::string> textureIdxToName = {
		{NO_TEX, "blank"},
		{ENEMIES_REMAINING_TEX, "enemies_remaining"},
		{HP_TEX, "hp"},
		{WAVE_TEX, "wave"},
		{NEONLAND_TEX, "neonland"},
		{NUM_KEYS_TEX, "num_keys"},
		{GROUND1_TEX, "ground1"},
		{GROUND2_TEX, "ground2"},
		{GROUND3_TEX, "ground3"},
		{LEVEL1_BT_TEX, "level1_bt"},
		{LEVEL2_BT_TEX, "level2_bt"},
		{LEVEL3_BT_TEX, "level3_bt"},
		{PAUSED_TEX, "paused"},
		{RESUME_BT_TEX, "resume_bt"},
		{EXIT_BT_TEX, "exit_bt"},
		{QUIT_BT_TEX, "quit_bt"},
		{GAME_OVER_TEX, "game_over"},
		{LEVEL_CLEARED_TEX, "level_cleared"},
		{SPHERE_HEART_TEX, "sphere_heart"},
		{SPHERE_360_SHOTS_TEX, "sphere_360_shots"},
		{LOCK_TEX, "lock"},
		{BY_TEX, "by"}
	};

	for (uint32_t i = 0; i < MeshTypeCount; i++)
	{
		auto data = loadFile(meshIdxToName.at(static_cast<MeshType>(i)));

		uint32_t vertexCount;
		uint32_t indexCount;

		LoadMesh(data.data(), _vertexBuffers[i].put(), _indexBuffers[i].put(), vertexCount, indexCount);

		_vertexBufferViews[i].BufferLocation = _vertexBuffers[i]->GetGPUVirtualAddress();
		_vertexBufferViews[i].StrideInBytes = sizeof(Vertex);
		_vertexBufferViews[i].SizeInBytes = sizeof(Vertex) * vertexCount;

		_indexBufferViews[i].BufferLocation = _indexBuffers[i]->GetGPUVirtualAddress();
		_indexBufferViews[i].SizeInBytes = sizeof(uint16_t) * indexCount;
		_indexBufferViews[i].Format = DXGI_FORMAT_R16_UINT;
	}

	// Wait for the command list to finish executing; the vertex/index buffers need to be uploaded to the GPU before the upload resources go out of scope.
	_deviceResources->WaitForGpu();

	_loadingComplete = true;
	Neon_Start();
}

// Initializes view parameters when the window size changes.
void Renderer::CreateWindowSizeDependentResources()
{
	Size outputSize = _deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;

	D3D12_VIEWPORT viewport = _deviceResources->GetScreenViewport();
	_scissorRect = { 0, 0, static_cast<int32_t>(viewport.Width), static_cast<int32_t>(viewport.Height) };
}

// Renders one frame using the vertex and pixel shaders.
bool Renderer::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!_loadingComplete)
	{
		return false;
	}

	Size outputSize = _deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	FrameData frameData = Neon_Render(aspectRatio);

	uint8_t* destination = _mappedGlobalUniformsBuffer + (_deviceResources->GetCurrentFrameIndex() * AlignedConstantBufferSize);
	memcpy(destination, &frameData.globalUniforms, sizeof(frameData.globalUniforms));

	winrt::check_hresult(_deviceResources->GetCommandAllocator()->Reset());

	// The command list can be reset anytime after ExecuteCommandList() is called.
	winrt::check_hresult(_commandList->Reset(_deviceResources->GetCommandAllocator(), _pipelineState.get()));

	// Set the graphics root signature and descriptor heaps to be used by this frame.
	_commandList->SetGraphicsRootSignature(_rootSignature.get());
	ID3D12DescriptorHeap* ppHeaps[] = { _cbvHeap.get() };
	_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// Bind the current frame's constant buffer to the pipeline.
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(_cbvHeap->GetGPUDescriptorHandleForHeapStart(), _deviceResources->GetCurrentFrameIndex(), _cbvDescriptorSize);
	_commandList->SetGraphicsRootDescriptorTable(0, gpuHandle);

	// Set the viewport and scissor rectangle.
	D3D12_VIEWPORT viewport = _deviceResources->GetScreenViewport();
	_commandList->RSSetViewports(1, &viewport);
	_commandList->RSSetScissorRects(1, &_scissorRect);

	// Indicate this resource will be in use as a render target.
	CD3DX12_RESOURCE_BARRIER renderTargetResourceBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(_deviceResources->GetRenderTarget(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	_commandList->ResourceBarrier(1, &renderTargetResourceBarrier);

	// Record drawing commands.
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

	_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	for (size_t groupIdx = 0; groupIdx < frameData.groupCount; groupIdx++)
	{
		uint32_t instanceCount = frameData.groupSizes[groupIdx];
		uint32_t meshIdx = frameData.groupMeshes[groupIdx];

		if (meshIdx != prevMeshIdx) {
			_commandList->IASetVertexBuffers(0, 1, &_vertexBufferViews[meshIdx]);
			_commandList->IASetIndexBuffer(&_indexBufferViews[meshIdx]);
			prevMeshIdx = meshIdx;
		}

		_commandList->DrawIndexedInstanced(_indexBufferViews[meshIdx].SizeInBytes / sizeof(uint16_t), 1, 0, 0, 0);
		startOffset += instanceCount;
	}

	// Indicate that the render target will now be used to present when the command list is done executing.
	CD3DX12_RESOURCE_BARRIER presentResourceBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(_deviceResources->GetRenderTarget(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	_commandList->ResourceBarrier(1, &presentResourceBarrier);

	winrt::check_hresult(_commandList->Close());

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { _commandList.get() };
	_deviceResources->GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	return true;
}
