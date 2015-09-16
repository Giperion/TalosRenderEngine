#pragma once
struct RenderArgs
{
	void* args;
	size_t size;
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

	virtual ~IEngineRenderer();
};

