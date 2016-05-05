#include "stdafx.h"
#include "GPUImage.h"

#include "cuda_gl_interop.h"


GPUImage::GPUImage(int width, int height, GPUImgType type)
{
	this->type = type;
	this->width = width;
	this->height = height;
	int bufferObject;
	isBinded = false;

	//Buffer located on RAM
	imgPtr = new byte[(width * height) * 4];

	pFrame offsetedimgPtr = imgPtr;
	size_t offset = ((width * height) * 4) / 10;
	for (int i = 10; i != 0; i--)
	{
		memset(offsetedimgPtr, 20 * i, offset);
		offsetedimgPtr += offset;
	}

	
	glGenBuffers(1, &hPBO);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, hPBO);
	const char* hPBOName = "GLTexturePixelBuffer";
	glObjectLabel(GL_BUFFER, hPBO, 16, hPBOName);
	glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, (width * height) * 4, imgPtr, GL_STREAM_COPY);
	

	glGenTextures(1, &hTex);
	glBindTexture(GL_TEXTURE_2D, hTex);
	const char* hTexName = "GLTexture";
	glObjectLabel(GL_TEXTURE, hTex, 12, hTexName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgPtr);

	switch (type)
	{
	case IT_OpenCL:
		break;
	case IT_OpenGL:
		break;
	case IT_CUDA:
		cudaGraphicsGLRegisterBuffer(&cudaPBO, hPBO, cudaGraphicsRegisterFlagsWriteDiscard);
		break;
	default:
		break;
	}
}

GLuint GPUImage::GetGLHandle() const
{
	return hTex;
}

void* GPUImage::Bind()
{
	cudaError_t errcode = cudaError_t::cudaSuccess;
	switch (type)
	{
	case IT_OpenCL:
		break;
	case IT_OpenGL:
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, hPBO);
		isBinded = true;
		return glMapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, GL_READ_WRITE);
		break;
	case IT_CUDA:

		if (isBinded)
		{
			void* result;
			size_t bytes;
			errcode = cudaGraphicsResourceGetMappedPointer(&result, &bytes, cudaPBO);
			return result;
		}
		//cuda based binding, if cuda image
		errcode = cudaGraphicsMapResources(1, &cudaPBO);
		void* result;
		size_t bytes;
		errcode = cudaGraphicsResourceGetMappedPointer(&result, &bytes, cudaPBO);
		isBinded = true;
		return result;
	default:
		Log::GetInstance()->PrintMsg(UnicodeString(L"GPUImage::Bind: Unsuppoted bind type"));
		return 0;
	}
	Log::GetInstance()->PrintMsg(UnicodeString(L"GPUImage::Bind: Unexpected behaviour"));
	return 0;
}

void GPUImage::UnBind()
{
	GLboolean UnmapStatus;
	if (isBinded)
	{
		glBindTexture(GL_TEXTURE_2D, hTex);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, hPBO);

		switch (type)
		{
		case IT_OpenCL:
			break;
		case IT_OpenGL:
			UnmapStatus = glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB);
			if (!UnmapStatus)
			{
				LOG(L"GPUImage::UnBind: glUnmapBuffer return false!");
			}
			break;
		case IT_CUDA:
			cudaGraphicsUnmapResources(1, &cudaPBO);
			break;
		default:
			break;
		}

		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		isBinded = false;
	}
}

GPUImage::~GPUImage()
{
	delete[] imgPtr;
	switch (type)
	{
	case IT_OpenCL:
		break;
	case IT_OpenGL:
		break;
	case IT_CUDA:
		cudaGraphicsUnregisterResource(cudaPBO);
		break;
	default:
		break;
	}

	//free OpenGL resources
	glDeleteTextures(1, &hTex);
	glDeleteBuffers(1, &hPBO);

}
