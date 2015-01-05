#pragma once
#include "IEngineRenderer.h"
class PCEngineRenderer :
	public IEngineRenderer
{
public:
	PCEngineRenderer();
	~PCEngineRenderer();
	virtual void Render();
};

