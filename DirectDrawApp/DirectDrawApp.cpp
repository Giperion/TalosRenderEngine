// DirectDrawApp.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "DirectDrawApp.h"
#include "DrawEngine.h"
#include "PrivateMacro.h"
#include "Window.h"

extern volatile unsigned int RenderCores;


LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

Window* window;
DrawEngine* pDrawEngine;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	MessageBox(NULL, L"Bla bla bla", L"Bla bla bla", MB_OK);

#pragma region DetectProcessorCores
#ifndef FIXEDCORE
#ifndef _WIN64
#error "Application used some functions, avaliable only in x64 mode. Please check code and correct function"
#endif

	//Автор в курсе про std::thread::hardware_concurency(). Автор просто недолюбливает std и если можно, то использует WinAPI
	SYSTEM_INFO sysinfo;
	ZeroMemory(&sysinfo, sizeof(sysinfo));
	GetNativeSystemInfo(&sysinfo);

	RenderCores = sysinfo.dwNumberOfProcessors;
#endif
#pragma endregion

	WindowParam param;
	param.width = ENGINEWIDTH;
	param.height = ENGINEHEIGHT;
	param.x = 100;
	param.y = 100;

	window = new Window(param, hInstance, nCmdShow);
	return window->Go(WndProc);

}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return window->MessageHandler(hWnd, message, wParam, lParam);
}

