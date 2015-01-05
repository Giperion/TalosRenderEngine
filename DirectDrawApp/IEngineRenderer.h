#pragma once
class IEngineRenderer
{
public:
	IEngineRenderer();
	int width;
	int height;
	int x;
	int y;

	virtual byte* GetRenderFrame() = 0;
	virtual void Render() = 0;

	virtual ~IEngineRenderer();
};

