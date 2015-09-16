#pragma once
#include "IEngineRenderer.h"
#include "GPUImage.h"


//CUDA C API
#include "EngineCuda_C.cuh"

enum CERModes
{
	CER_standart,
	CER_directDrawGL,
	CER_layerBasedGL
};

enum CERStatus
{
	CudaStatus_INIT,
	CUdaStatus_NOCUDA,
	CudaStatus_FAILED,
	CudaStatus_IDLE,

};



class CUDAEngineRenderer :
	public IEngineRenderer
{
public:
	CUDAEngineRenderer();
	~CUDAEngineRenderer();

	//Thread entry point
	DWORD MainThread(LPVOID param);

	virtual pFrame GetRenderFrame();
	virtual void Render(RenderArgs* args);
private:
	GPUImage* mainFrame;

	void* memAlloc(size_t size);
};

