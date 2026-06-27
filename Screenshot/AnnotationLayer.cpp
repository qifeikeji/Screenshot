#include "stdafx.h"
#include "AnnotationLayer.h"
#include <cmath>

CAnnotationLayer::CAnnotationLayer()
	: m_hBitmap(NULL)
	, m_size(0, 0)
	, m_pBits(NULL)
{
}

CAnnotationLayer::~CAnnotationLayer()
{
	Clear();
}

void CAnnotationLayer::ReleaseBitmap()
{
	if (m_hBitmap)
	{
		DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}
	m_size = CSize(0, 0);
	m_pBits = NULL;
}

void CAnnotationLayer::Clear()
{
	for (HBITMAP hb : m_undoStack)
	{
		if (hb)
			DeleteObject(hb);
	}
	m_undoStack.clear();
	ReleaseBitmap();
}

HBITMAP CAnnotationLayer::CreateTransparentBitmap(int cx, int cy)
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

void CAnnotationLayer::RefreshBitsPointer()
{
	m_pBits = NULL;
	if (!m_hBitmap)
		return;
	DIBSECTION ds = {};
	if (GetObject(m_hBitmap, sizeof(DIBSECTION), &ds) == sizeof(DIBSECTION))
		m_pBits = ds.dsBm.bmBits;
}

void CAnnotationLayer::FixAlphaChannel() const
{
	if (!m_pBits || m_size.cx <= 0 || m_size.cy <= 0)
		return;
	DWORD* px = static_cast<DWORD*>(m_pBits);
	const int n = m_size.cx * m_size.cy;
	for (int i = 0; i < n; ++i)
	{
		const DWORD c = px[i];
		const BYTE r = (BYTE)((c >> 16) & 0xFF);
		const BYTE g = (BYTE)((c >> 8) & 0xFF);
		const BYTE b = (BYTE)(c & 0xFF);
		if (r | g | b)
			px[i] = 0xFF000000 | (c & 0x00FFFFFF);
		else
			px[i] = 0;
	}
}

HBITMAP CAnnotationLayer::CloneBitmap(HBITMAP src) const
{
	if (!src)
		return NULL;
	BITMAP bm = {};
	GetObject(src, sizeof(bm), &bm);
	const int w = bm.bmWidth;
	const int h = abs(bm.bmHeight);

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = -h;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;
	void* pBits = NULL;
	HDC hdcScreen = GetDC(NULL);
	HBITMAP dst = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
	ReleaseDC(NULL, hdcScreen);
	if (!dst)
		return NULL;

	HDC hdc = CreateCompatibleDC(NULL);
	HGDIOBJ oldD = SelectObject(hdc, dst);
	HDC hdcS = CreateCompatibleDC(NULL);
	HGDIOBJ oldS = SelectObject(hdcS, src);
	BitBlt(hdc, 0, 0, w, h, hdcS, 0, 0, SRCCOPY);
	SelectObject(hdcS, oldS);
	DeleteDC(hdcS);
	SelectObject(hdc, oldD);
	DeleteDC(hdc);
	return dst;
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

	for (HBITMAP hb : m_undoStack)
	{
		if (hb)
			DeleteObject(hb);
	}
	m_undoStack.clear();
	ReleaseBitmap();
	m_hBitmap = CreateTransparentBitmap(cx, cy);
	if (m_hBitmap)
	{
		m_size = CSize(cx, cy);
		RefreshBitsPointer();
	}
}

bool CAnnotationLayer::IsValid() const
{
	return m_hBitmap != NULL && m_size.cx > 0 && m_size.cy > 0;
}

bool CAnnotationLayer::HasVisibleContent() const
{
	if (!m_pBits || m_size.cx <= 0 || m_size.cy <= 0)
		return false;
	const DWORD* px = static_cast<const DWORD*>(m_pBits);
	const int n = m_size.cx * m_size.cy;
	for (int i = 0; i < n; ++i)
	{
		if (px[i] & 0x00FFFFFF)
			return true;
	}
	return false;
}

HDC CAnnotationLayer::BeginDraw(HGDIOBJ* outOld)
{
	HDC hdc = CreateCompatibleDC(NULL);
	if (!hdc)
		return NULL;
	*outOld = SelectObject(hdc, m_hBitmap);
	return hdc;
}

void CAnnotationLayer::EndDraw(HDC hdc, HGDIOBJ old)
{
	if (!hdc)
		return;
	SelectObject(hdc, old);
	DeleteDC(hdc);
}

void CAnnotationLayer::PushUndoSnapshot()
{
	if (!m_hBitmap)
		return;
	HBITMAP copy = CloneBitmap(m_hBitmap);
	if (!copy)
		return;
	m_undoStack.push_back(copy);
	if (m_undoStack.size() > kMaxUndo)
	{
		DeleteObject(m_undoStack.front());
		m_undoStack.erase(m_undoStack.begin());
	}
}

BOOL CAnnotationLayer::Undo()
{
	if (m_undoStack.empty() || !m_hBitmap)
		return FALSE;
	HBITMAP prev = m_undoStack.back();
	m_undoStack.pop_back();
	HBITMAP replacement = CloneBitmap(prev);
	DeleteObject(prev);
	if (!replacement)
		return FALSE;
	DeleteObject(m_hBitmap);
	m_hBitmap = replacement;
	RefreshBitsPointer();
	return TRUE;
}

void CAnnotationLayer::DrawOn(HDC hdcDest, int destX, int destY) const
{
	if (!IsValid())
		return;

	FixAlphaChannel();

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

void CAnnotationLayer::DrawLine(int x1, int y1, int x2, int y2, int width, COLORREF color)
{
	if (!IsValid())
		return;
	HGDIOBJ old = NULL;
	HDC hdc = BeginDraw(&old);
	if (!hdc)
		return;
	HPEN pen = CreatePen(PS_SOLID, width, color);
	HGDIOBJ oldPen = SelectObject(hdc, pen);
	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);
	SelectObject(hdc, oldPen);
	DeleteObject(pen);
	EndDraw(hdc, old);
}

void CAnnotationLayer::DrawRectangle(int x1, int y1, int x2, int y2, int width, COLORREF color, BOOL filled)
{
	if (!IsValid())
		return;
	CRect r(x1, y1, x2, y2);
	r.NormalizeRect();
	HGDIOBJ old = NULL;
	HDC hdc = BeginDraw(&old);
	if (!hdc)
		return;
	HPEN pen = CreatePen(PS_SOLID, width, color);
	HGDIOBJ oldPen = SelectObject(hdc, pen);
	HBRUSH brush = (HBRUSH)GetStockObject(filled ? NULL_BRUSH : NULL_BRUSH);
	if (filled)
	{
		HBRUSH fillBrush = CreateSolidBrush(color);
		HGDIOBJ oldBr = SelectObject(hdc, fillBrush);
		Rectangle(hdc, r.left, r.top, r.right, r.bottom);
		SelectObject(hdc, oldBr);
		DeleteObject(fillBrush);
	}
	else
	{
		SelectObject(hdc, GetStockObject(NULL_BRUSH));
		Rectangle(hdc, r.left, r.top, r.right, r.bottom);
	}
	SelectObject(hdc, oldPen);
	DeleteObject(pen);
	EndDraw(hdc, old);
}

void CAnnotationLayer::DrawEllipse(int x1, int y1, int x2, int y2, int width, COLORREF color, BOOL filled)
{
	if (!IsValid())
		return;
	CRect r(x1, y1, x2, y2);
	r.NormalizeRect();
	HGDIOBJ old = NULL;
	HDC hdc = BeginDraw(&old);
	if (!hdc)
		return;
	HPEN pen = CreatePen(PS_SOLID, width, color);
	HGDIOBJ oldPen = SelectObject(hdc, pen);
	SelectObject(hdc, GetStockObject(NULL_BRUSH));
	if (filled)
	{
		HBRUSH fillBrush = CreateSolidBrush(color);
		HGDIOBJ oldBr = SelectObject(hdc, fillBrush);
		Ellipse(hdc, r.left, r.top, r.right, r.bottom);
		SelectObject(hdc, oldBr);
		DeleteObject(fillBrush);
	}
	else
		Ellipse(hdc, r.left, r.top, r.right, r.bottom);
	SelectObject(hdc, oldPen);
	DeleteObject(pen);
	EndDraw(hdc, old);
}

void CAnnotationLayer::DrawArrow(int x1, int y1, int x2, int y2, int width, COLORREF color)
{
	DrawLine(x1, y1, x2, y2, width, color);
	const double dx = x2 - x1;
	const double dy = y2 - y1;
	const double len = sqrt(dx * dx + dy * dy);
	if (len < 4.0)
		return;
	const double ux = dx / len;
	const double uy = dy / len;
	const int headLen = 8 + width * 2;
	const double px = -uy;
	const double py = ux;
	const int hx1 = (int)(x2 - ux * headLen + px * headLen * 0.5);
	const int hy1 = (int)(y2 - uy * headLen + py * headLen * 0.5);
	const int hx2 = (int)(x2 - ux * headLen - px * headLen * 0.5);
	const int hy2 = (int)(y2 - uy * headLen - py * headLen * 0.5);
	DrawLine(x2, y2, hx1, hy1, width, color);
	DrawLine(x2, y2, hx2, hy2, width, color);
}

void CAnnotationLayer::DrawTextAt(int x, int y, LPCTSTR text, int height, COLORREF color)
{
	if (!IsValid() || !text || !*text)
		return;
	HGDIOBJ old = NULL;
	HDC hdc = BeginDraw(&old);
	if (!hdc)
		return;
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, color);
	HFONT font = CreateFont(height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
		DEFAULT_PITCH | FF_SWISS, _T("Segoe UI"));
	HGDIOBJ oldFont = SelectObject(hdc, font);
	TextOut(hdc, x, y, text, (int)_tcslen(text));
	SelectObject(hdc, oldFont);
	DeleteObject(font);
	EndDraw(hdc, old);
}
