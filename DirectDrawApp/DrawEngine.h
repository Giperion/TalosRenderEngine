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
	DrawEngine(HWND hWnd);
	EXPERIMENTAL void DrawTest();
	EXPERIMENTAL void SetFractalOffset(const l_long offsetX, const l_long offsetY);
	EXPERIMENTAL void SetFractalScale(const l_long newScale);
	EXPERIMENTAL DWORD WINAPI ThreadEntryPoint(DWORD param);
	
	bool Render();
	void PostRender(double RenderTime);
	double GetLastRenderTime();

	//GetSetMethods
	RenderMethod GetCurrentMethod();
	void SetCurrentMethod(RenderMethod method);

	void SetCustomRenderMethod(void* _fastcall renderMethod);

	//Color methods
	Color MandelbrotSet(const int x, const int y);
	int MaxIteration;


	~DrawEngine();


	byte** m_FrameChunks;
	

	HWND AttachedHWND;
	LPDIRECTDRAW pDirectDraw;

	LPDIRECTDRAWSURFACE pPrimarySurface;
	LPDIRECTDRAWSURFACE pBackSurface;

	LPDIRECTDRAWCLIPPER pPrimaryClipper;

	DDSURFACEDESC DirectSurfaceDesc;


	DWORD RenderStatus;
	HANDLE Event_Rendering;
	HANDLE Event_RenderFinished;
private:
	double LastRenderTime;
	RenderMethod currentMethod;

	l_long offsetX;
	l_long offsetY;
	l_long FractalScale;
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