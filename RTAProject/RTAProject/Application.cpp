#include "Application.h"
#include "OBJECT_VS.csh"
#include "OBJECT_PS.csh"
//#include "FBXLib.h"

// Constructor
Application::Application(HINSTANCE _hinst, WNDPROC _proc)
{
	
	// Creates the window
	CreateAppWindow(_hinst, _proc);

	// Creates the swapchain and back buffer
	InitGraphics();

	// Creates the view port
	CreateViewPorts();

	// Initilizing the depth stencil view
	CreateDepthBuffer();

	// Creating the shaders
	CreateShaders();

	// Creating the layouts for the shaders
	CreateLayouts();

	// Create a sampler state
	CreateSamplerState();

	// Initilizing data that is being send to the  VS_Shader
	InitializeToShader();

	// Creating const buffers
	CreateConstBuffers();

	// Loading the models
	LoadObjects();

	// Initializing the start mouse position
	GetCursorPos(&m_oldMousePos);
	
}

// Destructor
Application::~Application()
{
}

// Loops the application
bool Application::Run()
{
	Input();
	Update();
	Render();
	return true;
}

// Runs input
void Application::Input()
{
	FPCamera(0.01f);
	LightsControls(0.01f);
}

// Updates the scene
void Application::Update()
{

}

// Renders the scene
void Application::Render()
{
	// Setting the render target with the depth buffer
	m_deviceContext->RSSetViewports(1, &m_viewPort);
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetViewToTexture.p, m_depthView.p);
																				
	// Clearing the screen
	COLOR clearColor{ 0.39f, 0.58f, 0.92f, 1 };
	ClearScreen(clearColor);

	// Bounding with the GPU for objects
	m_deviceContext->VSSetShader(m_VS_OBJECT.p, NULL, NULL);
	m_deviceContext->PSSetShader(m_PS_OBJECT.p, NULL, NULL);
	m_deviceContext->PSSetSamplers(0, 1, &m_samplerState.p);
	m_deviceContext->IASetInputLayout(m_inputLayoutObject);

	// Sets and maps the shaders
	MapShaders();

	// Rendering objects
	m_testObject.Render(m_deviceContext);

	// Presenting the screen
	m_swapChain->Present(0, 0);
}

// Terminates the application
bool Application::ShutDown()
{
	UnregisterClass(L"DirectXApplication", m_application);
	return true;
}

// Creates the window
void Application::CreateAppWindow(HINSTANCE _hinst, WNDPROC _proc)
{
	m_application = _hinst;
	m_appWndProc = _proc;
	// Setting up the window
	WNDCLASSEX  wndClass;
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndClass.lpfnWndProc = m_appWndProc;
	wndClass.hInstance = m_application;
	wndClass.cbSize = sizeof(WNDCLASSEX);
	//wndClass.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_WINLOGO));
	wndClass.hIconSm = wndClass.hIcon;
	wndClass.lpszClassName = L"RTAProject";
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
	RegisterClassEx(&wndClass);
	
	// Window size rectangle
	RECT window_size = { 0, 0, WINDOW_HEIGHT, WINDOW_WIDTH };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);
	// Creating the window
	m_window = CreateWindow(L"RTAProject", L"RTA Project", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		window_size.right - window_size.left, window_size.bottom - window_size.top, NULL, NULL, m_application, this);
	ShowWindow(m_window, SW_SHOW);
}

// Clears the screen
void Application::ClearScreen(COLOR _color)
{
	m_deviceContext->ClearRenderTargetView(m_renderTargetViewToTexture.p, _color.GetColor()); 
	m_deviceContext->ClearDepthStencilView(m_depthView.p, D3D11_CLEAR_DEPTH, 1, 0);
}

void Application::InitGraphics()
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	// Creating the swap chain and the device
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// Set the buffer parameters
	DXGI_MODE_DESC buffer;
	ZeroMemory(&buffer, sizeof(DXGI_MODE_DESC));
	buffer.Height = WINDOW_HEIGHT;
	buffer.Width = WINDOW_WIDTH;
	buffer.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// Set the swap chain desc parameters
	swapChainDesc.BufferDesc = buffer;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = m_window;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Create the device and swap chain
	HRESULT hresultHandle = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG,
		NULL, NULL, D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext);

	// Setting the backbuffer
	CComPtr<ID3D11Texture2D> backBuffer;
	m_swapChain->GetBuffer(0, __uuidof(backBuffer), reinterpret_cast<void**>(&backBuffer));

	// Creating the render view
	m_device->CreateRenderTargetView(backBuffer, NULL, &m_renderTargetViewToTexture.p);
}

// Creates depth buffer
void Application::CreateDepthBuffer()
{
	// Creation of the texture
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = WINDOW_WIDTH;
	textureDesc.Height = WINDOW_HEIGHT;
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	// Creation of the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	m_device->CreateTexture2D(&textureDesc, nullptr, &m_depthBuffer.p);
	m_device->CreateDepthStencilView(m_depthBuffer.p, &depthDesc, &m_depthView.p);
}

// Create view ports
void Application::CreateViewPorts()
{
	// Creating the view port
	ZeroMemory(&m_viewPort, sizeof(m_viewPort));
	m_viewPort.Width = WINDOW_WIDTH;
	m_viewPort.Height = WINDOW_HEIGHT;
	m_viewPort.MaxDepth = 1;
}

// Creates the layouts
void Application::CreateLayouts()
{

	D3D11_INPUT_ELEMENT_DESC vLayoutObject[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMALS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SHINE", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "WORLDPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "CAMERA_POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "LIGHT_PROJ", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	// Creating layout for object shader
	m_device->CreateInputLayout(vLayoutObject, ARRAYSIZE(vLayoutObject), OBJECT_VS, sizeof(OBJECT_VS), &m_inputLayoutObject.p);
}

// Create shaders
void Application::CreateShaders()
{
	// Creates the shaders for the Object
	m_device->CreateVertexShader(OBJECT_VS, sizeof(OBJECT_VS), nullptr, &m_VS_OBJECT.p);
	m_device->CreatePixelShader(OBJECT_PS, sizeof(OBJECT_PS), nullptr, &m_PS_OBJECT.p);
}

// Creates the sampler state that goes to the pixel shader texture
void Application::CreateSamplerState()
{
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	m_device->CreateSamplerState(&samplerDesc, &m_samplerState.p);
}

void Application::LoadObjects()
{
	XMFLOAT3 testPosition { 0, 0, 0 };
	m_testObject.InstantiateModel(m_device, "..\\RTAProject\\FBX Files\\ground.obj", testPosition, 0);
	m_testObject.TextureObject(m_device, L"..\\RTAProject\\Textures\\groundTexture.dds");
}

// Sets te data that will be going to the shaders
void Application::InitializeToShader()
{

	// Setting the projection matrix
	m_viewToShader.projectionMatrix = XMMatrixPerspectiveFovLH(45,WINDOW_HEIGHT/WINDOW_WIDTH, SCREEN_ZNEAR, SCREEN_ZFAR);

	// Setting the View matrix
	m_viewToShader.viewMatrix = XMMatrixIdentity();
	m_viewToShader.viewMatrix.r[3].m128_f32[1] = 2.0f;
	m_viewToShader.viewMatrix.r[3].m128_f32[2] = -7.0f;

	// Inversing the camera
	m_viewToShader.viewMatrix = XMMatrixInverse(nullptr, m_viewToShader.viewMatrix);

	// Initilizes lights data
	InitilizeLights();
}

void Application::CreateConstBuffers()
{
	// Creating const buffer for the scene
	D3D11_BUFFER_DESC constBufferDesc;
	ZeroMemory(&constBufferDesc, sizeof(D3D11_BUFFER_DESC));
	constBufferDesc.ByteWidth = sizeof(SCENE_TO_VRAM);
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.StructureByteStride = sizeof(float);
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_device->CreateBuffer(&constBufferDesc, NULL, &m_constBufferScene.p);

	ZeroMemory(&constBufferDesc, sizeof(D3D11_BUFFER_DESC));
	constBufferDesc.ByteWidth = sizeof(LIGHT_TO_VRAM);
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.StructureByteStride = sizeof(float);
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_device->CreateBuffer(&constBufferDesc, NULL, &m_dirLightConstBuffer.p);

	ZeroMemory(&constBufferDesc, sizeof(D3D11_BUFFER_DESC));
	constBufferDesc.ByteWidth = sizeof(LIGHT_TO_VRAM);
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.StructureByteStride = sizeof(float);
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_device->CreateBuffer(&constBufferDesc, NULL, &m_spotLightConstBuffer.p);
}

void Application::FPCamera(float _speed)
{
	// Get the cursor position
	POINT mousePos;
	ZeroMemory(&mousePos, sizeof(POINT));
	GetCursorPos(&mousePos);

	// Calculating the difference of the mouse position
	float deltaX = float(m_oldMousePos.x - mousePos.x);
	float deltaY = float(m_oldMousePos.y - mousePos.y);

	// Rotating when holding left key
	if (GetAsyncKeyState(VK_LBUTTON))
	{
		m_viewToShader.viewMatrix = XMMatrixInverse(0, m_viewToShader.viewMatrix);
		XMVECTOR storePosition;
		storePosition.m128_f32[0] = m_viewToShader.viewMatrix.r[3].m128_f32[0];
		storePosition.m128_f32[1] = m_viewToShader.viewMatrix.r[3].m128_f32[1];
		storePosition.m128_f32[2] = m_viewToShader.viewMatrix.r[3].m128_f32[2];

		m_viewToShader.viewMatrix.r[3].m128_f32[0] = 0;
		m_viewToShader.viewMatrix.r[3].m128_f32[1] = 0;
		m_viewToShader.viewMatrix.r[3].m128_f32[2] = 0;

		m_viewToShader.viewMatrix = XMMatrixMultiply(XMMatrixRotationX(-deltaY*0.0005f), m_viewToShader.viewMatrix);
		m_viewToShader.viewMatrix = XMMatrixMultiply(m_viewToShader.viewMatrix, XMMatrixRotationY(deltaX*0.0005f));

		m_viewToShader.viewMatrix.r[3].m128_f32[0] = storePosition.m128_f32[0];
		m_viewToShader.viewMatrix.r[3].m128_f32[1] = storePosition.m128_f32[1];
		m_viewToShader.viewMatrix.r[3].m128_f32[2] = storePosition.m128_f32[2];

		m_viewToShader.viewMatrix = XMMatrixInverse(0, m_viewToShader.viewMatrix);
	}

	m_oldMousePos = mousePos;

	if (GetAsyncKeyState('W'))
		m_viewToShader.viewMatrix.r[3].m128_f32[2] -= _speed;
	else if (GetAsyncKeyState('S'))
		m_viewToShader.viewMatrix.r[3].m128_f32[2] += _speed;
	if (GetAsyncKeyState('D'))
		m_viewToShader.viewMatrix.r[3].m128_f32[0] -= _speed;
	else if (GetAsyncKeyState('A'))
		m_viewToShader.viewMatrix.r[3].m128_f32[0] += _speed;
}

void Application::LightsControls(float _speed)
{
	// Turn on lights
	if (GetAsyncKeyState('O') && !m_keyPressed)
	{
		m_dirLightToShader.status = !m_dirLightToShader.status;
		m_keyPressed = true;
	}
	if (GetAsyncKeyState('I') && !m_keyPressed)
	{
		m_spotLightToShader.status = !m_spotLightToShader.status;
		m_keyPressed = true;
	}
	if (!GetAsyncKeyState('I') && !GetAsyncKeyState('O') &&  m_keyPressed)
		m_keyPressed = false;

	// Move point light
	if (m_spotLightToShader.status)
	{
		if (GetAsyncKeyState(VK_DOWN))
			m_spotLightToShader.transform.m128_f32[2] -= _speed;//float(Timer.Delta())* 2.0f;
		else if (GetAsyncKeyState(VK_UP))
			m_spotLightToShader.transform.m128_f32[2] += _speed;
		if (GetAsyncKeyState(VK_LEFT))
			m_spotLightToShader.transform.m128_f32[0] -= _speed;
		else if (GetAsyncKeyState(VK_RIGHT))
			m_spotLightToShader.transform.m128_f32[0] += _speed;
		if (GetAsyncKeyState('Z'))
			m_spotLightToShader.transform.m128_f32[1] += _speed;
		else if (GetAsyncKeyState('X'))
			m_spotLightToShader.transform.m128_f32[1] -= _speed;
	}
}

void Application::MapShaders()
{
	// Set constant buffers
	m_deviceContext->VSSetConstantBuffers(1, 1, &m_constBufferScene.p);
	m_deviceContext->PSSetConstantBuffers(1, 1, &m_constBufferScene.p);
	// Lights const buffer
	m_deviceContext->PSSetConstantBuffers(2, 1, &m_dirLightConstBuffer.p);
	m_deviceContext->PSSetConstantBuffers(4, 1, &m_spotLightConstBuffer.p);

	// Scene mapping
	D3D11_MAPPED_SUBRESOURCE mapSubresource;
	ZeroMemory(&mapSubresource, sizeof(mapSubresource));
	m_deviceContext->Map(m_constBufferScene, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubresource);
	memcpy(mapSubresource.pData, &m_viewToShader, sizeof(SCENE_TO_VRAM));
	m_deviceContext->Unmap(m_constBufferScene, NULL);
	// Directional light
	ZeroMemory(&mapSubresource, sizeof(mapSubresource));
	m_deviceContext->Map(m_dirLightConstBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubresource);
	memcpy(mapSubresource.pData, &m_dirLightToShader, sizeof(LIGHT_TO_VRAM));
	m_deviceContext->Unmap(m_dirLightConstBuffer, NULL);
	// Spot light
	ZeroMemory(&mapSubresource, sizeof(mapSubresource));
	m_deviceContext->Map(m_spotLightConstBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubresource);
	memcpy(mapSubresource.pData, &m_spotLightToShader, sizeof(LIGHT_TO_VRAM));
	m_deviceContext->Unmap(m_spotLightConstBuffer, NULL);
}

// Initilizes the light for the shader
void Application::InitilizeLights()
{
	// Direction light
	ZeroMemory(&m_dirLightToShader, sizeof(LIGHT_TO_VRAM));
	// Setting the color 
	m_dirLightToShader.color.r = 0.7f;
	m_dirLightToShader.color.g = 0.7f;
	m_dirLightToShader.color.b = 0.7f;
	m_dirLightToShader.color.a = 0.7f;
	// Setting the direction
	m_dirLightToShader.direction.x = -1.0f;
	m_dirLightToShader.direction.y = -0.5f;
	m_dirLightToShader.status = true;

	// Spot light
	ZeroMemory(&m_spotLightToShader, sizeof(LIGHT_TO_VRAM));
	m_spotLightToShader.direction.y = -1;
	m_spotLightToShader.radius = 0.9238f;
	m_spotLightToShader.transform.m128_f32[1] = 4;
	m_spotLightToShader.color.r = 1.0f;
	m_spotLightToShader.color.g = 1.0f;
	m_spotLightToShader.color.b = 1.0f;
	m_spotLightToShader.color.a = 1.0f;
}