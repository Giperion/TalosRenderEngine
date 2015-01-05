#pragma once

#include "PrivateMacro.h"
#include "resource.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "IWindow.hpp"

// Реализация интерфейса окна
class CWindow : public IWindow
{
private:

	// Хендл приложения
	HINSTANCE hInstance;

	// Хэндл окна
	HWND hWindow;

public:

	// Конструктор
	CWindow(void);

	// Деструктор
	~CWindow(void);

	// Инициализирует окно
	bool Initialize(void);

	// Уничтожает экземпляр окна
	void Release(void);

	// Обрабатывает сообщения
	bool Update(void);

	// Возвращает хендл окна
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