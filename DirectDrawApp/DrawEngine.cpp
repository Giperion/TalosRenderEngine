#include "stdafx.h"
#include <math.h>

extern volatile unsigned int RenderCores;
Color RandomMadness(const int x, const int y);
/*
Рендер поток in nutshell
Делает:
	а) Инициализация блока данных и установка переменных (буфер изображения, место и размер изображения)
	б) Рендер
*/
DWORD WINAPI EngineSatellite(LPVOID param)
{
	RenderSatelliteInfo* p_blockinfo = reinterpret_cast <RenderSatelliteInfo*> (param);
	RenderSatelliteInfo blockInfo = *p_blockinfo;

	if (p_blockinfo == nullptr)
	{
		MessageBox(NULL, L"Attempt to initialize satellite with null param", L"Critical Error", MB_OK | MB_ICONERROR);
	}

	byte* m_FrameChunk = new byte[(blockInfo.width * blockInfo.height) * 4];
	blockInfo.ourEngine->m_FrameChunks[blockInfo.ID] = m_FrameChunk;

	int CurrentX = blockInfo.StartX;
	int EndX = blockInfo.StartX + blockInfo.width;

	int CurrentY = blockInfo.StartY;
	int EndY = blockInfo.StartY + blockInfo.height;

	int DebugRenderTimes = 0;

	size_t CurrentByte = 0;
#pragma region renderProcess
render:
	//Start
	WaitForSingleObject(blockInfo.ourEngine->Event_Rendering, INFINITE);

	if (blockInfo.ourEngine->CurrentState == STATE_SHUTINGDOWN) goto end;

	for (CurrentY = blockInfo.StartY; CurrentY != EndY; CurrentY++)
	{
		for (CurrentX = blockInfo.StartX; CurrentX != EndX; CurrentX++)
		{
			//Add you Method here!
			Color CurrentColor = blockInfo.ourEngine->MandelbrotSet(CurrentX, CurrentY);
			//Color CurrentColor = RandomMadness (CurrentX, CurrentY);
			m_FrameChunk[CurrentByte] = CurrentColor.B;
			m_FrameChunk[CurrentByte + 1] = CurrentColor.G;
			m_FrameChunk[CurrentByte + 2] = CurrentColor.R;
			m_FrameChunk[CurrentByte + 3] = CurrentColor.R;
			CurrentByte += 4;
		}

	}

	CurrentByte = 0;
	CurrentX = blockInfo.StartX;
	CurrentY = blockInfo.StartY;
	DebugRenderTimes++;
	InterlockedIncrement(&blockInfo.ourEngine->RenderStatus);
	//End
#pragma endregion
	
	//Ждем пока остальные потоки дорендерят, прежде чем вернемся к началу цикла
	WaitForSingleObject(blockInfo.ourEngine->Event_RenderFinished, INFINITE);
	goto render;
	
end:
	delete[] m_FrameChunk;
	return 0;
}

/*
Мастер поток рендера
Делает:
	а) Создает рендер потоки
	б) Организует запуск рендера и его остановка, а так же вывод графики на экран через p_DrawEngine
	в) Ждет события рендера
*/
DWORD WINAPI RenderThread(LPVOID param)
{
	DrawEngine* p_DrawEngine = reinterpret_cast <DrawEngine*> (param);
	RenderSatelliteInfo initStr;
	
	int BlockWidth = ENGINEWIDTH;
	int BlockHeight = ENGINEHEIGHT / RenderCores;

	initStr.width = BlockWidth;
	initStr.height = BlockHeight;
	initStr.ourEngine = p_DrawEngine;

	int BlockX = 0;
	int BlockY = 0;

	for (int core = 0; core < RenderCores; core++)
	{
		initStr.StartX = BlockX;
		initStr.StartY = BlockY;
		initStr.ID = core;
		RenderSatelliteInfo* blockinfo = new RenderSatelliteInfo;
		memcpy(blockinfo, &initStr, sizeof(RenderSatelliteInfo));
		CreateThread(NULL, NULL, EngineSatellite, blockinfo, NULL, NULL);
		BlockY += BlockHeight;
	}

	int DebugRenderInvokes = 0;

#pragma region renderProcess
	RECT rcSource;
	RECT rcDest;
	POINT trWindow;

	LARGE_INTEGER startTime;
	LARGE_INTEGER endTime;
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

loopRender:
	while (p_DrawEngine->CurrentState != STATE_REQUEST_NEW_FRAME && p_DrawEngine->CurrentState != STATE_SHUTINGDOWN) Sleep(5);
	if (p_DrawEngine->CurrentState == STATE_SHUTINGDOWN)
	{
		SetEvent(p_DrawEngine->Event_Rendering);
		goto endloop;
	}

	p_DrawEngine->CurrentState = STATE_RENDER_IN_PROGRESS;
	QueryPerformanceCounter(&startTime);

	ResetEvent(p_DrawEngine->Event_RenderFinished);
	SetEvent(p_DrawEngine->Event_Rendering);
	//Not Ready
	int TimerTest = 0;
	Sleep(2);
	while (p_DrawEngine->RenderStatus != RenderCores)
	{
		Sleep(1);
		TimerTest++;
		if (TimerTest > 15000)
		{
			LPWSTR Buffer = new WCHAR[128];
			StringCbPrintf(Buffer, 128, L"RenderTime ends, but a picture doesn't ready!");
			MessageBox(NULL, Buffer, L"Critical Error", MB_OK | MB_ICONERROR);
			delete[] Buffer;
			SetEvent(p_DrawEngine->Event_Rendering);
			TimerTest = 0;
		}
	}
	ResetEvent(p_DrawEngine->Event_Rendering);
	p_DrawEngine->CurrentState = STATE_RENDER_FINISHED;
	p_DrawEngine->RenderStatus = 0;
	SetEvent(p_DrawEngine->Event_RenderFinished);

	p_DrawEngine->pBackSurface->Lock(NULL, &p_DrawEngine->DirectSurfaceDesc, DDLOCK_WAIT, NULL);
	int Allbytes = p_DrawEngine->DirectSurfaceDesc.lPitch * p_DrawEngine->DirectSurfaceDesc.dwHeight;
	unsigned char* VideoMemory = static_cast <unsigned char*> (p_DrawEngine->DirectSurfaceDesc.lpSurface);

	size_t CurrentByte = 0;
	size_t BlockBytes = (BlockWidth * BlockHeight) * 4;

	for (int renderResult = 0; renderResult < RenderCores; renderResult++)
	{
		memcpy(&VideoMemory[CurrentByte], p_DrawEngine->m_FrameChunks[renderResult], BlockBytes);
		
		CurrentByte += BlockBytes;
	}
	//memset(VideoMemory, 129, Allbytes);

	p_DrawEngine->pBackSurface->Unlock(p_DrawEngine->DirectSurfaceDesc.lpSurface);


	trWindow.x = 0; trWindow.y = 0;
	ClientToScreen(p_DrawEngine->AttachedHWND, &trWindow);
	GetClientRect(p_DrawEngine->AttachedHWND, &rcDest);
	OffsetRect(&rcDest, trWindow.x, trWindow.y);
	SetRect(&rcSource, 0, 0, ENGINEWIDTH, ENGINEHEIGHT);
	p_DrawEngine->pPrimarySurface->Blt(&rcDest, p_DrawEngine->pBackSurface, &rcSource, DDBLT_WAIT, NULL);

	QueryPerformanceCounter(&endTime);
	p_DrawEngine->CurrentState = STATE_IDLE;
	DebugRenderInvokes++;
	ResetEvent(p_DrawEngine->Event_RenderFinished);

	long RenderTime = endTime.QuadPart - startTime.QuadPart;
	double RenderTimeMs = (double)RenderTime * 1000 / (double)frequency.QuadPart;
	p_DrawEngine->PostRender(RenderTimeMs);

	goto loopRender;
#pragma endregion


endloop:
	return 0;

}


DrawEngine::DrawEngine(HWND hWnd, PresentMethod PMethod)
{
	AttachedHWND = hWnd;

	CurrentState = 0;
	RenderStatus = 0;

	switch (presentMethod)
	{
	case DirectDraw:
		if (!InitDirectDraw())
		{
			MessageBox(AttachedHWND, L"DirectDraw init error!", L"Critical error", MB_OK | MB_ICONERROR);
		}
		break;
	case GDI:
		MessageBox(AttachedHWND, L"GDI present method not implemented!", L"Error", MB_OK | MB_ICONERROR);
		CurrentState = 1;
		break;
	case DirectX:
		MessageBox(AttachedHWND, L"DirectX present method not implemented!", L"Error", MB_OK | MB_ICONERROR);
		CurrentState = 1;
		break;
	default:
		MessageBox(AttachedHWND, L"Undefined present method is choosen!", L"Critical Error", MB_OK | MB_ICONERROR);
		CurrentState = 1;
		break;
	}


	//Init render events 
	Event_Rendering = CreateEvent(NULL, TRUE, FALSE, L"Giperion_DrawEngine_Render");
	Event_RenderFinished = CreateEvent(NULL, TRUE, FALSE, L"Giperion_DrawEngine_RenderFinished");
#ifdef FIXEDCORE
	m_FrameChunks = new byte* [FIXEDCORE];
	RenderCores = RENDERCORES;
#else
	m_FrameChunks = new byte* [RenderCores];
#endif
}

bool DrawEngine::InitDirectDraw()
{
	HRESULT FuncResult;

	FuncResult = DirectDrawCreate(NULL, &pDirectDraw, NULL);
	if (FuncResult != DD_OK)
	{
		CurrentState = 1;
		return false;
	}
	FuncResult = pDirectDraw->SetCooperativeLevel(AttachedHWND, DDSCL_NORMAL);
	if (FuncResult != DD_OK)
	{
		CurrentState = 2;
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
		CurrentState = 3;
		return false;
	}

	FuncResult = pDirectDraw->CreateClipper(NULL, &pPrimaryClipper, NULL);
	if (FuncResult != DD_OK)
	{
		pPrimarySurface->Release();
		pDirectDraw->Release();
		CurrentState = 4;
		return false;
	}

	FuncResult = pPrimaryClipper->SetHWnd(NULL, AttachedHWND);
	if (FuncResult != DD_OK)
	{
		pPrimaryClipper->Release();
		pPrimarySurface->Release();
		pDirectDraw->Release();
		CurrentState = 5;
		return false;
	}

	FuncResult = pPrimarySurface->SetClipper(pPrimaryClipper);
	if (FuncResult != DD_OK)
	{
		pPrimaryClipper->Release();
		pPrimarySurface->Release();
		pDirectDraw->Release();
		CurrentState = 6;
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
		CurrentState = 7;
		return false;
	}
	return true;
}

DrawEngine::~DrawEngine()
{
	int Timer = 0;
	while (CurrentState != STATE_IDLE)
	{
		//Хьюстон, у нас тут рендеринг досихпор.
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
	InterlockedExchange8(&CurrentState, STATE_SHUTINGDOWN);
	//Мы выключаемся. Все все поняли?
	Sleep(400);

	switch (presentMethod)
	{
	case DirectDraw:
		ShutDownDirectDraw();
		break;
	case GDI:
		MessageBox(AttachedHWND, L"GDI present method can't be deinitialized!", L"Critical Error", MB_OK | MB_ICONERROR);
		break;
	case DirectX:
		MessageBox(AttachedHWND, L"DirectX present method can't be deinitialized!", L"Critical Error", MB_OK | MB_ICONERROR);
		break;
	default:
		MessageBox(AttachedHWND, L"Undefined deinit method called!", L"Critical Error", MB_OK | MB_ICONERROR);
		break;
	}

	CloseHandle(Event_Rendering);
	CloseHandle(Event_RenderFinished);
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

bool DrawEngine::Render()
{
	if (CurrentState == STATE_IDLE)
	{
		CurrentState = STATE_REQUEST_NEW_FRAME;
		return true;
	}
	return false;
}

double DrawEngine::GetLastRenderTime()
{
	return LastRenderTime;
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

Color RandomMadness(const int x, const int y)
{
	Color result;

	int value = rand() % 255;
	result.R = value; result.G = value; result.B = value;
	return result;
}