#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <string>

#include "ImageBuffer.h"
#include "Vector3.h"
#include "scene/Sphere.h"
#include "scene/Scene.h"
#include "Camera.h"


using std::ifstream;
using std::cout;
using std::string;
using std::endl;


#include <ctime>

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10math.h>
#include "D3DCompiler.h"
#include "Windowsx.h"

//#include "DXApplication.h"

//HWND			g_hWnd = NULL;
//DXApplication	application;

ID3D11Device*				m_pd3dDevice;
ID3D11DeviceContext*		m_pImmediateContext;
IDXGISwapChain*				m_pSwapChain;
ID3D11RenderTargetView*		m_pRenderTargetView;

ID3D11Buffer*				m_srcDataGPUBuffer;
ID3D11ShaderResourceView*	m_srcDataGPUBufferView;

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

struct VertexType
{
	D3DXVECTOR3 position;
	D3DXVECTOR4 color;
	D3DXVECTOR4 uv;
};

struct MatrixBufferType
{
	D3DXMATRIX world;
	D3DXMATRIX view;
	D3DXMATRIX projection;
};

struct tPixel
{
	float r;
	float g;
	float b;
	float a;
};

__declspec(align(16)) struct tConstantBuffer
{
	int width;
	int height;
};

__declspec(align(16)) struct tConstantBufferCamera
{
	float cam_pos_x, cam_pos_y, cam_pos_z; //gCameraPosition
	float cam_orientation_00, cam_orientation_01, cam_orientation_02;
	float cam_orientation_10, cam_orientation_11, cam_orientation_12;
	float cam_orientation_20, cam_orientation_21, cam_orientation_22; 
};

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch( message )
	{
	case WM_PAINT:
		hdc = BeginPaint( hWnd, &ps );
		EndPaint( hWnd, &ps );
		break;

	case WM_DESTROY:
		PostQuitMessage( 0 );
		break;

	default:
		return DefWindowProc( hWnd, message, wParam, lParam );
	}

	return 0;
}
void ParseDescription( const char* filename, int& w_in_pix, int& h_in_pix )
{
	ifstream indata; // indata is like cin
	indata.open( filename ); // opens the file

	if( !indata )
	{ // file couldn't be opened
      return;
	}
	
	string temp;
	indata >> temp;
	indata >> w_in_pix;
	
	indata >> temp;
	indata >> h_in_pix;


	int num_spheres;
	indata >> temp;
	indata >> num_spheres;
	for( int i = 0; i < num_spheres; ++i )
	{
		Vector3 pos;
		float radius;
		indata >> pos.mX;
		indata >> pos.mY;
		indata >> pos.mZ;
		indata >> radius;		
		Scene::Instance()->AddSphere( pos, radius );
	}

	int num_triangles;
	indata >> temp;
	indata >> num_triangles;
	for( int i = 0; i < num_triangles; ++i )
	{
		Vector3 p1;
		Vector3 p2;
		Vector3 p3;
		indata >> p1.mX;
		indata >> p1.mY;
		indata >> p1.mZ;
		indata >> p2.mX;
		indata >> p2.mY;
		indata >> p2.mZ;
		indata >> p3.mX;
		indata >> p3.mY;
		indata >> p3.mZ;		
		Scene::Instance()->AddTriangle( p1, p2, p3 );
	}
	
	int num_point_lights;
	indata >> temp;
	indata >> num_point_lights;
	for( int i = 0; i < num_point_lights; ++i )
	{
		Vector3 pos;
		ColourRGB colour;
		float size;
		int taps;
		indata >> pos.mX;
		indata >> pos.mY;
		indata >> pos.mZ;
		indata >> colour.mR;
		indata >> colour.mG;
		indata >> colour.mB;
		indata >> size;		
		indata >> taps;	
		Scene::Instance()->AddPointLight( pos, colour, size, taps );
	}

	indata.close();
}
bool InitWindowAndDevice( int width, int height ) 
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int pos_x, pos_y;

	// Get the instance of this application.
	HINSTANCE m_hinstance = GetModuleHandle(NULL);
	
	HWND hWnd;

	LPCWSTR m_applicationName;
	m_applicationName = L"RayCast";

	// Setup the windows class with default settings.
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = m_hinstance;
	wc.hIcon		 = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm       = wc.hIcon;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize        = sizeof(WNDCLASSEX);
	
	// Register the window class.
	RegisterClassEx(&wc);

	// Determine the resolution of the clients desktop screen.
	int screen_width  = GetSystemMetrics(SM_CXSCREEN);
	int screen_height = GetSystemMetrics(SM_CYSCREEN);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if(false/*full screen*/)
	{
		// If full screen set the screen to maximum size of the users desktop and 32bit.
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize       = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth  = (unsigned long)screen_width;
		dmScreenSettings.dmPelsHeight = (unsigned long)screen_height;
		dmScreenSettings.dmBitsPerPel = 32;			
		dmScreenSettings.dmFields     = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Change the display settings to full screen.
		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		// Set the position of the window to the top left corner.
		pos_x = pos_y = 0;
	}
	else
	{
		// If windowed then set it to 800x600 resolution.
		screen_width  = width;
		screen_height = height;

		// Place the window in the middle of the screen.
		pos_x = (GetSystemMetrics(SM_CXSCREEN) - screen_width)  / 2;
		pos_y = (GetSystemMetrics(SM_CYSCREEN) - screen_height) / 2;
	}

	// Create the window with the screen settings and get the handle to it.
	hWnd = CreateWindowEx(WS_EX_APPWINDOW, m_applicationName, m_applicationName, 
						    WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
						    pos_x, pos_y, screen_width, screen_height, NULL, NULL, m_hinstance, NULL);

	// Bring the window up on the screen and set it as main focus.
	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);

	/*
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof( WNDCLASSEX );
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = NULL;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"Ray Cast";
	wcex.hIconSm = NULL;
	if( !RegisterClassEx( &wcex ) )
		return;

	// Create window
	RECT rc = { 0, 0, width, height };
	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	hWnd = CreateWindowEx( WS_EX_APPWINDOW, L"Ray Cast", L"Ray Cast", 
						    WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
						    0, 0, width, height, NULL, NULL, NULL, NULL);*/

	if( !hWnd )
	{
		DWORD err = GetLastError();
		return false;
	}


	HRESULT hr = S_OK;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif


	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof( sd ) );
	sd.BufferCount = 1;
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_DRIVER_TYPE         driverType = D3D_DRIVER_TYPE_HARDWARE;
	D3D_FEATURE_LEVEL       desired_featureLevel = D3D_FEATURE_LEVEL_11_0;
	D3D_FEATURE_LEVEL       featureLevel = D3D_FEATURE_LEVEL_11_0;
	hr = D3D11CreateDeviceAndSwapChain( NULL, driverType, NULL, createDeviceFlags, &desired_featureLevel, 1,
		D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice, &featureLevel, &m_pImmediateContext );
	if( FAILED( hr ) )
		return false;


	// Get the pointer to the back buffer.
	ID3D11Texture2D* backBufferPtr;
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if(FAILED(hr))
	{
		return false;
	}

	// Create the render target view with the back buffer pointer.
	hr = m_pd3dDevice->CreateRenderTargetView(backBufferPtr, NULL, &m_pRenderTargetView);
	if(FAILED(hr))
	{
		return false;
	}

	
	m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL/*m_depthStencilView*/);


	////////////////////////////////////////
	// INPUT BUFFERS
	////////////////////////////////////////

	tPixel colour;
	colour.r = 0.0f;
	colour.g = 0.0f;
	colour.b = 0.0f;

	// First we create a buffer in GPU memory
	D3D11_BUFFER_DESC input_descGPUBuffer;
	ZeroMemory( &input_descGPUBuffer, sizeof(input_descGPUBuffer) );
	input_descGPUBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	input_descGPUBuffer.ByteWidth = sizeof(tPixel);//m_textureDataSize;
	input_descGPUBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	input_descGPUBuffer.StructureByteStride = sizeof(tPixel);

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &colour;//m_srcTextureData; //THIS IS WHERE THE SCENE INFORMATION GOES!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	hr = m_pd3dDevice->CreateBuffer( &input_descGPUBuffer, &InitData, &m_srcDataGPUBuffer );
	if( FAILED( hr ) )
		return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC input_descView;
	ZeroMemory( &input_descView, sizeof(input_descView) );
	input_descView.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	input_descView.Format = DXGI_FORMAT_UNKNOWN;
	input_descView.BufferEx.FirstElement = 0;
	input_descView.BufferEx.NumElements = 1;
		
	if(FAILED(m_pd3dDevice->CreateShaderResourceView( m_srcDataGPUBuffer, &input_descView, &m_srcDataGPUBufferView )))
		return false;



	////////////////////////////////////////
	// OUTPUT BUFFER
	////////////////////////////////////////
	D3D11_BUFFER_DESC output_descGPUBuffer;
	ZeroMemory( &output_descGPUBuffer, sizeof(output_descGPUBuffer) );
	output_descGPUBuffer.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	output_descGPUBuffer.ByteWidth = sizeof(tPixel) * width * height;
	output_descGPUBuffer.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	output_descGPUBuffer.StructureByteStride = sizeof(tPixel);
	if(FAILED(m_pd3dDevice->CreateBuffer( &output_descGPUBuffer, NULL, &m_destDataGPUBuffer )))
		return false;

	// The view we need for the output is an unordered access view. This is to allow the compute shader to write anywhere in the buffer.
	D3D11_UNORDERED_ACCESS_VIEW_DESC output_descView;
	ZeroMemory( &output_descView, sizeof(output_descView) );
	output_descView.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	output_descView.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
	output_descView.Buffer.FirstElement = 0;
	output_descView.Buffer.NumElements = width * height; 
	if(FAILED(m_pd3dDevice->CreateUnorderedAccessView( m_destDataGPUBuffer, &output_descView, &m_destDataGPUBufferView )))
		return false;
	
	D3D11_SHADER_RESOURCE_VIEW_DESC output_srv_descView;
	ZeroMemory( &output_srv_descView, sizeof(output_srv_descView) );
	output_srv_descView.Format = DXGI_FORMAT_UNKNOWN;
	output_srv_descView.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	output_srv_descView.BufferEx.FirstElement = 0;
	output_srv_descView.BufferEx.NumElements = width * height;
	output_srv_descView.BufferEx.Flags = 0;
	if(FAILED(m_pd3dDevice->CreateShaderResourceView( m_destDataGPUBuffer, &output_srv_descView, &m_destDataSRV )))
		return false;



	////////////////////////////////////////	
	// Load the compute shader now
	////////////////////////////////////////	
	// We load and compile the shader. If we fail, we bail out here.
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( _DEBUG )
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	LPCSTR pProfile = ( m_pd3dDevice->GetFeatureLevel() >= D3D_FEATURE_LEVEL_11_0 ) ? "cs_5_0" : "cs_4_0";

	ID3DBlob* pErrorBlob = NULL;
	ID3DBlob* pBlob = NULL;
	hr = D3DX11CompileFromFile( L"RayCast.hlsl", NULL, NULL, "CSMain", pProfile, dwShaderFlags, NULL, NULL, &pBlob, &pErrorBlob, NULL );
	if ( FAILED(hr) )
	{
		if ( pErrorBlob )
			OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
		if(pErrorBlob)
			pErrorBlob->Release();
		if(pBlob)
			pBlob->Release();

		return false;
	}
	else
	{
		hr = m_pd3dDevice->CreateComputeShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &m_computeShader );
		if(pErrorBlob)
			pErrorBlob->Release();
		if(pBlob)
			pBlob->Release();
	}



	////////////////////////////////////////	
	// Load the plain shader now
	////////////////////////////////////////
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;

	D3D11_SAMPLER_DESC samplerDesc;

	// Initialize the pointers this function will use to null.
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

    // Compile the vertex shader code.
	hr = D3DX11CompileFromFile( L"Color.vs", NULL, NULL, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
								   &vertexShaderBuffer, &errorMessage, NULL);
	if(FAILED(hr))
	{
		// If the shader failed to compile it should have writen something to the error message.
		/*if(errorMessage)
		{
			//OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			//MessageBox(hwnd, vsFilename, L"Missing Shader File", MB_OK);
		}*/

		return false;
	}

    // Compile the pixel shader code.
	hr = D3DX11CompileFromFile( L"Color.ps", NULL, NULL, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, NULL, 
								   &pixelShaderBuffer, &errorMessage, NULL);
	if(FAILED(hr))
	{
		// If the shader failed to compile it should have writen something to the error message.
		/*if(errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// If there was nothing in the error message then it simply could not find the file itself.
		else
		{
			MessageBox(hwnd, psFilename, L"Missing Shader File", MB_OK);
		}*/
		return false;
	}
	
	// Create the vertex shader from the buffer.
    hr = m_pd3dDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if(FAILED(hr))
	{
		return false;
	}

    // Create the pixel shader from the buffer.
    hr = m_pd3dDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if(FAILED(hr))
	{
		return false;
	}

	// Create the vertex input layout description.
	// This setup needs to match the VertexType stucture in the ModelClass and in the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "COLOR";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	polygonLayout[2].SemanticName = "TEXCOORD";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

	// Get a count of the elements in the layout.
    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	hr = m_pd3dDevice->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), 
									   vertexShaderBuffer->GetBufferSize(), &m_layout);
	if(FAILED(hr))
	{
		return false;
	}

	// Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

    // Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	hr = m_pd3dDevice->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if(FAILED(hr))
	{
		return false;
	}

	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	hr = m_pd3dDevice->CreateSamplerState(&samplerDesc, &m_sampleState);
	if(FAILED(hr))
	{
		return false;
	}



	/////////////////Create vertex and index buffers
	VertexType* vertices;
	D3D11_BUFFER_DESC vertexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData;

	// Set the number of vertices in the vertex array.
	m_vertexCount = 4;

	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];
	if(!vertices)
	{
		return false;
	}

	// Load the vertex array with data.
	vertices[0].position = D3DXVECTOR3( -1.0f, 1.0f, 0.5f);  // Bottom left.
	vertices[0].color = D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f);
	vertices[0].uv = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);
	
	vertices[1].position = D3DXVECTOR3( 1.0f, 1.0f, 0.5f);  // Bottom right.
	vertices[1].color = D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f);
	vertices[1].uv = D3DXVECTOR4(1.0f, 0.0f, 0.0f, 0.0f);

	vertices[2].position = D3DXVECTOR3( -1.0f, -1.0f, 0.5f);  // Top left.
	vertices[2].color = D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f);
	vertices[2].uv = D3DXVECTOR4(0.0f, 1.0f, 0.0f, 0.0f);

	vertices[3].position = D3DXVECTOR3( 1.0f, -1.0f, 0.5f);  // Top right.
	vertices[3].color = D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f);
	vertices[3].uv = D3DXVECTOR4(1.0f, 1.0f, 0.0f, 0.0f);


	// Set up the description of the static vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
    hr = m_pd3dDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(hr))
	{
		return false;
	}


	// Setup the raster description which will determine how and what polygons will be drawn.
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	hr = m_pd3dDevice->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if(FAILED(hr))
	{
		return false;
	}

	// Now set the rasterizer state.
	m_pImmediateContext->RSSetState(m_rasterState);
	
	// Setup the viewport for rendering.
	D3D11_VIEWPORT viewport;
    viewport.Width = (float)width;
    viewport.Height = (float)height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;

	// Create the viewport.
    m_pImmediateContext->RSSetViewports(1, &viewport);




	return true;
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	/*if( argc != 3 ) //we need 2 arguments
	{
		cout << "light_tracer.exe scene_description_file.txt output_image.bmp\n";
		cout << "scene_description_file.txt example:\n";
		cout << "	width 400\n";
		cout << "	height 300\n";
		cout << "	num_spheres 3\n";
		cout << "	0.0 0.0 5.0 1.0\n";
		cout << "	0.1 -0.05	3.0 0.2\n";
		cout << "	1.0	1.0	7.0 1.0\n";
		cout << "	num_triangles 2\n";
		cout << "	-10 -2 0 10 -2 10 10 -2 0\n";
		cout << "	-10 -2 0 -10 -2 10 10 -2 10\n";
		cout << "	num_point_lights 2\n";
		cout << "	1.0 1.0 0 1.0 1.0 1.0 0.2 50\n";
		cout << "	-0.5 -1.5 0 0 0 1.0 0.2 50\n";
		return 1;
	}*/

	Scene::Init();

	int w_in_pix, h_in_pix = 0;
	ParseDescription( "scene.txt", w_in_pix, h_in_pix );
	
	const float aspect = (float)w_in_pix / h_in_pix;
	//const float fov = 0.75f;

	bool initialized = InitWindowAndDevice( w_in_pix, h_in_pix );
	if( !initialized )
	{
		exit(-1);
	}

	
	
	double elapsed_secs;	
	//ImageBuffer* image = new ImageBuffer( w_in_pix, h_in_pix );



	//START THE TIMER!
	std::clock_t begin = std::clock();


	ID3D11Buffer* constant_bufer;
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.ByteWidth = sizeof(tConstantBuffer);
	bd.Usage = D3D11_USAGE_DEFAULT;
	HRESULT hr = m_pd3dDevice->CreateBuffer(&bd, 0, &constant_bufer);
	if( hr != S_OK )
	{
		return false;
	}
	tConstantBuffer cb;
	cb.width = w_in_pix;
	cb.height = h_in_pix;
	
	m_pImmediateContext->UpdateSubresource( constant_bufer, 0 , 0, &cb, 0, 0 );


	

	ID3D11Buffer* constant_bufer_camera;
	D3D11_BUFFER_DESC bd_camera;
	ZeroMemory(&bd_camera, sizeof(bd_camera));
	bd_camera.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd_camera.ByteWidth = sizeof(tConstantBufferCamera);
	bd_camera.Usage = D3D11_USAGE_DEFAULT;
	hr = m_pd3dDevice->CreateBuffer(&bd_camera, 0, &constant_bufer_camera);
	if( hr != S_OK )
	{
		return false;
	}
	tConstantBufferCamera cb_camera;
	
	Camera cam;
	cam.SetPosition( Vector3( 0, 0, 0 ) );
	cam.SetLookDir( Vector3( 0, 0, 1 ) );
	cam.SetUpDir( Vector3( 0, 1, 0 ) );


	bool done = false;
	while( !done )
	{
		// Handle the windows messages.
		MSG msg;
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// If windows signals to end the application then exit out.
		if(msg.message == WM_QUIT)
		{
			done = true;
		}
		else
		{
			static float x_move = 0;
			static float z_move = 0;
			
			static float mouse_x_move = 0;
			static float mouse_y_move = 0;

			switch(msg.message)
			{
				// Check if a key has been pressed on the keyboard.
				case WM_KEYDOWN:
				{
					if( msg.wParam == 'W' )
					{	
						z_move = 0.001f;
					}
					else if( msg.wParam == 'S' )
					{
						z_move = -0.001f;
					}
					else if( msg.wParam == 'A' )
					{
						x_move = -0.001f;
					}
					else if( msg.wParam == 'D' )
					{
						x_move = 0.001f;
					}
					else if( msg.wParam == 'I' )
					{
						cam.RotateUpDown( 0.01f );
					}
					else if( msg.wParam == 'K' )
					{
						cam.RotateUpDown( -0.01f );
					}
					else if( msg.wParam == 'J' )
					{
						cam.RotateLeftRight( 0.01f );
					}
					else if( msg.wParam == 'L' )
					{
						cam.RotateLeftRight( -0.01f );
					}
				}
				break;

				// Check if a key has been released on the keyboard.
				case WM_KEYUP:
				{
					if( msg.wParam == 'W' || msg.wParam == 'S' )
					{	
						z_move = 0;
					}
					else if( msg.wParam == 'A' || msg.wParam == 'D' )
					{
						x_move = 0;
					}
				}
				break;

				/*case WM_MOUSEMOVE:
				{
					int xPos = GET_X_LPARAM(msg.lParam); 
					int yPos = GET_Y_LPARAM(msg.lParam); 
					SetCursorPos( 420, 280 );
					
					static float x_sensitivity = -0.001f;
					static float y_sensitivity = -0.001f;
					cam.RotateUpDown( (yPos - 100)  * y_sensitivity );
					cam.RotateLeftRight( (xPos - 100) * x_sensitivity );
				}*/
				break;
			}
			
			cam.MoveForwardBack( z_move );
			cam.StrafeLeftRight( x_move );



			cb_camera.cam_pos_x = cam.GetPosition()->mX;
			cb_camera.cam_pos_y = cam.GetPosition()->mY;
			cb_camera.cam_pos_z = cam.GetPosition()->mZ;		
			
			/*
			zaxis = normal(At - Eye)
			xaxis = normal(cross(Up, zaxis))
			yaxis = cross(zaxis, xaxis)

			 xaxis.x           yaxis.x           zaxis.x          0
			 xaxis.y           yaxis.y           zaxis.y          0
			 xaxis.z           yaxis.z           zaxis.z          0
			*/
			Vector3 zaxis = *cam.GetView();
			Vector3 xaxis = *cam.GetUpVector() % zaxis;
			Vector3 yaxis = zaxis % xaxis;

			cb_camera.cam_orientation_00 = xaxis.mX; cb_camera.cam_orientation_01 = yaxis.mX; cb_camera.cam_orientation_02 = zaxis.mX; //float3x3;
			cb_camera.cam_orientation_10 = xaxis.mY; cb_camera.cam_orientation_11 = yaxis.mY; cb_camera.cam_orientation_12 = zaxis.mY; 
			cb_camera.cam_orientation_20 = xaxis.mZ; cb_camera.cam_orientation_21 = yaxis.mZ; cb_camera.cam_orientation_22 = zaxis.mZ; 
			
			m_pImmediateContext->UpdateSubresource( constant_bufer_camera, 0 , 0, &cb_camera, 0, 0 );

			float color[4];
			color[0] = (float)rand() / RAND_MAX;
			color[1] = 1.0f;
			color[2] = 0.0f;
			color[3] = 1.0f;

			// Clear the back buffer.
			m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, color);
    
			// Clear the depth buffer.
			//m_pImmediateContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);




			m_pImmediateContext->CSSetConstantBuffers( 0, 1, &constant_bufer );
			m_pImmediateContext->CSSetConstantBuffers( 1, 1, &constant_bufer_camera );
	

			// We now set up the shader and run it
			m_pImmediateContext->CSSetShader( m_computeShader, NULL, 0 );
			m_pImmediateContext->CSSetShaderResources( 0, 1, &m_srcDataGPUBufferView );
			m_pImmediateContext->CSSetUnorderedAccessViews( 0, 1, &m_destDataGPUBufferView, NULL );
		
			const int threads_dim = 32;
			m_pImmediateContext->Dispatch( (w_in_pix + (threads_dim -1)) / threads_dim, (h_in_pix + (threads_dim -1)) / threads_dim, 1 );

			m_pImmediateContext->CSSetShader( NULL, NULL, 0 );
			ID3D11UnorderedAccessView* ppUAViewNULL[1] = { NULL };
			ID3D11ShaderResourceView* ppSRVNULL[2] = { NULL, NULL };
			m_pImmediateContext->CSSetUnorderedAccessViews( 0, 1, ppUAViewNULL, NULL );
			m_pImmediateContext->CSSetShaderResources( 0, 2, ppSRVNULL );
			






			//render ray cast result to the back buffer
			unsigned int stride;
			unsigned int offset;

			// Set vertex buffer stride and offset.
			stride = sizeof(VertexType); 
			offset = 0;
    
			// Set the vertex buffer to active in the input assembler so it can be rendered.
			m_pImmediateContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
			
			// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
			m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			m_pImmediateContext->IASetInputLayout(m_layout);

			// Set the vertex and pixel shaders that will be used to render this triangle.
			m_pImmediateContext->VSSetShader(m_vertexShader, NULL, 0);
			m_pImmediateContext->PSSetShader(m_pixelShader, NULL, 0);
		
			// Set the sampler state in the pixel shader.
			m_pImmediateContext->PSSetSamplers(0, 1, &m_sampleState);
			
			m_pImmediateContext->PSSetShaderResources(0, 1, &m_destDataSRV);

			m_pImmediateContext->PSSetConstantBuffers(0, 1, &constant_bufer);

			// Render the triangle.
			m_pImmediateContext->Draw( m_vertexCount, 0 );
						
			m_pImmediateContext->PSSetShaderResources(0, 1, ppSRVNULL );

			m_pSwapChain->Present(0, 0);
		}
	}
	
	/*ID3D11Buffer* debugbuf = NULL;
	
	D3D11_BUFFER_DESC desc;
	ZeroMemory( &desc, sizeof(desc) );
	m_destDataGPUBuffer->GetDesc( &desc );

	UINT byteSize = desc.ByteWidth;

	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.MiscFlags = 0;

	if ( SUCCEEDED(m_pd3dDevice->CreateBuffer(&desc, NULL, &debugbuf)) )
	{
		m_pImmediateContext->CopyResource( debugbuf, m_destDataGPUBuffer );

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		if(m_pImmediateContext->Map(debugbuf, 0, D3D11_MAP_READ, 0, &mappedResource) != S_OK)
			return false;

		memcpy(image->GetBuffer(), mappedResource.pData, byteSize);

		m_pImmediateContext->Unmap(debugbuf, 0);

		debugbuf->Release();
	}*/
	
	
	//STOP THE TIMER!
	std::clock_t end = std::clock();
	elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

	//image->WriteToBMP( argv[2] );
	
	//printf( "Time to render: %f\n", elapsed_secs );
	//system( "pause" );

    return 0;
}
