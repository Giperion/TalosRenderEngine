#include "stdafx.h"
#include <math.h>


DrawEngine::DrawEngine(HWND hWnd, PresentMethod PMethod)
{
	AttachedHWND = hWnd;

	presentMethod = PMethod;

	CurrentState = DrawEngineState::DES_INIT;

	switch (presentMethod)
	{
	case PM_DirectDraw:
		if (!InitDirectDraw())
		{
			MessageBox(AttachedHWND, L"DirectDraw init error!", L"Critical error", MB_OK | MB_ICONERROR);
		}
		break;
	case PM_GDI:
		MessageBox(AttachedHWND, L"GDI present method not implemented!", L"Error", MB_OK | MB_ICONERROR);
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		break;
	case PM_OpenGL:
		MessageBox(AttachedHWND, L"OpenGL present method not implemented!", L"Error", MB_OK | MB_ICONERROR);
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		break;
	default:
		MessageBox(AttachedHWND, L"Undefined present method is choosen!", L"Critical Error", MB_OK | MB_ICONERROR);
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		break;
	}
	CurrentState = DrawEngineState::DES_IDLE;
}

bool DrawEngine::InitDirectDraw()
{
	HRESULT FuncResult;

	FuncResult = DirectDrawCreate(NULL, &pDirectDraw, NULL);
	if (FuncResult != DD_OK)
	{
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		return false;
	}
	FuncResult = pDirectDraw->SetCooperativeLevel(AttachedHWND, DDSCL_NORMAL);
	if (FuncResult != DD_OK)
	{
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		pDirectDraw->Release();
		return false;
	}

	memset(&DirectSurfaceDesc, 0, sizeof(DDSURFACEDESC));
	DirectSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
	DirectSurfaceDesc.dwFlags = DDSD_CAPS;
	DirectSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	FuncResult = pDirectDraw->CreateSurface(&DirectSurfaceDesc, &pPrimarySurface, NULL);
	if (FuncResult != DD_OK)
	{
		pDirectDraw->Release();
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		return false;
	}

	FuncResult = pDirectDraw->CreateClipper(NULL, &pPrimaryClipper, NULL);
	if (FuncResult != DD_OK)
	{
		pPrimarySurface->Release();
		pDirectDraw->Release();
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		return false;
	}

	FuncResult = pPrimaryClipper->SetHWnd(NULL, AttachedHWND);
	if (FuncResult != DD_OK)
	{
		pPrimaryClipper->Release();
		pPrimarySurface->Release();
		pDirectDraw->Release();
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		return false;
	}

	FuncResult = pPrimarySurface->SetClipper(pPrimaryClipper);
	if (FuncResult != DD_OK)
	{
		pPrimaryClipper->Release();
		pPrimarySurface->Release();
		pDirectDraw->Release();
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		return false;
	}

	memset(&DirectSurfaceDesc, 0, sizeof(DDSURFACEDESC));
	DirectSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
	DirectSurfaceDesc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	DirectSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	DirectSurfaceDesc.dwWidth = ENGINEWIDTH;
	DirectSurfaceDesc.dwHeight = ENGINEHEIGHT;

	FuncResult = pDirectDraw->CreateSurface(&DirectSurfaceDesc, &pBackSurface, NULL);
	if (FuncResult != DD_OK)
	{
		pPrimaryClipper->Release();
		pPrimarySurface->Release();
		pDirectDraw->Release();
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		return false;
	}
	return true;
}

bool DrawEngine::InitOpenGL()
{
	gl = OpenGL::GetInstance();
	if (!gl->CreateCompatableOpenGLHandle(AttachedHWND))
	{
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		return false;
	}
	if (!gl->CreateOpenGLHandle(AttachedHWND))
	{
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		return false;
	}
	

}

DrawEngine::~DrawEngine()
{
	int Timer = 0;
	while (CurrentState != DrawEngineState::DES_IDLE)
	{
		//Хьюстон, у нас тут рендеринг до сих пор.
		Sleep(5);
		Timer++;
		if (Timer > 120)
		{
			LPWSTR Buffer = new WCHAR[512];
			StringCbPrintf(Buffer, 512, L"Render engine can't deinitialize! Current state: %d", CurrentState);
			MessageBox(NULL, Buffer, L"Critical Error", MB_OK || MB_ICONERROR);
			delete[] Buffer;
		}
	}
	InterlockedExchange((unsigned int*)&CurrentState, (unsigned int)DrawEngineState::DES_SHUTINGDOWN);
	for (int i = renderers.size(); i > 0; i--)
	{
		int real = i - 1;
		delete renderers[real];
		PopRenderer();
	}

	switch (presentMethod)
	{
	case PM_DirectDraw:
		ShutDownDirectDraw();
		break;
	case PM_GDI:
		MessageBox(AttachedHWND, L"GDI present method can't be deinitialized!", L"Critical Error", MB_OK | MB_ICONERROR);
		break;
	case PM_OpenGL:
		MessageBox(AttachedHWND, L"OpenGL present method can't be deinitialized!", L"Critical Error", MB_OK | MB_ICONERROR);
		break;
	default:
		MessageBox(AttachedHWND, L"Undefined deinit method called!", L"Critical Error", MB_OK | MB_ICONERROR);
		break;
	}
}

void DrawEngine::ShutDownDirectDraw()
{
	pPrimaryClipper->Release();
	pBackSurface->Release();
	pPrimarySurface->Release();
	pDirectDraw->Release();

	pPrimaryClipper = nullptr;
	pBackSurface = nullptr;
	pPrimarySurface = nullptr;
	pDirectDraw = nullptr;
}


EXPERIMENTAL void DrawEngine::DrawTest()
{
	memset(&DirectSurfaceDesc, 0, sizeof (DDSURFACEDESC));
	DirectSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
	DirectSurfaceDesc.dwFlags = DDSD_CAPS;
	DirectSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	pPrimarySurface->Lock(NULL, &DirectSurfaceDesc, DDLOCK_WAIT, NULL);
	memset(DirectSurfaceDesc.lpSurface, 0, DirectSurfaceDesc.lPitch * DirectSurfaceDesc.dwHeight);
	pPrimarySurface->Unlock(DirectSurfaceDesc.lpSurface);
}

DWORD WINAPI DrawEngine::ThreadEntryPoint(DWORD param)
{
	return 3;
}

RenderMethod DrawEngine::GetCurrentRenderMethod()
{
	return renderMethod;
}

double DrawEngine::GetLastRenderTime()
{
	return LastRenderTime;
}

void NYI DrawEngine::SetRenderMethod(RenderMethod newMethod)
{
	
}

PresentMethod DrawEngine::GetCurrentPresentMethod()
{
	return presentMethod;
}

void NYI DrawEngine::SetPresentMethod(PresentMethod newPresentMethod)
{

}

void DrawEngine::PushRenderer(IEngineRenderer* iRenderer)
{
	if (CurrentState != DES_IDLE)
	{
		MessageBox(AttachedHWND, L"Trying add new render while DrawEngine not initialized", L"Critical Error", MB_OK | MB_ICONERROR);
	}
	else
	{
		renderers.push_back(iRenderer);
	}
}

void DrawEngine::PopRenderer()
{
	if (renderers.size() > 0)
	{
		renderers.pop_back();
	}
}

//TODO a normal blitter
bool DrawEngine::Render()
{
	RECT rcSource;
	RECT rcDest;
	POINT trWindow;

	if (!(renderers.size() > 0))
	{
		MessageBox(AttachedHWND, L"Render call without renderers", L"Critical Error", MB_OK | MB_ICONERROR);
		return false;
	}

	if (CurrentState == DrawEngineState::DES_IDLE)
	{
		CurrentState = DrawEngineState::DES_REQUEST_NEW_FRAME;
		for (int i = 0; i < renderers.size(); i++)
		{
			renderers[i]->Render();
		}
		for (int i = 0; i < renderers.size(); i++)
		{
			switch (presentMethod)
			{
			case PM_DirectDraw:
				pFrame newFrame = renderers[i]->GetRenderFrame();
				pBackSurface->Lock(NULL, &DirectSurfaceDesc, DDLOCK_WAIT, NULL);
				int Allbytes = DirectSurfaceDesc.lPitch * DirectSurfaceDesc.dwHeight;
				unsigned char* VideoMemory = static_cast <unsigned char*> (DirectSurfaceDesc.lpSurface);
				memcpy(VideoMemory, newFrame, Allbytes);
				pBackSurface->Unlock(DirectSurfaceDesc.lpSurface);
				break;
			case PM_GDI:
				break;
			case PM_OpenGL:

				break;
			default:
				break;
			}


		}
		trWindow.x = 0; trWindow.y = 0;
		ClientToScreen(AttachedHWND, &trWindow);
		GetClientRect(AttachedHWND, &rcDest);
		OffsetRect(&rcDest, trWindow.x, trWindow.y);
		SetRect(&rcSource, 0, 0, ENGINEWIDTH, ENGINEHEIGHT);
		pPrimarySurface->Blt(&rcDest, pBackSurface, &rcSource, DDBLT_WAIT, NULL);
		CurrentState = DrawEngineState::DES_IDLE;
		return true;
	}
	return false;
}

void DrawEngine::PostRender(double RenderTime)
{
	InvalidateRect(AttachedHWND, NULL, FALSE);
	LastRenderTime = RenderTime;
}

inline int Lerp(int start, int end, double value)
{
	return start + (end - start) * value;
}

Color DrawEngine::MandelbrotSet(const int x, const int y)
{
	Color result;

	l_long ResultX;
	l_long ResultY;

	l_long Zx = 0;
	l_long Zy = 0;
	l_long Zx_x2 = 0;
	l_long Zy_x2 = 0;

	const l_long MinimumResultX = -2.5L;
	const l_long MaximumResultX = 1.5L;
	const l_long MinimumResultY = -2.0L;
	const l_long MaximumResultY = 2.0L;

	l_long PixelWidth = (MaximumResultX - MinimumResultX) / ENGINEWIDTH;
	l_long PixelHeight = (MaximumResultY - MinimumResultY) / ENGINEHEIGHT;

	int Iteration = 0;

	const l_long EscapeRadius = 2.0L;
	l_long EscapeRadius_x2 = EscapeRadius * EscapeRadius;

	ResultX = (MinimumResultX + PixelWidth * x) * 1.0;
	ResultY = (MinimumResultY + PixelHeight * y) * 1.0;

	if (fabs(ResultY) < PixelHeight / 2) ResultY = 0.0;

	for (; Iteration < 20 && ((Zx_x2 + Zy_x2) < EscapeRadius_x2); Iteration++)
	{
		Zy = 2 * Zx * Zy + ResultY;
		Zx = Zx_x2 - Zy_x2 + ResultX;
		Zx_x2 = Zx * Zx;
		Zy_x2 = Zy * Zy;
	}

	double Value = (1.0 / (double)20) *  Iteration;
	//Раньше возвращали цвет по таблице, но увы, это не эффективный способ
	//return ResoulveColor(Iteration);
	int grayscaleComp = Lerp(0, 255, Value);
	
	result.R = grayscaleComp; result.G = grayscaleComp; result.B = grayscaleComp;
	return result;
}
