#pragma once

#include "PrivateMacro.h"
#include "resource.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "IWindow.hpp"

// ���������� ���������� ����
class CWindow : public IWindow
{
private:

	// ����� ����������
	HINSTANCE hInstance;

	// ����� ����
	HWND hWindow;

public:

	// �����������
	CWindow(void);

	// ����������
	~CWindow(void);

	// �������������� ����
	bool Initialize(void);

	// ���������� ��������� ����
	void Release(void);

	// ������������ ���������
	bool Update(void);

	// ���������� ����� ����
	HWND GetHandle(void);
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class Window
{
private:

	bool MessageLoop();
	MSG msg;
	HACCEL hAccelTable;
	ATOM WindowClass;
	HWND hWnd;

	int nCmdShow;
	HINSTANCE instance;
	WindowParam coord;

	LPWSTR StatusText;
	DrawEngine* pDrawEngine;

	bool RunMutex;

public:

	// Default constructor
	Window(WindowParam param, HINSTANCE instance, int nCmdShow);

	// Destructor
	~Window();

	//
	DWORD Go(WNDPROC proc);

	//
	LRESULT CALLBACK MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	//
	WindowParam GetWindowParam();
};