#include "stdafx.h"
#include "DarkHotKeyFrame.h"
#include <GdiPlus.h>
using namespace Gdiplus;

CDarkHotKeyFrame::CDarkHotKeyFrame()
{
}

BOOL CDarkHotKeyFrame::Create(CWnd* pParent, const CRect& rect, UINT hotKeyCtrlId)
{
	if (!CreateEx(0, AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW),
		_T("DarkHotKeyFrame"), WS_CHILD | WS_VISIBLE, rect, pParent, 0))
		return FALSE;

	CWnd* pHot = pParent->GetDlgItem(hotKeyCtrlId);
	if (pHot && pHot->GetSafeHwnd())
	{
		pHot->SetParent(this);
		pHot->ModifyStyle(WS_BORDER, 0);
		CRect inner(6, 4, rect.Width() - 6, rect.Height() - 4);
		pHot->MoveWindow(&inner);
	}
	return TRUE;
}

BEGIN_MESSAGE_MAP(CDarkHotKeyFrame, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CDarkHotKeyFrame::OnPaint()
{
	CPaintDC dc(this);
	CRect rc;
	GetClientRect(&rc);
	if (rc.IsRectEmpty())
		return;

	Graphics g(dc.m_hDC);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	GraphicsPath path;
	const REAL r = 8.f;
	path.AddArc((REAL)rc.left, (REAL)rc.top, r * 2, r * 2, 180, 90);
	path.AddArc((REAL)rc.right - r * 2, (REAL)rc.top, r * 2, r * 2, 270, 90);
	path.AddArc((REAL)rc.right - r * 2, (REAL)rc.bottom - r * 2, r * 2, r * 2, 0, 90);
	path.AddArc((REAL)rc.left, (REAL)rc.bottom - r * 2, r * 2, r * 2, 90, 90);
	path.CloseFigure();
	SolidBrush fill(Color(255, 42, 42, 44));
	Pen pen(Color(255, 68, 68, 70), 1.f);
	g.FillPath(&fill, &path);
	g.DrawPath(&pen, &path);
}

BOOL CDarkHotKeyFrame::OnEraseBkgnd(CDC* pDC)
{
	UNREFERENCED_PARAMETER(pDC);
	return TRUE;
}
