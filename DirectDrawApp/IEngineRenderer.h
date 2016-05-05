#pragma once
struct RenderArgs
{
	void* args;
	size_t size;
};

enum RendererStatus : unsigned char
{
	RSTATUS_OK,
	RSTATUS_INVALIDPARAM,
	RSTATUS_INTERNALERROR,
	RSTATUS_DEVICEBUSY,
	RSTATUS_RENDERING
};

class IEngineRenderer
{
public:
	IEngineRenderer();
	int width;
	int height;
	int x;
	int y;

	virtual pFrame GetRenderFrame();
	virtual void Render(RenderArgs* args);

	//init, deinit
	virtual RendererStatus RenderInit(enum PresentMethod method, class DrawEngine* presenter){ return RSTATUS_OK; };
	virtual void RenderDestroy(PresentMethod method, class DrawEngine* presenter){};

	virtual ~IEngineRenderer();
};

