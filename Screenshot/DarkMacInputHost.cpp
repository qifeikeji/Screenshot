#include "stdafx.h"
#include "DarkMacInputHost.h"
#include <GdiPlus.h>
using namespace Gdiplus;

CDarkMacInputHost::CDarkMacInputHost()
	: m_childId(0)
{
}

BOOL CDarkMacInputHost::Create(CWnd* pParent, const CRect& rect, UINT childCtrlId)
{
	m_childId = childCtrlId;
	m_brInner.CreateSolidBrush(RGB(45, 45, 48));

	if (!CreateEx(0, AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW),
		_T("DarkMacInputHost"), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
		rect, pParent, 0))
		return FALSE;

	CWnd* pChild = pParent->GetDlgItem(childCtrlId);
	if (pChild && pChild->GetSafeHwnd())
	{
		pChild->SetParent(this);
		pChild->ModifyStyle(WS_BORDER | WS_EX_CLIENTEDGE, 0);
		pChild->ModifyStyleEx(WS_EX_CLIENTEDGE, 0);
		const int padX = 10;
		const int padY = 5;
		CRect inner(padX, padY, rect.Width() - padX, rect.Height() - padY);
		if (inner.Height() < 14)
			inner.bottom = inner.top + 14;
		pChild->MoveWindow(&inner);
		pChild->ShowWindow(SW_SHOW);
	}
	return TRUE;
}

BEGIN_MESSAGE_MAP(CDarkMacInputHost, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

void CDarkMacInputHost::OnPaint()
{
	CPaintDC dc(this);
	CRect rc;
	GetClientRect(&rc);
	if (rc.IsRectEmpty())
		return;
	rc.DeflateRect(1, 1, 3, 3);
	if (rc.Width() < 4 || rc.Height() < 4)
		return;

	Graphics g(dc.m_hDC);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	GraphicsPath path;
	const REAL r = min(8.f, (REAL)min(rc.Width(), rc.Height()) / 2.f);
	path.AddArc((REAL)rc.left, (REAL)rc.top, r * 2, r * 2, 180, 90);
	path.AddArc((REAL)rc.right - r * 2, (REAL)rc.top, r * 2, r * 2, 270, 90);
	path.AddArc((REAL)rc.right - r * 2, (REAL)rc.bottom - r * 2, r * 2, r * 2, 0, 90);
	path.AddArc((REAL)rc.left, (REAL)rc.bottom - r * 2, r * 2, r * 2, 90, 90);
	path.CloseFigure();
	SolidBrush fill(Color(255, 45, 45, 48));
	Pen pen(Color(255, 72, 72, 76), 1.f);
	g.FillPath(&fill, &path);
	g.DrawPath(&pen, &path);
}

BOOL CDarkMacInputHost::OnEraseBkgnd(CDC* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	return TRUE;
}

HBRUSH CDarkMacInputHost::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	pDC->SetTextColor(RGB(240, 240, 242));
	if (nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)::GetStockObject(HOLLOW_BRUSH);
	}
	if (nCtlColor == CTLCOLOR_EDIT)
	{
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)::GetStockObject(HOLLOW_BRUSH);
	}
	pDC->SetBkColor(RGB(45, 45, 48));
	return (HBRUSH)m_brInner.GetSafeHandle();
}
