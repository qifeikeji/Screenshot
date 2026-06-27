#include "stdafx.h"
#include "DarkButton.h"
#include <GdiPlus.h>
using namespace Gdiplus;

CDarkButton::CDarkButton()
	: m_hover(false)
	, m_pressed(false)
	, m_tracking(false)
	, m_accent(true)
{
}

void CDarkButton::SetAccent(bool accent)
{
	m_accent = accent;
	if (GetSafeHwnd())
		Invalidate();
}

void CDarkButton::PreSubclassWindow()
{
	CButton::PreSubclassWindow();
	ModifyStyle(0, BS_OWNERDRAW | BS_NOTIFY);
}

BEGIN_MESSAGE_MAP(CDarkButton, CButton)
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, &CDarkButton::OnMouseLeave)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CDarkButton::DrawItem(LPDRAWITEMSTRUCT lp)
{
	if (!lp)
		return;
	CDC dc;
	dc.Attach(lp->hDC);
	CRect rc(lp->rcItem);
	const bool pressed = (lp->itemState & ODS_SELECTED) != 0 || m_pressed;
	DrawButton(dc, rc, m_hover, pressed);
	dc.Detach();
}

void CDarkButton::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_tracking)
	{
		TRACKMOUSEEVENT tme = {};
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
		m_tracking = true;
	}
	if (!m_hover)
	{
		m_hover = true;
		Invalidate();
	}
	CButton::OnMouseMove(nFlags, point);
}

LRESULT CDarkButton::OnMouseLeave(WPARAM, LPARAM)
{
	m_tracking = false;
	m_hover = false;
	Invalidate();
	return 0;
}

void CDarkButton::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_pressed = true;
	Invalidate();
	CButton::OnLButtonDown(nFlags, point);
}

void CDarkButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_pressed = false;
	Invalidate();
	CButton::OnLButtonUp(nFlags, point);
}

BOOL CDarkButton::OnEraseBkgnd(CDC* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	return TRUE;
}

void CDarkButton::DrawButton(CDC& dc, const CRect& rc, bool hover, bool pressed)
{
	Graphics g(dc.m_hDC);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	const int radius = 8;
	Color fill(255, 58, 58, 60);
	Color border(255, 72, 72, 74);
	if (m_accent)
	{
		fill = Color(255, 10, 132, 255);
		border = Color(255, 64, 156, 255);
	}
	if (pressed)
		fill = Color(255, 40, 40, 42);
	else if (hover)
		fill = m_accent ? Color(255, 38, 148, 255) : Color(255, 72, 72, 74);

	GraphicsPath path;
	const REAL r = (REAL)radius;
	path.AddArc((REAL)rc.left, (REAL)rc.top, r * 2, r * 2, 180, 90);
	path.AddArc((REAL)rc.right - r * 2, (REAL)rc.top, r * 2, r * 2, 270, 90);
	path.AddArc((REAL)rc.right - r * 2, (REAL)rc.bottom - r * 2, r * 2, r * 2, 0, 90);
	path.AddArc((REAL)rc.left, (REAL)rc.bottom - r * 2, r * 2, r * 2, 90, 90);
	path.CloseFigure();

	SolidBrush brush(fill);
	Pen pen(border, 1.f);
	g.FillPath(&brush, &path);
	g.DrawPath(&pen, &path);

	CString text;
	GetWindowText(text);
	FontFamily ff(L"Segoe UI");
	Font font(&ff, 9.f, FontStyleRegular, UnitPoint);
	SolidBrush textBrush(Color(255, 245, 245, 247));
	StringFormat fmt;
	fmt.SetAlignment(StringAlignmentCenter);
	fmt.SetLineAlignment(StringAlignmentCenter);
	RectF layout((REAL)rc.left, (REAL)rc.top, (REAL)rc.Width(), (REAL)rc.Height());
	g.DrawString(text, -1, &font, layout, &fmt, &textBrush);
}
