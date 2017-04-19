#include "stdafx.h"
#include "CUDAEngineRenderer.h"

CUDAEngineRenderer::CUDAEngineRenderer()
{
	//Start CUDA parallel system...
	mainFrame = nullptr;
	if (!cuda_init())
	{
		Log::GetInstance()->PrintMsg(UnicodeString(L"Cuda init failed!"));
	}
	//CreateThread 
	//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)CUDAThreadStartRoutine, this, NULL, NULL);
}

bool CUDAEngineRenderer::Render(RenderArgs* args)
{
	if (mainFrame == nullptr) return false;
	void* cudaFrame = mainFrame->Bind();
	cudaError_t errcode = cudaError_t::cudaSuccess;
	if (args != nullptr)
	{
		errcode = temp_callKernels(width, height, (pFrame)cudaFrame, args->args, args->size);
	}
	else
	{
		errcode = temp_callKernels(width, height, (pFrame)cudaFrame);
	}
	
	if (errcode != cudaSuccess)
	{
		const char* errString = GetErrorString(errcode);
		Log::GetInstance()->PrintMsg(UnicodeString(errString));
        return false;
	}
    return true;
}

void CUDAEngineRenderer::SettingsChanged(GlobalSettings NewSettings)
{
	//in case for future multithreading
	GPUImage* TempMainFrame = mainFrame;
	mainFrame = nullptr;
	delete TempMainFrame;
	width = NewSettings.Width;
	height = NewSettings.Height;
	mainFrame = new GPUImage(width, height, GPUImgType::IT_CUDA);
}

RendererStatus CUDAEngineRenderer::RenderInit(enum PresentMethod method, class DrawEngine* presenter)
{
	GlobalSettings Settings = presenter->GetGlobalSettings();
	width = Settings.Width;
	height = Settings.Height;
	//#TODO: Render in specific region (support x and y members)
	mainFrame = new GPUImage(width, height, GPUImgType::IT_CUDA);
	return RSTATUS_OK;
}

pFrame CUDAEngineRenderer::GetRenderFrame()
{
	//return OGL frameBuffer handle
	//WaitCudaThread();
	mainFrame->UnBind();
	return (pFrame)mainFrame->GetGLHandle();
}


CUDAEngineRenderer::~CUDAEngineRenderer()
{
	delete mainFrame;
}

