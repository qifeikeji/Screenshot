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
	, m_selectedColorIndex(0)
	, m_selectedThicknessIndex(0)
	, m_trackingLeave(false)
{
}

CDarkToolBar::~CDarkToolBar()
{
	if (m_hImageList)
		ImageList_Destroy(m_hImageList);
}

int CDarkToolBar::ToolbarWidth() const
{
	return kPadding * 2 + kRow1Count * kButtonSize + (kRow1Count - 1) * 4;
}

int CDarkToolBar::ToolbarHeight() const
{
	return kPadding * 2 + kButtonSize * 2 + kRowGap;
}

BOOL CDarkToolBar::Create(CWnd* pParent)
{
	const int w = ToolbarWidth();
	const int h = ToolbarHeight();
	if (!CreateEx(0, AfxRegisterWndClass(CS_DBLCLKS),
		_T("DarkToolBar"), WS_CHILD | WS_CLIPSIBLINGS, 0, 0, w, h, pParent->GetSafeHwnd(), NULL))
		return FALSE;

	m_hImageList = ImageList_Create(18, 18, ILC_COLOR32, 10, 1);
	static const UINT kIconIds[10] = {
		IDB_UNDO, IDB_ARROW, IDB_RECTANGLE, IDB_CIRCLE, IDB_BRUSH,
		IDB_TEXT, IDB_SAVE, IDB_FOLDER, IDB_EXIT, IDB_FINISH
	};
	for (int i = 0; i < 10; ++i)
	{
		Bitmap* pImage = NULL;
		if (ImageFromIDResource(kIconIds[i], _T("PNG"), pImage) && pImage)
		{
			HBITMAP hb = NULL;
			pImage->GetHBitmap(Color(0, 0, 0, 0), &hb);
			ImageList_Add(m_hImageList, hb, NULL);
			DeleteObject(hb);
			delete pImage;
		}
	}
	return TRUE;
}

void CDarkToolBar::SetSelectedColorIndex(int index)
{
	if (index < 0 || index > 3)
		return;
	if (m_selectedColorIndex == index)
		return;
	const int prev = m_selectedColorIndex;
	m_selectedColorIndex = index;
	InvalidateButton(DarkToolBarBtnColorRed + prev);
	InvalidateButton(DarkToolBarBtnColorRed + index);
}

void CDarkToolBar::SetSelectedThicknessIndex(int index)
{
	if (index < 0 || index > 1)
		return;
	if (m_selectedThicknessIndex == index)
		return;
	const int prev = m_selectedThicknessIndex;
	m_selectedThicknessIndex = index;
	InvalidateButton(DarkToolBarBtnThickSmall + prev);
	InvalidateButton(DarkToolBarBtnThickSmall + index);
}

void CDarkToolBar::SetBelowSelectionClient(const CRect& selectionClient, const CRect& parentClient)
{
	if (!m_hWnd)
		return;

	CRect sel = selectionClient;
	sel.NormalizeRect();
	if (sel.IsRectEmpty())
		return;

	const int w = ToolbarWidth();
	const int h = ToolbarHeight();
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
	if (index < 0 || index >= DarkToolBarBtnCount)
		return btn;
	int row = 0;
	int col = index;
	if (index >= kRow1Count)
	{
		row = 1;
		col = index - kRow1Count;
	}
	const int y = kPadding + row * (kButtonSize + kRowGap);
	const int x = kPadding + col * (kButtonSize + 4);
	btn.SetRect(x, y, x + kButtonSize, y + kButtonSize);
	return btn;
}

void CDarkToolBar::InvalidateButton(int index)
{
	if (index < 0 || index >= DarkToolBarBtnCount)
		return;
	CRect r = ButtonRect(index);
	r.InflateRect(3, 3);
	InvalidateRect(&r, FALSE);
}

void CDarkToolBar::InvalidateHoverRegion(int prevIndex, int newIndex)
{
	CRect dirty;
	if (prevIndex >= 0)
	{
		dirty = ButtonRect(prevIndex);
		dirty.InflateRect(3, 3);
	}
	if (newIndex >= 0)
	{
		CRect r = ButtonRect(newIndex);
		r.InflateRect(3, 3);
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
	for (int i = 0; i < DarkToolBarBtnCount; ++i)
	{
		if (ButtonRect(i).PtInRect(pt))
			return i;
	}
	return -1;
}

namespace {

const Color kSelectBorderColor(255, 51, 153, 255);
constexpr float kSelectBorderWidth = 3.f;

void DrawButtonChrome(Graphics& g, const CRect& rc, bool hover, bool pressed, bool selected)
{
	RectF rect((REAL)rc.left, (REAL)rc.top, (REAL)rc.Width(), (REAL)rc.Height());
	Color fill(220, 44, 44, 46);
	if (pressed)
		fill = Color(240, 28, 28, 30);
	else if (hover)
		fill = Color(240, 72, 72, 74);
	SolidBrush brush(fill);
	Pen pen(Color(180, 90, 90, 92), 1.f);
	g.FillRectangle(&brush, rect);
	g.DrawRectangle(&pen, rect);
	if (selected)
	{
		Pen selPen(kSelectBorderColor, kSelectBorderWidth);
		g.DrawRectangle(&selPen, rect);
	}
}

void DrawIconButton(Graphics& g, HIMAGELIST imageList, int imageIndex, const CRect& rc, bool hover,
	bool pressed, bool selected)
{
	DrawButtonChrome(g, rc, hover, pressed, selected);
	if (!imageList || imageIndex < 0)
		return;
	HICON hIcon = ImageList_GetIcon(imageList, imageIndex, ILD_TRANSPARENT);
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

void DrawColorButton(Graphics& g, COLORREF color, const CRect& rc, bool hover, bool pressed, bool selected)
{
	DrawButtonChrome(g, rc, hover, pressed, selected);
	const int pad = 5;
	RectF swatch((REAL)(rc.left + pad), (REAL)(rc.top + pad),
		(REAL)(rc.Width() - pad * 2), (REAL)(rc.Height() - pad * 2));
	SolidBrush fill(Color(255, GetRValue(color), GetGValue(color), GetBValue(color)));
	g.FillRectangle(&fill, swatch);
	Pen edge(Color(160, 120, 120, 120), 1.f);
	g.DrawRectangle(&edge, swatch);
}

void DrawThicknessButton(Graphics& g, int dotDiameterPx, const CRect& rc, bool hover, bool pressed,
	bool selected)
{
	DrawButtonChrome(g, rc, hover, pressed, selected);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	const float cx = (rc.left + rc.right) * 0.5f;
	const float cy = (rc.top + rc.bottom) * 0.5f;
	const float r = dotDiameterPx * 0.5f;
	SolidBrush dotBrush(Color(255, 51, 153, 255));
	g.FillEllipse(&dotBrush, cx - r, cy - r, r * 2.f, r * 2.f);
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

	static const COLORREF kColors[4] = {
		RGB(255, 60, 60),
		RGB(255, 220, 0),
		RGB(60, 120, 255),
		RGB(255, 255, 255)
	};
	static const int kThickDots[2] = { 3, 6 };

	for (int i = 0; i < DarkToolBarBtnCount; ++i)
	{
		const CRect btn = ButtonRect(i);
		const bool hover = (i == m_hoverIndex);
		const bool pressed = (i == m_pressedIndex);

		if (i < DarkToolBarBtnColorRed)
		{
			DrawIconButton(g, m_hImageList, i, btn, hover, pressed, false);
		}
		else if (i >= DarkToolBarBtnColorRed && i <= DarkToolBarBtnColorWhite)
		{
			const int ci = i - DarkToolBarBtnColorRed;
			DrawColorButton(g, kColors[ci], btn, hover, pressed, ci == m_selectedColorIndex);
		}
		else if (i >= DarkToolBarBtnThickSmall && i <= DarkToolBarBtnThickLarge)
		{
			const int ti = i - DarkToolBarBtnThickSmall;
			DrawThicknessButton(g, kThickDots[ti], btn, hover, pressed, ti == m_selectedThicknessIndex);
		}
		else if (i == DarkToolBarBtnExit)
			DrawIconButton(g, m_hImageList, 8, btn, hover, pressed, false);
		else if (i == DarkToolBarBtnFinish)
			DrawIconButton(g, m_hImageList, 9, btn, hover, pressed, false);
	}

	dc.BitBlt(0, 0, rc.Width(), rc.Height(), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBmp);
}

void CDarkToolBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	int hit = m_pressedIndex;
	if (hit < 0)
		hit = HitTest(point);
	if (hit >= 0 && GetParent())
	{
		if (hit >= DarkToolBarBtnColorRed && hit <= DarkToolBarBtnColorWhite)
			SetSelectedColorIndex(hit - DarkToolBarBtnColorRed);
		else if (hit >= DarkToolBarBtnThickSmall && hit <= DarkToolBarBtnThickLarge)
			SetSelectedThicknessIndex(hit - DarkToolBarBtnThickSmall);

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
