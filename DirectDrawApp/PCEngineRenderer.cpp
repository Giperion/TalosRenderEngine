#include "stdafx.h"
#include "PCEngineRenderer.h"
#include "GPUImage.h"

struct EngineThreadInitInfo
{
	LPVOID pEngine;
	LPVOID param;
};

inline int Lerp(int start, int end, double value)
{
	return start + (end - start) * value;
}

Color MandelbrotSet(const int x, const int y, const void* args)
{
	Color target;

	MandelbrotView* mView = (MandelbrotView*)args;
	double Scale = mView->scale;

	double centerX = -0.5;
	double centerY = 0.5;

	double ResultX;
	double ResultY;

	double Zx = 0;
	double Zy = 0;
	double Zx_x2 = 0;
	double Zy_x2 = 0;

	const double MinimumResultX = centerX - Scale;
	const double MaximumResultX = centerX + Scale;
	const double MinimumResultY = centerY - Scale;
	const double MaximumResultY = centerY + Scale;


	double PixelWidth = (MaximumResultX - MinimumResultX) / ENGINEWIDTH;
	double PixelHeight = (MaximumResultY - MinimumResultY) / ENGINEHEIGHT;


	int Iteration = 0;
	int MaxIteration = mView->iteration;

	const double EscapeRadius = 2.0L;
	double EscapeRadius_x2 = EscapeRadius * EscapeRadius;

	ResultX = (MinimumResultX + PixelWidth * x) + mView->x;
	ResultY = (MinimumResultY + PixelHeight * y) + mView->y;

	if (fabs(ResultY) < PixelHeight / 2) ResultY = 0.0;

	for (; Iteration < MaxIteration && ((Zx_x2 + Zy_x2) < EscapeRadius_x2); Iteration++)
	{
		Zy = 2 * Zx * Zy + ResultY;
		Zx = Zx_x2 - Zy_x2 + ResultX;
		Zx_x2 = Zx * Zx;
		Zy_x2 = Zy * Zy;
	}

	double Value = (1.0 / (double)80) *  Iteration;
	//Ðàíüøå âîçâðàùàëè öâåò ïî òàáëèöå, íî óâû, ýòî íå ýôôåêòèâíûé ñïîñîá
	//return ResoulveColor(Iteration);
	int grayscaleComp = Lerp(0, 255, Value);

	target.R = grayscaleComp; target.G = grayscaleComp; target.B = grayscaleComp;
	target.A = 255;
	return target;
}


DWORD WINAPI EngineSatellite(LPVOID param)
{
	EngineThreadInitInfo* eti = reinterpret_cast<EngineThreadInitInfo*> (param);
	PCEngineRenderer* pEngine = reinterpret_cast<PCEngineRenderer*> (eti->pEngine);
	LPVOID parameter = eti->param;
	delete eti;
	return pEngine->SatelliteThread(parameter);
}

DWORD WINAPI RenderThread(LPVOID param)
{
	EngineThreadInitInfo* eti = reinterpret_cast<EngineThreadInitInfo*> (param);
	PCEngineRenderer* pEngine = reinterpret_cast<PCEngineRenderer*> (eti->pEngine);
	LPVOID parameter = eti->param;
	delete eti;
	return pEngine->MainThread(parameter);
}


PCEngineRenderer::PCEngineRenderer(int width, int height, PCEngineMode mode)
{
	CurrentState = RS_INIT;
	aliveCores = 0;
	this->mode = mode;
	this->width = width;
	this->height = height;
	Event_Render = CreateEvent(NULL, TRUE, FALSE, L"Giperion_DrawEngine_Render");
	Event_RenderFinished = CreateEvent(NULL, TRUE, FALSE, L"Giperion_DrawEngine_RenderFinished");

	//Maybe control how many cores we needed to use??
	UsedCores = GetProcessorCoresCount();

	HardwareFrame = new GPUImage(width, height, GPUImgType::IT_OpenGL);

	EngineThreadInitInfo* eti = new EngineThreadInitInfo;
	eti->pEngine = this;
	eti->param = 0;
	//Shall i use init in constructor? : Yes
	CreateThread(NULL, 0, RenderThread, eti, 0, NULL);
	CurrentState = RS_IDLE;
}

void PCEngineRenderer::Render(RenderArgs* args)
{
	LastArgs = args->args;
	if (CurrentState == RS_IDLE)
	{
		CurrentState = RS_REQUEST_NEW_FRAME;
	}
	else
	{
		QuequeFrame = true;
	}
}

int PCEngineRenderer::GetProcessorCoresCount()
{
	SYSTEM_INFO sysInfo;
	ZeroMemory(&sysInfo, sizeof(SYSTEM_INFO));
	GetNativeSystemInfo(&sysInfo);
	return sysInfo.dwNumberOfProcessors;
}

void PCEngineRenderer::BlitToHardware()
{
	pFrame HardwareFramePtr = (pFrame) HardwareFrame->Bind();
	memcpy(HardwareFramePtr, RenderFrame, (width * height) * 4);
	HardwareFrame->UnBind();
}

DWORD PCEngineRenderer::SatelliteThread(LPVOID param)
{
	DWORD* pCoreID = reinterpret_cast<DWORD*>(param);
	DWORD coreID = *pCoreID;
	pCoreID = nullptr;
	RenderSatelliteInfo blockInfo = m_RSI[coreID];

	byte* m_FrameChunk = RenderFrame + ((width * 4) * blockInfo.StartY);

	int CurrentX = 0;
	int EndX = width;

	int CurrentY = blockInfo.StartY;
	int EndY = blockInfo.StartY + blockInfo.height;

	size_t CurrentByte = 0;

	//We alive
	InterlockedIncrement(&aliveCores);
#pragma region renderProcess
render:
	//Start
	WaitForSingleObject(Event_Render, INFINITE);

	if (CurrentState == RS_SHUTINGDOWN) goto end;

	for (CurrentY = blockInfo.StartY; CurrentY != EndY; CurrentY++)
	{
		for (CurrentX = 0; CurrentX != EndX; CurrentX++)
		{
			//Add you Method here!
			Color CurrentColor = MandelbrotSet(CurrentX, CurrentY, LastArgs);
			//Color CurrentColor = RandomMadness (CurrentX, CurrentY);
			m_FrameChunk[CurrentByte] = CurrentColor.B;
			m_FrameChunk[CurrentByte + 1] = CurrentColor.G;
			m_FrameChunk[CurrentByte + 2] = CurrentColor.R;
			m_FrameChunk[CurrentByte + 3] = CurrentColor.R;
			CurrentByte += 4;

			//Check, if we need to shutdown?
			if (CurrentState == RS_SHUTINGDOWN) goto end;
		}

	}

	CurrentByte = 0;
	CurrentX = 0;
	CurrentY = blockInfo.StartY;
	InterlockedIncrement(&ReadySignal);
	//End
#pragma endregion

	//Ждем пока остальные потоки дорендерят, прежде чем вернемся к началу цикла
	WaitForSingleObject(Event_RenderFinished, INFINITE);
	goto render;

end:
	InterlockedDecrement(&aliveCores);
	return 0;
}

Color PCEngineRenderer::ClearYellow(const int x, const int y)
{
	Color result;
	result.R = 250;
	result.G = 20;
	result.B = 20;
	return result;
}

DWORD PCEngineRenderer::MainThread(LPVOID param)
{
	//Init MainFrame
	RenderFrame = new byte[(width * height) * 4];

	mutex = true;
	ReadySignal = 0;

	//Init satellites
	RenderSatelliteInfo initStr;
	m_RSI = new RenderSatelliteInfo[UsedCores];
	EngineThreadInitInfo eti;
	eti.pEngine = this;

	int BlockWidth = ENGINEWIDTH;
	int BlockHeight = ENGINEHEIGHT / UsedCores;

	initStr.width = BlockWidth;
	initStr.height = BlockHeight;
	initStr.unused = 0;

	int BlockY = 0;

	for (int core = 0; core < UsedCores; core++)
	{
		initStr.StartY = BlockY;
		initStr.ID = core;

		memcpy(&m_RSI[core], &initStr, sizeof(RenderSatelliteInfo));

		DWORD* coreID = new DWORD ();
		memcpy(coreID, &core, sizeof(DWORD));

		eti.param = coreID;
		EngineThreadInitInfo* newEti = new EngineThreadInitInfo;
		memcpy(newEti, &eti, sizeof(EngineThreadInitInfo));

		CreateThread(NULL, NULL, EngineSatellite, newEti, NULL, NULL);
		BlockY += BlockHeight;
	}


#pragma region renderProcess

	//LARGE_INTEGER startTime;
	//LARGE_INTEGER endTime;
	//LARGE_INTEGER frequency;
	//QueryPerformanceFrequency(&frequency);

loopRender:
	while (CurrentState != RS_REQUEST_NEW_FRAME && CurrentState != RS_SHUTINGDOWN) Sleep(5);
	if (CurrentState == RS_SHUTINGDOWN)
	{
		SetEvent(Event_Render);
		goto endloop;
	}

	//QueryPerformanceCounter(&startTime);
	ResetEvent(Event_RenderFinished);
	SetEvent(Event_Render);
	//Not Ready
	int TimerTest = 0;
	Sleep(10);
	while (ReadySignal != UsedCores)
	{
		Sleep(2);
		TimerTest++;
		if (TimerTest > 15000)
		{
			LPWSTR Buffer = new WCHAR[128];
			StringCbPrintf(Buffer, 128, L"RenderTime ends, but a picture doesn't ready!");
			MessageBox(NULL, Buffer, L"Critical Error", MB_OK | MB_ICONERROR);
			delete[] Buffer;
			ReadySignal = UsedCores;
			TimerTest = 0;
		}
	}
	ResetEvent(Event_Render);
	CurrentState = RS_IDLE;
	
	ReadySignal = 0;
	SetEvent(Event_RenderFinished);

	goto loopRender;
#pragma endregion

endloop:
	//Wait all satellites
	if (aliveCores != 0) Sleep(5);
	delete[] m_RSI;
	mutex = false;
	return 0;

}

pFrame PCEngineRenderer::GetRenderFrame()
{
	if (CurrentState != RendererState::RS_IDLE)
	{
		WaitForSingleObject(Event_RenderFinished, INFINITE);
	}

	//perform blt operation
	BlitToHardware();

	return (pFrame)HardwareFrame->GetGLHandle();
}

PCEngineRenderer::~PCEngineRenderer()
{
	CurrentState = RS_SHUTINGDOWN;
	while (mutex) Sleep(5);
	CloseHandle(Event_Render);
	CloseHandle(Event_RenderFinished);
	delete HardwareFrame;
}
