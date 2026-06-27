#include "stdafx.h"
#include "ScreenCapture.h"
#include <shellscalingapi.h>

#pragma comment(lib, "Shcore.lib")

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

namespace {

struct MonitorDpiCtx
{
	UINT firstDpiX;
	UINT firstDpiY;
	BOOL hasMonitor;
	BOOL mixed;
};

static POINT g_captureCursorAnchor = { INT_MIN, INT_MIN };

static BOOL CALLBACK MonitorDpiProc(HMONITOR hMonitor, HDC, LPRECT, LPARAM lp)
{
	MonitorDpiCtx* ctx = reinterpret_cast<MonitorDpiCtx*>(lp);
	UINT dpiX = 0;
	UINT dpiY = 0;
	if (FAILED(GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY)))
		return TRUE;
	if (!ctx->hasMonitor)
	{
		ctx->firstDpiX = dpiX;
		ctx->firstDpiY = dpiY;
		ctx->hasMonitor = TRUE;
		return TRUE;
	}
	if (dpiX != ctx->firstDpiX || dpiY != ctx->firstDpiY)
		ctx->mixed = TRUE;
	return TRUE;
}

} // namespace

void SetCaptureCursorAnchor(POINT screenPt)
{
	g_captureCursorAnchor = screenPt;
}

BOOL GetCaptureCursorAnchor(POINT* outScreenPt)
{
	if (!outScreenPt || g_captureCursorAnchor.x == INT_MIN)
		return FALSE;
	*outScreenPt = g_captureCursorAnchor;
	return TRUE;
}

BOOL HasMixedMonitorScaling()
{
	MonitorDpiCtx ctx = {};
	ctx.hasMonitor = FALSE;
	ctx.mixed = FALSE;
	EnumDisplayMonitors(NULL, NULL, MonitorDpiProc, reinterpret_cast<LPARAM>(&ctx));
	return ctx.mixed;
}

// Full virtual-desktop bitmaps use DIB sections (not CreateCompatibleBitmap on the
// primary display DC) so width is not capped by the main GPU/GDI surface (~900px gaps).

BOOL GetBitmapPixelSize(HBITMAP hbmp, int* outWidth, int* outHeight)
{
	if (!hbmp || !outWidth || !outHeight)
		return FALSE;
	BITMAP bm = {};
	if (::GetObject(hbmp, sizeof(bm), &bm) == 0)
		return FALSE;
	*outWidth = bm.bmWidth;
	*outHeight = (bm.bmHeight < 0) ? -bm.bmHeight : bm.bmHeight;
	return TRUE;
}

static HBITMAP CreateScreenDibBitmap(int width, int height)
{
	if (width <= 0 || height <= 0)
		return NULL;

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* pBits = NULL;
	HDC hdc = GetDC(NULL);
	if (!hdc)
		return NULL;
	HBITMAP hbmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
	ReleaseDC(NULL, hdc);
	if (!hbmp || !pBits)
		return NULL;
	ZeroMemory(pBits, static_cast<SIZE_T>(width) * height * 4);

	int actualW = 0;
	int actualH = 0;
	if (!GetBitmapPixelSize(hbmp, &actualW, &actualH) || actualW != width || actualH != height)
	{
		DeleteObject(hbmp);
		return NULL;
	}
	return hbmp;
}

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
	if (c->screen && w > 0 && h > 0)
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
	HBITMAP hBitmap = CreateScreenDibBitmap(info.width, info.height);
	if (!hBitmap)
	{
		ReleaseDC(NULL, hScreen);
		return NULL;
	}
	HDC hMemDC = CreateCompatibleDC(hScreen);
	if (!hMemDC)
	{
		DeleteObject(hBitmap);
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
	HBITMAP hBitmap = CreateScreenDibBitmap(info.width, info.height);
	if (!hBitmap)
	{
		ReleaseDC(NULL, hScreen);
		return NULL;
	}
	HDC hMemDC = CreateCompatibleDC(hScreen);
	if (!hMemDC)
	{
		DeleteObject(hBitmap);
		ReleaseDC(NULL, hScreen);
		return NULL;
	}
	HGDIOBJ hOld = SelectObject(hMemDC, hBitmap);

	CaptureCtx cap = { hMemDC, hScreen, info.originX, info.originY };
	EnumDisplayMonitors(NULL, NULL, MonitorCaptureProc, reinterpret_cast<LPARAM>(&cap));

	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);
	ReleaseDC(NULL, hScreen);
	return hBitmap;
}
