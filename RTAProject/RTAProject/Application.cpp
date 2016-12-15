#include "Application.h"
#include "OBJECT_VS.csh"
#include "OBJECT_PS.csh"
#include "ANIMATION_VS.csh"
#include "Log.h"

// Constructor
Application::Application(HINSTANCE _hinst, WNDPROC _proc)
{
	// Helps debugging
	//LogSetUp(L"RTA Project Application");
	m_temptimeer = 0;
	m_loopAnimation = false;
	m_thirdPersonCam = false;

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

	// Creating const buffers
	CreateConstBuffers();

	// Loading the models
	LoadObjects();

	// Initilizing data that is being send to the  VS_Shader
	InitializeToShader();

	// Initializing the start mouse position
	GetCursorPos(&m_oldMousePos);


}

// Destructor
Application::~Application()
{
	for (unsigned int i = 0; i < m_testbonesVec.size(); ++i)
		delete m_testbonesVec[i];
	for (unsigned int i = 0; i < m_mageBonesVec.size(); ++i)
		delete m_mageBonesVec[i];
	for (unsigned int i = 0; i < m_bearBonesVec.size(); ++i)
		delete m_bearBonesVec[i];
	
	//dll_loader.UnloadDLL();
}

// Resize Window
void Application::ResizeWindow(unsigned int _width, unsigned int _height)
{
	
	// Resizing the depth buffer
	m_depthBuffer.Release();
	m_depthView.Release();
	m_renderTargetView.Release();

	m_swapChain->ResizeBuffers(1, _width, _height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	// Creation of the texture
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = _width;
	textureDesc.Height = _height;
	textureDesc.Format = DXGI_FORMAT_D32_FLOAT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	m_device->CreateTexture2D(&textureDesc, nullptr, &m_depthBuffer.p);

	// Creation of the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthDesc.Texture2D.MipSlice = 0;
	m_device->CreateDepthStencilView(m_depthBuffer.p, nullptr, &m_depthView.p);

	// Resizing the viewport
	ZeroMemory(&m_viewPort, sizeof(m_viewPort));
	m_viewPort.Width = float(_width);
	m_viewPort.Height = float(_height);
	m_viewPort.MaxDepth = 1;

	// Setting the backbuffer
	ID3D11Texture2D *backBuffer;
	m_swapChain->GetBuffer(0, __uuidof(backBuffer), reinterpret_cast<void**>(&backBuffer));
	// Creating the render view
	m_device->CreateRenderTargetView(backBuffer, NULL, &m_renderTargetView.p);

	// It releases the back buffer from data so it can take new data
	backBuffer->Release();

	// Setting the projection matrix
	m_viewToShader.projectionMatrix = XMMatrixPerspectiveFovLH(45, float(_width) / float(_height), SCREEN_ZNEAR, SCREEN_ZFAR);
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

	TPCamera(m_fbxMage,0.01f);
	LightsControls(0.01f);
	FrameInput(m_fbxMage);

	if (!GetAsyncKeyState('I') && !GetAsyncKeyState('O') && !GetAsyncKeyState('R') && !GetAsyncKeyState('T') && !GetAsyncKeyState('Y') && !GetAsyncKeyState('B') && m_keyPressed)
		m_keyPressed = false;
}

// Updates the scene
void Application::Update()
{

	LoopAnimation(m_fbxMage,30);
	UpdateFrames(m_fbxMage, m_mageBonesVec);
	//TPCamera(m_fbxMage, 0.03f);
}

// Renders the scene
void Application::Render()
{
	// Setting the render target with the depth buffer
	m_deviceContext->RSSetViewports(1, &m_viewPort);
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView.p, m_depthView.p);

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
    m_groundObject.Render(m_deviceContext);

	//RenderBones();
	m_fbxBear.Render(m_deviceContext);

	m_fbxTest.Render(m_deviceContext);
	// Render animated objects
	m_deviceContext->IASetInputLayout(m_inputLayoutAnimation);
	m_deviceContext->VSSetShader(m_VS_ANIMATION.p, NULL, NULL);

	m_fbxMage.Render(m_deviceContext);

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
	m_deviceContext->ClearRenderTargetView(m_renderTargetView.p, _color.GetColor());
	m_deviceContext->ClearDepthStencilView(m_depthView.p, D3D11_CLEAR_DEPTH, 1, 0);
}

// Initializes graphics
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
	m_device->CreateRenderTargetView(backBuffer, NULL, &m_renderTargetView.p);
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
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMALS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	    { "BITANGENTS", 0,  DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SKIN_INDICES", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SKIN_WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3D11_INPUT_ELEMENT_DESC vLayoutAnimation[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMALS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "UV", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENTS", 0,  DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SKIN_INDICES", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SKIN_WEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	// Creating layout for object shader
	m_device->CreateInputLayout(vLayoutObject, ARRAYSIZE(vLayoutObject), OBJECT_VS, sizeof(OBJECT_VS), &m_inputLayoutObject.p);
	m_device->CreateInputLayout(vLayoutAnimation, ARRAYSIZE(vLayoutAnimation), ANIMATION_VS, sizeof(ANIMATION_VS), &m_inputLayoutAnimation.p);
}

// Create shaders
void Application::CreateShaders()
{
	// Creates the shaders for the Object
	m_device->CreateVertexShader(OBJECT_VS, sizeof(OBJECT_VS), nullptr, &m_VS_OBJECT.p);
	m_device->CreatePixelShader(OBJECT_PS, sizeof(OBJECT_PS), nullptr, &m_PS_OBJECT.p);
	m_device->CreateVertexShader(ANIMATION_VS, sizeof(ANIMATION_VS), nullptr, &m_VS_ANIMATION.p);
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

// Loads objects
void Application::LoadObjects()
{
	XMFLOAT3 groundPosition{ 0, 0, 0 };
	m_groundObject.InstantiateModel(m_device, "..\\RTAProject\\Assets\\ground.obj", groundPosition,0);
	m_groundObject.TextureObject(m_device, L"..\\RTAProject\\Assets\\Textures\\groundTexture.dds", L"..\\RTAProject\\Assets\\Textures\\sss2NormalMap.dds");

	XMFLOAT3 testBoxPos{ 0, 0, 0 };
	m_fbxTest.InstantiateFBX(m_device, "..\\RTAProject\\Assets\\FBX Files\\Testbox\\Box_Idle.fbx", testBoxPos, 1);
	m_fbxTest.TextureObject(m_device, L"..\\RTAProject\\Assets\\Textures\\TestCubeTexture.dds", L"..\\RTAProject\\Assets\\Textures\\sss2NormalMap.dds", L"..\\RTAProject\\Assets\\Textures\\specularBoxMap.dds");
	InitializeBones(m_fbxTest, m_testbonesVec);
		
	XMFLOAT3 magePos{ 5.0f, 0.0f, 0.0f };
	m_fbxMage.InstantiateFBX(m_device, "..\\RTAProject\\Assets\\FBX Files\\Mage\\Walk.fbx", magePos, 1);
	m_fbxMage.TextureObject(m_device, L"..\\RTAProject\\Assets\\Textures\\MageTexture.dds", L"..\\RTAProject\\Assets\\Textures\\mageNormalMap.dds" , L"..\\RTAProject\\Assets\\Textures\\mageSpecularMap.dds");
	InitializeBones(m_fbxMage, m_mageBonesVec);
	m_bonesToShader.positionOffset = XMMatrixIdentity();
	m_bonesToShader.positionOffset.r[3].m128_f32[0] += 5.0f;

	XMFLOAT3 bearPos{ -5.0f, 0.0f, 0.0f };
	m_fbxBear.InstantiateFBX(m_device, "..\\RTAProject\\Assets\\FBX Files\\Teddy\\Teddy_Attack2.fbx", bearPos, 1);
 	m_fbxBear.SetWorldMatrix(XMMatrixMultiply(m_fbxBear.GetWorldMatrix(),XMMatrixScaling(0.03f,0.03f,0.03f)));
	m_fbxBear.TextureObject(m_device, L"..\\RTAProject\\Assets\\Textures\\bearTexture.dds");

	m_fbxBear.SetPosition(bearPos.x, bearPos.y, bearPos.z);
	InitializeBones(m_fbxBear, m_bearBonesVec);
}

// Sets te data that will be going to the shaders
void Application::InitializeToShader()
{

	// Setting the projection matrix
	m_viewToShader.projectionMatrix = XMMatrixPerspectiveFovLH(45, WINDOW_HEIGHT / WINDOW_WIDTH, SCREEN_ZNEAR, SCREEN_ZFAR);

	// Setting the View matrix
	m_viewToShader.viewMatrix = XMMatrixIdentity();
	m_viewToShader.viewMatrix.r[3].m128_f32[1] = 5.0f;
	m_viewToShader.viewMatrix.r[3].m128_f32[2] = 15.0f;
	XMMATRIX nintyRotMatrix = XMMatrixMultiply(XMMatrixRotationY(3.141f), m_viewToShader.viewMatrix);
	m_viewToShader.viewMatrix = nintyRotMatrix;

	// Inversing the camera
	m_viewToShader.viewMatrix = XMMatrixInverse(nullptr, m_viewToShader.viewMatrix);

	// Initilizes lights data
	InitilizeLights();

	// Initialize bones data for the shader

	//for (unsigned int i = 0; i < 28; ++i)
		//m_bonesToShader.bones[i] = m_bearBonesVec[i]->GetWorldMatrix();
}

// Creates constant buffers
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

	ZeroMemory(&constBufferDesc, sizeof(D3D11_BUFFER_DESC));
	constBufferDesc.ByteWidth = sizeof(BONES_TO_VRAM);
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.StructureByteStride = sizeof(float);
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_device->CreateBuffer(&constBufferDesc, NULL, &m_bonesConstBuffer.p);

}

// Movement for the camera
void Application::FPCamera(float _speed)
{
	if (!m_thirdPersonCam)
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
			m_viewToShader.viewMatrix = XMMatrixMultiply(m_viewToShader.viewMatrix, XMMatrixRotationY(-deltaX*0.0005f));

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

		if (GetAsyncKeyState('Q'))
			m_viewToShader.viewMatrix.r[3].m128_f32[1] += _speed;
		else if (GetAsyncKeyState('E'))
			m_viewToShader.viewMatrix.r[3].m128_f32[1] -= _speed;

		// Spot light 
		m_viewToShader.viewMatrix = XMMatrixInverse(0, m_viewToShader.viewMatrix);

		m_spotLightToShader.transform.m128_f32[0] = m_viewToShader.viewMatrix.r[3].m128_f32[0];
		m_spotLightToShader.transform.m128_f32[1] = m_viewToShader.viewMatrix.r[3].m128_f32[1];
		m_spotLightToShader.transform.m128_f32[2] = m_viewToShader.viewMatrix.r[3].m128_f32[2];
		m_spotLightToShader.direction.x = m_viewToShader.viewMatrix.r[2].m128_f32[0];
		m_spotLightToShader.direction.y = m_viewToShader.viewMatrix.r[2].m128_f32[1];
		m_spotLightToShader.direction.z = m_viewToShader.viewMatrix.r[2].m128_f32[2];

		m_viewToShader.viewMatrix = XMMatrixInverse(0, m_viewToShader.viewMatrix);
	}
}

// Controls for the lights
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

	// Move point light
	//if (m_spotLightToShader.status)
	//{
	//	if (GetAsyncKeyState(VK_DOWN))
	//		m_spotLightToShader.transform.m128_f32[2] -= _speed;//float(Timer.Delta())* 2.0f;
	//	else if (GetAsyncKeyState(VK_UP))
	//		m_spotLightToShader.transform.m128_f32[2] += _speed;
	//	if (GetAsyncKeyState(VK_LEFT))
	//		m_spotLightToShader.transform.m128_f32[0] -= _speed;
	//	else if (GetAsyncKeyState(VK_RIGHT))
	//		m_spotLightToShader.transform.m128_f32[0] += _speed;
	//	if (GetAsyncKeyState('Z'))
	//		m_spotLightToShader.transform.m128_f32[1] += _speed;
	//	else if (GetAsyncKeyState('X'))
	//		m_spotLightToShader.transform.m128_f32[1] -= _speed;
	//}
}

// Maps shaders before rendering
void Application::MapShaders()
{
	// Set constant buffers
	m_deviceContext->VSSetConstantBuffers(1, 1, &m_constBufferScene.p);
	m_deviceContext->PSSetConstantBuffers(5, 1, &m_constBufferScene.p);
	// Lights const buffer
	m_deviceContext->PSSetConstantBuffers(2, 1, &m_dirLightConstBuffer.p);
	m_deviceContext->PSSetConstantBuffers(4, 1, &m_spotLightConstBuffer.p);

	// Bones const buffer
	m_deviceContext->VSSetConstantBuffers(2,1, &m_bonesConstBuffer.p);

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

	// Bones data
	ZeroMemory(&mapSubresource, sizeof(mapSubresource));
	m_deviceContext->Map(m_bonesConstBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubresource);
	memcpy(mapSubresource.pData, &m_bonesToShader, sizeof(BONES_TO_VRAM));
	m_deviceContext->Unmap(m_bonesConstBuffer, NULL);
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

// Update animation frame data
void Application::FrameInput(Object& _object)
{
	if (GetAsyncKeyState('R') && !m_keyPressed)
	{
		m_keyPressed = true;
		_object.BackwardFrame();
	}
	if (GetAsyncKeyState('T') && !m_keyPressed)
	{
		m_keyPressed = true;
		_object.ForwardFrame();
	}

	if (GetAsyncKeyState('Y') && !m_keyPressed)
	{
		m_keyPressed = true;
		m_loopAnimation = !m_loopAnimation;
	}
}

void Application::UpdateFrames(Object& _object, vector<Object*>& _renderedBones)
{
	Animation currAnimation = _object.GetAnimation();
	for (unsigned int i = 0; i < currAnimation.m_keyFrame.size(); ++i)
	{
		// Data that goes to the shader
		XMMATRIX bindPose = _object.GetFBXBones()[i].m_worldMatrix;

		XMMATRIX inverseBindpose = XMMatrixInverse(0, bindPose); // Getting inverse of bind pose
		XMMATRIX currBoneAtFrame = currAnimation.m_keyFrame[i].m_bones[_object.GetCurrFrame()].m_worldMatrix; // Getting current frame

		m_bonesToShader.bones[i] = XMMatrixMultiply(inverseBindpose, currBoneAtFrame); // Sends the data to the shader
		_renderedBones[i]->SetWorldMatrix(currBoneAtFrame); // Updates the bones object to their new position

	}
}

void Application::LoopAnimation(Object& _object, unsigned int _speed)
{
	if (m_loopAnimation)
	{
		++m_temptimeer;
		if (m_temptimeer > _speed)
		{
			m_temptimeer = 0;
			_object.ForwardFrame();
		}
	}
}

void Application::TPCamera(Object& _object, float _speed)
{

	if (GetAsyncKeyState('B') && !m_keyPressed)
	{
		m_thirdPersonCam = !m_thirdPersonCam;
		m_keyPressed = true;
	}

	if (m_thirdPersonCam)
	{
		XMMATRIX newPos = _object.GetWorldMatrix();
		if (GetAsyncKeyState('W'))
		{			
			++m_temptimeer;
			newPos.r[3] = newPos.r[3] + newPos.r[2] * _speed;
			if (m_temptimeer > 10)
			{
				m_temptimeer = 0;
				_object.ForwardFrame();
			}
		}
		else if (GetAsyncKeyState('S'))
		{
			++m_temptimeer;
			newPos.r[3] = newPos.r[3] + newPos.r[2] * -_speed;
			if (m_temptimeer > 10)
			{
				m_temptimeer = 0;
				_object.ForwardFrame();
			}
		}
		if (GetAsyncKeyState('D'))
			newPos = XMMatrixMultiply(XMMatrixRotationY(_speed * 0.5f), newPos);
		else if (GetAsyncKeyState('A'))
			newPos = XMMatrixMultiply(XMMatrixRotationY(-_speed * 0.5f), newPos);
	
		_object.SetWorldMatrix(newPos);
		m_bonesToShader.positionOffset = newPos;

		m_viewToShader.viewMatrix = XMMatrixInverse(0, _object.GetWorldMatrix());
		m_viewToShader.viewMatrix.r[3].m128_f32[1] = m_viewToShader.viewMatrix.r[3].m128_f32[1] - 10.0f;
		m_viewToShader.viewMatrix.r[3].m128_f32[2] = m_viewToShader.viewMatrix.r[3].m128_f32[2] + 6.0f;
		m_viewToShader.viewMatrix = XMMatrixInverse(0, m_viewToShader.viewMatrix);
		m_viewToShader.viewMatrix = XMMatrixMultiply(XMMatrixRotationX(0.5853f), m_viewToShader.viewMatrix);

		// Setting the spotlight
		m_spotLightToShader.transform.m128_f32[0] = m_viewToShader.viewMatrix.r[3].m128_f32[0];
		m_spotLightToShader.transform.m128_f32[1] = m_viewToShader.viewMatrix.r[3].m128_f32[1];
		m_spotLightToShader.transform.m128_f32[2] = m_viewToShader.viewMatrix.r[3].m128_f32[2];
		m_spotLightToShader.direction.x = m_viewToShader.viewMatrix.r[2].m128_f32[0];
		m_spotLightToShader.direction.y = m_viewToShader.viewMatrix.r[2].m128_f32[1];
		m_spotLightToShader.direction.z = m_viewToShader.viewMatrix.r[2].m128_f32[2];

		m_viewToShader.viewMatrix = XMMatrixInverse(0, m_viewToShader.viewMatrix);
	}
}

void Application::InitializeBones(Object& _object, vector<Object*>& _boneObject)
{
	vector<Transform> tempTransformBones = _object.GetFBXBones(); 
	XMFLOAT3 objPos = XMFLOAT3(_object.GetWorldMatrix().r[3].m128_f32[0], _object.GetWorldMatrix().r[3].m128_f32[1], _object.GetWorldMatrix().r[3].m128_f32[2]);
	for (unsigned int i = 0; i < tempTransformBones.size(); ++i)
	{
		Object* bone = new Object();
		bone->InstantiateModel(m_device, "..\\RTAProject\\Assets\\boneSphere.obj", objPos, 0);
		bone->TextureObject(m_device, L"..\\RTAProject\\Assets\\Textures\\TestCubeTexture.dds");
		_boneObject.push_back(bone);
	}
}

void Application::RenderBones()
{
	for (unsigned int i = 0; i < m_mageBonesVec.size(); ++i)
		m_mageBonesVec[i]->Render(m_deviceContext);
	for (unsigned int i = 0; i < m_testbonesVec.size(); ++i)
		m_testbonesVec[i]->Render(m_deviceContext);
	for (unsigned int i = 0; i < m_bearBonesVec.size(); ++i)
		m_bearBonesVec[i]->Render(m_deviceContext);
}