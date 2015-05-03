#ifndef __DrawEngine
#define __DrawEngine
#include <Windows.h>

#include <ddraw.h>
#include "OpenGL.h"

#include "PrivateMacro.h"
#include <deque>
#include "IEngineRenderer.h"

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

enum EngineType : byte
{
	ET_Unknow,
	ET_Native,
	ET_CUDA,
	ET_OpenCL,
	ET_OpenGL
};

enum WindowType
{
	WT_Graphic,
	WT_Console
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
	volatile DrawEngineState CurrentState;
	DrawEngine(HWND hWnd, PresentMethod PMethod = PM_DirectDraw);

	EXPERIMENTAL void DrawTest();
	EXPERIMENTAL DWORD WINAPI ThreadEntryPoint(DWORD param);
	
	//RenderEngine command
	bool Render();
	void PostRender(double RenderTime);

	bool NYI SetScreenBufferSize(int width, int height);

	//GetSetMethods
	double GetLastRenderTime();
	RenderMethod GetCurrentRenderMethod();
	void NYI SetRenderMethod(RenderMethod newMethod);
	PresentMethod GetCurrentPresentMethod();
	void NYI SetPresentMethod(PresentMethod newPresentMethod);
	void SetCurrentMethod(RenderMethod method);


	void PushRenderer(IEngineRenderer* iRenderer);
	void PopRenderer();

	void SetCustomRenderMethod(void* _fastcall renderMethod);

	//Color methods, must be moved
	Color MandelbrotSet(const int x, const int y);


	~DrawEngine();

	//Windows and screen buffer
	HWND AttachedHWND;

	//OpenGL
	OpenGL* gl;
	
	//DirectDraw
	LPDIRECTDRAW pDirectDraw;

	LPDIRECTDRAWSURFACE pPrimarySurface;
	LPDIRECTDRAWSURFACE pBackSurface;

	LPDIRECTDRAWCLIPPER pPrimaryClipper;

	DDSURFACEDESC DirectSurfaceDesc;
private:
	double LastRenderTime;
	RenderMethod renderMethod;
	PresentMethod presentMethod;

	std::deque <IEngineRenderer*> renderers;

	//InitPresenters
	bool InitDirectDraw();
	bool InitOpenGL();

	//DeInitPresenters

	void ShutDownDirectDraw();
	void ShutDownOpenGL();

	//Bliters
	void BltMainBuffer();

	pFrame Blt_DirectDraw();
	pFrame Blt_GDI();
	pFrame Blt_OpenGL();
};

struct WindowParam
{
	int width;
	int height;
	int x;
	int y;
	WindowType type;
};

struct RenderSatelliteInfo
{
	int width;
	int height;
	int StartY;

	int ID;
	LPVOID unused;

	//UNREFERENCE, for future
	RenderMethod Method;
};

#endif