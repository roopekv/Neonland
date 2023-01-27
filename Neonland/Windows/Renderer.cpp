#include "pch.h"

#include "Renderer.hpp"

#include "../Neonland.h"
#include "../Engine/MathUtils.hpp"

using namespace DirectX;

struct Vertex {
	float3 position;
	float3 normal;
	float2 texCoords;
};

const uint32_t Renderer::AlignedInstanceBufferSize = (sizeof(Instance) * MAX_INSTANCE_COUNT + 255) & ~255;

Renderer::Renderer(const std::shared_ptr<DeviceResources>& deviceResources) :
	_mappedGlobalUniformsBuffer(nullptr),
	_mappedInstanceBuffer(nullptr),
	_loadingComplete{ false },
	_deviceResources(deviceResources),
	_CBV_SRV_UAV_ViewDescriptorSize(_deviceResources->GetD3DDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV))
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

void Renderer::LoadMesh(MeshType type, ID3D12Resource* vertexUploadBuffer, ID3D12Resource* indexUploadBuffer) {

	static const std::map<MeshType, std::wstring> meshIdxToName = {
		{PLANE_MESH, L"plane"},
		{CROSSHAIR_MESH, L"crosshair"},
		{SPREAD_MESH, L"spread_circle"},
		{CUBE_MESH, L"cube"},
		{SHARD_MESH, L"shard"},
		{SPHERE_MESH, L"sphere"}
	};

	std::wstring filename = meshIdxToName.at(type) + L".obj";

	WaveFrontReader<uint16_t> wfReader;
	HRESULT hr = wfReader.Load(filename.c_str());
	if (FAILED(hr)) {
		std::abort();
	}

	uint16_t vertexCount = wfReader.vertices.size();
	uint16_t indexCount = wfReader.indices.size();

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

	_vertexBufferViews[type].BufferLocation = _vertexBuffers[type]->GetGPUVirtualAddress();
	_vertexBufferViews[type].StrideInBytes = sizeof(Vertex);
	_vertexBufferViews[type].SizeInBytes = sizeof(Vertex) * vertexCount;

	_indexBufferViews[type].BufferLocation = _indexBuffers[type]->GetGPUVirtualAddress();
	_indexBufferViews[type].SizeInBytes = sizeof(uint16_t) * indexCount;
	_indexBufferViews[type].Format = DXGI_FORMAT_R16_UINT;
}

void Renderer::LoadTexture(TextureType type, ID3D12Resource* uploadBuffer) {

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

	std::wstring filename = textureIdxToName.at(type) + L".dds";

	TexMetadata metadata;
	ScratchImage img;
	LoadFromDDSFile(filename.c_str(), DDS_FLAGS_NONE, &metadata, img);

	D3D12_RESOURCE_DESC texResourceDesc = {};
	texResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texResourceDesc.Alignment = 0;
	texResourceDesc.Width = metadata.width;
	texResourceDesc.Height = metadata.height;
	texResourceDesc.DepthOrArraySize = 1;
	texResourceDesc.MipLevels = 1;
	texResourceDesc.Format = metadata.format;
	texResourceDesc.SampleDesc.Count = 1;
	texResourceDesc.SampleDesc.Quality = 0;
	texResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	auto device = _deviceResources->GetD3DDevice();

	CD3DX12_HEAP_PROPERTIES defaultHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	winrt::check_hresult(device->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&texResourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(_textureBuffers[type].put())));

	uint64_t textureUploadBufferSize;
	uint64_t rowSizeInBytes;
	device->GetCopyableFootprints(&texResourceDesc, 0, 1, 0, nullptr, nullptr, &rowSizeInBytes, &textureUploadBufferSize);

	CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC textureBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(textureUploadBufferSize);
	winrt::check_hresult(device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&textureBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadBuffer)));

	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = img.GetPixels();
	textureData.RowPitch = rowSizeInBytes;
	textureData.SlicePitch = rowSizeInBytes * texResourceDesc.Height;

	UpdateSubresources(_commandList.get(), _textureBuffers[type].get(), uploadBuffer, 0, 0, 1, &textureData);

	CD3DX12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(_textureBuffers[type].get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	_commandList->ResourceBarrier(1, &resourceBarrier);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = metadata.format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(_descHeap->GetCPUDescriptorHandleForHeapStart(), MaxFramesInFlight + type, _CBV_SRV_UAV_ViewDescriptorSize);
	device->CreateShaderResourceView(_textureBuffers[type].get(), &srvDesc, cpuHandle);

	TexSize texSize;
	texSize.width = metadata.width;
	texSize.height = metadata.height;
	Neon_UpdateTextureSize(type, texSize);
}

void Renderer::CreateDeviceDependentResources()
{
	auto device = _deviceResources->GetD3DDevice();

	CD3DX12_DESCRIPTOR_RANGE uniformsRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, 0);
	CD3DX12_ROOT_PARAMETER uniformsParam;
	uniformsParam.InitAsDescriptorTable(1, &uniformsRange, D3D12_SHADER_VISIBILITY_VERTEX);

	CD3DX12_DESCRIPTOR_RANGE textureRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, 3);
	CD3DX12_ROOT_PARAMETER textureParam;
	textureParam.InitAsDescriptorTable(1, &textureRange, D3D12_SHADER_VISIBILITY_PIXEL);

	std::array<CD3DX12_ROOT_PARAMETER, 2> rootParameters;

	rootParameters[0] = uniformsParam;
	rootParameters[1] = textureParam;

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

	CD3DX12_STATIC_SAMPLER_DESC staticSampler(0);
	staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	CD3DX12_ROOT_SIGNATURE_DESC descRootSignature(rootParameters.size(), rootParameters.data(), 1, &staticSampler, rootSignatureFlags);

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

	static const std::array<D3D12_INPUT_ELEMENT_DESC, 8> inputLayout =
	{
		D3D12_INPUT_ELEMENT_DESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		D3D12_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		D3D12_INPUT_ELEMENT_DESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

		D3D12_INPUT_ELEMENT_DESC{ "INSTANCETF",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		D3D12_INPUT_ELEMENT_DESC{ "INSTANCETF",  1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		D3D12_INPUT_ELEMENT_DESC{ "INSTANCETF",  2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		D3D12_INPUT_ELEMENT_DESC{ "INSTANCETF",  3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
		D3D12_INPUT_ELEMENT_DESC{ "INSTANCECOLOR",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 },
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC state = {};
	state.InputLayout = { inputLayout.data(), inputLayout.size()};
	state.pRootSignature = _rootSignature.get();
	state.VS = CD3DX12_SHADER_BYTECODE(vertexShader.data(), vertexShader.size());
	state.PS = CD3DX12_SHADER_BYTECODE(fragmentShader.data(), fragmentShader.size());
	state.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	state.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	state.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	state.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	state.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	state.NumRenderTargets = 1;
	state.RTVFormats[0] = _deviceResources->GetBackBufferFormat();
	state.DSVFormat = _deviceResources->GetDepthBufferFormat();
	state.SampleDesc.Count = 1;

	winrt::check_hresult(_deviceResources->GetD3DDevice()->CreateGraphicsPipelineState(&state, IID_PPV_ARGS(&_pipelineState)));

	winrt::check_hresult(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _deviceResources->GetCommandAllocator(), _pipelineState.get(), IID_PPV_ARGS(&_commandList)));

	std::array<winrt::com_ptr<ID3D12Resource>, MeshTypeCount> vertexUploadBuffers;
	std::array<winrt::com_ptr<ID3D12Resource>, MeshTypeCount> indexUploadBuffers;

	for (uint32_t i = 0; i < MeshTypeCount; i++)
	{
		LoadMesh(static_cast<MeshType>(i), vertexUploadBuffers[i].get(), indexUploadBuffers[i].get());
	}

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.NumDescriptors = MaxFramesInFlight + TextureTypeCount;
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	winrt::check_hresult(device->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&_descHeap)));

	CD3DX12_HEAP_PROPERTIES uploadHeapProperties(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC globalUniformsBufferDesc = CD3DX12_RESOURCE_DESC::Buffer(MaxFramesInFlight * AlignedGlobalUnformsBufferSize);
	winrt::check_hresult(device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&globalUniformsBufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_globalUniformsBuffer)));

	D3D12_GPU_VIRTUAL_ADDRESS globalUniformsViewGpuAddress = _globalUniformsBuffer->GetGPUVirtualAddress();
	CD3DX12_CPU_DESCRIPTOR_HANDLE globalUniformsViewCpuHandle(_descHeap->GetCPUDescriptorHandleForHeapStart());

	for (int n = 0; n < MaxFramesInFlight; n++)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
		desc.BufferLocation = globalUniformsViewGpuAddress;
		desc.SizeInBytes = AlignedGlobalUnformsBufferSize;
		device->CreateConstantBufferView(&desc, globalUniformsViewCpuHandle);

		globalUniformsViewGpuAddress += desc.SizeInBytes;
		globalUniformsViewCpuHandle.Offset(_CBV_SRV_UAV_ViewDescriptorSize);
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

	std::array<winrt::com_ptr<ID3D12Resource>, TextureTypeCount> textureUploadBuffers;
	for (uint32_t i = 0; i < MeshTypeCount; i++)
	{
		LoadTexture(static_cast<TextureType>(i), textureUploadBuffers[i].get());
	}

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

	// Update
	auto outputSize = _deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	FrameData frameData = Neon_Render(aspectRatio);

	uint8_t* currentGlobalUniforms = _mappedGlobalUniformsBuffer + (_deviceResources->GetCurrentFrameIndex() * AlignedGlobalUnformsBufferSize);
	memcpy(currentGlobalUniforms, &frameData.globalUniforms, sizeof(frameData.globalUniforms));

	uint8_t* currentInstances = _mappedInstanceBuffer + (_deviceResources->GetCurrentFrameIndex() * AlignedInstanceBufferSize);
	memcpy(currentInstances, frameData.instances, frameData.instanceCount * sizeof(Instance));

	// Prepare for rendering
	winrt::check_hresult(_deviceResources->GetCommandAllocator()->Reset());
	winrt::check_hresult(_commandList->Reset(_deviceResources->GetCommandAllocator(), _pipelineState.get()));

	_commandList->SetGraphicsRootSignature(_rootSignature.get());

	std::array<ID3D12DescriptorHeap*, 1> heaps = { _descHeap.get() };
	_commandList->SetDescriptorHeaps(heaps.size(), heaps.data());

	CD3DX12_GPU_DESCRIPTOR_HANDLE globalUniformsGpuHandle(_descHeap->GetGPUDescriptorHandleForHeapStart(), _deviceResources->GetCurrentFrameIndex(), _CBV_SRV_UAV_ViewDescriptorSize);
	_commandList->SetGraphicsRootDescriptorTable(0, globalUniformsGpuHandle);

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

	auto device = _deviceResources->GetD3DDevice();
	for (size_t groupIdx = 0; groupIdx < frameData.groupCount; groupIdx++)
	{
		size_t instanceCount = frameData.groupSizes[groupIdx];
		uint32_t meshIdx = frameData.groupMeshes[groupIdx];
		uint32_t texIdx = frameData.groupTextures[groupIdx];

		if (meshIdx != prevMeshIdx) {
			_commandList->IASetVertexBuffers(0, 1, &_vertexBufferViews[meshIdx]);
			_commandList->IASetVertexBuffers(1, 1, &instanceBufferView);
			_commandList->IASetIndexBuffer(&_indexBufferViews[meshIdx]);
			prevMeshIdx = meshIdx;
		}

		if (texIdx != prevTexIdx) {
			CD3DX12_GPU_DESCRIPTOR_HANDLE texGpuHandle(_descHeap->GetGPUDescriptorHandleForHeapStart(), MaxFramesInFlight + texIdx, _CBV_SRV_UAV_ViewDescriptorSize);
			_commandList->SetGraphicsRootDescriptorTable(1, texGpuHandle);
			prevTexIdx = texIdx;
		}

		size_t indexCount = _indexBufferViews[meshIdx].SizeInBytes / sizeof(uint16_t);
		_commandList->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, startOffset);
		startOffset += instanceCount;
	}

	CD3DX12_RESOURCE_BARRIER presentResourceBarrier =
		CD3DX12_RESOURCE_BARRIER::Transition(_deviceResources->GetRenderTarget(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	_commandList->ResourceBarrier(1, &presentResourceBarrier);

	winrt::check_hresult(_commandList->Close());

	std::array<ID3D12CommandList*, 1> commandLists = { _commandList.get() };
	_deviceResources->GetCommandQueue()->ExecuteCommandLists(commandLists.size(), commandLists.data());

	return true;
}