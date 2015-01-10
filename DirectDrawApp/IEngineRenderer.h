#pragma once
class IEngineRenderer
{
public:
	IEngineRenderer();
	int width;
	int height;
	int x;
	int y;

	virtual pFrame GetRenderFrame();
	virtual void Render();

	virtual ~IEngineRenderer();
};