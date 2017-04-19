#ifndef __DrawEngine
#define __DrawEngine
#include <Windows.h>

#include <ddraw.h>
#include "OpenGL.h"

#include "PrivateMacro.h"
#include <deque>
#include <vector>
#include "IEngineRenderer.h"
#include "GlobalSettings.h"

class DrawEngine
{
public:
	volatile DrawEngineState CurrentState;
	DrawEngine(HWND hWnd, GlobalSettings InSettings, PresentMethod PMethod = PM_DirectDraw);

	EXPERIMENTAL void DrawTest();
	
	//RenderEngine command
	bool Render(RenderArgs* args);
	void PostRender(double RenderTime);

	void SetGlobalSettings(GlobalSettings& InSettings);

	//GetSetMethods
	double GetLastRenderTime() const;
	RenderMethod GetCurrentRenderMethod();
	PresentMethod GetCurrentPresentMethod();

	const GlobalSettings GetGlobalSettings () const;

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
	GlobalSettings Settings;

	LARGE_INTEGER frequency;

	std::deque <IEngineRenderer*> renderers;

	EngineType::Value GetEngineType(IEngineRenderer* Renderer);
	IEngineRenderer* CreateRenderEngine(EngineType::Value eEngineType) const;

	//InitPresenters
	bool InitDirectDraw();
	bool InitOpenGL();

	//DeInitPresenters

	void ShutDownDirectDraw();
	void ShutDownOpenGL();

	//Bliters
	void BltMainBuffer();

	void Blt_DirectDraw();
	void Blt_GDI();
	void Blt_OpenGL();

	//utility stuff
	UnicodeString GetTextFromStatus(DrawEngineState status);

	GLuint DefaultVertexArrayID;
	GLuint DefaultVertexBufferID;
	GLuint DefaultVertexIndexID;
	GLhandleARB DefaultShaderObject;
	GLCALL bool PreparePlaneScene();
	LPCSTR LoadShaderCodeInternal(LPCWSTR filename);
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