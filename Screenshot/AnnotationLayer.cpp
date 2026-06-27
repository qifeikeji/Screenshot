#include "stdafx.h"
#include "AnnotationLayer.h"

#pragma comment(lib, "Msimg32.lib")

CAnnotationLayer::CAnnotationLayer()
	: m_hBitmap(NULL)
	, m_size(0, 0)
{
}

CAnnotationLayer::~CAnnotationLayer()
{
	ReleaseBitmap();
}

void CAnnotationLayer::ReleaseBitmap()
{
	if (m_hBitmap)
	{
		DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}
	m_size = CSize(0, 0);
}

void CAnnotationLayer::Clear()
{
	ReleaseBitmap();
}

HBITMAP CAnnotationLayer::CreateTransparentBitmap(int cx, int cy) const
{
	if (cx <= 0 || cy <= 0)
		return NULL;

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = cx;
	bmi.bmiHeader.biHeight = -cy;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* pBits = NULL;
	HDC hdcScreen = GetDC(NULL);
	HBITMAP hBmp = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
	ReleaseDC(NULL, hdcScreen);
	if (hBmp && pBits)
		ZeroMemory(pBits, static_cast<SIZE_T>(cx) * cy * 4);
	return hBmp;
}

void CAnnotationLayer::EnsureSize(int cx, int cy)
{
	if (cx <= 0 || cy <= 0)
	{
		Clear();
		return;
	}
	if (m_hBitmap && m_size.cx == cx && m_size.cy == cy)
		return;

	ReleaseBitmap();
	m_hBitmap = CreateTransparentBitmap(cx, cy);
	if (m_hBitmap)
		m_size = CSize(cx, cy);
}

bool CAnnotationLayer::IsValid() const
{
	return m_hBitmap != NULL && m_size.cx > 0 && m_size.cy > 0;
}

void CAnnotationLayer::DrawOn(HDC hdcDest, int destX, int destY) const
{
	if (!IsValid())
		return;

	HDC hdcSrc = CreateCompatibleDC(hdcDest);
	if (!hdcSrc)
		return;

	HGDIOBJ hOld = SelectObject(hdcSrc, m_hBitmap);
	BLENDFUNCTION blend = {};
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = 255;
	blend.AlphaFormat = AC_SRC_ALPHA;
	AlphaBlend(hdcDest, destX, destY, m_size.cx, m_size.cy,
		hdcSrc, 0, 0, m_size.cx, m_size.cy, blend);
	SelectObject(hdcSrc, hOld);
	DeleteDC(hdcSrc);
}
