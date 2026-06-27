#pragma once

struct VirtualScreenInfo
{
	int originX;
	int originY;
	int width;
	int height;
};

void QueryVirtualScreen(VirtualScreenInfo* info);
BOOL QueryMonitorAtPoint(POINT screenPt, VirtualScreenInfo* info);
BOOL GetBitmapPixelSize(HBITMAP hbmp, int* outWidth, int* outHeight);
HBITMAP CaptureScreenRect(const VirtualScreenInfo& info);
HBITMAP CaptureVirtualDesktop(const VirtualScreenInfo& info);
