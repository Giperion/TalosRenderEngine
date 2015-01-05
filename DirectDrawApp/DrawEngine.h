#ifndef __DrawEngine
#define __DrawEngine
#include <Windows.h>
#include <ddraw.h>
#include "PrivateMacro.h"

DWORD WINAPI RenderThread(LPVOID param);
DWORD WINAPI EngineSatellite(LPVOID param);

enum RenderMethod : byte
{
	MandelBrot,
	SolidColor,
	Custom
};

enum PresentMethod : byte
{
	DirectDraw,
	GDI,
	DirectX
};

struct Color
{
	byte R;
	byte G;
	byte B;
};
class DrawEngine
{
public:
	char CurrentState;
	DrawEngine(HWND hWnd, PresentMethod PMethod = DirectDraw);

	EXPERIMENTAL void DrawTest();
	EXPERIMENTAL DWORD WINAPI ThreadEntryPoint(DWORD param);
	
	//RenderEngine command
	bool Render();
	void PostRender(double RenderTime);
	double GetLastRenderTime();

	bool NYI SetScreenBufferSize(int width, int height);

	//GetSetMethods
	RenderMethod GetCurrentRenderMethod();
	void NYI SetRenderMethod(RenderMethod newMethod);
	PresentMethod GetCurrentPresentMethod();
	void NYI SetPresentMethod(PresentMethod newPresentMethod);
	void SetCurrentMethod(RenderMethod method);

	void SetCustomRenderMethod(void* _fastcall renderMethod);

	//Color methods, must be moved
	Color MandelbrotSet(const int x, const int y);
	~DrawEngine();

	//Windows and screen buffer
	HWND AttachedHWND;
	byte** m_FrameChunks;
	

	//DirectDraw
	LPDIRECTDRAW pDirectDraw;

	LPDIRECTDRAWSURFACE pPrimarySurface;
	LPDIRECTDRAWSURFACE pBackSurface;

	LPDIRECTDRAWCLIPPER pPrimaryClipper;

	DDSURFACEDESC DirectSurfaceDesc;

	//Sync
	DWORD RenderStatus;
	HANDLE Event_Rendering;
	HANDLE Event_RenderFinished;
private:
	double LastRenderTime;
	RenderMethod renderMethod;
	PresentMethod presentMethod;



	//InitPresenters
	bool InitDirectDraw();

	//DeInitPresenters

	void ShutDownDirectDraw();
};

struct WindowParam
{
	int width;
	int height;
	int x;
	int y;
};

struct RenderSatelliteInfo
{
	int width;
	int height;
	int StartX;
	int StartY;

	int ID;
	DrawEngine* ourEngine;

	//UNREFERENCE, for future
	RenderMethod Method;
};

#endif