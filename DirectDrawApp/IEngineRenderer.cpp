#include "stdafx.h"
#include "IEngineRenderer.h"


IEngineRenderer::IEngineRenderer()
{
}


IEngineRenderer::~IEngineRenderer()
{
}

pFrame IEngineRenderer::GetRenderFrame()
{
	return nullptr;
}

bool IEngineRenderer::Render(RenderArgs* args)
{
    return false;
}
