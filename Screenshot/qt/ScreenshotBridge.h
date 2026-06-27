#pragma once

#include <windows.h>

void StartScreenshotCapture(HWND mainWindowHwnd);

void RegisterGlobalScreenshotHotKey(HWND hwnd, UINT id);
void UnregisterGlobalScreenshotHotKey(HWND hwnd, UINT id);

void ApplyQtDarkTheme(class QApplication* app);
