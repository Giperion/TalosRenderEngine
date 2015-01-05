#include "stdafx.h"
#include "IWindow.hpp"

IWindow *pWindow = nullptr;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//
	// ������� ����
	//

	if((pWindow = WindowCreate()) == nullptr) return 1;

	if(pWindow->Initialize() == false) return 1;

	//
	// �������� ����
	//

	while(pWindow->Update())
	{
	}

	//
	// ������������ ��������
	//

	pWindow->Release();

	//
	// ���������� ���������
	//

	return 0;
}