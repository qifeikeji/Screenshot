#include "stdafx.h"
#include "AppSettings.h"
#include "ScreenshotBridge.h"
#include "CatchScreenDlg.h"
#include "ScreenCapture.h"
#include "Screenshot.h"

#include <QApplication>
#include <QGuiApplication>
#include <QPalette>
#include <QStyleFactory>
#include <QColor>

#include <QColor>

namespace {

struct ScreenshotCaptureParams
{
	HWND hwndMain = NULL;
	BOOL restoreMainWindowVisible = FALSE;
};

static UINT ScreenshotCaptureThreadProc(LPVOID param)
{
	ScreenshotCaptureParams* ctx = reinterpret_cast<ScreenshotCaptureParams*>(param);
	::Sleep(80);
	CCatchScreenDlg dlg;
	dlg.DoModal();
	if (ctx)
	{
		const HWND hwnd = ctx->hwndMain;
		if (hwnd && ::IsWindow(hwnd) && ctx->restoreMainWindowVisible)
			::ShowWindow(hwnd, SW_SHOW);
		delete ctx;
	}
	return 0;
}

} // namespace

void StartScreenshotCapture(HWND mainWindowHwnd)
{
	if (!mainWindowHwnd || !::IsWindow(mainWindowHwnd))
		return;

	POINT cursor = {};
	::GetCursorPos(&cursor);
	SetCaptureCursorAnchor(cursor);

	const BOOL wasVisible = ::IsWindowVisible(mainWindowHwnd);
	if (wasVisible)
		::ShowWindow(mainWindowHwnd, SW_HIDE);

	auto* ctx = new ScreenshotCaptureParams();
	ctx->hwndMain = mainWindowHwnd;
	ctx->restoreMainWindowVisible = wasVisible;

	AfxBeginThread(ScreenshotCaptureThreadProc, ctx);
}

void RegisterGlobalScreenshotHotKey(HWND hwnd, UINT id)
{
	if (!hwnd)
		return;
	const AppSettings& s = GetAppSettings();
	::UnregisterHotKey(hwnd, id);
	::RegisterHotKey(hwnd, id, s.hotkeyModifiers, s.hotkeyVk);
}

void UnregisterGlobalScreenshotHotKey(HWND hwnd, UINT id)
{
	if (hwnd)
		::UnregisterHotKey(hwnd, id);
}

void ApplyQtDarkTheme(QApplication* app)
{
	if (!app)
		return;
	app->setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
	QPalette pal;
	pal.setColor(QPalette::Window, QColor(28, 28, 30));
	pal.setColor(QPalette::WindowText, QColor(235, 235, 235));
	pal.setColor(QPalette::Base, QColor(45, 45, 48));
	pal.setColor(QPalette::AlternateBase, QColor(38, 38, 40));
	pal.setColor(QPalette::Text, QColor(240, 240, 242));
	pal.setColor(QPalette::Button, QColor(58, 58, 60));
	pal.setColor(QPalette::ButtonText, QColor(245, 245, 247));
	pal.setColor(QPalette::Highlight, QColor(10, 132, 255));
	pal.setColor(QPalette::HighlightedText, Qt::white);
	pal.setColor(QPalette::PlaceholderText, QColor(160, 160, 165));
	app->setPalette(pal);
}
