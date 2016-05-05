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
	~PCEngineRenderer();
	virtual void Render(RenderArgs* args);
	virtual pFrame GetRenderFrame();

	int GetUsedCores();
	bool mutex;

	//Thread Entry Point, DO NOT CALL EXTERNAL! : Не вызывайте эти методы где либо еще!
	DWORD MainThread(LPVOID param);
	DWORD SatelliteThread(LPVOID param);

private:
	//true если вызван Render и мы уже рендерим
	bool QuequeFrame;
	//Сколько используются ядер (потоков)
	int UsedCores;
	RenderSatelliteInfo* m_RSI;
	//Текущеё состояние
	RendererState CurrentState;
	//Режим рендера
	PCEngineMode mode;
	void* LastArgs;

	//живых потоков
	DWORD aliveCores;
	//Utility
	int GetProcessorCoresCount();

	void BlitToHardware();
	//DebugColorMethod
	Color ClearYellow(const int x, const int y);


	//Sync stuff
	HANDLE Event_Render;
	HANDLE Event_RenderFinished;
	volatile DWORD ReadySignal;

	//RenderFrames and chunks
	class GPUImage* HardwareFrame;
	pFrame RenderFrame;
};