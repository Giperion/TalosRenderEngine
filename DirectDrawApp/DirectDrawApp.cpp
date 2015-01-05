#include "stdafx.h"
#include "IWindow.hpp"

IWindow *pWindow = nullptr;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//
	// Создаем окно
	//

	if((pWindow = WindowCreate()) == nullptr) return 1;

	if(pWindow->Initialize() == false) return 1;

	//
	// Основной цикл
	//

	while(pWindow->Update())
	{
	}

	//
	// Освобождение ресурсов
	//

	pWindow->Release();

	//
	// Возвращаем результат
	//

	return 0;
}