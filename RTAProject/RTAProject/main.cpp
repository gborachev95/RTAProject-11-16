#include "includes.h"
#include "Application.h"

// Prototypes
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
Application* myAppAccessor;
// Main loop 
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(-1);

	srand(unsigned int(time(0)));
	Application mainApp(hInstance, (WNDPROC)WndProc);
	myAppAccessor = &mainApp;
	MSG msg; ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT && mainApp.Run())
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	mainApp.ShutDown();
	return 0;
}

// Callback function
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;

	switch (message)
	{
	case (WM_DESTROY) : { PostQuitMessage(0); }
		break;
	case WM_SIZE:
	{
		// Getting the new height and width
		int newWidth = (int)LOWORD(lParam);
		int newHeight = (int)HIWORD(lParam);
		if (myAppAccessor)
			myAppAccessor->ResizeWindow(newWidth, newHeight);
		break;
	}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}