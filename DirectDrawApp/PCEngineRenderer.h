#pragma once
#include "IEngineRenderer.h"

enum RendererState
{
	RS_INIT,
	RS_IDLE,
	RS_REQUEST_NEW_FRAME,
	RS_ReSetup,
	RS_SHUTINGDOWN
};

enum PCEngineMode {
	PC_MODE_STANDART,
	PC_MODE_BLOCK10,
	PC_MODE_BLOCK6
};


class PCEngineRenderer :
	public IEngineRenderer
{
public:
	PCEngineRenderer(int width, int height, PCEngineMode mode = PCEngineMode::PC_MODE_STANDART);
	PCEngineRenderer();


	virtual ~PCEngineRenderer();
	virtual bool Render(RenderArgs* args);
	virtual pFrame GetRenderFrame();

	bool mutex;

	//Thread Entry Point, DO NOT CALL EXTERNAL! : Не вызывайте эти методы где либо еще!
	DWORD MainThread(LPVOID param);
	DWORD SatelliteThread(LPVOID param);

    const RendererState GetRendererState();
	virtual void SettingsChanged(struct GlobalSettings NewSettings) override;

	virtual RendererStatus RenderInit(enum PresentMethod method, class DrawEngine* presenter) override;

private:
	//Сколько используются ядер (потоков)
	int UsedCores;
	RenderSatelliteInfo* m_RSI;
	//Текущее состояние
	RendererState CurrentState;
    CRITICAL_SECTION CurrentStateGuard;
	//Режим рендера
	PCEngineMode mode;
    RenderArgs LastArgs;

	//живых потоков
	DWORD aliveCores;
	//Utility
	void Init(int width, int height, PCEngineMode mode);
	void DeInit();
	int GetProcessorCoresCount();

    void SetRenderState (const RendererState NewState);

	void BlitToHardware();
	//DebugColorMethod
	Color ClearYellow(const int x, const int y);

    void MoveRenderArgs(RenderArgs* pNewArgs);

	//Sync stuff
	HANDLE Event_Render;
	HANDLE Event_RenderFinished;
	volatile DWORD ReadySignal;

	//RenderFrames and chunks
	class GPUImage* HardwareFrame;
	pFrame RenderFrame;
};