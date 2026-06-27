#include "stdafx.h"
#include "ScreenCapture.h"

struct MonitorEnumCtx
{
	RECT bounds;
};

struct CaptureCtx
{
	HDC dest;
	int originX;
	int originY;
};

static BOOL CALLBACK MonitorBoundsProc(HMONITOR, HDC, LPRECT lprc, LPARAM lp)
{
	MonitorEnumCtx* ctx = reinterpret_cast<MonitorEnumCtx*>(lp);
	UnionRect(&ctx->bounds, &ctx->bounds, lprc);
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
	HDC hScr = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	if (hScr)
	{
		BitBlt(c->dest, dx, dy, w, h, hScr, mi.rcMonitor.left, mi.rcMonitor.top, SRCCOPY);
		DeleteDC(hScr);
	}
	return TRUE;
}

void QueryVirtualScreen(VirtualScreenInfo* info)
{
	if (!info)
		return;
	MonitorEnumCtx ctx = {};
	ctx.bounds.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
	ctx.bounds.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
	ctx.bounds.right = ctx.bounds.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
	ctx.bounds.bottom = ctx.bounds.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);
	EnumDisplayMonitors(NULL, NULL, MonitorBoundsProc, reinterpret_cast<LPARAM>(&ctx));
	info->originX = ctx.bounds.left;
	info->originY = ctx.bounds.top;
	info->width = ctx.bounds.right - ctx.bounds.left;
	info->height = ctx.bounds.bottom - ctx.bounds.top;
}

HBITMAP CaptureVirtualDesktop(const VirtualScreenInfo& info)
{
	if (info.width <= 0 || info.height <= 0)
		return NULL;

	HDC hScrDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	if (!hScrDC)
		return NULL;
	HDC hMemDC = CreateCompatibleDC(hScrDC);
	if (!hMemDC)
	{
		DeleteDC(hScrDC);
		return NULL;
	}
	HBITMAP hBitmap = CreateCompatibleBitmap(hScrDC, info.width, info.height);
	if (!hBitmap)
	{
		DeleteDC(hMemDC);
		DeleteDC(hScrDC);
		return NULL;
	}
	HGDIOBJ hOld = SelectObject(hMemDC, hBitmap);

	CaptureCtx cap = { hMemDC, info.originX, info.originY };
	EnumDisplayMonitors(NULL, NULL, MonitorCaptureProc, reinterpret_cast<LPARAM>(&cap));

	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);
	DeleteDC(hScrDC);
	return hBitmap;
}
