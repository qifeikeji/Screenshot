#include "StdAfx.h"
#include "DarkToolBar.h"
#include <GdiPlus.h>
#include "resource.h"
#include "ScreenCapture.h"

using namespace Gdiplus;

static BOOL ImageFromIDResource(UINT nID, LPCTSTR sTR, Bitmap*& pImg)
{
	HINSTANCE hInst = AfxGetResourceHandle();
	HRSRC hRsrc = FindResource(hInst, MAKEINTRESOURCE(nID), sTR);
	if (!hRsrc)
		return FALSE;
	DWORD len = SizeofResource(hInst, hRsrc);
	BYTE* lpRsrc = (BYTE*)LoadResource(hInst, hRsrc);
	if (!lpRsrc)
		return FALSE;
	HGLOBAL m_hMem = GlobalAlloc(GMEM_FIXED, len);
	BYTE* pmem = (BYTE*)GlobalLock(m_hMem);
	memcpy(pmem, lpRsrc, len);
	IStream* pstm = NULL;
	CreateStreamOnHGlobal(m_hMem, FALSE, &pstm);
	pImg = Bitmap::FromStream(pstm);
	GlobalUnlock(m_hMem);
	pstm->Release();
	FreeResource(lpRsrc);
	return pImg != NULL;
}

CDarkToolBar::CDarkToolBar()
	: m_hImageList(NULL)
	, m_hoverIndex(-1)
	, m_pressedIndex(-1)
	, m_trackingLeave(false)
{
}

CDarkToolBar::~CDarkToolBar()
{
	if (m_hImageList)
		ImageList_Destroy(m_hImageList);
}

BOOL CDarkToolBar::Create(CWnd* pParent)
{
	const int w = kPadding * 2 + kButtonCount * kButtonSize + (kButtonCount - 1) * 4;
	const int h = kPadding * 2 + kButtonSize;
	if (!CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, AfxRegisterWndClass(CS_DBLCLKS),
		_T("DarkToolBar"), WS_POPUP | WS_VISIBLE, 0, 0, w, h, pParent->GetSafeHwnd(), NULL))
		return FALSE;

	m_hImageList = ImageList_Create(18, 18, ILC_COLOR32, kButtonCount, 1);
	for (int i = 0; i < kButtonCount; ++i)
	{
		Bitmap* pImage = NULL;
		if (ImageFromIDResource(IDB_RECTANGLE + i, _T("PNG"), pImage) && pImage)
		{
			HBITMAP hb = NULL;
			pImage->GetHBITMAP(Color(0, 0, 0, 0), &hb);
			ImageList_Add(m_hImageList, hb, NULL);
			DeleteObject(hb);
			delete pImage;
		}
	}
	return TRUE;
}

void CDarkToolBar::SetShowPlaceScreen(int screenX, int screenY)
{
	if (!m_hWnd)
		return;

	CRect rc;
	GetWindowRect(&rc);
	const int w = rc.Width();
	const int h = rc.Height();

	int x = screenX - w;
	int y = screenY + 4;

	VirtualScreenInfo vsi = {};
	QueryVirtualScreen(&vsi);
	const int maxX = vsi.originX + vsi.width - w;
	const int maxY = vsi.originY + vsi.height - h;
	if (x < vsi.originX)
		x = vsi.originX;
	if (y < vsi.originY)
		y = vsi.originY;
	if (x > maxX)
		x = maxX;
	if (y > maxY)
		y = maxY;

	SetWindowPos(&wndTop, x, y, w, h, SWP_SHOWWINDOW);
}

void CDarkToolBar::ShowBar()
{
	ShowWindow(SW_SHOW);
}

void CDarkToolBar::HideBar()
{
	ShowWindow(SW_HIDE);
}

int CDarkToolBar::HitTest(CPoint pt) const
{
	CRect rc;
	GetClientRect(&rc);
	if (!rc.PtInRect(pt))
		return -1;
	int x = kPadding;
	for (int i = 0; i < kButtonCount; ++i)
	{
		CRect btn(x, kPadding, x + kButtonSize, kPadding + kButtonSize);
		if (btn.PtInRect(pt))
			return i;
		x += kButtonSize + 4;
	}
	return -1;
}

namespace {

void DrawToolButton(Gdiplus::Graphics& g, HIMAGELIST imageList, const CRect& rc, int index, bool hover, bool pressed)
{
	Gdiplus::GraphicsPath path;
	const Gdiplus::REAL r = 7.f;
	path.AddArc((Gdiplus::REAL)rc.left, (Gdiplus::REAL)rc.top, r * 2, r * 2, 180, 90);
	path.AddArc((Gdiplus::REAL)rc.right - r * 2, (Gdiplus::REAL)rc.top, r * 2, r * 2, 270, 90);
	path.AddArc((Gdiplus::REAL)rc.right - r * 2, (Gdiplus::REAL)rc.bottom - r * 2, r * 2, r * 2, 0, 90);
	path.AddArc((Gdiplus::REAL)rc.left, (Gdiplus::REAL)rc.bottom - r * 2, r * 2, r * 2, 90, 90);
	path.CloseFigure();

	Gdiplus::Color fill(220, 44, 44, 46);
	if (pressed)
		fill = Gdiplus::Color(240, 28, 28, 30);
	else if (hover)
		fill = Gdiplus::Color(240, 72, 72, 74);
	Gdiplus::SolidBrush brush(fill);
	Gdiplus::Pen pen(Gdiplus::Color(180, 90, 90, 92), 1.f);
	g.FillPath(&brush, &path);
	g.DrawPath(&pen, &path);

	if (imageList && index >= 0)
	{
		HICON hIcon = ImageList_GetIcon(imageList, index, ILD_TRANSPARENT);
		if (hIcon)
		{
			HDC hdc = g.GetHDC();
			const int ix = rc.left + (rc.Width() - 18) / 2;
			const int iy = rc.top + (rc.Height() - 18) / 2;
			::DrawIconEx(hdc, ix, iy, hIcon, 18, 18, 0, NULL, DI_NORMAL);
			g.ReleaseHDC(hdc);
			DestroyIcon(hIcon);
		}
	}
}

} // namespace

BEGIN_MESSAGE_MAP(CDarkToolBar, CWnd)
 ON_WM_PAINT()
 ON_WM_LBUTTONDOWN()
 ON_WM_MOUSEMOVE()
 ON_MESSAGE(WM_MOUSELEAVE, &CDarkToolBar::OnMouseLeave)
 ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CDarkToolBar::OnPaint()
{
 CPaintDC dc(this);
 Graphics g(dc.m_hDC);
 g.SetSmoothingMode(SmoothingModeAntiAlias);
 CRect rc;
 GetClientRect(&rc);
 GraphicsPath bg;
 const REAL r = 10.f;
 bg.AddArc((REAL)rc.left, (REAL)rc.top, r * 2, r * 2, 180, 90);
 bg.AddArc((REAL)rc.right - r * 2, (REAL)rc.top, r * 2, r * 2, 270, 90);
 bg.AddArc((REAL)rc.right - r * 2, (REAL)rc.bottom - r * 2, r * 2, r * 2, 0, 90);
 bg.AddArc((REAL)rc.left, (REAL)rc.bottom - r * 2, r * 2, r * 2, 90, 90);
 bg.CloseFigure();
 SolidBrush panel(Color(235, 36, 36, 38));
 Pen border(Color(200, 58, 58, 60), 1.f);
 g.FillPath(&panel, &bg);
 g.DrawPath(&border, &bg);

 int x = kPadding;
 for (int i = 0; i < kButtonCount; ++i)
 {
  CRect btn(x, kPadding, x + kButtonSize, kPadding + kButtonSize);
  DrawToolButton(g, m_hImageList, btn, i, i == m_hoverIndex, i == m_pressedIndex);
  x += kButtonSize + 4;
 }
}

void CDarkToolBar::OnLButtonDown(UINT nFlags, CPoint point)
{
 int hit = HitTest(point);
 if (hit >= 0 && GetParent())
 {
  m_pressedIndex = hit;
  Invalidate();
  GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(DarkToolBar_CommandBase + hit, BN_CLICKED),
   (LPARAM)m_hWnd);
 }
 CWnd::OnLButtonDown(nFlags, point);
}

void CDarkToolBar::OnMouseMove(UINT nFlags, CPoint point)
{
 if (!m_trackingLeave)
 {
  TRACKMOUSEEVENT tme = {};
  tme.cbSize = sizeof(tme);
  tme.dwFlags = TME_LEAVE;
  tme.hwndTrack = m_hWnd;
  TrackMouseEvent(&tme);
  m_trackingLeave = true;
 }
 int hit = HitTest(point);
 if (hit != m_hoverIndex)
 {
  m_hoverIndex = hit;
  Invalidate();
 }
 CWnd::OnMouseMove(nFlags, point);
}

LRESULT CDarkToolBar::OnMouseLeave(WPARAM, LPARAM)
{
 m_trackingLeave = false;
 m_hoverIndex = -1;
 m_pressedIndex = -1;
 Invalidate();
 return 0;
}

BOOL CDarkToolBar::OnEraseBkgnd(CDC* pDC)
{
 UNREFERENCED_PARAMETER(pDC);
 return TRUE;
}
