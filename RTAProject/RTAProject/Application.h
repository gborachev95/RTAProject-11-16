#pragma once	
#include "includes.h"
#include "Object.h"

//enum INPUT_ACTION {TOGGLE_FLASH,TOGGLE_DIRLIGHT,};
 class Application
	{
	private:
		// Window handles
		HWND							  m_window;			  
		HINSTANCE				          m_application;
		WNDPROC						      m_appWndProc;
		// Graphics handles
		CComPtr<IDXGISwapChain>			  m_swapChain = NULL;
		CComPtr<ID3D11Device>			  m_device = NULL;
		CComPtr<ID3D11DeviceContext>	  m_deviceContext = NULL;
		CComPtr<ID3D11Texture2D>		  m_depthBuffer = NULL;
		CComPtr<ID3D11DepthStencilView>   m_depthView = NULL;
		CComPtr<ID3D11RenderTargetView>   m_renderTargetView = NULL;
		D3D11_VIEWPORT					  m_viewPort;
		CComPtr<ID3D11VertexShader>       m_VS_OBJECT;
		CComPtr<ID3D11VertexShader>       m_VS_ANIMATION;
		CComPtr<ID3D11PixelShader>        m_PS_OBJECT;
		CComPtr<ID3D11InputLayout>        m_inputLayoutObject;
		CComPtr<ID3D11InputLayout>        m_inputLayoutAnimation;
		CComPtr<ID3D11SamplerState>       m_samplerState;
		CComPtr<ID3D11Buffer>             m_constBufferScene;
		CComPtr<ID3D11Buffer>             m_dirLightConstBuffer;
		CComPtr<ID3D11Buffer>             m_spotLightConstBuffer;

		//Skin
		CComPtr<ID3D11Buffer>             m_bonesConstBuffer;

        // Test variables
		Object                            m_groundObject;
		vector<Object*>                   m_testbonesVec;
		vector<Object*>                   m_mageBonesVec;
		vector<Object*>                   m_bearBonesVec;
		// Fbx files
		Object							  m_fbxTest;
		Object                            m_fbxMage;
		Object                            m_fbxBear;
		// Other
		vector<Transform>				  m_testBones;
		SCENE_TO_VRAM				      m_viewToShader;
		LIGHT_TO_VRAM                     m_dirLightToShader;
		LIGHT_TO_VRAM                     m_spotLightToShader;
		BONES_TO_VRAM					  m_bonesToShader;
		POINT                             m_oldMousePos;
		bool                              m_keyPressed;
		bool                              m_loopAnimation;
		unsigned int					  m_temptimeer;

		// Private methods that are called only inside of the class
	private:
		// Creates the window
		void CreateAppWindow(HINSTANCE _hinst, WNDPROC _proc);
		void Input();
		void Update();
		void Render();
		void ClearScreen(COLOR _color);
		void CreateDepthBuffer();
		void InitGraphics();
		void CreateViewPorts();
		void CreateLayouts();
		void CreateShaders();
		void CreateSamplerState();
		void LoadObjects();
		void InitializeToShader();
		void CreateConstBuffers();
		void FPCamera(float _speed);
		void LightsControls(float _speed);
		void MapShaders();
		void InitilizeLights();
		void FrameInput();
		void UpdateFrames(Object& _object, vector<Object*> _renderedBones, XMVECTOR _offset);
		void LoopAnimation(Object& _object, unsigned int _speed);
		// Public methods that are called outside of the class
	public:
		// Application Methods
		Application(HINSTANCE _hinst, WNDPROC _proc);
		~Application();
		bool Run();
		void ResizeWindow(unsigned int _width, unsigned int _height);
		bool ShutDown();
	};
