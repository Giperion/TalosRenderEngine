#include "stdafx.h"
#include <math.h>
#include "resource.h"
#include "PCEngineRenderer.h"
#pragma warning( disable : 4566 )

DrawEngine::DrawEngine(HWND hWnd, GlobalSettings InSettings, PresentMethod PMethod /*= PM_DirectDraw*/)
{
	AttachedHWND = hWnd;
	presentMethod = PMethod;
	CurrentState = DrawEngineState::DES_INIT;
	LastRenderTime = 0.0;
	Settings = InSettings;

	QueryPerformanceFrequency(&frequency);

	switch (presentMethod)
	{
	case PM_DirectDraw:
		if (!InitDirectDraw())
		{
			MessageBox(AttachedHWND, L"DirectDraw init error!", L"Critical error", MB_OK | MB_ICONERROR);
			return;
		}
		break;
	case PM_GDI:
		MessageBox(AttachedHWND, L"GDI present method not implemented!", L"Error", MB_OK | MB_ICONERROR);
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		break;
	case PM_OpenGL:
		if (!InitOpenGL())
		{
			MessageBox(AttachedHWND, L"OpenGL init error!", L"Error", MB_OK | MB_ICONERROR);
			return;
		}
		break;
	default:
		MessageBox(AttachedHWND, L"Undefined present method is chosen!", L"Critical Error", MB_OK | MB_ICONERROR);
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		break;
	}
	//create default render engine
	IEngineRenderer* renderer = CreateRenderEngine(Settings.eEngineType);
	if (renderer == nullptr)
	{
		Log::GetInstance()->PrintMsg(L"Error, trying create a default render engine type: %s", EngineType::ToString(Settings.eEngineType));
		renderer = new PCEngineRenderer();
	}
	CurrentState = DrawEngineState::DES_IDLE;

	PushRenderer(renderer);
}


bool DrawEngine::InitDirectDraw()
{
	HRESULT FuncResult;

	FuncResult = DirectDrawCreate(NULL, &pDirectDraw, NULL);
	if (FuncResult != DD_OK)
	{
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		return false;
	}
	FuncResult = pDirectDraw->SetCooperativeLevel(AttachedHWND, DDSCL_NORMAL);
	if (FuncResult != DD_OK)
	{
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		pDirectDraw->Release();
		return false;
	}

	memset(&DirectSurfaceDesc, 0, sizeof(DDSURFACEDESC));
	DirectSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
	DirectSurfaceDesc.dwFlags = DDSD_CAPS;
	DirectSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	FuncResult = pDirectDraw->CreateSurface(&DirectSurfaceDesc, &pPrimarySurface, NULL);
	if (FuncResult != DD_OK)
	{
		pDirectDraw->Release();
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		return false;
	}

	FuncResult = pDirectDraw->CreateClipper(NULL, &pPrimaryClipper, NULL);
	if (FuncResult != DD_OK)
	{
		pPrimarySurface->Release();
		pDirectDraw->Release();
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		return false;
	}

	FuncResult = pPrimaryClipper->SetHWnd(NULL, AttachedHWND);
	if (FuncResult != DD_OK)
	{
		pPrimaryClipper->Release();
		pPrimarySurface->Release();
		pDirectDraw->Release();
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		return false;
	}

	FuncResult = pPrimarySurface->SetClipper(pPrimaryClipper);
	if (FuncResult != DD_OK)
	{
		pPrimaryClipper->Release();
		pPrimarySurface->Release();
		pDirectDraw->Release();
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		return false;
	}

	memset(&DirectSurfaceDesc, 0, sizeof(DDSURFACEDESC));
	DirectSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
	DirectSurfaceDesc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	DirectSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	DirectSurfaceDesc.dwWidth = ENGINEWIDTH;
	DirectSurfaceDesc.dwHeight = ENGINEHEIGHT;

	FuncResult = pDirectDraw->CreateSurface(&DirectSurfaceDesc, &pBackSurface, NULL);
	if (FuncResult != DD_OK)
	{
		pPrimaryClipper->Release();
		pPrimarySurface->Release();
		pDirectDraw->Release();
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		return false;
	}
	return true;
}

bool DrawEngine::InitOpenGL()
{
	gl = OpenGL::GetInstance();
	if (!gl->CreateCompatableOpenGLHandle(AttachedHWND))
	{
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		return false;
	}
	if (!gl->CreateOpenGLHandle(AttachedHWND))
	{
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		return false;
	}
	Log::GetInstance()->PrintMsg(gl->GetDeviceInfo());
	if (!PreparePlaneScene())
	{
		CurrentState = DrawEngineState::DES_PRESENTERFAILED;
		return false;
	}
	return true;
}

DrawEngine::~DrawEngine()
{
	int Timer = 0;
	while (CurrentState != DrawEngineState::DES_IDLE)
	{
		//Õüþñòîí, ó íàñ òóò ðåíäåðèíã äî ñèõ ïîð.
		Sleep(5);
		Timer++;
		if (Timer > 120)
		{
			LPWSTR Buffer = new WCHAR[512];
			StringCbPrintf(Buffer, 512, L"Render engine can't deinitialize! Current state: %d", CurrentState);
			MessageBox(NULL, Buffer, L"Critical Error", MB_OK || MB_ICONERROR);
			delete[] Buffer;
		}
	}
	InterlockedExchange((unsigned int*)&CurrentState, (unsigned int)DrawEngineState::DES_SHUTINGDOWN);
	for (int i = renderers.size(); i > 0; i--)
	{
		PopRenderer();
	}

	switch (presentMethod)
	{
	case PM_DirectDraw:
		ShutDownDirectDraw();
		break;
	case PM_GDI:
		MessageBox(AttachedHWND, L"GDI present method can't be deinitialized!", L"Critical Error", MB_OK | MB_ICONERROR);
		break;
	case PM_OpenGL:
		if (gl != nullptr)
		{
			delete gl;
		}
		break;
	default:
		MessageBox(AttachedHWND, L"Undefined deinit method called!", L"Critical Error", MB_OK | MB_ICONERROR);
		break;
	}

	Settings.Save();
}

void DrawEngine::ShutDownDirectDraw()
{
	pPrimaryClipper->Release();
	pBackSurface->Release();
	pPrimarySurface->Release();
	pDirectDraw->Release();

	pPrimaryClipper = nullptr;
	pBackSurface = nullptr;
	pPrimarySurface = nullptr;
	pDirectDraw = nullptr;
}

void DrawEngine::ShutDownOpenGL()
{
	delete gl;
}

U_ICU_NAMESPACE::UnicodeString DrawEngine::GetTextFromStatus(DrawEngineState status)
{
	switch (status)
	{
	case DES_INIT:
		return UnicodeString(L"DES_INIT");
	case DES_IDLE:
		return UnicodeString(L"DES_IDLE");
	case DES_PRESENTERFAILED:
		return UnicodeString(L"DES_PRESENTERFAILED");
	case DES_RENDER_IN_PROGRESS:
		return UnicodeString(L"DES_RENDER_IN_PROGRESS");
	case DES_RENDER_FINISHED:
		return UnicodeString(L"DES_RENDER_FINISHED");
	case DES_REQUEST_NEW_FRAME:
		return UnicodeString(L"DES_REQUEST_NEW_FRAME");
	case DES_SHUTINGDOWN:
		return UnicodeString(L"DES_SHUTINGDOWN");
	default:
		return UnicodeString(L"UNDEFINED");
	}
}

EXPERIMENTAL void DrawEngine::DrawTest()
{
	memset(&DirectSurfaceDesc, 0, sizeof (DDSURFACEDESC));
	DirectSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
	DirectSurfaceDesc.dwFlags = DDSD_CAPS;
	DirectSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	pPrimarySurface->Lock(NULL, &DirectSurfaceDesc, DDLOCK_WAIT, NULL);
	memset(DirectSurfaceDesc.lpSurface, 0, DirectSurfaceDesc.lPitch * DirectSurfaceDesc.dwHeight);
	pPrimarySurface->Unlock(DirectSurfaceDesc.lpSurface);
}

RenderMethod DrawEngine::GetCurrentRenderMethod()
{
	return renderMethod;
}

double DrawEngine::GetLastRenderTime() const
{
	return LastRenderTime;
}

PresentMethod DrawEngine::GetCurrentPresentMethod()
{
	return presentMethod;
}

const GlobalSettings DrawEngine::GetGlobalSettings() const
{
	return Settings;
}

void DrawEngine::PushRenderer(IEngineRenderer* iRenderer)
{
	if (CurrentState == DES_IDLE)
	{
#ifdef OPENGL_BLT_TEX
		//if OpenGL we need create a framebufffer
		if (presentMethod == PresentMethod::PM_OpenGL)
		{

			GLuint newFrame;

			glGenTextures(1, &newFrame);
			glBindTexture(GL_TEXTURE_2D, newFrame);
			glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, ENGINEWIDTH, ENGINEHEIGHT);
			glBindTexture(GL_TEXTURE_2D, 0);
			FrameBuffers.push_back(newFrame);

		}
#endif
		RendererStatus status = iRenderer->RenderInit(presentMethod, this);
		
		if (status == RSTATUS_OK)
			renderers.push_back(iRenderer);
		else
			Log::GetInstance()->PrintMsg(UnicodeString(L"DrawEngine::PushRenderer: Renderer initialization failed"));
	}
	else
	{
		Log::GetInstance()->PrintMsg(UnicodeString(L"DrawEngine::PushRenderer: Calling while DrawEngine in state: ") + GetTextFromStatus(CurrentState));
	}
}

void DrawEngine::PopRenderer()
{
	if (renderers.size() > 0)
	{
		IEngineRenderer* Renderer = renderers[renderers.size() - 1];
		Renderer->RenderDestroy(presentMethod, this);
		delete Renderer;
		renderers.pop_back();
	}
}

//TODO a normal blitter
bool DrawEngine::Render(RenderArgs* args)
{
	pFrame newFrame = 0;
	LARGE_INTEGER result;
    LARGE_INTEGER startTime;
    LARGE_INTEGER endTime;

	QueryPerformanceCounter(&startTime);
	
	if (!(renderers.size() > 0))
	{
		MessageBox(AttachedHWND, L"Render call without renderers", L"Critical Error", MB_OK | MB_ICONERROR);
		return false;
	}

	if (CurrentState == DrawEngineState::DES_IDLE)
	{
		for (int i = 0; i < renderers.size(); i++)
		{
            if (!renderers[i]->Render(args))
            {
                return false;
            }
		}
		for (int i = 0; i < renderers.size(); i++)
		{
			switch (presentMethod)
			{
			case PM_DirectDraw:
            {
                int Allbytes = 0;
                RECT rcSource;
                RECT rcDest;
                POINT trWindow;
                unsigned char* VideoMemory = 0;
				newFrame = renderers[i]->GetRenderFrame();
				pBackSurface->Lock(NULL, &DirectSurfaceDesc, DDLOCK_WAIT, NULL);
				Allbytes = DirectSurfaceDesc.lPitch * DirectSurfaceDesc.dwHeight;
				VideoMemory = static_cast <unsigned char*> (DirectSurfaceDesc.lpSurface);
				memcpy(VideoMemory, newFrame, Allbytes);
				pBackSurface->Unlock(DirectSurfaceDesc.lpSurface);

				trWindow.x = 0; trWindow.y = 0;
				ClientToScreen(AttachedHWND, &trWindow);
				GetClientRect(AttachedHWND, &rcDest);
				OffsetRect(&rcDest, trWindow.x, trWindow.y);
				SetRect(&rcSource, 0, 0, ENGINEWIDTH, ENGINEHEIGHT);
				pPrimarySurface->Blt(&rcDest, pBackSurface, &rcSource, DDBLT_WAIT, NULL);
            }
				break;
			case PM_GDI:


				break;
			case PM_OpenGL:
				//Test Draw
#ifdef OPENGL_BLT_TEX
				GLuint Tex = FrameBuffers[i];
				pFrame newFrame = renderers[i]->GetRenderFrame();
				glBindTexture(GL_TEXTURE_2D, Tex);
				glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, ENGINEWIDTH, ENGINEHEIGHT, GL_RGBA, GL_UNSIGNED_INT, newFrame);
#endif
				{

					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
					glViewport(0, 0, Settings.Width, Settings.Height);
					glBindVertexArray(DefaultVertexArrayID);
					glBindBuffer(GL_ARRAY_BUFFER, DefaultVertexBufferID);
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, DefaultVertexIndexID);
					glUseProgramObjectARB(DefaultShaderObject);

					//this is texture 2D handle
					newFrame = renderers[i]->GetRenderFrame();
					if (newFrame == nullptr) break;
					glBindTexture(GL_TEXTURE_2D, (GLuint)newFrame);
					glUniform1i(0, 0);
					glActiveTexture(GL_TEXTURE0);

					glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
					gl->OGLSwapBuffers();
				}
				break;
			default:
				break;
			}


		}
		QueryPerformanceCounter(&endTime);

		result.QuadPart = endTime.QuadPart - startTime.QuadPart;
		result.QuadPart *= 1000;
		LastRenderTime = (double)result.QuadPart / (double)frequency.QuadPart;

		CurrentState = DrawEngineState::DES_IDLE;
		return true;
	}
	return false;
}

void DrawEngine::PostRender(double RenderTime)
{
	InvalidateRect(AttachedHWND, NULL, FALSE);
	LastRenderTime = RenderTime;
}

void DrawEngine::SetGlobalSettings(GlobalSettings& InSettings)
{
	//#HOTFIX: Sometimes we receive 0, 0. Throw that off
	if (InSettings.Width == 0 || InSettings.Height == 0) return;

	IEngineRenderer* NewEngine = nullptr;
	if (Settings.IsEngineTypeChanged(InSettings))
	{
		//On Engine type changed

		if (renderers.size() > 0)
		{
			//#NYI: Check other's engine's, not only first
			IEngineRenderer* FirstRenderer = renderers[0];
			EngineType::Value eEngineType = GetEngineType(FirstRenderer);

			if (eEngineType != Settings.eEngineType)
			{
				Log::GetInstance()->PrintMsg(L"Error: Current engine type: \"%s\", while settings tells that we have \"%s\"!", EngineType::ToString(eEngineType), EngineType::ToString(Settings.eEngineType));
			}

			PopRenderer();

			NewEngine = CreateRenderEngine(InSettings.eEngineType);
		}
		else
		{
			//we have no render engine's yet, create a new one
			NewEngine = CreateRenderEngine(InSettings.eEngineType);
		}
	}

	if (Settings.IsResolutionChanged(InSettings))
	{
		for (IEngineRenderer* renderer : renderers)
		{
			renderer->SettingsChanged(InSettings);
		}
	}

	Settings = InSettings;

	if (NewEngine != nullptr)
	{
		PushRenderer(NewEngine);
	}
}

GLCALL bool DrawEngine::PreparePlaneScene()
{
	Log* log = Log::GetInstance();
	static const GLchar viboLabel[] = "DefaultModel Vertex Index Buffer";


	glGenVertexArrays(1, &DefaultVertexArrayID);
	glBindVertexArray(DefaultVertexArrayID);

	static const GLchar vboLabel[] = "DefaultModel Vertex Buffer";
	glGenBuffers(1, &DefaultVertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, DefaultVertexBufferID);
	int sizeofVB = sizeof(vboLabel);
	glObjectLabel(GL_BUFFER, DefaultVertexBufferID, sizeofVB, &vboLabel[0]);


	glGenBuffers(1, &DefaultVertexIndexID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, DefaultVertexIndexID);
	sizeofVB = sizeof(viboLabel);
	glObjectLabel(GL_BUFFER, DefaultVertexIndexID, sizeofVB, &viboLabel[0]);

	

	static const GLfloat vertexs[]
	{
		-1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f
	};
	static const GLfloat TexUV[]
	{
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};

	static const GLuint indexes[]
	{
		0, 1, 2,
		2, 3, 0
	};
	DWORD elemSize = sizeof(GLfloat);
	DWORD vertSize = sizeof(vertexs);
	DWORD uvSize = sizeof(TexUV);

	glBufferData(GL_ARRAY_BUFFER, vertSize + uvSize, 0, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, vertSize, &vertexs);
	glBufferSubData(GL_ARRAY_BUFFER, vertSize, uvSize, &TexUV);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexes), &indexes, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, GL_FALSE, (void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, GL_FALSE, (void*)vertSize);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);




	//Setup shaders and material
	LPCSTR vertexCode = nullptr;
	LPCSTR fragmentCode = nullptr;

	HRSRC hRes = FindResourceW(NULL, MAKEINTRESOURCE(IDR_VERTCODE), MAKEINTRESOURCE(RT_HTML));
	HGLOBAL pCode = LoadResource(NULL, hRes);
	vertexCode = (LPCSTR)pCode;

	hRes = FindResourceW(NULL, MAKEINTRESOURCE(IDR_FRAGCODE), MAKEINTRESOURCE(RT_HTML));
	pCode = LoadResource(NULL, hRes);
	fragmentCode = (LPCSTR)pCode;

	if (vertexCode == nullptr || fragmentCode == nullptr)
	{
		log->PrintMsg(UnicodeString(L"Фатальная ошибка! Отсутсвует код шейдеров: файлы VertexShader.glsl и FragmentShader.glsl"));
		return false;
	}

	GLenum errorcode;
	GLhandleARB VertexShader;
	GLhandleARB PixelShader;



	VertexShader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	PixelShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

	glShaderSourceARB(VertexShader, 1, &vertexCode, NULL);
	glShaderSourceARB(PixelShader, 1, &fragmentCode, NULL);

	LPCSTR CompileLog = new char[1024];
	GLint CompileStatus;
	GLint InfoLogLenght = 0;

	glCompileShaderARB(VertexShader);

	glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &CompileStatus);
	glGetShaderiv(VertexShader, GL_INFO_LOG_LENGTH, &InfoLogLenght);
	if (CompileStatus == GL_FALSE)
	{
		log->PrintMsg(UnicodeString ("Произошла ошибка при компиляции вертексного шейдера. Смотрите информацию ниже"));
		glGetShaderInfoLog(VertexShader, 1024, NULL, (GLchar*)CompileLog);
		UnicodeString& errorlog = UnicodeString(CompileLog, InfoLogLenght);
		log->PrintMsg(UnicodeString(errorlog));

		delete[] CompileLog;
		glDeleteShader(VertexShader);
		glDeleteShader(PixelShader);
		return false;
	}
	if (InfoLogLenght > 1)
	{
		log->PrintMsg(UnicodeString("Вертексный шейдер скомпилирован, смотрите информацию ниже"));
		glGetShaderInfoLog(VertexShader, 1024, 0, (GLchar*)CompileLog);
		UnicodeString& infolog = UnicodeString(CompileLog, InfoLogLenght);
		log->PrintMsg(UnicodeString(infolog));
		ZeroMemory((void*)CompileLog, 1024);
	}

	glCompileShaderARB(PixelShader);

	glGetShaderiv(PixelShader, GL_COMPILE_STATUS, &CompileStatus);
	glGetShaderiv(PixelShader, GL_INFO_LOG_LENGTH, &InfoLogLenght);
	if (CompileStatus == GL_FALSE)
	{
		log->PrintMsg(UnicodeString("Произошла ошибка при компиляции фрагментного шейдера. Смотрите информацию ниже"));
		glGetShaderInfoLog(PixelShader, 1024, NULL, (GLchar*)CompileLog);
		UnicodeString& errorlog = UnicodeString(CompileLog, InfoLogLenght);
		log->PrintMsg(errorlog);

		delete[] CompileLog;
		glDeleteShader(VertexShader);
		glDeleteShader(PixelShader);
		return false;
	}
	if (InfoLogLenght > 1)
	{
		log->PrintMsg(UnicodeString("Пиксельный шейдер скомпилирован, смотрите информацию ниже"));
		glGetShaderInfoLog(PixelShader, 1024, 0, (GLchar*)CompileLog);
		UnicodeString& infolog = UnicodeString(CompileLog, InfoLogLenght);
		log->PrintMsg(infolog);
		ZeroMemory((void*)CompileLog, 1024);
	}


	DefaultShaderObject = glCreateProgramObjectARB();
	glAttachObjectARB(DefaultShaderObject, VertexShader);

	errorcode = glGetError();
	if (errorcode != GL_NO_ERROR)
	{
		log->PrintMsg(UnicodeString("Неизвестная ошибка линковки шейдера"));
		return false;
	}

	glAttachObjectARB(DefaultShaderObject, PixelShader);

	errorcode = glGetError();
	if (errorcode != GL_NO_ERROR)
	{
		log->PrintMsg(UnicodeString("Неизвестная ошибка линковки шейдера"));
		return false;
	}

	glLinkProgramARB(DefaultShaderObject);
	errorcode = glGetError();
	if (errorcode != GL_NO_ERROR)
	{
		log->PrintMsg(UnicodeString("Неизвестная ошибка линковки программы"));
		return false;
	}

	delete[] CompileLog;

	glUseProgramObjectARB(DefaultShaderObject);
	//We leave active shader program and scene
	return true;
}

inline int Lerp(int start, int end, double value)
{
	return start + (end - start) * value;
}

LPCSTR DrawEngine::LoadShaderCodeInternal(LPCWSTR filename)
{
	Log* log = Log::GetInstance();
	HANDLE ShaderFile = CreateFile(filename, GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (ShaderFile == INVALID_HANDLE_VALUE)
	{
		log->PrintMsg(UnicodeString("Файл шейдера по-умолчанию не найден\n"));
		return nullptr;
	}

	LARGE_INTEGER shadersize;
	GetFileSizeEx(ShaderFile, &shadersize);
	if (shadersize.HighPart != NULL)
	{
		log->PrintMsg(UnicodeString("Размер внутреннего шейдера слишком большой\n"));
		CloseHandle(ShaderFile);
		return nullptr;
	}
	if (shadersize.LowPart == NULL)
	{
		log->PrintMsg(UnicodeString("Файл шейдера пустой\n"));
		CloseHandle(ShaderFile);
		return nullptr;
	}
	LPCSTR ShaderCode = new char[shadersize.LowPart];
	ZeroMemory((void*)ShaderCode, shadersize.LowPart);
	DWORD bytereaded;
	ReadFile(ShaderFile, (LPVOID)ShaderCode, shadersize.LowPart - 1, &bytereaded, NULL);
	CloseHandle(ShaderFile);
	return ShaderCode;
}
Color DrawEngine::MandelbrotSet(const int x, const int y)
{
	Color result;

	l_long ResultX;
	l_long ResultY;

	l_long Zx = 0;
	l_long Zy = 0;
	l_long Zx_x2 = 0;
	l_long Zy_x2 = 0;

	const l_long MinimumResultX = -2.5L;
	const l_long MaximumResultX = 1.5L;
	const l_long MinimumResultY = -2.0L;
	const l_long MaximumResultY = 2.0L;

	l_long PixelWidth = (MaximumResultX - MinimumResultX) / ENGINEWIDTH;
	l_long PixelHeight = (MaximumResultY - MinimumResultY) / ENGINEHEIGHT;

	int Iteration = 0;

	const l_long EscapeRadius = 2.0L;
	l_long EscapeRadius_x2 = EscapeRadius * EscapeRadius;

	ResultX = (MinimumResultX + PixelWidth * x) * 1.0;
	ResultY = (MinimumResultY + PixelHeight * y) * 1.0;

	if (fabs(ResultY) < PixelHeight / 2) ResultY = 0.0;

	for (; Iteration < 20 && ((Zx_x2 + Zy_x2) < EscapeRadius_x2); Iteration++)
	{
		Zy = 2 * Zx * Zy + ResultY;
		Zx = Zx_x2 - Zy_x2 + ResultX;
		Zx_x2 = Zx * Zx;
		Zy_x2 = Zy * Zy;
	}

	double Value = (1.0 / (double)20) *  Iteration;
	//Ðàíüøå âîçâðàùàëè öâåò ïî òàáëèöå, íî óâû, ýòî íå ýôôåêòèâíûé ñïîñîá
	//return ResoulveColor(Iteration);
	int grayscaleComp = Lerp(0, 255, Value);
	
	result.R = grayscaleComp; result.G = grayscaleComp; result.B = grayscaleComp;
	return result;
}
