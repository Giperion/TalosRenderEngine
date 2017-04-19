#include "stdafx.h"
#include <windows.h>
#include "GlobalSettings.h"
#include "Log.h"

void GlobalSettings::Load()
{
	HKEY AppFolderKey = OpenAppFolder();
	if (AppFolderKey == 0) return;
	Load(AppFolderKey);
	RegCloseKey(AppFolderKey);
}

void GlobalSettings::Load(HKEY AppFolderKey)
{
	LSTATUS ErrCode = 0;
	DWORD Width = 0;
	DWORD WidthSize = sizeof(Width);

	ErrCode = RegQueryValueEx(AppFolderKey, L"Width", 0, NULL, (LPBYTE)&Width, &WidthSize);
	if (ErrCode == ERROR_FILE_NOT_FOUND)
	{
		SaveDefault(AppFolderKey);
		return;
	}

	DWORD Height = 0;
	DWORD HeightSize = sizeof(Height);
	ErrCode = RegQueryValueEx(AppFolderKey, L"Height", 0, NULL, (LPBYTE)&Height, &HeightSize);
	if (ErrCode == ERROR_FILE_NOT_FOUND)
	{
		SaveDefault(AppFolderKey);
		return;
	}

	DWORD EngineTypeStrLen = 0;

	ErrCode = RegQueryValueEx(AppFolderKey, L"EngineType", 0, NULL, NULL, &EngineTypeStrLen);
	if (ErrCode == ERROR_FILE_NOT_FOUND)
	{
		SaveDefault(AppFolderKey);
		return;
	}

	if (EngineTypeStrLen == 0 || EngineTypeStrLen > 64)
	{
		SaveDefault(AppFolderKey);
		return;
	}

	LPCWSTR EngineTypeStr = (LPCWSTR)_alloca(EngineTypeStrLen);
	ErrCode = RegQueryValueEx(AppFolderKey, L"EngineType", 0, NULL, (LPBYTE)EngineTypeStr, &EngineTypeStrLen);

	this->Width = Width;
	this->Height = Height;
	this->eEngineType = EngineType::FromString(EngineTypeStr);
}

HKEY GlobalSettings::OpenAppFolder()
{
	HKEY blueDayFolder;
	DWORD Disposition;
	LSTATUS ErrCode = 0;
	//folder not created - create
	ErrCode = RegCreateKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\blUe Day Studio", 0, nullptr, 0, KEY_ALL_ACCESS, NULL, &blueDayFolder, &Disposition);
	if (ErrCode != 0)
	{
		Log::GetInstance()->PrintMsg(L"Cannot open main catalog in registry");
		return 0;
	}

	HKEY DrawEngineFolder;
	ErrCode = RegCreateKeyEx(blueDayFolder, L"DrawEngine", 0, nullptr, 0, KEY_ALL_ACCESS, 0, &DrawEngineFolder, &Disposition);
	if (ErrCode != 0)
	{
		Log::GetInstance()->PrintMsg(L"Cannot open folder \"DrawEngineFolder\" in registry");
		RegCloseKey(blueDayFolder);
		return 0;
	}
	RegCloseKey(blueDayFolder);

	return DrawEngineFolder;
}

void GlobalSettings::Save()
{
	HKEY AppFolderKey = OpenAppFolder();
	if (AppFolderKey == 0) return;
	LSTATUS ErrCode = 0;
	Save(AppFolderKey);
	RegCloseKey(AppFolderKey);
}

void GlobalSettings::Save(HKEY AppFolderKey)
{
	LSTATUS ErrCode = 0;

	ErrCode = RegSetValueEx(AppFolderKey, L"Width", 0, REG_DWORD, (BYTE*)&Width, sizeof(DWORD));
	if (ErrCode != 0)
	{
		Log::GetInstance()->PrintMsg(L"Can't save Width key in registry!");
		return;
	}

	ErrCode = RegSetValueEx(AppFolderKey, L"Height", 0, REG_DWORD, (BYTE*)&Height, sizeof(DWORD));
	if (ErrCode != 0)
	{
		Log::GetInstance()->PrintMsg(L"Can't save Height key in registry!");
		return;
	}

	LPCWSTR EnumStr = EngineType::ToString(eEngineType);
	int StrLen = lstrlenW(EnumStr);

	ErrCode = RegSetValueEx(AppFolderKey, L"EngineType", 0, REG_SZ, (BYTE*)EnumStr, StrLen * sizeof(wchar_t));
	if (ErrCode != 0)
	{
		Log::GetInstance()->PrintMsg(L"Can't save EngineType key in registry!");
		return;
	}
}

void GlobalSettings::SaveDefault(HKEY AppFolderKey)
{
	Width = ENGINEWIDTH;
	Height = ENGINEHEIGHT;
	eEngineType = EngineType::Native;
	Save(AppFolderKey);
}
