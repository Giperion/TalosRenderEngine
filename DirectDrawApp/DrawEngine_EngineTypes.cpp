//Added, because probably i replace this later
#include "stdafx.h"
#include "DrawEngine.h"

//include all known engines
#include "CUDAEngineRenderer.h"
#include "GLEngineRenderer.h"
#include "CLEngineRenderer.h"
#include "PCEngineRenderer.h"

//#HACK: That's awful solution, can we do better?
EngineType::Value DrawEngine::GetEngineType(IEngineRenderer* Renderer)
{
	if (dynamic_cast<PCEngineRenderer*>		(Renderer)) return EngineType::Native;
	if (dynamic_cast<CUDAEngineRenderer*>	(Renderer)) return EngineType::CUDA;
	if (dynamic_cast<CLEngineRenderer*>		(Renderer)) return EngineType::OpenCL;
	if (dynamic_cast<GLEngineRenderer*>		(Renderer)) return EngineType::OpenGL;
	return EngineType::Unknown;
}

IEngineRenderer* DrawEngine::CreateRenderEngine(EngineType::Value eEngineType) const
{
	switch (eEngineType)
	{
	case EngineType::Native:
		return new PCEngineRenderer();
	case EngineType::CUDA:
		return new CUDAEngineRenderer();
	case EngineType::OpenCL:
		return new CLEngineRenderer();
	case EngineType::OpenGL:
		return new GLEngineRenderer();
	case EngineType::Unknown:
	default:
		return nullptr;
	}
	return nullptr;
}