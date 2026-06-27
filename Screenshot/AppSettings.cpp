#include "stdafx.h"
#include "AppSettings.h"

static AppSettings g_settings;

AppSettings& GetAppSettings()
{
	return g_settings;
}

AppSettings::AppSettings()
{
	SetDefaults();
}

void AppSettings::SetDefaults()
{
	windowWidth = 280;
	windowHeight = 88;
	maskGray = 128;
	maskOpacity = 39;
}

void AppSettings::Clamp()
{
	if (windowWidth < 220) windowWidth = 220;
	if (windowWidth > 800) windowWidth = 800;
	if (windowHeight < 80) windowHeight = 80;
	if (windowHeight > 600) windowHeight = 600;
	if (maskGray < 0) maskGray = 0;
	if (maskGray > 255) maskGray = 255;
	if (maskOpacity < 0) maskOpacity = 0;
	if (maskOpacity > 100) maskOpacity = 100;
}

CString AppSettings::GetSettingsFilePath() const
{
	TCHAR path[MAX_PATH] = {};
	GetModuleFileName(NULL, path, MAX_PATH);
	CString file(path);
	int slash = file.ReverseFind(_T('\\'));
	if (slash >= 0)
		file = file.Left(slash + 1);
	file += _T("settings.json");
	return file;
}

static int ParseJsonInt(const CString& json, LPCTSTR key, int fallback)
{
	CString pattern;
	pattern.Format(_T("\"%s\""), key);
	int pos = json.Find(pattern);
	if (pos < 0)
		return fallback;
	pos = json.Find(_T(':'), pos);
	if (pos < 0)
		return fallback;
	pos++;
	while (pos < json.GetLength() && (json[pos] == _T(' ') || json[pos] == _T('\t')))
		pos++;
	return _ttoi(json.Mid(pos));
}

BOOL AppSettings::Load()
{
	SetDefaults();
	CString path = GetSettingsFilePath();
	CStdioFile file;
	if (!file.Open(path, CFile::modeRead | CFile::typeText))
		return FALSE;

	CString json, line;
	while (file.ReadString(line))
		json += line;
	file.Close();

	windowWidth = ParseJsonInt(json, _T("windowWidth"), windowWidth);
	windowHeight = ParseJsonInt(json, _T("windowHeight"), windowHeight);
	maskGray = ParseJsonInt(json, _T("maskGray"), maskGray);
	maskOpacity = ParseJsonInt(json, _T("maskOpacity"), maskOpacity);
	Clamp();
	return TRUE;
}

BOOL AppSettings::Save() const
{
	AppSettings temp = *this;
	temp.Clamp();
	CString path = temp.GetSettingsFilePath();
	CString json;
	json.Format(
		_T("{\r\n")
		_T("  \"windowWidth\": %d,\r\n")
		_T("  \"windowHeight\": %d,\r\n")
		_T("  \"maskGray\": %d,\r\n")
		_T("  \"maskOpacity\": %d\r\n")
		_T("}\r\n"),
		temp.windowWidth, temp.windowHeight, temp.maskGray, temp.maskOpacity);

	try
	{
		CStdioFile file(path, CFile::modeCreate | CFile::modeWrite | CFile::typeText);
		file.WriteString(json);
		file.Close();
	}
	catch (CFileException*)
	{
		return FALSE;
	}
	return TRUE;
}
