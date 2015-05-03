#pragma once
#include "GL\glew.h"
#include "GL\wglew.h"
#include "Log.h"
#include "unicode\unistr.h"
class OpenGL
{
public:
	HGLRC CurrentContext;
	HDC CurrentDrawContext;

	HANDLE mut_ContextLocker;
	static OpenGL* GetInstance();
	static void StartOpenGLOperations(DWORD ThreadID);
	static void StopOpenGLOperations(DWORD ThreadID);
	bool CreateCompatableOpenGLHandle(HWND target, bool JustCheck = true);
	bool CreateOpenGLHandle(HWND target);
	UnicodeString GetDeviceInfo();
	UnicodeString GetExtension();
	void* AllocateBuffer(GLenum target, size_t BufferSize);
	void ReleaseBuffer(GLenum target);
	void FreeBuffer(GLuint BufferID);
	bool AreYouReady();
	~OpenGL();
private:
	HWND targetWindow;
	bool Inited;
	Log* log;
	DWORD CurrentThreadID;
	OpenGL();
	static OpenGL* instance;

};