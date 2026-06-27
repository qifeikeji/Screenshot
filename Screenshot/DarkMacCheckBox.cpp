#include "stdafx.h"
#include "DarkMacCheckBox.h"
#include <GdiPlus.h>
using namespace Gdiplus;

CDarkMacCheckBox::CDarkMacCheckBox()
{
}

void CDarkMacCheckBox::PreSubclassWindow()
{
	ModifyStyle(BS_AUTOCHECKBOX | BS_AUTO3STATE, BS_OWNERDRAW);
	CButton::PreSubclassWindow();
}

BEGIN_MESSAGE_MAP(CDarkMacCheckBox, CButton)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

void CDarkMacCheckBox::OnLButtonDown(UINT nFlags, CPoint point)
{
	CButton::OnLButtonDown(nFlags, point);
}

void CDarkMacCheckBox::OnLButtonUp(UINT nFlags, CPoint point)
{
	const HWND hCapture = ::GetCapture();
	CButton::OnLButtonUp(nFlags, point);
	if (hCapture != m_hWnd)
		return;

	CRect rc;
	GetClientRect(&rc);
	if (!rc.PtInRect(point))
		return;

	const int next = (GetCheck() == BST_CHECKED) ? BST_UNCHECKED : BST_CHECKED;
	SetCheck(next);
	Invalidate(FALSE);
}

void CDarkMacCheckBox::DrawItem(LPDRAWITEMSTRUCT lp)
{
	if (!lp)
		return;

	CDC dc;
	dc.Attach(lp->hDC);
	CRect rc(lp->rcItem);
	const bool checked = (GetCheck() == BST_CHECKED);

	dc.FillSolidRect(rc, RGB(30, 30, 30));

	const int box = 16;
	CRect boxRc(rc.left, rc.top + (rc.Height() - box) / 2, rc.left + box, rc.top + (rc.Height() - box) / 2 + box);

	Graphics g(dc.m_hDC);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	GraphicsPath path;
	const REAL r = 4.f;
	path.AddArc((REAL)boxRc.left, (REAL)boxRc.top, r * 2, r * 2, 180, 90);
	path.AddArc((REAL)boxRc.right - r * 2, (REAL)boxRc.top, r * 2, r * 2, 270, 90);
	path.AddArc((REAL)boxRc.right - r * 2, (REAL)boxRc.bottom - r * 2, r * 2, r * 2, 0, 90);
	path.AddArc((REAL)boxRc.left, (REAL)boxRc.bottom - r * 2, r * 2, r * 2, 90, 90);
	path.CloseFigure();
	SolidBrush fill(Color(255, 45, 45, 48));
	Pen pen(Color(255, 90, 90, 94), 1.f);
	if (checked)
	{
		fill.SetColor(Color(255, 10, 132, 255));
		pen.SetColor(Color(255, 64, 156, 255));
	}
	g.FillPath(&fill, &path);
	g.DrawPath(&pen, &path);

	if (checked)
	{
		Pen checkPen(Color(255, 255, 255, 255), 2.f);
		checkPen.SetLineCap(LineCapRound, LineCapRound, DashCapRound);
		g.DrawLine(&checkPen, (REAL)boxRc.left + 4, (REAL)boxRc.top + 8,
			(Gdiplus::REAL)boxRc.left + 7, (REAL)boxRc.top + 11);
		g.DrawLine(&checkPen, (REAL)boxRc.left + 7, (REAL)boxRc.top + 11,
			(Gdiplus::REAL)boxRc.right - 4, (REAL)boxRc.top + 5);
	}

	CString text;
	GetWindowText(text);
	CRect textRc = rc;
	textRc.left = boxRc.right + 8;
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(RGB(235, 235, 237));
	CFont* pFont = GetFont();
	CFont* pOld = pFont ? dc.SelectObject(pFont) : NULL;
	dc.DrawText(text, textRc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	if (pOld)
		dc.SelectObject(pOld);

	dc.Detach();
}
