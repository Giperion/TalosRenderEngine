#pragma once
#include "PrivateMacro.h"
#include "DrawEngine.h"
#include "IEngineRenderer.h"
#include "CL/opencl.h"


class CLEngineRenderer : public IEngineRenderer
{
public:
	CLEngineRenderer();
	~CLEngineRenderer();

	virtual pFrame GetRenderFrame() override;

	virtual void Render(RenderArgs* args) override;

	virtual RendererStatus RenderInit(PresentMethod method, class DrawEngine* presenter) override;

	virtual void RenderDestroy(PresentMethod method, class DrawEngine* presenter) override;

	static bool CheckClResult(cl_int CL_code, UnicodeString& FunctionName);
private:

};