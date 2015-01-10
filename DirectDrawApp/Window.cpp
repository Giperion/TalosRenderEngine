#include "stdafx.h"
#include "Window.h"
#include "PCEngineRenderer.h"

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

Window::Window(WindowParam param, HINSTANCE instance, int nCmdShow)
{
	RunMutex = false;
	this->coord = param;
	this->instance = instance;
	this->nCmdShow = nCmdShow;
}

DWORD Window::Go(WNDPROC proc)
{
	if (RunMutex) return 1;
	RunMutex = true;

	WCHAR Title[50];
	WCHAR ClassName[50];

	LoadString(instance, IDS_APP_TITLE, Title, 50);
	LoadString(instance, IDC_DIRECTDRAWAPP, ClassName, 50);

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = proc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = instance;
	wcex.hIcon = LoadIcon(instance, MAKEINTRESOURCE(IDI_DIRECTDRAWAPP));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_DIRECTDRAWAPP);
	wcex.lpszClassName = ClassName;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	WindowClass = RegisterClassEx(&wcex);


	hWnd = CreateWindow(ClassName, Title, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, ENGINEWIDTH, ENGINEHEIGHT, NULL, NULL, instance, NULL);

	if (!hWnd)
	{
		return 1;
	}

	if (AUTORENDER)
	{
		unsigned int latency = 1000 / AR_FPSLIMIT;
		SetTimer(hWnd, TIMER_RENDER, latency, NULL);
	}
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	hAccelTable = LoadAccelerators(instance, MAKEINTRESOURCE(IDC_DIRECTDRAWAPP));

	//RunMessageLoop
	while (MessageLoop());

	return (int)msg.wParam;
}

Window::~Window()
{
}

LRESULT CALLBACK Window::MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	RECT TextCoord;
	PCEngineRenderer* pEngine;


	switch (message)
	{
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(instance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_CREATE:
		//Теперь мне кажется это плохой идеей. Надо создавать его в другом потоке. И вызывать не вызовом pDrawEngine -> Render() а переключением переменной
		StatusText = new WCHAR[256];

		pDrawEngine = new DrawEngine(hWnd);
		if (pDrawEngine->CurrentState != DrawEngineState::DES_IDLE)
		{
			LPWSTR ErrorMessage = new WCHAR[100];
			StringCbPrintf(ErrorMessage, 100 * 2, L"Error, while creating DirectDraw context, on state: %c", pDrawEngine->CurrentState);
			MessageBox(hWnd, ErrorMessage, L"Critical Error", MB_OK | MB_ICONERROR);
			delete pDrawEngine;
			pDrawEngine = nullptr;
		}
		//CreateThread(NULL, NULL, RenderThread, pDrawEngine, NULL, NULL);
		pEngine = new PCEngineRenderer(ENGINEWIDTH, ENGINEHEIGHT);
		pDrawEngine->PushRenderer(pEngine);
		Sleep(1000);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		TextCoord.bottom = 5;
		TextCoord.top = 5;
		TextCoord.left = 5;
		TextCoord.right = 5;
		SetTextColor(hdc, RGB(255, 56, 255));
		StringCbPrintf(StatusText, 256 * sizeof(WCHAR), L"RenderTime: %f ms", pDrawEngine->GetLastRenderTime());
		DrawText(hdc, StatusText, -1, &TextCoord, DT_SINGLELINE | DT_NOCLIP);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		delete pDrawEngine;
		delete[] StatusText;
		pDrawEngine = nullptr;
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) DestroyWindow(hWnd);
		if (wParam == VK_F2)
		{
			pDrawEngine->Render();
			InvalidateRect(hWnd, NULL, false);
		}
		break;
	case WM_SIZE:
		pDrawEngine->Render();
		break;
	case WM_TIMER:
		if (wParam == TIMER_RENDER)
		{
			pDrawEngine->Render();
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

bool Window::MessageLoop()
{
	if (GetMessage(&msg, NULL, NULL, NULL))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			return true;
		}
		return true;
	}
	else
	{
		return false;
	}
}

//globals


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
