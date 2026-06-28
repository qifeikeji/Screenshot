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

#include <QMessageBox>
#include <QAbstractButton>

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

void ShowAlreadyRunningMessage()
{
	QMessageBox box;
	box.setWindowTitle(QStringLiteral("\u622a\u56fe\u5de5\u5177"));
	box.setIcon(QMessageBox::Information);
	box.setText(QStringLiteral("\u8f6f\u4ef6\u5df2\u5728\u8fd0\u884c\u4e2d\u3002"));
	box.setInformativeText(QStringLiteral("\u8bf7\u4ece\u901a\u77e5\u533a\u57df\u56fe\u6807\u6216\u5df2\u6253\u5f00\u7684\u4e3b\u7a97\u53e3\u4f7f\u7528\u622a\u56fe\u5de5\u5177\u3002"));
	box.setStandardButtons(QMessageBox::Ok);
	box.setDefaultButton(QMessageBox::Ok);
	if (QAbstractButton* ok = box.button(QMessageBox::Ok))
		ok->setText(QStringLiteral("\u786e\u5b9a"));
	box.setWindowModality(Qt::ApplicationModal);
	box.exec();
}
