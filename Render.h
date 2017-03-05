#pragma once

#include "BasicDefs.h"


/*#include <d3d11.h>
#include <DXGI.h>
#include "D3DCompiler.h"*/

#include <d3d12.h>
#include <wrl.h>
#include <vector>

using namespace Microsoft::WRL;

//#define D3DCOMPILE_DEBUG 1

static const int QUEUE_SLOT_COUNT = 3;

class Render
{
public:
	static Render* Instance;

	Render();
	~Render();	

	void DoFrame();

private:
	bool InitDevice();
	
	void PrepareRender();
	void UpdateBuffers();
	void DoRender();
	void FinalizeRender();
	void Present();


	ComPtr<ID3D12Device>		mD3DDevice;
	ComPtr<ID3D12CommandQueue>	mCommandQueue;
	ComPtr<IDXGISwapChain>		mSwapChain;


	HANDLE						mFrameFenceEvents[QUEUE_SLOT_COUNT];
	ComPtr<ID3D12Fence>			mFrameFences[QUEUE_SLOT_COUNT];
	UINT64						mCurrentFenceValue;
	UINT64						mFenceValues[QUEUE_SLOT_COUNT];


	D3D12_VIEWPORT mViewport;
	D3D12_RECT mRectScissor;

	ComPtr<ID3D12Resource>		mRenderTargets[QUEUE_SLOT_COUNT];
	ComPtr<ID3D12DescriptorHeap> mRenderTargetDescriptorHeap;
	std::int32_t				mRenderTargetViewDescriptorSize;
	int							mCurrentBackBuffer;


	ComPtr<ID3D12CommandAllocator> mCommandAllocators[QUEUE_SLOT_COUNT];
	ComPtr<ID3D12GraphicsCommandList> mCommandLists[QUEUE_SLOT_COUNT];



	ComPtr<ID3D12Resource> mConstantBuffers[QUEUE_SLOT_COUNT]; //JUST STICK EVERYTHING IN 1 CONSTANT BUFFER FOR NOW


	ComPtr<ID3D12RootSignature> mFSPassRootSignature;
	ComPtr<ID3D12PipelineState> mFSPassPSO;

	ComPtr<ID3D12RootSignature> mRayCastRootSignature;
	ComPtr<ID3D12PipelineState> mRayCastPSO;

	ComPtr<ID3D12Fence> mUploadFence;

	ComPtr<ID3D12Resource> mUploadBuffer;
	ComPtr<ID3D12Resource> mVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVertexBufferView;


	ComPtr<ID3D12DescriptorHeap> mSRVUAVDescriptorHeap;
	std::int32_t				mSRVUAVDescriptorSize;

	ComPtr<ID3D12Resource> mRayCastOutput; 
	ComPtr<ID3D12Resource> mRayCastUAV;
	ComPtr<ID3D12Resource> mRayCastSRV;

	ComPtr<ID3D12Resource> mUploadImage;
	ComPtr<ID3D12Resource> mNoiseTexture;
	

	//whats this for?
	std::vector<std::uint8_t> imageData_;






/*
	ID3D11Buffer*				mLightDataGPUBuffer;
	ID3D11ShaderResourceView*	mLightDataGPUBufferView;
	ID3D11Buffer*				mSphereDataGPUBuffer;
	ID3D11ShaderResourceView*	mSphereDataGPUBufferView;
	ID3D11Buffer*				mTriDataGPUBuffer;
	ID3D11ShaderResourceView*	mTriDataGPUBufferView;


	ID3D11Buffer*				mCSDestDataBuffer;
	ID3D11UnorderedAccessView*	mCSDestDataBufferView;
	ID3D11ShaderResourceView*	mDestDataSRV;


	ID3D11ComputeShader*		mComputeShader;
*/

};

