#pragma once
#include "cuda_runtime.h"

enum GPUImgType
{
	IT_OpenCL,
	IT_OpenGL,
	IT_CUDA
};

/*
Only support GL presenter and cuda renderer
*/
GLCALL class GPUImage
{
public:
	GPUImage(int width, int height, GPUImgType type);
	~GPUImage();
	void* Bind();
	void UnBind();
	GLuint GetGLHandle() const;
private:
	int width;
	int height;
	pFrame imgPtr;

	bool isBinded;

	cudaGraphicsResource* cudaPBO;
	GLuint hPBO;
	GLuint hTex;
	GPUImgType type;
};
