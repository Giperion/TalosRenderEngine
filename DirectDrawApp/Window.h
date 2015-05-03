#pragma once
#include "PrivateMacro.h"
#include "resource.h"

class Window
{
public:
	Window(WindowParam param, HINSTANCE instance, int nCmdShow);
	DWORD Go(WNDPROC proc);

	LRESULT CALLBACK MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	WindowParam GetWindowParam();

	~Window();
private:

	bool MessageLoop();
	MSG msg;
	HACCEL hAccelTable;
	ATOM WindowClass;
	HWND hWnd;

	int nCmdShow;
	HINSTANCE instance;
	WindowParam Description;

	LPWSTR StatusText;
	DrawEngine* pDrawEngine;

	bool RunMutex;
};