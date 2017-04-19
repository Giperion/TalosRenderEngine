#include "stdafx.h"
#include "Window.h"


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

Window::Window(WindowParam param, HINSTANCE instance, int nCmdShow)
{
	if (param.type == WindowType::WT_Graphic)
	{
		
	}
	else if (param.type == WindowType::WT_Console)
	{
		if (!AllocConsole())
		{
			ExitProcess(1);
		}
		SetConsoleTitle(L"Log");
	}
	else
	{
		MessageBox(NULL, L"Undefined window type! Can't continue.", L"Critical Error", MB_OK | MB_ICONERROR);
		ExitProcess(1);
	}

	RunMutex = false;
	this->Description = param;
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

	GlobalSettings Settings;
	Settings.Load();
	

	hWnd = CreateWindow(ClassName, Title, (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME),
		Description.x, Description.y, Settings.Width, Settings.Height, NULL, NULL, instance, &Settings);

	
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
	if (Description.type == WindowType::WT_Console)
	{
		FreeConsole();
	}
}

LRESULT CALLBACK Window::MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
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
		case ID_ENGINE_CPU:
		{

			GlobalSettings globalSettings = pDrawEngine->GetGlobalSettings();
			globalSettings.eEngineType = EngineType::Native;
			pDrawEngine->SetGlobalSettings(globalSettings);
			RenderFrame(hWnd, false);
		}
			break;
		case ID_ENGINE_CUDA:
		{
			GlobalSettings globalSettings = pDrawEngine->GetGlobalSettings();
			globalSettings.eEngineType = EngineType::CUDA;
			pDrawEngine->SetGlobalSettings(globalSettings);
			RenderFrame(hWnd, false);
		}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_CREATE:
		//Теперь мне кажется это плохой идеей. Надо создавать его в другом потоке. И вызывать не вызовом pDrawEngine -> Render() а переключением переменной
		//#UPDATE: БЛДАЖ. Когда я изначально писал этот код, никто не сказал мне, что статические переменные типа таких, лучше создавать как статические переменные
	{
		StatusText = new WCHAR[256];
		pRenderParam = new RenderArgs();
		mView = new MandelbrotView();
		ZeroMemory(pRenderParam, sizeof(RenderArgs));
		ZeroMemory(mView, sizeof(MandelbrotView));

		pRenderParam->args = mView;
		pRenderParam->size = sizeof(MandelbrotView);

		mView->scale = 1.0;
		mView->iteration = 20;

		LPCREATESTRUCT pCreate = (LPCREATESTRUCT)lParam;
		GlobalSettings* Settings = (GlobalSettings*)pCreate->lpCreateParams;

		pDrawEngine = new DrawEngine(hWnd, *Settings, PresentMethod::PM_OpenGL);
		if (pDrawEngine->CurrentState != DrawEngineState::DES_IDLE)
		{
			LPWSTR ErrorMessage = new WCHAR[100];
			StringCbPrintf(ErrorMessage, 100 * 2, L"Error, while creating DirectDraw context, on state: %d", pDrawEngine->CurrentState);
			MessageBox(hWnd, ErrorMessage, L"Critical Error", MB_OK | MB_ICONERROR);
			ExitProcess(1);
		}
	}
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc;
		RECT TextCoord;
		hdc = BeginPaint(hWnd, &ps);
		TextCoord.bottom = 5;
		TextCoord.top = 5;
		TextCoord.left = 5;
		TextCoord.right = 5;
		SetTextColor(hdc, RGB(255, 56, 255));
		StringCbPrintf(StatusText, 256 * sizeof(WCHAR), L"RenderTime: %f ms\r\n", pDrawEngine->GetLastRenderTime());
		DrawTextW(hdc, StatusText, -1, &TextCoord, DT_SINGLELINE | DT_NOCLIP);
		EndPaint(hWnd, &ps);
	}
		
		break;
	case WM_DESTROY:
		delete pDrawEngine;
		delete[] StatusText;
		pDrawEngine = nullptr;
		delete pRenderParam;
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) DestroyWindow(hWnd);
		if (wParam == VK_F2)
		{
			RenderFrame(hWnd, false);
		}
		if (wParam == VK_NUMPAD1)
		{
			mView->scale *= 1.1;
			RenderFrame(hWnd, false);
		}
		if (wParam == VK_NUMPAD3)
		{
			mView->scale *= 0.9;
			RenderFrame(hWnd, false);
		}

		if (wParam == VK_NUMPAD2)
		{
			mView->y -= 0.05 * mView->scale;
			RenderFrame(hWnd, false);
		}
		if (wParam == VK_NUMPAD8)
		{
			mView->y += 0.05 * mView->scale;
			RenderFrame(hWnd, false);
		}
		if (wParam == VK_NUMPAD4)
		{
			mView->x -= 0.05 * mView->scale;
			RenderFrame(hWnd, false);
		}
		if (wParam == VK_NUMPAD6)
		{
			mView->x += 0.05 * mView->scale;
			RenderFrame(hWnd, false);
		}

		if (wParam == VK_NUMPAD7)
		{
			mView->iteration += 5;
			RenderFrame(hWnd, false);
		}
		if (wParam == VK_NUMPAD9)
		{
			mView->iteration -= 5;
			RenderFrame(hWnd, true);
		}

		break;
	case WM_SIZE:
	{
		WORD NewWidth = LOWORD(lParam);
		WORD NewHeight = HIWORD(lParam);
		GlobalSettings Settings = pDrawEngine->GetGlobalSettings();
		Settings.Width = NewWidth;
		Settings.Height = NewHeight;
		pDrawEngine->SetGlobalSettings(Settings);
		RenderFrame(hWnd, false);
	}
		break;
	case WM_TIMER:
		if (wParam == TIMER_RENDER)
		{
			RenderFrame(hWnd, false);
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void Window::RenderFrame(HWND hWnd, bool ShowNewIterationCount)
{
    if (pDrawEngine->Render(pRenderParam))
    {
        InvalidateRect(hWnd, NULL, false);

        if (ShowNewIterationCount)
        {
            StringCbPrintf(StatusText, 256 * sizeof(WCHAR), L"New Iteration: %d ", mView->iteration);
            Log::GetInstance()->PrintMsg(UnicodeString(StatusText));
        }

        StringCbPrintf(StatusText, 256 * sizeof(WCHAR), L"RenderTime: %f ms", pDrawEngine->GetLastRenderTime());
        Log::GetInstance()->PrintMsg(UnicodeString(StatusText));
    }
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
