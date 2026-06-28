#pragma once

CString GetExecutablePath();
BOOL IsRunAtStartup();
BOOL SetRunAtStartup(BOOL enable);
void ApplyLaunchAtStartupSetting(BOOL shouldRun);

CString GetDefaultSaveDirectory();
BOOL EnsureDirectoryExists(const CString& dir);
void OpenFolderInExplorer(const CString& dir);

BOOL TryBeginSingleInstance();
void ActivateExistingScreenshotInstance();
void ReleaseSingleInstanceMutex();
