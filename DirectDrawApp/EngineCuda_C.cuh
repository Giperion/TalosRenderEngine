//Cuda API

//This file reading from two compilers, so hack is fix one problem...
#ifndef __device__
#define __device__
#define __host__

#endif




#include "PrivateMacro.h"

#define CENGINE_STATUS_INIT -1
#define CENGINE_STATUS_NOCUDA -2
#define CENGINE_STATUS_IDLE 1
#define CENGINE_STATUS_RENDERING 2
#define CENGINE_STATUS_RECONFIGURATE 3
#define CENGINE_STATUS_RENDERFINISHED 4
#define CENGINE_STATUS_FATALERROR 999

//Код под вопросом (Код под котом, доо)
#define CENGINE_MAX_CUDA_RESOURCES 256
struct cudaImage
{
	int width;
	int height;
	size_t bytes;
	void* cudaData;
};



extern "C"
{
	bool cuda_init();
	__global__ void testKernelFunc(int width, int height, pFrame frame, void* args);
	__global__ void JuliaKernelFunc(int width, int height, pFrame frame, void* args);
	bool cuda_GL_init();
	cudaImage cuda_AllocTexture(int width, int height);
	void cuda_FreeTexture(cudaImage image);
	void cuda_deinit();
	void cuda_execCode(char* code);
	const char* GetErrorString(cudaError_t errcode);

	//temp function to call kernel. It's needed, because in cpp wraper no cuda compiler specific commands
	cudaError_t temp_callKernels(int width, int height, pFrame frame, void* args = nullptr, int argsSize = 0);
	void WaitCudaThread();
}