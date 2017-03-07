#include "Render.h"

#include "Game.h"

#include "Camera.h"

#include "WICTextureLoader12.h"

#include <dxgi1_4.h>
#include <d3dx12.h>
#include <d3dcompiler.h>


struct VertexType
{
	float position[3];
	float uv[2];
};


struct tPixel
{
	float r;
	float g;
	float b;
	float a;
};



struct tPointLight
{
	Vector3 mPosition;
	float padding1;
	Vector3 mColour;
	float padding2;
};

struct tSphere
{
	Vector3 mPosition;
	float mRadius;
};

struct tTri
{
	Vector3 mV0;
	float padding1;
	Vector3 mV1;
	float padding2;
	Vector3 mV2;
	float padding3;
};





const int MAX_LIGHTS = 32;
const int MAX_SPHERES = 32;
const int MAX_TRIS = 32;

struct tSharedCB
{
	
	//------tResolutionCB
	int width;
	int height;
	
	//------tCameraCB
	float cam_pos_x, cam_pos_y, cam_pos_z;
	float cam_orientation_00, cam_orientation_01, cam_orientation_02;
	float cam_orientation_10, cam_orientation_11, cam_orientation_12;
	float cam_orientation_20, cam_orientation_21, cam_orientation_22;

	//------tCameraCB
	int num_lights;
	int num_spheres;
	int num_tris;

	//------tFrameValuesCB
	float noise_offset_x;
	float noise_offset_y;
	float time;


	tPointLight lights[MAX_LIGHTS];
	tSphere spheres[MAX_SPHERES];
	tTri tris[MAX_TRIS];
};





extern HWND gHWnd;
extern int gWidth;
extern int gHeight;
extern bool gFullScreen;
extern bool gVsync;


void D3D_FN(HRESULT result)
{
	if (result != S_OK)
	{
		Log::Printf("D3D api error! (0x%.8X)\n", result);
	}
};



void WaitForFence(ID3D12Fence* fence, UINT64 completionValue, HANDLE waitEvent)
{
	if (fence->GetCompletedValue() < completionValue)
	{
		fence->SetEventOnCompletion(completionValue, waitEvent);
		WaitForSingleObject(waitEvent, INFINITE);
	}
}


Render* Render::Instance = NULL;

Render::Render()
{
	Instance = this;
	
	mCurrentBackBuffer = 0;

	InitDevice();
}


Render::~Render()
{
}



bool Render::InitDevice()
{
	// Enable the debug layers when in debug mode
	// If this fails, install the Graphics Tools for Windows. On Windows 10, open settings, Apps, Apps & Features, Optional features, Add Feature, and add the graphics tools

	UINT dxgiFactoryFlags = 0;
#ifdef _DEBUG
	ComPtr<ID3D12Debug> debugController;
	D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));

	if (debugController)
	{
		debugController->EnableDebugLayer();
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif

	/////////////////////////////  CreateDeviceAndSwapChain  /////////////////////////////
	ComPtr<IDXGIFactory4> factory;
	CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
	if (false) //use warp device
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));

		D3D12CreateDevice( warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mD3DDevice) );
	}
	else
	{
		D3D12CreateDevice( NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mD3DDevice) );
	}
	


	
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	D3D_FN( mD3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)) );


	ComPtr<IDXGIFactory4> dxgiFactory;
	D3D_FN( CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) );


	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	swapChainDesc.BufferCount = QUEUE_SLOT_COUNT;
	// This is _UNORM but we'll use a _SRGB view on this. See CreateRenderTargetView() for details, it must match what we specify here
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferDesc.Width = gWidth;
	swapChainDesc.BufferDesc.Height = gHeight;
	swapChainDesc.OutputWindow = gHWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Windowed = true;

	D3D_FN( dxgiFactory->CreateSwapChain( mCommandQueue.Get(), &swapChainDesc, &mSwapChain ) );

	
	mRenderTargetViewDescriptorSize = mD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);


	///////////////////////////// SetupSwapChain /////////////////////////////
	// This is the first fence value we'll set, has to be != our initial value below so we can wait on the first fence correctly
	mCurrentFenceValue = 1;

	// Create fences for each frame so we can protect resources and wait for
	// any given frame
	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i)
	{
		mFrameFenceEvents[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		mFenceValues[i] = 0;
		mD3DDevice->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFrameFences[i]) );
	}

	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i)
	{
		mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mRenderTargets[i]));
	}
		
	
	///////////////////////////// SetupRenderTargets /////////////////////////////
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = QUEUE_SLOT_COUNT;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	mD3DDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mRenderTargetDescriptorHeap));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle{ mRenderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart() };

	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i)
	{
		D3D12_RENDER_TARGET_VIEW_DESC viewDesc;
		viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipSlice = 0;
		viewDesc.Texture2D.PlaneSlice = 0;

		mD3DDevice->CreateRenderTargetView(mRenderTargets[i].Get(), &viewDesc, rtvHandle);

		rtvHandle.Offset(mRenderTargetViewDescriptorSize);
	}



	///////////////////////////// CreateAllocatorsAndCommandLists /////////////////////////////
	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i)
	{
		mD3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mCommandAllocators[i]));
		mD3DDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mCommandAllocators[i].Get(), nullptr, IID_PPV_ARGS(&mCommandLists[i]));
		mCommandLists[i]->Close();
	}



	///////////////////////////// CreateViewportScissor /////////////////////////////
	mRectScissor = { 0, 0, gWidth, gHeight };
	mViewport = { 0.0f, 0.0f, static_cast<float>(gWidth), static_cast<float>(gHeight), 0.0f, 1.0f };


	
	///////////////////////////// Descriptor Heaps /////////////////////////////
	// We need one descriptor heap to store our texture SRV which cannot go into the root signature. So create a SRV type heap with one entry
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.NumDescriptors = 4;
	// This heap contains SRV, UAV or CBVs -- in our case one SRV
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	mD3DDevice->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&mSRVUAVDescriptorHeap));
	mSRVUAVDescriptorSize = mD3DDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);



	///////////////////////////// CreateRootSignature /////////////////////////////
	// Compute root signature.
	{
		CD3DX12_DESCRIPTOR_RANGE1 uav_range;
		uav_range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE);

		CD3DX12_ROOT_PARAMETER1 rootParameters[2];
		rootParameters[0].InitAsDescriptorTable(1, &uav_range, D3D12_SHADER_VISIBILITY_ALL);
		rootParameters[1].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
		
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC computeRootSignatureDesc;
		computeRootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		D3DX12SerializeVersionedRootSignature(&computeRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
		mD3DDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&mRayCastRootSignature));
	}
	
	//FS PASS
	ComPtr<ID3DBlob> rootBlob;
	ComPtr<ID3DBlob> errorBlob;

	// We have two root parameters, one is a pointer to a descriptor heap with a SRV, the second is a constant buffer view
	CD3DX12_ROOT_PARAMETER parameters[2];

	// Create a descriptor table with one entry in our descriptor heap
	CD3DX12_DESCRIPTOR_RANGE range;
	range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0, 0);
	parameters[0].InitAsDescriptorTable(1, &range);

	// Our constant buffer view
	parameters[1].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL );

	// We don't use another descriptor heap for the sampler, instead we use a static sampler
	CD3DX12_STATIC_SAMPLER_DESC samplers[1];
	samplers[0].Init(0, D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT);

	CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;

	// Create the root signature
	descRootSignature.Init(2, parameters, 1, samplers, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	//descRootSignature.Init(1, rootParameters, 0, NULL, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob);
	mD3DDevice->CreateRootSignature(0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), IID_PPV_ARGS(&mFSPassRootSignature));



	///////////////////////////// CreatePipelineStateObject /////////////////////////////

	// FS PASS
	static const D3D12_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	
	ID3DBlob*   errorMessage;
	ComPtr<ID3DBlob> vertexShader;
	HRESULT hr = D3DCompileFromFile(L"shaders/fspass.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", NULL, NULL, &vertexShader, &errorMessage);
	if (FAILED(hr))
	{
		if (errorMessage) { OutputDebugStringA((char*)errorMessage->GetBufferPointer()); }
		if (errorMessage) { errorMessage->Release(); }
		return false;
	}

	ComPtr<ID3DBlob> pixelShader;
	hr = D3DCompileFromFile(L"shaders/fspass.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", NULL, NULL, &pixelShader, &errorMessage);
	if (FAILED(hr))
	{
		if (errorMessage) { OutputDebugStringA((char*)errorMessage->GetBufferPointer()); }
		if (errorMessage) { errorMessage->Release(); }
		return false;
	}


	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.VS.BytecodeLength = vertexShader->GetBufferSize();
	psoDesc.VS.pShaderBytecode = vertexShader->GetBufferPointer();
	psoDesc.PS.BytecodeLength = pixelShader->GetBufferSize();
	psoDesc.PS.pShaderBytecode = pixelShader->GetBufferPointer();
	psoDesc.pRootSignature = mFSPassRootSignature.Get();
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
	psoDesc.InputLayout.NumElements = std::extent<decltype(layout)>::value;
	psoDesc.InputLayout.pInputElementDescs = layout;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	// Simple alpha blending
	psoDesc.BlendState.RenderTarget[0].BlendEnable = true;
	psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.DepthStencilState.DepthEnable = false;
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.SampleMask = 0xFFFFFFFF;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	D3D_FN( mD3DDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mFSPassPSO)) );


	// RAY CAST PASS
	ComPtr<ID3DBlob> computeShader;
	hr = D3DCompileFromFile(L"shaders/raycast.hlsl", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_0", NULL, NULL, &computeShader, &errorMessage);
	if (FAILED(hr))
	{
		if (errorMessage) { OutputDebugStringA((char*)errorMessage->GetBufferPointer()); }
		if (errorMessage) { errorMessage->Release(); }
		return false;
	}


	D3D12_COMPUTE_PIPELINE_STATE_DESC cps_desc = {};
	cps_desc.CS.BytecodeLength = computeShader->GetBufferSize();
	cps_desc.CS.pShaderBytecode = computeShader->GetBufferPointer();
	cps_desc.pRootSignature = mRayCastRootSignature.Get();

	mD3DDevice->CreateComputePipelineState(&cps_desc, IID_PPV_ARGS(&mRayCastPSO));


	///////////////////////////// create Ray Cast Output UAV /////////////////////////////
	{
		// Initialize the data in the buffers.
		const UINT data_size = sizeof(tPixel) * gWidth * gHeight;

		D3D12_HEAP_PROPERTIES defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(data_size, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

		mD3DDevice->CreateCommittedResource(
			&defaultHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, //goes between D3D12_RESOURCE_STATE_UNORDERED_ACCESS and D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
			nullptr,
			IID_PPV_ARGS(&mRayCastOutput));
		
		D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
		uav_desc.Format = DXGI_FORMAT_UNKNOWN;
		uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uav_desc.Buffer.FirstElement = 0;
		uav_desc.Buffer.NumElements = gWidth * gHeight;
		uav_desc.Buffer.StructureByteStride = sizeof(tPixel);
		uav_desc.Buffer.CounterOffsetInBytes = 0;
		uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		CD3DX12_CPU_DESCRIPTOR_HANDLE uav_handle(mSRVUAVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 0, mSRVUAVDescriptorSize);
		mD3DDevice->CreateUnorderedAccessView(mRayCastOutput.Get(), nullptr, &uav_desc, uav_handle);


		D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
		srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srv_desc.Format = DXGI_FORMAT_UNKNOWN;
		srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srv_desc.Buffer.FirstElement = 0;
		srv_desc.Buffer.NumElements = gWidth * gHeight;
		srv_desc.Buffer.StructureByteStride = sizeof(tPixel);
		srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		CD3DX12_CPU_DESCRIPTOR_HANDLE srv_handle(mSRVUAVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 0, mSRVUAVDescriptorSize);
		mD3DDevice->CreateShaderResourceView(mRayCastOutput.Get(), &srv_desc, srv_handle);
	}




	///////////////////////////// CreateConstantBuffer /////////////////////////////
	tSharedCB cb;
	ZeroMemory(&cb, sizeof(cb));

	for (int i = 0; i < QUEUE_SLOT_COUNT; ++i)
	{
		// These will remain in upload heap because we use them only once per frame.
		mD3DDevice->CreateCommittedResource( &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(sizeof(tSharedCB)), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&mConstantBuffers[i]) );

		void* p;
		mConstantBuffers[i]->Map(0, nullptr, &p);
		::memcpy(p, &cb, sizeof(cb));
		mConstantBuffers[i]->Unmap(0, nullptr);
	}




	// Create our upload fence, command list and command allocator
	// This will be only used while creating the mesh buffer and the texture to upload data to the GPU.
	mD3DDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mUploadFence));

	ComPtr<ID3D12CommandAllocator> uploadCommandAllocator;
	mD3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&uploadCommandAllocator));
	ComPtr<ID3D12GraphicsCommandList> uploadCommandList;
	mD3DDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, uploadCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&uploadCommandList));

	//CreateMeshBuffers
	static const VertexType vertices[4] = {
		{ { -1.0f, 1.0f, 0 }, { 0, 0 } }, // Upper Left
		{ { 1.0f, 1.0f, 0 }, { 1, 0 } }, // Upper Right
		{ { -1.0f, -1.0f, 0 },{ 0, 1 } }, // Bottom left
		{ { 1.0f, -1.0f, 0 }, { 1, 1 } } // Bottom right
	};

	static const int uploadBufferSize = sizeof(vertices);

	// Create upload buffer on CPU
	mD3DDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&mUploadBuffer));

	// Create vertex & index buffer on the GPU
	// HEAP_TYPE_DEFAULT is on GPU, we also initialize with COPY_DEST state so we don't have to transition into this before copying into them
	mD3DDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)), D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&mVertexBuffer));

	// Create buffer views
	mVertexBufferView.BufferLocation = mVertexBuffer->GetGPUVirtualAddress();
	mVertexBufferView.SizeInBytes = sizeof(vertices);
	mVertexBufferView.StrideInBytes = sizeof(VertexType);

	// Copy data on CPU into the upload buffer
	void* p;
	mUploadBuffer->Map(0, nullptr, &p);
	memcpy(p, vertices, sizeof(vertices));
	mUploadBuffer->Unmap(0, nullptr);

	// Copy data from upload buffer on CPU into the index/vertex buffer on the GPU
	uploadCommandList->CopyBufferRegion(mVertexBuffer.Get(), 0, mUploadBuffer.Get(), 0, sizeof(vertices));

	// Barriers, batch them together
	CD3DX12_RESOURCE_BARRIER barriers[1] = { CD3DX12_RESOURCE_BARRIER::Transition(mVertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER), };

	uploadCommandList->ResourceBarrier(1, barriers);





	///////////////////////////// CreateTexture(uploadCommandList.Get()); /////////////////////////////

	//DirectX::LoadWICTextureFromFile(mD3DDevice.Get(), L"textures/noise.png", &mNoiseTexture, decodedData, subresource);

	//https://github.com/Microsoft/DirectXTK12/wiki/WICTextureLoader


	// Note: ComPtr's are CPU objects but this resource needs to stay in scope until
	// the command list that references it has finished executing on the GPU.
	// We will flush the GPU at the end of this method to ensure the resource is not
	// prematurely destroyed.
	ComPtr<ID3D12Resource> textureUploadHeap;

	// Create the texture.
	{
		std::unique_ptr<uint8_t[]> decodedData;
		D3D12_SUBRESOURCE_DATA subresource;
		DirectX::LoadWICTextureFromFile(mD3DDevice.Get(), L"textures/noise.png", &mNoiseTexture, decodedData, subresource);

		const UINT64 uploadTexBufferSize = GetRequiredIntermediateSize(mNoiseTexture.Get(), 0, 1);

		// Create the GPU upload buffer.
		mD3DDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(uploadTexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&textureUploadHeap));

		// Copy data to the intermediate upload heap and then schedule a copy from the upload heap to the Texture2D.

		UpdateSubresources(uploadCommandList.Get(), mNoiseTexture.Get(), textureUploadHeap.Get(), 0, 0, 1, &subresource);
		uploadCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mNoiseTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

		// Describe and create a SRV for the texture.
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = mNoiseTexture.Get()->GetDesc().Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		CD3DX12_CPU_DESCRIPTOR_HANDLE srv_handle(mSRVUAVDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), 1, mSRVUAVDescriptorSize);
		mD3DDevice->CreateShaderResourceView(mNoiseTexture.Get(), &srvDesc, srv_handle);
	}




	uploadCommandList->Close();

	// Execute the upload and finish the command list
	ID3D12CommandList* commandLists[] = { uploadCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(std::extent<decltype(commandLists)>::value, commandLists);
	mCommandQueue->Signal(mUploadFence.Get(), 1);

	auto waitEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	WaitForFence(mUploadFence.Get(), 1, waitEvent);

	// Cleanup our upload handle
	uploadCommandAllocator->Reset();

	CloseHandle(waitEvent);

	return true;
}






void Render::DoFrame()
{
	PrepareRender();
	UpdateBuffers();
	DoRender();
	FinalizeRender();
	Present();
}




void Render::PrepareRender()
{
	mCommandAllocators[mCurrentBackBuffer]->Reset();

	ID3D12GraphicsCommandList* command_list = mCommandLists[mCurrentBackBuffer].Get();
	command_list->Reset(mCommandAllocators[mCurrentBackBuffer].Get(), NULL);

	D3D12_CPU_DESCRIPTOR_HANDLE rt_handle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE::InitOffsetted(rt_handle, mRenderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), mCurrentBackBuffer, mRenderTargetViewDescriptorSize);

	command_list->OMSetRenderTargets(1, &rt_handle, true, NULL);
	command_list->RSSetViewports(1, &mViewport);
	command_list->RSSetScissorRects(1, &mRectScissor);

	// Transition back buffer
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = mRenderTargets[mCurrentBackBuffer].Get();
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	command_list->ResourceBarrier(1, &barrier);

	static const float clearColor[] = { 0.042f, 0.042f, 0.042f, 1 };

	command_list->ClearRenderTargetView(rt_handle, clearColor, 0, NULL);
}




extern float m0;
extern float m1;
extern float m2;
extern float m3;

void Render::UpdateBuffers()
{
	void* p;
	mConstantBuffers[mCurrentBackBuffer]->Map(0, NULL, &p);
	tSharedCB* cb = static_cast<tSharedCB*>(p);

	int sz = sizeof(tSharedCB);

	cb->width = gWidth;
	cb->height = gHeight;

	//Set camera constants to cb
	cb->cam_pos_x = Camera::Instance->GetPosition()->mX;
	cb->cam_pos_y = Camera::Instance->GetPosition()->mY;
	cb->cam_pos_z = Camera::Instance->GetPosition()->mZ;

	Matrix3x3 view_mat = Camera::Instance->GetViewMatrix();
	cb->cam_orientation_00 = view_mat.m00; cb->cam_orientation_01 = view_mat.m01; cb->cam_orientation_02 = view_mat.m02;
	cb->cam_orientation_10 = view_mat.m10; cb->cam_orientation_11 = view_mat.m11; cb->cam_orientation_12 = view_mat.m12;
	cb->cam_orientation_20 = view_mat.m20; cb->cam_orientation_21 = view_mat.m21; cb->cam_orientation_22 = view_mat.m22;
	
	cb->noise_offset_x = RandFloat();
	cb->noise_offset_y = RandFloat();
	cb->time = Game::Instance->GetTime();

	//STUBBED NUMBER OF PRIMS, get this data from the scene
	cb->num_lights = 4;
	cb->num_spheres = 5;
	cb->num_tris = 2;
	
	float c = 0.8f;
	//c = c*c;
	Vector3 colour = Vector3(c, c, c);
	cb->lights[0].mPosition = Vector3(1.0f + cosf(Game::Instance->GetTime()), 8.4f, 3.0f + sinf(Game::Instance->GetTime()));
	cb->lights[0].mColour = colour;

	cb->lights[1].mPosition = Vector3(cosf(Game::Instance->GetTime()), 8.0f, 1.0f);
	cb->lights[1].mColour = colour;

	cb->lights[2].mPosition = Vector3(cosf(Game::Instance->GetTime() * 4), 7 + sinf(Game::Instance->GetTime() * 4), 3.01f);
	cb->lights[2].mColour = colour;

	cb->lights[3].mPosition = Vector3(sinf(Game::Instance->GetTime() * 0.25f), 7.0f + cosf(Game::Instance->GetTime()*0.4f), 3.01f);
	cb->lights[3].mColour = colour;
	
	
	cb->spheres[0].mPosition = Vector3(-1.5f, 0.1f, 4.0f);
	cb->spheres[0].mRadius = m0;
	cb->spheres[1].mPosition = Vector3(-0.5f, 0.2f, 4.0f);
	cb->spheres[1].mRadius = m1;
	cb->spheres[2].mPosition = Vector3(0.5f, 0.3f, 4.0f);
	cb->spheres[2].mRadius = m2;
	cb->spheres[3].mPosition = Vector3(1.5f, 0.4f, 4.0f);
	cb->spheres[3].mRadius = m3;

	cb->spheres[4].mPosition = Vector3(0.0f, 1.5f + 0.5f*sinf(Game::Instance->GetTime()*0.5f), 4.0f);
	cb->spheres[4].mRadius = 0.75f;
	

	float big = 10000.0f;
	cb->tris[0].mV0 = Vector3(-big, 0, -big);
	cb->tris[0].mV1 = Vector3(big - 1, 0, big);
	cb->tris[0].mV2 = Vector3(big, 0, -big);
	cb->tris[1].mV0 = Vector3(-big, 0, -big);
	cb->tris[1].mV1 = Vector3(-big, 0, big);
	cb->tris[1].mV2 = Vector3(big + 1, 0, big);
	
	mConstantBuffers[mCurrentBackBuffer]->Unmap(0, NULL);
}



void Render::DoRender()
{
	ID3D12GraphicsCommandList* command_list = mCommandLists[mCurrentBackBuffer].Get();

	//DO DISPATCH FOR RAY CAST
	{
		command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mRayCastOutput.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

		command_list->SetPipelineState(mRayCastPSO.Get());
		command_list->SetComputeRootSignature(mRayCastRootSignature.Get());

		ID3D12DescriptorHeap* ppHeaps[] = { mSRVUAVDescriptorHeap.Get() };
		command_list->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		command_list->SetComputeRootDescriptorTable(0, mSRVUAVDescriptorHeap->GetGPUDescriptorHandleForHeapStart() );
		command_list->SetComputeRootConstantBufferView(1, mConstantBuffers[mCurrentBackBuffer]->GetGPUVirtualAddress());


		const int threads_dim = 16;
		command_list->Dispatch((gWidth + (threads_dim - 1)) / threads_dim, (gHeight + (threads_dim - 1)) / threads_dim, 1);


		command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mRayCastOutput.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	}
	


	//DO FULLSCREEN PASS
	{
		// Set our state (shaders, etc.)
		command_list->SetPipelineState(mFSPassPSO.Get());

		// Set our root signature
		command_list->SetGraphicsRootSignature(mFSPassRootSignature.Get());

		// Set the descriptor heap containing the texture srv
		ID3D12DescriptorHeap* heaps[] = { mSRVUAVDescriptorHeap.Get() };
		command_list->SetDescriptorHeaps(1, heaps);

		// Set slot 0 of our root signature to point to our descriptor heap with the texture SRV and raycast output SRV
		command_list->SetGraphicsRootDescriptorTable(0, mSRVUAVDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

		// Set slot 1 of our root signature to the constant buffer view
		command_list->SetGraphicsRootConstantBufferView(1, mConstantBuffers[mCurrentBackBuffer]->GetGPUVirtualAddress());

		command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		command_list->IASetVertexBuffers(0, 1, &mVertexBufferView);
		command_list->DrawInstanced(4, 1, 0, 0);
	}
}




void Render::FinalizeRender()
{
	// Transition the swap chain back to present
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = mRenderTargets[mCurrentBackBuffer].Get();
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	ID3D12GraphicsCommandList* command_list = mCommandLists[mCurrentBackBuffer].Get();
	command_list->ResourceBarrier(1, &barrier);

	command_list->Close();

	// Execute our commands
	ID3D12CommandList* command_lists[] = { command_list };
	mCommandQueue->ExecuteCommandLists(std::extent<decltype(command_lists)>::value, command_lists);
}





void Render::Present()
{
	//todo, use gVsync

	mSwapChain->Present(1, 0);

	// Mark the fence for the current frame.
	const auto fence_value = mCurrentFenceValue;
	mCommandQueue->Signal(mFrameFences[mCurrentBackBuffer].Get(), fence_value);
	mFenceValues[mCurrentBackBuffer] = fence_value;
	++mCurrentFenceValue;

	// Take the next back buffer from our chain
	mCurrentBackBuffer = (mCurrentBackBuffer + 1) % QUEUE_SLOT_COUNT;

	//Wait for gpu
	WaitForFence(mFrameFences[mCurrentBackBuffer].Get(), mFenceValues[mCurrentBackBuffer], mFrameFenceEvents[mCurrentBackBuffer]);
}