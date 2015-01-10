// DirectDrawApp.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "DirectDrawApp.h"
#include "DrawEngine.h"
#include "PrivateMacro.h"
#include "Window.h"



LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

Window* window;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	WindowParam param;
	param.width = ENGINEWIDTH;
	param.height = ENGINEHEIGHT;
	param.x = 100;
	param.y = 100;

	window = new Window(param, hInstance, nCmdShow);
	DWORD returnResult = window->Go(WndProc);
	delete window;
	return returnResult;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return window->MessageHandler(hWnd, message, wParam, lParam);
}

