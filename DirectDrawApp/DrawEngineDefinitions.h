#pragma once
#include <windows.h>

enum RenderMethod : byte
{
	MandelBrot,
	SolidColor,
	Custom
};

enum DrawEngineState : int
{
	DES_INIT,
	DES_IDLE,
	DES_PRESENTERFAILED,
	DES_RENDER_IN_PROGRESS,
	DES_RENDER_FINISHED,
	DES_REQUEST_NEW_FRAME,
	DES_SHUTINGDOWN
};

enum PresentMethod : byte
{
	PM_DirectDraw,
	PM_GDI,
	PM_OpenGL
};

namespace EngineType
{
	enum Value
	{
		Unknown,
		Native,
		CUDA,
		OpenCL,
		OpenGL
	};

	__forceinline LPCWSTR ToString(EngineType::Value InValue)
	{
		switch (InValue)
		{
		case Native:
			return L"Native";
		case CUDA:
			return L"CUDA";
		case OpenCL:
			return L"OpenCL";
		case OpenGL:
			return L"OpenGL";
		case Unknown:
		default:
			return L"Unknown";
		}
	}

	__forceinline EngineType::Value FromString(LPCWSTR InStr)
	{
		int CmpResult = 0;
		CmpResult = wcscmp(InStr, L"Native");
		if (CmpResult == 0) return Native;

		CmpResult = wcscmp(InStr, L"CUDA");
		if (CmpResult == 0) return CUDA;

		CmpResult = wcscmp(InStr, L"OpenCL");
		if (CmpResult == 0) return OpenCL;

		CmpResult = wcscmp(InStr, L"OpenGL");
		if (CmpResult == 0) return OpenGL;

		return Unknown;
	}

}

enum WindowType
{
	WT_Graphic,
	WT_Console
};


