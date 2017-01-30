#pragma once

#include "BasicDefs.h"


#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10math.h>
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



	ID3D11Device*				m_pd3dDevice;
	ID3D11DeviceContext*		m_pImmediateContext;
	IDXGISwapChain*				m_pSwapChain;
	ID3D11RenderTargetView*		m_pRenderTargetView;

	ID3D11Buffer*				m_srcDataGPUBuffer;
	ID3D11ShaderResourceView*	m_srcDataGPUBufferView;


	ID3D11Buffer* constant_bufer;
	ID3D11Buffer* constant_bufer_camera;



	ID3D11Buffer*				m_destDataGPUBuffer;
	ID3D11UnorderedAccessView*	m_destDataGPUBufferView;
	ID3D11ShaderResourceView*	m_destDataSRV;

	ID3D11ComputeShader*		m_computeShader;


	ID3D11VertexShader*			m_vertexShader;
	ID3D11PixelShader*			m_pixelShader;
	ID3D11InputLayout*			m_layout;
	ID3D11Buffer*				m_matrixBuffer;
	ID3D11SamplerState*			m_sampleState;

	ID3D11Buffer *m_vertexBuffer;
	int m_vertexCount;

	ID3D11RasterizerState*		m_rasterState;

};

