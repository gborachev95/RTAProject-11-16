#pragma once
#include "includes.h"

// Constructor
Application::Application(HINSTANCE _hinst, WNDPROC _proc)
{
	CreateAppWindow(_hinst, _proc);
}

// Destructor
Application::~Application()
{
}

// Loops the application
bool Application::Run()
{
	return true;
}

// Terminates the application
bool Application::ShutDown()
{
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
	wndClass.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_WINLOGO));
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