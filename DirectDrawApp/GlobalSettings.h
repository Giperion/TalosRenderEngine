#pragma once
#include "DrawEngineDefinitions.h"

struct GlobalSettings
{
	DWORD Width;
	DWORD Height;
	EngineType::Value eEngineType;

	GlobalSettings() :
		Width(0), Height(0), eEngineType(EngineType::Native)
	{}

	GlobalSettings(int InWidth, int InHeight) :
		Width(InWidth), Height(InHeight), eEngineType(EngineType::Native)
	{}

	GlobalSettings(int InWidth, int InHeight, EngineType::Value InEngineType) :
		Width(InWidth), Height(InHeight), eEngineType(InEngineType)
	{}

	void Load();
	void Save();

	inline bool IsResolutionChanged(GlobalSettings& Other)
	{
		return Other.Width != Width || Other.Height != Height;
	}
	inline bool IsEngineTypeChanged(GlobalSettings& Other)
	{
		return Other.eEngineType != eEngineType;
	}

private:
	void SaveDefault(HKEY AppFolderKey);
	void Save(HKEY AppFolderKey);
	void Load(HKEY AppFolderKey);

	HKEY OpenAppFolder();
};


