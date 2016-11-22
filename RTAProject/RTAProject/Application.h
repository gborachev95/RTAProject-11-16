#pragma once	
#include "includes.h"

	class Application
	{
	private:
		// Window handles
		HWND					     m_window;			  
		HINSTANCE				     m_application;
		WNDPROC						 m_appWndProc;
		// Graphics handles
		CComPtr<IDXGISwapChain>      m_swapChain = NULL;
		CComPtr<ID3D11Device>        m_device = NULL;
		CComPtr<ID3D11DeviceContext> m_deviceContext = NULL;
		D3D11_VIEWPORT               m_viewPort;
        
		//std::vector
		// Private methods that are called only inside of the class
	private:
		// Creates the window
		void CreateAppWindow(HINSTANCE _hinst, WNDPROC _proc);
		void Input();
		void Update();
		void Render();

		// Public methods that are called outside of the class
	public:
		// Application Methods
		Application(HINSTANCE _hinst, WNDPROC _proc);
		~Application();
		bool Run();
		bool ShutDown();
	};
