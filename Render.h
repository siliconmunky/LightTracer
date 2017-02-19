#pragma once

#include "BasicDefs.h"


#include <d3d11.h>
#include <DXGI.h>
#include "D3DCompiler.h"


class Render
{
public:
	static Render* Instance;

	Render();
	~Render();	

	void DoFrame();

private:
	bool InitDevice();


	void UpdateBuffers();



	ID3D11Device*				mD3DDevice;
	ID3D11DeviceContext*		mImmediateContext;
	IDXGISwapChain*				mSwapChain;

	ID3D11RenderTargetView*		mRenderTargetView;
	ID3D11RasterizerState*		mRasterState;


	ID3D11Buffer*				mLightDataGPUBuffer;
	ID3D11ShaderResourceView*	mLightDataGPUBufferView;
	ID3D11Buffer*				mSphereDataGPUBuffer;
	ID3D11ShaderResourceView*	mSphereDataGPUBufferView;
	ID3D11Buffer*				mTriDataGPUBuffer;
	ID3D11ShaderResourceView*	mTriDataGPUBufferView;


	ID3D11Buffer*				mResConstantBuffer;
	ID3D11Buffer*				mCameraConstantBuffer;
	ID3D11Buffer*				mPrimitivesConstantBuffer;



	ID3D11Buffer*				mCSDestDataBuffer;
	ID3D11UnorderedAccessView*	mCSDestDataBufferView;
	ID3D11ShaderResourceView*	mDestDataSRV;


	ID3D11ComputeShader*		mComputeShader;




	ID3D11InputLayout*			mLayout;
	ID3D11Buffer *mVertexBuffer;
	int mVertexCount;
	ID3D11VertexShader*			mVertexShader;

	ID3D11PixelShader*			mPixelShader;

};

