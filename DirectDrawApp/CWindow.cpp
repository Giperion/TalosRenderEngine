#include "CWindow.hpp"
#include "stdafx.h"

//
// Создает новый экземпляр интерфейса к окну и возвращает указатель на него
//
IWindow *CALLTYPE WindowCreate(void)
{
	// Временная переменная
	IWindow *pWindow = nullptr;

	// Пытаемся выделить память
	try { pWindow = new CWindow(); }
	catch(...) { pWindow = nullptr; }

	// Возвращаем результат
	return pWindow;
}

// Обработчик сообщений Windows
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

// Конструктор
CWindow::CWindow(void)
{
	hInstance = NULL;
	hWindow = NULL;
}

// Деструктор
CWindow::~CWindow(void)
{
	hInstance = NULL;
	hWindow = NULL;
}

// Инициализирует окно
bool CWindow::Initialize(void)
{
	return true;
}

// Уничтожает экземпляр окна
void CWindow::Release(void)
{
	delete this;
}

// Обрабатывает сообщения
bool CWindow::Update(void)
{
	return true;
}

// Возвращает хендл окна
HWND CWindow::GetHandle(void)
{
	return (this->hWindow);
}