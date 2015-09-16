#include "stdafx.h"
#include "CUDAEngineRenderer.h"
DWORD CUDAThreadStartRoutine(LPVOID param)
{
	CUDAEngineRenderer* CER = (CUDAEngineRenderer*)param;
	return CER->MainThread(NULL);
}

CUDAEngineRenderer::CUDAEngineRenderer()
{
	//Start CUDA parallel system...
	mainFrame = nullptr;
	if (!cuda_init())
	{
		Log::GetInstance()->PrintMsg(UnicodeString(L"Cuda init failed!"));
	}
	mainFrame = new GPUImage(ENGINEWIDTH, ENGINEHEIGHT, GPUImgType::IT_CUDA);
	//CreateThread 
	//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)CUDAThreadStartRoutine, this, NULL, NULL);
}

void CUDAEngineRenderer::Render(RenderArgs* args)
{
	//call kernels, and wait for result
	if (mainFrame == nullptr) return;
	void* cudaFrame = mainFrame->Bind();
	cudaError_t errcode;
	if (args != nullptr)
	{
		errcode = temp_callKernels(ENGINEWIDTH, ENGINEHEIGHT, (pFrame)cudaFrame, args->args, args->size);
	}
	else
	{
		errcode = temp_callKernels(ENGINEWIDTH, ENGINEHEIGHT, (pFrame)cudaFrame);
	}
	
	if (errcode != cudaSuccess)
	{
		const char* errString = GetErrorString(errcode);
		Log::GetInstance()->PrintMsg(UnicodeString(errString));
	}
}

pFrame CUDAEngineRenderer::GetRenderFrame()
{
	//return OGL frameBuffer handle
	WaitCudaThread();
	mainFrame->UnBind();
	return (pFrame)mainFrame->GetGLHandle();
}

DWORD CUDAEngineRenderer::MainThread(LPVOID param)
{


	while (true)
	{
		Sleep(100);
	}
	return 1;
}


CUDAEngineRenderer::~CUDAEngineRenderer()
{
	delete mainFrame;
}

