#include "StdAfx.h"
#include "DarkToolBar.h"
#include <GdiPlus.h>
#include "resource.h"

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
	if (!CreateEx(0, AfxRegisterWndClass(CS_DBLCLKS),
		_T("DarkToolBar"), WS_CHILD | WS_CLIPSIBLINGS, 0, 0, w, h, pParent->GetSafeHwnd(), NULL))
		return FALSE;

	m_hImageList = ImageList_Create(18, 18, ILC_COLOR32, kButtonCount, 1);
	static const UINT kIconIds[kButtonCount] = {
		IDB_UNDO, IDB_ARROW, IDB_RECTANGLE, IDB_CIRCLE, IDB_BRUSH,
		IDB_TEXT, IDB_SAVE, IDB_FOLDER, IDB_EXIT, IDB_FINISH
	};
	for (int i = 0; i < kButtonCount; ++i)
	{
		Bitmap* pImage = NULL;
		if (ImageFromIDResource(kIconIds[i], _T("PNG"), pImage) && pImage)
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

void CDarkToolBar::SetBelowSelectionClient(const CRect& selectionClient, const CRect& parentClient)
{
	if (!m_hWnd)
		return;

	CRect sel = selectionClient;
	sel.NormalizeRect();
	if (sel.IsRectEmpty())
		return;

	const int w = kPadding * 2 + kButtonCount * kButtonSize + (kButtonCount - 1) * 4;
	const int h = kPadding * 2 + kButtonSize;
	const int margin = 6;

	int x = sel.right - w;
	int y = sel.bottom + margin;

	if (x < sel.left)
		x = sel.left;
	if (!parentClient.IsRectEmpty())
	{
		if (x + w > parentClient.right)
			x = parentClient.right - w;
		if (x < parentClient.left)
			x = parentClient.left;
		if (y + h > parentClient.bottom)
			y = sel.top - h - margin;
		if (y < parentClient.top)
			y = parentClient.top;
	}

	SetWindowPos(&CWnd::wndTop, x, y, w, h, SWP_NOACTIVATE);
}

CRect CDarkToolBar::ButtonRect(int index) const
{
	CRect btn;
	if (index < 0 || index >= kButtonCount)
		return btn;
	const int x = kPadding + index * (kButtonSize + 4);
	btn.SetRect(x, kPadding, x + kButtonSize, kPadding + kButtonSize);
	return btn;
}

void CDarkToolBar::InvalidateHoverRegion(int prevIndex, int newIndex)
{
	CRect dirty;
	if (prevIndex >= 0)
	{
		dirty = ButtonRect(prevIndex);
		dirty.InflateRect(2, 2);
	}
	if (newIndex >= 0)
	{
		CRect r = ButtonRect(newIndex);
		r.InflateRect(2, 2);
		if (dirty.IsRectEmpty())
			dirty = r;
		else
			dirty.UnionRect(&dirty, &r);
	}
	if (!dirty.IsRectEmpty())
		InvalidateRect(&dirty, FALSE);
	else
		Invalidate(FALSE);
}

void CDarkToolBar::ShowBar()
{
	ShowWindow(SW_SHOW);
	SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
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
	for (int i = 0; i < kButtonCount; ++i)
	{
		if (ButtonRect(i).PtInRect(pt))
			return i;
	}
	return -1;
}

namespace {

void DrawToolButton(Gdiplus::Graphics& g, HIMAGELIST imageList, const CRect& rc, int index, bool hover, bool pressed)
{
	Gdiplus::RectF rect((Gdiplus::REAL)rc.left, (Gdiplus::REAL)rc.top,
		(Gdiplus::REAL)rc.Width(), (Gdiplus::REAL)rc.Height());
	Gdiplus::Color fill(220, 44, 44, 46);
	if (pressed)
		fill = Gdiplus::Color(240, 28, 28, 30);
	else if (hover)
		fill = Gdiplus::Color(240, 72, 72, 74);
	Gdiplus::SolidBrush brush(fill);
	Gdiplus::Pen pen(Gdiplus::Color(180, 90, 90, 92), 1.f);
	g.FillRectangle(&brush, rect);
	g.DrawRectangle(&pen, rect);

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
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, &CDarkToolBar::OnMouseLeave)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CDarkToolBar::OnPaint()
{
	CPaintDC dc(this);
	CRect rc;
	GetClientRect(&rc);
	if (rc.IsRectEmpty())
		return;

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
	CBitmap* pOldBmp = memDC.SelectObject(&bmp);

	Graphics g(memDC.m_hDC);
	g.SetSmoothingMode(SmoothingModeNone);
	RectF panel((REAL)rc.left, (REAL)rc.top, (REAL)rc.Width(), (REAL)rc.Height());
	SolidBrush panelBrush(Color(235, 36, 36, 38));
	Pen border(Color(200, 58, 58, 60), 1.f);
	g.FillRectangle(&panelBrush, panel);
	g.DrawRectangle(&border, panel);

	for (int i = 0; i < kButtonCount; ++i)
	{
		CRect btn = ButtonRect(i);
		DrawToolButton(g, m_hImageList, btn, i, i == m_hoverIndex, i == m_pressedIndex);
	}

	dc.BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBmp);
}

void CDarkToolBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	const int hit = m_pressedIndex;
	if (hit < 0)
		hit = HitTest(point);
	if (hit >= 0 && GetParent())
	{
		GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(DarkToolBar_CommandBase + hit, BN_CLICKED),
			(LPARAM)m_hWnd);
	}
	m_pressedIndex = -1;
	CWnd::OnLButtonUp(nFlags, point);
}

void CDarkToolBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	int hit = HitTest(point);
	if (hit >= 0)
	{
		m_pressedIndex = hit;
		InvalidateHoverRegion(-1, hit);
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
		const int prev = m_hoverIndex;
		m_hoverIndex = hit;
		InvalidateHoverRegion(prev, hit);
	}
	CWnd::OnMouseMove(nFlags, point);
}

LRESULT CDarkToolBar::OnMouseLeave(WPARAM, LPARAM)
{
	m_trackingLeave = false;
	const int prev = m_hoverIndex;
	m_hoverIndex = -1;
	m_pressedIndex = -1;
	InvalidateHoverRegion(prev, -1);
	return 0;
}

BOOL CDarkToolBar::OnEraseBkgnd(CDC* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	return TRUE;
}
