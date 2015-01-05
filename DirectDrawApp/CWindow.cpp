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

//
// Обработчик сообщений Windows
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
// Конструктор
//
CWindow::CWindow(void)
{
	hInstance = NULL;
	hWindow = NULL;
}

//
// Деструктор
//
CWindow::~CWindow(void)
{
	hInstance = NULL;
	hWindow = NULL;
}

//
// Инициализирует окно
//
bool CWindow::Initialize(void)
{
	//
	// Получаем хэндл приложения
	//

	hInstance = GetModuleHandle(NULL);

	//
	// Регистрируем класс окна
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
// Уничтожает экземпляр окна
//
void CWindow::Release(void)
{
	//
	// Освобождаем класс окна
	//

	UnregisterClass(TEXT("MAIN_WINDOW"), hInstance);

	//
	// Удаляем экземпляр
	//

	delete this;
}

//
// Обрабатывает сообщения
//
bool CWindow::Update(void)
{
	//
	// Временная переменная
	//

	MSG msg;

	//
	// Обрабатываем сообщение
	//

	while(PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE) == TRUE)
	{
		//
		// При сообщении выхода - возвращаем false
		//

		if(msg.message == WM_QUIT) return false;

		//
		// Переводим виртуальные клавиши
		//

		TranslateMessage(&msg);

		//
		// Вызываем обработчик
		//

		DispatchMessage(&msg);
	}

	//
	// Продолжаем работу
	//

	return true;
}

//
// Возвращает хендл окна
//
HWND CWindow::GetHandle(void)
{
	return (this->hWindow);
}