#pragma once

struct VirtualScreenInfo
{
	int originX;
	int originY;
	int width;
	int height;
};

void QueryVirtualScreen(VirtualScreenInfo* info);
HBITMAP CaptureVirtualDesktop(const VirtualScreenInfo& info);
