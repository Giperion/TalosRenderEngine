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

Color MandelbrotSetOLD(const int x, const int y, const void* args)
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


FColor HSV2RGB(double Hue, double Saturation, double Value)
{
    double      hh, p, q, t, ff;
    long        i;
    FColor        out;
    out.A = 1.0;

    if (Saturation <= 0.0) {       // < is bogus, just shuts up warnings
        out.R = Value;
        out.G = Value;
        out.B = Value;
        return out;
    }
    hh = Hue;
    if (hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = Value * (1.0 - Saturation);
    q = Value * (1.0 - (Saturation * ff));
    t = Value * (1.0 - (Saturation * (1.0 - ff)));

    switch (i) {
    case 0:
        out.R = Value;
        out.G = t;
        out.B = p;
        break;
    case 1:
        out.R = q;
        out.G = Value;
        out.B = p;
        break;
    case 2:
        out.R = p;
        out.G = Value;
        out.B = t;
        break;

    case 3:
        out.R = p;
        out.G = q;
        out.B = Value;
        break;
    case 4:
        out.R = t;
        out.G = p;
        out.B = Value;
        break;
    case 5:
    default:
        out.R = Value;
        out.G = p;
        out.B = q;
        break;
    }
    return out;
}

Color FColor2Color(FColor inColor)
{
    Color Out;

    Out.R = Lerp(0, 255, inColor.R);
    Out.G = Lerp(0, 255, inColor.G);
    Out.B = Lerp(0, 255, inColor.B);
    Out.A = Lerp(0, 255, inColor.A);

    return Out;
}

Color MandelbrotSet(const int x, const int y, const int width, const int height, const void* args)
{
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


    double PixelWidth = (MaximumResultX - MinimumResultX) / width;
    double PixelHeight = (MaximumResultY - MinimumResultY) / height;


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

    double NewValue = 1.0 * (Iteration % mView->iteration);
    FColor HDRColor = HSV2RGB(Iteration % 361, 0.01, NewValue);
    return FColor2Color(HDRColor);
}


DWORD WINAPI EngineSatellite(LPVOID param)
{
    EngineThreadInitInfo* eti = reinterpret_cast<EngineThreadInitInfo*> (param);
    PCEngineRenderer* pEngine = reinterpret_cast<PCEngineRenderer*> (eti->pEngine);
	return pEngine->SatelliteThread(eti->param);
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
	: PCEngineRenderer()
{
	Init(width, height, mode);
}

PCEngineRenderer::PCEngineRenderer()
{
	CurrentState = RS_INIT;
	aliveCores = 0;
	Event_Render = CreateEvent(NULL, TRUE, FALSE, L"Giperion_DrawEngine_Render");
	Event_RenderFinished = CreateEvent(NULL, TRUE, FALSE, L"Giperion_DrawEngine_RenderFinished");
    InitializeCriticalSection(&CurrentStateGuard);

	//Maybe control how many cores we needed to use??
	UsedCores = GetProcessorCoresCount();
}

void PCEngineRenderer::Init(int width, int height, PCEngineMode mode)
{
	this->mode = mode;
	this->width = width;
	this->height = height;
	HardwareFrame = new GPUImage(width, height, GPUImgType::IT_OpenGL);

	EngineThreadInitInfo* eti = new EngineThreadInitInfo;
	eti->pEngine = this;
	eti->param = 0;
	//Shall i use init in constructor? : Yes
    SetRenderState(RS_INIT);
	CreateThread(NULL, 0, RenderThread, eti, 0, NULL);
}

void PCEngineRenderer::DeInit()
{
	SetRenderState(RS_SHUTINGDOWN);
	while (mutex) Sleep(5);
	delete HardwareFrame; HardwareFrame = nullptr;
}

bool PCEngineRenderer::Render(RenderArgs* args)
{
	if (GetRendererState() == RS_IDLE)
	{
        MoveRenderArgs(args);
		SetRenderState(RS_REQUEST_NEW_FRAME);
	}
	else
	{
		return false;
	}

    return true;
}

int PCEngineRenderer::GetProcessorCoresCount()
{
	SYSTEM_INFO sysInfo;
	ZeroMemory(&sysInfo, sizeof(SYSTEM_INFO));
	GetNativeSystemInfo(&sysInfo);
	return sysInfo.dwNumberOfProcessors;
}

void PCEngineRenderer::SetRenderState(const RendererState NewState)
{
    EnterCriticalSection(&CurrentStateGuard);
    CurrentState = NewState;
    LeaveCriticalSection(&CurrentStateGuard);
}

void PCEngineRenderer::BlitToHardware()
{
	pFrame HardwareFramePtr = (pFrame) HardwareFrame->Bind();
	MemoryBarrier();
	memcpy(HardwareFramePtr, RenderFrame, (width * height) * sizeof(Color));
	MemoryBarrier();
	HardwareFrame->UnBind();
}

DWORD PCEngineRenderer::SatelliteThread(LPVOID param)
{
	DWORD coreID = reinterpret_cast<DWORD>(param);
	RenderSatelliteInfo blockInfo = m_RSI[coreID];

	byte* m_FrameChunk = RenderFrame + ((width * sizeof(Color)) * blockInfo.StartY);

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

	if (GetRendererState() == RS_SHUTINGDOWN) goto end;

	Color* FramePixel = reinterpret_cast <Color*> (m_FrameChunk);

	for (CurrentY = blockInfo.StartY; CurrentY != EndY; CurrentY++)
	{
		for (CurrentX = 0; CurrentX != EndX; CurrentX++)
		{
			//Add you Method here!
			Color CurrentColor = MandelbrotSet(CurrentX, CurrentY, width, height, LastArgs.args);
			*FramePixel = CurrentColor;
			FramePixel++;
		}

        //Check, if we need to shutdown?
        if (GetRendererState() == RS_SHUTINGDOWN)
        {
            goto end;
        }

	}

	CurrentByte = 0;
	CurrentX = 0;
	CurrentY = blockInfo.StartY;

	if (ReadySignal >= aliveCores) DebugBreak();

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

const RendererState PCEngineRenderer::GetRendererState()
{
    RendererState RetValue = RendererState::RS_ReSetup;
    EnterCriticalSection(&CurrentStateGuard);
    RetValue = CurrentState;
    LeaveCriticalSection(&CurrentStateGuard);

    return RetValue;
}

void PCEngineRenderer::SettingsChanged(struct GlobalSettings NewSettings)
{
	DeInit();
	Init(NewSettings.Width, NewSettings.Height, PC_MODE_STANDART);
}

RendererStatus PCEngineRenderer::RenderInit(enum PresentMethod method, class DrawEngine* presenter)
{
	if (GetRendererState() != RS_INIT) return RSTATUS_OK;

	GlobalSettings Settings = presenter->GetGlobalSettings();

	Init(Settings.Width, Settings.Height, PC_MODE_STANDART);
	return RSTATUS_OK;
}

Color PCEngineRenderer::ClearYellow(const int x, const int y)
{
	Color result;
	result.R = 250;
	result.G = 20;
	result.B = 20;
	return result;
}

void PCEngineRenderer::MoveRenderArgs(RenderArgs* pNewArgs)
{
    delete LastArgs.args;

    LastArgs.size = pNewArgs->size;
    LastArgs.args = new byte[LastArgs.size];
    memcpy (LastArgs.args, pNewArgs->args, LastArgs.size);
}

DWORD PCEngineRenderer::MainThread(LPVOID param)
{
	//Init MainFrame
	RenderFrame = new byte[(width * height) * sizeof(Color)];
    BOOL EventDebugResult = FALSE;

	mutex = true;
	ReadySignal = 0;
    aliveCores = 0;

    EventDebugResult = ResetEvent(Event_Render);
    EventDebugResult = SetEvent(Event_RenderFinished);

	//Init satellites
	RenderSatelliteInfo initStr;
	m_RSI = new RenderSatelliteInfo[UsedCores];
	EngineThreadInitInfo eti;
	eti.pEngine = this;

	int BlockWidth = width;
	int BlockHeight = height / UsedCores;

	initStr.width = BlockWidth;
	initStr.height = BlockHeight;
	initStr.unused = 0;

	int BlockY = 0;

	for (int core = 0; core < UsedCores; core++)
	{
		initStr.StartY = BlockY;
		initStr.ID = core;

		memcpy(&m_RSI[core], &initStr, sizeof(RenderSatelliteInfo));

		eti.param = (LPVOID)core;
		EngineThreadInitInfo* newEti = new EngineThreadInitInfo;
		memcpy(newEti, &eti, sizeof(EngineThreadInitInfo));

		HANDLE NewThreadHandle = CreateThread(NULL, NULL, EngineSatellite, newEti, NULL, NULL);
        if (NewThreadHandle == NULL) DebugBreak();
		BlockY += BlockHeight;
	}

	SetRenderState(RS_IDLE);
#pragma region renderProcess
loopRender:
    RendererState RState = GetRendererState();
	while (RState != RS_REQUEST_NEW_FRAME && RState != RS_SHUTINGDOWN) 
    {
        Sleep(5);
        RState = GetRendererState();
    }
    RState = GetRendererState();
	if (RState == RS_SHUTINGDOWN)
	{
        EventDebugResult = SetEvent(Event_Render);
        EventDebugResult = SetEvent(Event_RenderFinished);
		goto endloop;
	}

	//QueryPerformanceCounter(&startTime);
    EventDebugResult = ResetEvent(Event_RenderFinished);
    EventDebugResult = SetEvent(Event_Render);
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
	if (GetRendererState() == RS_SHUTINGDOWN) goto endloop;
	ResetEvent(Event_Render);
	SetRenderState(RS_IDLE);
	
	InterlockedExchange (&ReadySignal, 0);
	SetEvent(Event_RenderFinished);

	goto loopRender;
#pragma endregion

endloop:
	//Wait all satellites
	if (aliveCores != 0) Sleep(5);
	delete[] m_RSI;
	mutex = false;
	delete[] RenderFrame;
	return 0;

}

pFrame PCEngineRenderer::GetRenderFrame()
{
    RendererState RState = GetRendererState();
	if (RState == RS_SHUTINGDOWN) return nullptr;
	if (RState != RendererState::RS_IDLE)
	{
		WaitForSingleObject(Event_RenderFinished, INFINITE);
	}

	//perform blt operation
	BlitToHardware();

	return (pFrame)HardwareFrame->GetGLHandle();
}

PCEngineRenderer::~PCEngineRenderer()
{
	DeInit();
    CloseHandle(Event_Render); Event_Render = NULL;
    CloseHandle(Event_RenderFinished); Event_RenderFinished = NULL;
    DeleteCriticalSection(&CurrentStateGuard);
}
