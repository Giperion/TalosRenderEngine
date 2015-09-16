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
	GLenum lasterror = glGetError();
	const char* hPBOName = "GLTexturePixelBuffer";
	glObjectLabel(GL_BUFFER, hPBO, 16, hPBOName);
	lasterror = glGetError();
	glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, (width * height) * 4, imgPtr, GL_STREAM_COPY);
	lasterror = glGetError();
	

	glGenTextures(1, &hTex);
	glBindTexture(GL_TEXTURE_2D, hTex);
	lasterror = glGetError();
	const char* hTexName = "GLTexture";
	glObjectLabel(GL_TEXTURE, hTex, 12, hTexName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgPtr);
	lasterror = glGetError();

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

	switch (type)
	{
	case IT_OpenCL:
		break;
	case IT_OpenGL:
		break;
	case IT_CUDA:
		cudaError_t errcode;
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
		Log::GetInstance()->PrintMsg(UnicodeString(L"Unsuppoted bind type"));
		return 0;
	}

}

void GPUImage::UnBind()
{
	if (isBinded)
	{
		cudaGraphicsUnmapResources(1, &cudaPBO);


		glBindTexture(GL_TEXTURE_2D, hTex);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, hPBO);

		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		isBinded = false;
	}
}

GPUImage::~GPUImage()
{
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

}
