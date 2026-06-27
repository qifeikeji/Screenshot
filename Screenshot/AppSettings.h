#pragma once

struct AppSettings
{
	int windowWidth;
	int windowHeight;
	int maskGray;
	int maskOpacity;

	AppSettings();

	void SetDefaults();
	void Clamp();
	CString GetSettingsFilePath() const;
	BOOL Load();
	BOOL Save() const;
};

AppSettings& GetAppSettings();
