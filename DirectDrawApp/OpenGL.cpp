#include "stdafx.h"
#include "OpenGL.h"

OpenGL* OpenGL::instance = nullptr;


#ifdef VELIYA_DEBUG
void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
{
	UnicodeString str;
	switch (type)
	{
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		str += L"(deprecated) ";
		break;
	case GL_DEBUG_TYPE_ERROR:
		str += L"(error) ";
		break;
	case GL_DEBUG_TYPE_MARKER:
		str += L"(marker) ";
		break;
	case GL_DEBUG_TYPE_OTHER:
		str += L"(other) ";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		str += L"(perfomance) ";
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		str += L"(pop group) ";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		str += L"(portability) ";
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		str += L"(push group) ";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		str += L"(undefined behavior) ";
		break;
	}
	str += UnicodeString(message, length);
	Log::GetInstance()->PrintMsg(str);
}
#endif

OpenGL* OpenGL::GetInstance()
{
	if (instance == nullptr)
	{
		instance = new OpenGL();
	}
	return instance;
}

OpenGL::OpenGL()
{
	Inited = false;
	CurrentContext = nullptr;
	CurrentDrawContext = nullptr;
	CurrentThreadID = -1;
	log = Log::GetInstance();
}

bool OpenGL::CreateCompatableOpenGLHandle(HWND target, bool JustCheck)
{
	
	static PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(pfd));
	CurrentThreadID = GetCurrentThreadId();

	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cDepthBits = 24;
	pfd.cColorBits = 32;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;
	targetWindow = target;
	CurrentDrawContext = GetWindowDC (target);
	int PixelFormat;
	PixelFormat = ChoosePixelFormat(CurrentDrawContext, &pfd);
	if (PixelFormat == 0)
	{
		log->PrintMsg(UnicodeString(L"ChoosePixelFormat failed! Function: CreateCompatableOpenGLHandle\n"));
		return false;
	}
	if (SetPixelFormat(CurrentDrawContext, PixelFormat, &pfd) == FALSE)
	{
		log->PrintMsg(UnicodeString(L"SetPixelFormat failed! Function: CreateCompatableOpenGLHandle\n"));
		return false;
	}

	HGLRC hrc;
	hrc = wglCreateContext(CurrentDrawContext);
	if (wglMakeCurrent(CurrentDrawContext, hrc) == FALSE)
	{
		log->PrintMsg(UnicodeString(L"wglMakeCurrent failed! Function: CreateCompatableOpenGLHandle\n"));
		return false;
	}
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		log->PrintMsg(UnicodeString(L"Glew initialization failed. Are you have OpenGL driver? Function: CreateCompatableOpenGLHandle\n"));
		return false;
	}

	if (JustCheck)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hrc);
		ReleaseDC(targetWindow, CurrentDrawContext);
		targetWindow = 0;
		CurrentDrawContext = 0;
		CurrentContext = 0;
		log->PrintMsg(UnicodeString(L"Compatable OpenGL device created, trying create full device handle\n"));
		return true;
	}
	CurrentContext = hrc;
	log->PrintMsg(UnicodeString(L"Compatable OpenGL device created!\n"));
	return true;
}

bool OpenGL::CreateOpenGLHandle(HWND target)
{
	if (WGLEW_ARB_create_context && WGLEW_ARB_pixel_format)
	{
		CurrentThreadID = GetCurrentThreadId();
		CurrentDrawContext = GetWindowDC(target);
		PIXELFORMATDESCRIPTOR pfd;
		ZeroMemory(&pfd, sizeof(pfd));
		const int PixelFormat[]
		{
			//MODE,					VARIABLE
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
				WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
				WGL_COLOR_BITS_ARB, 32,
				WGL_DEPTH_BITS_ARB, 24,
				WGL_STENCIL_BITS_ARB, 8,
				0
		};
#ifdef VELIYA_DEBUG
		const int ContextAttrib[]
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
				WGL_CONTEXT_MINOR_VERSION_ARB, 4,
				WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_DEBUG_BIT_ARB,
				0
		};
		log->PrintMsg(UnicodeString(L"Warning: OpenGL debug logging system enabled!\n"));
#else
		const int ContextAttrib[]
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
				WGL_CONTEXT_MINOR_VERSION_ARB, 4,
				WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
				0
		};
#endif

		int iPixelFormat, NumFormat;
		if (wglChoosePixelFormatARB(CurrentDrawContext, PixelFormat, NULL, 1, &iPixelFormat, (UINT*)(&NumFormat)) == FALSE)
		{
			log->PrintMsg(UnicodeString(L"wglChoosePixelFormatARB failed. Function: CreateOpenGLHandle\n"));
			return false;
		}
		if (SetPixelFormat(CurrentDrawContext, iPixelFormat, &pfd) == FALSE)
		{
			log->PrintMsg(UnicodeString(L"SetPixelFormat failed. Function: CreateOpenGLHandle\n"));
			return false;
		}

		HGLRC hrc = wglCreateContextAttribsARB(CurrentDrawContext, NULL, ContextAttrib);
		if (hrc)
		{
			wglMakeCurrent(CurrentDrawContext, hrc);
			OpenGL::CurrentContext = hrc;
			log->PrintMsg(UnicodeString(L"CreateOpenGLHandle success!\n\n"));

#ifdef VELIYA_DEBUG
			glEnable(GL_DEBUG_OUTPUT);
			GLDEBUGPROC procfunc = (GLDEBUGPROC)&DebugCallback;
			glDebugMessageCallback(procfunc, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif
			glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
			Inited = true;
			//mut_ContextLocker.lock();
			return true;
		}
		else
		{
			log->PrintMsg(UnicodeString(L"wglCreateContextAttribsARB failed. Function: CreateOpenGLHandle\n"));
			return false;
		}
	}
	else
	{
		log->PrintMsg(UnicodeString(L"OpenGL 4.4 not supported. Please enable DirectDraw method, for rendering.\n"));
		return false;
	}
}

UnicodeString OpenGL::GetDeviceInfo()
{
	
	if (CurrentContext == nullptr)
	{
		return L"Попытка получить данные видеоускорителя, при неинициализованом OpenGL!";
	}
	if (wglGetCurrentContext() == nullptr)
	{
		StopOpenGLOperations(GetCurrentThreadId());
		return L"Попытка получить данные видеоускорителя, в потоке, который не присвоил себе OGL контекст!";
	}
	const GLubyte* Test = glGetString(GL_VENDOR);
	GLenum error = glGetError();
	if (error == GL_INVALID_OPERATION)
	{
		return L"Недопустимая операция, при получений данных видеоускорителя";
	}

	LPCSTR Vendor = (LPCSTR)glGetString(GL_VENDOR);
	LPCSTR Renderer = (LPCSTR)glGetString(GL_RENDERER);
	LPCSTR Version = (LPCSTR)glGetString(GL_VERSION);
	LPCSTR ShaderVersion = (LPCSTR)glGetString(GL_SHADING_LANGUAGE_VERSION_ARB);
	LPSTR Total = new char[765];
	wsprintfA(Total, "\r\nVideo card vendor is: %hs \r\nRenderer is: %hs \r\nOpenGL Version: %hs \r\nGLSL Shader version: %hs ", Vendor, Renderer, Version, ShaderVersion);
	UnicodeString test = UnicodeString(Total);
	delete[] Total;
	return test;
}

bool OpenGL::AreYouReady()
{
	return Inited;
}


OpenGL::~OpenGL()
{
	if (CurrentContext != nullptr)
	{
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(CurrentContext);
	}
}
/*
Создает новый блок данных в видеопамяти, и создает указатели в памяти приложения
Для каждого вызова AllocateBuffer нужно делать вызов Release Buffer
target - тип буфера GL_ARRAY_BUFFER, GL_ATOMIC_COUNTER_BUFFER, GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, GL_DRAW_INDIRECT_BUFFER, GL_DISPATCH_INDIRECT_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_PIXEL_PACK_BUFFER, GL_PIXEL_UNPACK_BUFFER, GL_QUERY_BUFFER, GL_SHADER_STORAGE_BUFFER, GL_TEXTURE_BUFFER, GL_TRANSFORM_FEEDBACK_BUFFER or GL_UNIFORM_BUFFER
BufferSize - размер буфера
*/
void* OpenGL::AllocateBuffer(GLenum target, size_t BufferSize)
{
	GLuint BufferID;
	glGenBuffers(1, &BufferID);
	glBindBuffer(target, BufferID);
	glBufferData(target, BufferSize, NULL, GL_STATIC_DRAW);
	return glMapBuffer(target, GL_READ_WRITE);
}
/*
Завершает редактирование нового буфера и направляет данные в видеопамять.
Для каждого вызова AllocateBuffer нужно делать ReleaseBuffer
target - тип буфера
*/
void OpenGL::ReleaseBuffer(GLenum target)
{
	glUnmapBuffer(target);
}

void OpenGL::FreeBuffer(GLuint BufferID)
{
	GLboolean isBuffer = glIsBuffer(BufferID);
	if (isBuffer == GL_FALSE)
	{
		//Debug::Log(L"(LOW LEVEL CODE) Невозможно удалить буфер! Возможна порча кучи! Отмена операции...", true);
	}
	else
	{
		glDeleteBuffers(1, &BufferID);
	}

}

UnicodeString OpenGL::GetExtension()
{
	UnicodeString result;
	GLint ExtensionNum;
	glGetIntegerv(GL_NUM_EXTENSIONS, &ExtensionNum);
	result.append(L'\r');
	result.append(L'\n');
	LPWSTR tempBuffer = new wchar_t[512];
	for (int i = 0; i < ExtensionNum; i++)
	{
		LPCSTR Extension = nullptr;
		Extension = (LPCSTR)glGetStringi(GL_EXTENSIONS, i);
		if (Extension == nullptr) break;
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, Extension, -1, tempBuffer, 512);
		result.append(tempBuffer, 512);
		result.append(L'\r');
		result.append(L'\n');
	}
	delete[] tempBuffer;
	return result;
}

void OpenGL::StartOpenGLOperations(DWORD ThreadID)
{
	//OpenGL* p_gl = OpenGL::GetInstance();
	//if (p_gl == nullptr)
	//{
	//	//Debug::ErrorLog(L"(LOW LEVEL CODE) Попытка выполнить OGL операции, при неинициализированом контексте OGL!", true);
	//	return;
	//}
	//while (!p_gl->AreYouReady())
	//{
	//	//YOU ARE NOT PREPARED
	//	Sleep(5);
	//}
	//if (ThreadID == p_gl->CurrentThreadID) return;
	//p_gl->mut_ContextLocker.lock();
	//p_gl->CurrentThreadID = ThreadID;
	//wglMakeCurrent(p_gl->CurrentDrawContext, p_gl->CurrentContext);
}

void OpenGL::StopOpenGLOperations(DWORD ThreadID)
{
	//OpenGL* p_gl = OpenGL::GetInstance();
	//if (p_gl == nullptr)
	//{
	//	Debug::ErrorLog(L"(LOW LEVEL CODE) Попытка закончить OGL операции, при удаленом контексте OGL!", true);
	//	return;
	//}
	//p_gl->CurrentThreadID = -1;
	//wglMakeCurrent(0, 0);
	//p_gl->mut_ContextLocker.unlock();
}