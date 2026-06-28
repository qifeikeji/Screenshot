#include "stdafx.h"
#include "StartupUtil.h"
#include <ShlObj.h>

namespace {

const wchar_t kRunKeyPath[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
const wchar_t kRunValueName[] = L"ScreenshotTool";

} // namespace

CString GetExecutablePath()
{
	CString path;
	DWORD len = GetModuleFileNameW(nullptr, path.GetBuffer(MAX_PATH), MAX_PATH);
	path.ReleaseBuffer(len);
	return path;
}

BOOL IsRunAtStartup()
{
	HKEY hKey = nullptr;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunKeyPath, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
		return FALSE;

	DWORD type = 0;
	DWORD cb = 0;
	const LSTATUS q = RegQueryValueExW(hKey, kRunValueName, nullptr, &type, nullptr, &cb);
	RegCloseKey(hKey);
	return (q == ERROR_SUCCESS && type == REG_SZ && cb > 2);
}

BOOL SetRunAtStartup(BOOL enable)
{
	HKEY hKey = nullptr;
	if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunKeyPath, 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS)
		return FALSE;

	if (!enable)
	{
		const LSTATUS del = RegDeleteValueW(hKey, kRunValueName);
		RegCloseKey(hKey);
		return (del == ERROR_SUCCESS || del == ERROR_FILE_NOT_FOUND);
	}

	const CString exe = GetExecutablePath();
	CString quoted;
	quoted.Format(_T("\"%s\""), (LPCTSTR)exe);
	const LSTATUS set = RegSetValueExW(hKey, kRunValueName, 0, REG_SZ,
		(const BYTE*)(LPCWSTR)quoted, (DWORD)((quoted.GetLength() + 1) * sizeof(wchar_t)));
	RegCloseKey(hKey);
	return set == ERROR_SUCCESS;
}

void ApplyLaunchAtStartupSetting(BOOL shouldRun)
{
	SetRunAtStartup(shouldRun);
}

CString GetDefaultSaveDirectory()
{
	PWSTR pictures = nullptr;
	CString result;
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Pictures, 0, nullptr, &pictures)) && pictures)
	{
		result = pictures;
		CoTaskMemFree(pictures);
	}
	else
	{
		TCHAR path[MAX_PATH] = {};
		if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_MYPICTURES, nullptr, SHGFP_TYPE_CURRENT, path)))
			result = path;
	}
	if (!result.IsEmpty())
	{
		if (result[result.GetLength() - 1] != _T('\\'))
			result += _T('\\');
		result += _T("Screenshots");
	}
	return result;
}

BOOL EnsureDirectoryExists(const CString& dir)
{
	if (dir.IsEmpty())
		return FALSE;
	CString path = dir;
	path.Trim();
	while (!path.IsEmpty() && (path[path.GetLength() - 1] == _T('\\') || path[path.GetLength() - 1] == _T('/')))
		path = path.Left(path.GetLength() - 1);
	if (path.IsEmpty())
		return FALSE;
	if (GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES)
		return TRUE;
	return SHCreateDirectoryExW(nullptr, path, nullptr) == ERROR_SUCCESS;
}

void OpenFolderInExplorer(const CString& dir)
{
	CString path = dir;
	path.Trim();
	if (path.IsEmpty())
		return;
	EnsureDirectoryExists(path);
	ShellExecuteW(nullptr, L"open", path, nullptr, nullptr, SW_SHOWNORMAL);
}

namespace {

const wchar_t kSingleInstanceMutexName[] = L"Local\\ScreenshotTool_SingleInstance_v1";
const wchar_t kMainWindowTitle[] = L"\u622a\u56fe\u5de5\u5177";
HANDLE g_singleInstanceMutex = nullptr;

} // namespace

BOOL TryBeginSingleInstance()
{
	if (g_singleInstanceMutex)
		return TRUE;
	g_singleInstanceMutex = CreateMutexW(nullptr, TRUE, kSingleInstanceMutexName);
	if (!g_singleInstanceMutex || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		if (g_singleInstanceMutex)
		{
			CloseHandle(g_singleInstanceMutex);
			g_singleInstanceMutex = nullptr;
		}
		return FALSE;
	}
	return TRUE;
}

void ActivateExistingScreenshotInstance()
{
	HWND hwnd = FindWindowW(nullptr, kMainWindowTitle);
	if (!hwnd)
		return;
	if (!IsWindowVisible(hwnd))
		ShowWindow(hwnd, SW_SHOW);
	ShowWindow(hwnd, SW_RESTORE);
	SetForegroundWindow(hwnd);
}

void ReleaseSingleInstanceMutex()
{
	if (g_singleInstanceMutex)
	{
		ReleaseMutex(g_singleInstanceMutex);
		CloseHandle(g_singleInstanceMutex);
		g_singleInstanceMutex = nullptr;
	}
}
