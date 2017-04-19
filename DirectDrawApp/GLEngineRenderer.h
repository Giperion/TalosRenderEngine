#pragma once
class GLEngineRenderer : public IEngineRenderer
{
public:
	GLEngineRenderer();
	virtual ~GLEngineRenderer();

	virtual void SettingsChanged(struct GlobalSettings NewSettings) override;

};

