#include "stdafx.h"
#include "ScreenCapture.h"

struct MonitorEnumCtx
{
	RECT bounds;
	BOOL hasMonitor;
};

struct CaptureCtx
{
	HDC dest;
	HDC screen;
	int originX;
	int originY;
};

static BOOL CALLBACK MonitorBoundsProc(HMONITOR, HDC, LPRECT lprc, LPARAM lp)
{
	MonitorEnumCtx* ctx = reinterpret_cast<MonitorEnumCtx*>(lp);
	if (!ctx->hasMonitor)
	{
		ctx->bounds = *lprc;
		ctx->hasMonitor = TRUE;
	}
	else
	{
		UnionRect(&ctx->bounds, &ctx->bounds, lprc);
	}
	return TRUE;
}

static BOOL CALLBACK MonitorCaptureProc(HMONITOR hMon, HDC, LPRECT, LPARAM lp)
{
	CaptureCtx* c = reinterpret_cast<CaptureCtx*>(lp);
	MONITORINFO mi = {};
	mi.cbSize = sizeof(mi);
	if (!GetMonitorInfo(hMon, &mi))
		return TRUE;
	const int w = mi.rcMonitor.right - mi.rcMonitor.left;
	const int h = mi.rcMonitor.bottom - mi.rcMonitor.top;
	const int dx = mi.rcMonitor.left - c->originX;
	const int dy = mi.rcMonitor.top - c->originY;
	if (c->screen)
	{
		BitBlt(c->dest, dx, dy, w, h, c->screen, mi.rcMonitor.left, mi.rcMonitor.top, SRCCOPY);
	}
	return TRUE;
}

void QueryVirtualScreen(VirtualScreenInfo* info)
{
	if (!info)
		return;

	MonitorEnumCtx ctx = {};
	ctx.hasMonitor = FALSE;
	EnumDisplayMonitors(NULL, NULL, MonitorBoundsProc, reinterpret_cast<LPARAM>(&ctx));

	if (ctx.hasMonitor)
	{
		info->originX = ctx.bounds.left;
		info->originY = ctx.bounds.top;
		info->width = ctx.bounds.right - ctx.bounds.left;
		info->height = ctx.bounds.bottom - ctx.bounds.top;
		return;
	}

	info->originX = GetSystemMetrics(SM_XVIRTUALSCREEN);
	info->originY = GetSystemMetrics(SM_YVIRTUALSCREEN);
	info->width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	info->height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
}

BOOL QueryMonitorAtPoint(POINT screenPt, VirtualScreenInfo* info)
{
	if (!info)
		return FALSE;
	HMONITOR hMon = MonitorFromPoint(screenPt, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi = {};
	mi.cbSize = sizeof(mi);
	if (!GetMonitorInfo(hMon, &mi))
		return FALSE;
	info->originX = mi.rcMonitor.left;
	info->originY = mi.rcMonitor.top;
	info->width = mi.rcMonitor.right - mi.rcMonitor.left;
	info->height = mi.rcMonitor.bottom - mi.rcMonitor.top;
	return TRUE;
}

HBITMAP CaptureScreenRect(const VirtualScreenInfo& info)
{
	if (info.width <= 0 || info.height <= 0)
		return NULL;

	HDC hScreen = GetDC(NULL);
	if (!hScreen)
		return NULL;
	HDC hMemDC = CreateCompatibleDC(hScreen);
	if (!hMemDC)
	{
		ReleaseDC(NULL, hScreen);
		return NULL;
	}
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, info.width, info.height);
	if (!hBitmap)
	{
		DeleteDC(hMemDC);
		ReleaseDC(NULL, hScreen);
		return NULL;
	}
	HGDIOBJ hOld = SelectObject(hMemDC, hBitmap);
	BitBlt(hMemDC, 0, 0, info.width, info.height, hScreen, info.originX, info.originY, SRCCOPY);
	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);
	ReleaseDC(NULL, hScreen);
	return hBitmap;
}

HBITMAP CaptureVirtualDesktop(const VirtualScreenInfo& info)
{
	if (info.width <= 0 || info.height <= 0)
		return NULL;

	HDC hScreen = GetDC(NULL);
	if (!hScreen)
		return NULL;
	HDC hMemDC = CreateCompatibleDC(hScreen);
	if (!hMemDC)
	{
		ReleaseDC(NULL, hScreen);
		return NULL;
	}
	HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, info.width, info.height);
	if (!hBitmap)
	{
		DeleteDC(hMemDC);
		ReleaseDC(NULL, hScreen);
		return NULL;
	}
	HGDIOBJ hOld = SelectObject(hMemDC, hBitmap);

	RECT fill = { 0, 0, info.width, info.height };
	FillRect(hMemDC, &fill, (HBRUSH)GetStockObject(BLACK_BRUSH));

	CaptureCtx cap = { hMemDC, hScreen, info.originX, info.originY };
	EnumDisplayMonitors(NULL, NULL, MonitorCaptureProc, reinterpret_cast<LPARAM>(&cap));

	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);
	ReleaseDC(NULL, hScreen);
	return hBitmap;
}
