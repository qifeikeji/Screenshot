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
	hotkeyModifiers = MOD_CONTROL | MOD_ALT;
	hotkeyVk = (UINT)'A';
	copyAndExitAfterSelect = FALSE;
	singleMonitorCapture = FALSE;
	saveDirectory.Empty();
}

void AppSettings::Clamp()
{
	if (windowWidth < 220) windowWidth = 220;
	if (windowWidth > 800) windowWidth = 800;
	if (windowHeight < 72) windowHeight = 72;
	if (windowHeight > 600) windowHeight = 600;
	if (hotkeyVk == 0)
		hotkeyVk = (UINT)'A';
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

static BOOL ParseJsonBool(const CString& json, LPCTSTR key, BOOL fallback)
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
	if (json.Mid(pos, 4).CompareNoCase(_T("true")) == 0)
		return TRUE;
	if (json.Mid(pos, 5).CompareNoCase(_T("false")) == 0)
		return FALSE;
	return _ttoi(json.Mid(pos)) != 0;
}

static CString ParseJsonString(const CString& json, LPCTSTR key, const CString& fallback)
{
	CString pattern;
	pattern.Format(_T("\"%s\""), key);
	int pos = json.Find(pattern);
	if (pos < 0)
		return fallback;
	pos = json.Find(_T(':'), pos);
	if (pos < 0)
		return fallback;
	pos = json.Find(_T('\"'), pos);
	if (pos < 0)
		return fallback;
	const int start = pos + 1;
	const int end = json.Find(_T('\"'), start);
	if (end < 0)
		return fallback;
	return json.Mid(start, end - start);
}

static CString JsonEscape(const CString& s)
{
	CString out;
	for (int i = 0; i < s.GetLength(); ++i)
	{
		const TCHAR c = s[i];
		if (c == _T('\\') || c == _T('\"'))
			out += _T('\\');
		out += c;
	}
	return out;
}

void HotKeyToHotKeyCtrl(UINT mod, UINT vk, WORD& hotKeyWord)
{
	BYTE hotkeyF = 0;
	if (mod & MOD_SHIFT)
		hotkeyF |= HOTKEYF_SHIFT;
	if (mod & MOD_CONTROL)
		hotkeyF |= HOTKEYF_CONTROL;
	if (mod & MOD_ALT)
		hotkeyF |= HOTKEYF_ALT;
	hotKeyWord = MAKEWORD((BYTE)vk, hotkeyF);
}

void HotKeyFromHotKeyCtrl(WORD hotKeyWord, UINT& mod, UINT& vk)
{
	vk = LOBYTE(hotKeyWord);
	mod = 0;
	const BYTE hotkeyF = HIBYTE(hotKeyWord);
	if (hotkeyF & HOTKEYF_SHIFT)
		mod |= MOD_SHIFT;
	if (hotkeyF & HOTKEYF_CONTROL)
		mod |= MOD_CONTROL;
	if (hotkeyF & HOTKEYF_ALT)
		mod |= MOD_ALT;
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
	hotkeyModifiers = (UINT)ParseJsonInt(json, _T("hotkeyModifiers"), (int)hotkeyModifiers);
	hotkeyVk = (UINT)ParseJsonInt(json, _T("hotkeyVk"), (int)hotkeyVk);
	copyAndExitAfterSelect = ParseJsonBool(json, _T("copyAndExitAfterSelect"), copyAndExitAfterSelect);
	singleMonitorCapture = ParseJsonBool(json, _T("singleMonitorCapture"), singleMonitorCapture);
	saveDirectory = ParseJsonString(json, _T("saveDirectory"), saveDirectory);
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
		_T("  \"hotkeyModifiers\": %u,\r\n")
		_T("  \"hotkeyVk\": %u,\r\n")
		_T("  \"copyAndExitAfterSelect\": %s,\r\n")
		_T("  \"singleMonitorCapture\": %s,\r\n")
		_T("  \"saveDirectory\": \"%s\"\r\n")
		_T("}\r\n"),
		temp.windowWidth, temp.windowHeight, temp.hotkeyModifiers, temp.hotkeyVk,
		temp.copyAndExitAfterSelect ? _T("true") : _T("false"),
		temp.singleMonitorCapture ? _T("true") : _T("false"),
		(LPCTSTR)JsonEscape(temp.saveDirectory));

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
