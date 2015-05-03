#pragma once
#include "IEngineRenderer.h"

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
private:
	void* memAlloc(size_t size);
};