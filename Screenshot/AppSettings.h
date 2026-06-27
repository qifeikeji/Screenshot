#pragma once

struct AppSettings
{
	int windowWidth;
	int windowHeight;
	UINT hotkeyModifiers;
	UINT hotkeyVk;

	AppSettings();

	void SetDefaults();
	void Clamp();
	CString GetSettingsFilePath() const;
	BOOL Load();
	BOOL Save() const;
};

AppSettings& GetAppSettings();

void HotKeyToHotKeyCtrl(UINT mod, UINT vk, WORD& hotKeyWord);
void HotKeyFromHotKeyCtrl(WORD hotKeyWord, UINT& mod, UINT& vk);
