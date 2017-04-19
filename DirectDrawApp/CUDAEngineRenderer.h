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
	virtual ~CUDAEngineRenderer();

	virtual pFrame GetRenderFrame();
	virtual bool Render(RenderArgs* args);

	virtual void SettingsChanged(struct GlobalSettings NewSettings) override;

	virtual RendererStatus RenderInit(enum PresentMethod method, class DrawEngine* presenter) override;

private:
	GPUImage* mainFrame;
};

