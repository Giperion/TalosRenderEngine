#include "CWindow.hpp"
#include "stdafx.h"

//
// ������� ����� ��������� ���������� � ���� � ���������� ��������� �� ����
//
IWindow *CALLTYPE WindowCreate(void)
{
	// ��������� ����������
	IWindow *pWindow = nullptr;

	// �������� �������� ������
	try { pWindow = new CWindow(); }
	catch(...) { pWindow = nullptr; }

	// ���������� ���������
	return pWindow;
}

//
// ���������� ��������� Windows
//
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

//
// �����������
//
CWindow::CWindow(void)
{
	hInstance = NULL;
	hWindow = NULL;
}

//
// ����������
//
CWindow::~CWindow(void)
{
	hInstance = NULL;
	hWindow = NULL;
}

//
// �������������� ����
//
bool CWindow::Initialize(void)
{
	//
	// �������� ����� ����������
	//

	hInstance = GetModuleHandle(NULL);

	//
	// ������������ ����� ����
	//

	WNDCLASSEX wcex;

	wcex.cbClsExtra = NULL;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.cbWndExtra = NULL;
	wcex.hbrBackground = static_cast<HBRUSH>(GetStockObject(DKGRAY_BRUSH));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDC_MYICON));
	wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDC_MYICON));
	wcex.hInstance = hInstance;
	wcex.lpfnWndProc = MsgProc;
	wcex.lpszClassName = TEXT("MAIN_WINDOW");
	wcex.lpszMenuName = NULL;
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

	if(RegisterClassEx(&wcex) == NULL)
		return false;

	return true;
}

//
// ���������� ��������� ����
//
void CWindow::Release(void)
{
	//
	// ����������� ����� ����
	//

	UnregisterClass(TEXT("MAIN_WINDOW"), hInstance);

	//
	// ������� ���������
	//

	delete this;
}

//
// ������������ ���������
//
bool CWindow::Update(void)
{
	//
	// ��������� ����������
	//

	MSG msg;

	//
	// ������������ ���������
	//

	while(PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE) == TRUE)
	{
		//
		// ��� ��������� ������ - ���������� false
		//

		if(msg.message == WM_QUIT) return false;

		//
		// ��������� ����������� �������
		//

		TranslateMessage(&msg);

		//
		// �������� ����������
		//

		DispatchMessage(&msg);
	}

	//
	// ���������� ������
	//

	return true;
}

//
// ���������� ����� ����
//
HWND CWindow::GetHandle(void)
{
	return (this->hWindow);
}