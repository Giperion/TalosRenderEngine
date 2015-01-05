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

// ���������� ��������� Windows
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

// �����������
CWindow::CWindow(void)
{
	hInstance = NULL;
	hWindow = NULL;
}

// ����������
CWindow::~CWindow(void)
{
	hInstance = NULL;
	hWindow = NULL;
}

// �������������� ����
bool CWindow::Initialize(void)
{
	return true;
}

// ���������� ��������� ����
void CWindow::Release(void)
{
	delete this;
}

// ������������ ���������
bool CWindow::Update(void)
{
	return true;
}

// ���������� ����� ����
HWND CWindow::GetHandle(void)
{
	return (this->hWindow);
}