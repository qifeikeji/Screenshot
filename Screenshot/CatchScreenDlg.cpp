/******************************************************************************
*  FileName        :   CatchScreenDlg.cpp
*  Author          :   Unknown
*  Mender          :   sudami
*  Time            :   2007/09/09
*
*  Comment         :
*
******************************************************************************/

#include "stdafx.h"
#include "Screenshot.h"
#include "CatchScreenDlg.h"
#include "ScreenCapture.h"
#include "AppSettings.h"
#include "resource.h"

#include <GdiPlus.h>
using namespace  Gdiplus;

namespace {

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num = 0;
	UINT size = 0;
	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;
	ImageCodecInfo* pInfo = (ImageCodecInfo*)malloc(size);
	if (!pInfo)
		return -1;
	GetImageEncoders(num, size, pInfo);
	for (UINT i = 0; i < num; ++i)
	{
		if (wcscmp(pInfo[i].MimeType, format) == 0)
		{
			*pClsid = pInfo[i].Clsid;
			free(pInfo);
			return (int)i;
		}
	}
	free(pInfo);
	return -1;
}

} // namespace

namespace {

const COLORREF kAnnotColor = RGB(255, 60, 60);
const int kAnnotPenWidth = 3;
const int kAnnotTextHeight = 22;

class CAnnotTextDlg : public CDialog
{
public:
	CString m_text;
	enum { IDD = IDD_ANNOT_TEXT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX)
	{
		CDialog::DoDataExchange(pDX);
		DDX_Text(pDX, IDC_EDIT_ANNOT_TEXT, m_text);
	}
};

} // namespace

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CCatchScreenDlg dialog

CCatchScreenDlg::CCatchScreenDlg(CWnd* pParent /*=NULL*/)
: CDialog(CCatchScreenDlg::IDD, pParent)
{
	m_bLBtnDown = FALSE;
	//????????????,??????resizeMiddle ????
	m_rectTracker.m_nStyle = CMyTracker::solidLine | CMyTracker::resizeOutside;
	m_rectTracker.m_rect.SetRect(-1, -2, -3, -4);
	//??????????
	m_rectTracker.SetRectColor(RGB(10, 100, 130));

	m_hCursor = AfxGetApp()->LoadCursor(IDC_CURSOR1);

	m_bDraw = FALSE;
	m_bFirstDraw = FALSE;
	m_bQuit = FALSE;
	m_bToolBarShown = FALSE;
	m_startPt = 0;
	m_rectPrevDrag.SetRectEmpty();

	VirtualScreenInfo vsi = {};
	POINT cursor = {};
	if (!GetCaptureCursorAnchor(&cursor))
		GetCursorPos(&cursor);

	if (HasMixedMonitorScaling())
	{
		if (!QueryMonitorAtPoint(cursor, &vsi))
			QueryVirtualScreen(&vsi);
		m_hBitmap = CaptureScreenRect(vsi);
		if (!m_hBitmap)
			m_hBitmap = CaptureVirtualDesktop(vsi);
	}
	else
	{
		QueryVirtualScreen(&vsi);
		m_hBitmap = CaptureVirtualDesktop(vsi);
		if (!m_hBitmap)
			m_hBitmap = CaptureScreenRect(vsi);
	}

	m_nOriginX = vsi.originX;
	m_nOriginY = vsi.originY;
	m_nScreenWidth = vsi.width;
	m_nScreenHeight = vsi.height;

	SetCaptureCursorAnchor(POINT{ INT_MIN, INT_MIN });

	int bmpW = 0;
	int bmpH = 0;
	if (m_hBitmap && GetBitmapPixelSize(m_hBitmap, &bmpW, &bmpH))
	{
		m_nScreenWidth = bmpW;
		m_nScreenHeight = bmpH;
	}

	m_pBitmap = CBitmap::FromHandle(m_hBitmap);

	m_annotationRect.SetRectEmpty();
	m_activeTool = AnnotToolNone;
	m_bAnnotating = FALSE;
	m_annotStart = CPoint(0, 0);
	m_annotLast = CPoint(0, 0);
	m_previewRect.SetRectEmpty();
}

void CCatchScreenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_tipEdit);
}

BEGIN_MESSAGE_MAP(CCatchScreenDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_ERASEBKGND()
	ON_WM_SETCURSOR()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

void CCatchScreenDlg::InvalidateSelectionFrame(const CRect& rect)
{
	CRect inv = rect;
	inv.NormalizeRect();
	if (inv.IsRectEmpty())
		return;
	inv.InflateRect(12, 12);
	InvalidateRect(&inv, FALSE);
}

void CCatchScreenDlg::InvalidateAroundRect(const CRect& area)
{
	CRect inv = area;
	inv.NormalizeRect();
	inv.InflateRect(16, 16);
	if (m_toolBar.GetSafeHwnd() && m_toolBar.IsWindowVisible())
	{
		CRect tb;
		m_toolBar.GetWindowRect(&tb);
		ScreenToClient(&tb);
		inv.UnionRect(&inv, &tb);
	}
	InvalidateRect(&inv, FALSE);
}

void CCatchScreenDlg::CancelCurrentSelection()
{
	CRect old = m_rectTracker.m_rect;
	old.NormalizeRect();

	m_bFirstDraw = FALSE;
	m_bDraw = FALSE;
	m_bToolBarShown = FALSE;
	m_bAnnotating = FALSE;
	m_activeTool = AnnotToolNone;
	m_previewRect.SetRectEmpty();
	m_rectTracker.m_rect.SetRect(-1, -1, -1, -1);
	ClearAnnotationLayer();
	m_toolBar.HideBar();

	if (!old.IsRectEmpty() && old.Width() > 0 && old.Height() > 0)
		InvalidateSelectionFrame(old);
}

void CCatchScreenDlg::BeginSelectionAt(CPoint point)
{
	CancelCurrentSelection();
	m_startPt = point;
	m_bDraw = TRUE;
	m_bFirstDraw = TRUE;
	m_bToolBarShown = FALSE;
	m_rectTracker.m_rect.SetRect(point.x, point.y, point.x, point.y);
	SetCapture();
	InvalidateRect(NULL, FALSE);
}

void CCatchScreenDlg::PositionToolBar()
{
	CRect r = m_rectTracker.m_rect;
	r.NormalizeRect();
	CRect client;
	GetClientRect(&client);
	m_toolBar.SetBelowSelectionClient(r, client);
}

BOOL CCatchScreenDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ModifyStyle(WS_CAPTION | WS_SYSMENU | DS_MODALFRAME | DS_3DLOOK, WS_POPUP);
	ModifyStyleEx(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE, WS_EX_TOPMOST);

	if (m_pBitmap)
	{
		int bmpW = 0;
		int bmpH = 0;
		if (GetBitmapPixelSize(m_hBitmap, &bmpW, &bmpH))
		{
			m_nScreenWidth = bmpW;
			m_nScreenHeight = bmpH;
		}
	}

	CRect wr(0, 0, m_nScreenWidth, m_nScreenHeight);
	CalcWindowRect(&wr, CWnd::adjustBorder);
	SetWindowPos(&wndTopMost, m_nOriginX, m_nOriginY, wr.Width(), wr.Height(),
		SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOZORDER);

	CRect client;
	GetClientRect(&client);
	if (client.Width() != m_nScreenWidth || client.Height() != m_nScreenHeight)
	{
		CRect winRect;
		GetWindowRect(&winRect);
		const int dw = m_nScreenWidth - client.Width();
		const int dh = m_nScreenHeight - client.Height();
		SetWindowPos(&wndTopMost, m_nOriginX, m_nOriginY,
			winRect.Width() + dw, winRect.Height() + dh,
			SWP_NOACTIVATE | SWP_NOZORDER);
	}

	m_tipEdit.ShowWindow(SW_HIDE);

	m_toolBar.Create(this);
	m_toolBar.HideBar();

	((CScreenshotApp *)AfxGetApp())->m_hwndDlg = m_hWnd;
	return TRUE;
}

BOOL CCatchScreenDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		if (m_bFirstDraw)
			CancelCurrentSelection();
		PostQuitMessage(0);
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}
 
void CCatchScreenDlg::OnPaint()
{
	// ?????????????????
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, (WPARAM)dc.GetSafeHdc(), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

	}
	else
	{
		CPaintDC dc(this);
		CRect client;
		GetClientRect(&client);

		CDC memDC;
		memDC.CreateCompatibleDC(&dc);
		CBitmap frame;
		frame.CreateCompatibleBitmap(&dc, client.Width(), client.Height());
		CBitmap* pOld = memDC.SelectObject(&frame);

		CDC srcDC;
		srcDC.CreateCompatibleDC(&dc);
		srcDC.SelectObject(m_pBitmap);

		BITMAP bm = {};
		m_pBitmap->GetBitmap(&bm);
		const int copyW = bm.bmWidth;
		const int copyH = (bm.bmHeight < 0) ? -bm.bmHeight : bm.bmHeight;
		if (copyW > 0 && copyH > 0)
			memDC.BitBlt(0, 0, min(client.Width(), copyW), min(client.Height(), copyH),
				&srcDC, 0, 0, SRCCOPY);

		if (m_bFirstDraw || m_bDraw)
		{
			m_rectTracker.Draw(&memDC);
			if (m_annotation.IsValid() && !m_annotationRect.IsRectEmpty())
				m_annotation.DrawOn(memDC.m_hDC, m_annotationRect.left, m_annotationRect.top);
			if (m_bAnnotating && !m_previewRect.IsRectEmpty())
				DrawPreviewShape(memDC.m_hDC);
		}

		dc.BitBlt(0, 0, client.Width(), client.Height(), &memDC, 0, 0, SRCCOPY);
		memDC.SelectObject(pOld);
	}
}

void CCatchScreenDlg::OnCancel()
{
	if (m_bFirstDraw)
	{
		CancelCurrentSelection();
		InvalidateRect(NULL, FALSE);
	}
	else
	{
		CDialog::OnCancel();
	}
}

void CCatchScreenDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bAnnotating && m_bFirstDraw && !m_bDraw)
	{
		CPoint local;
		if (ClientToAnnot(point, local))
		{
			if (m_activeTool == AnnotToolBrush)
			{
				m_annotation.DrawLine(m_annotLast.x, m_annotLast.y, local.x, local.y,
					kAnnotPenWidth, kAnnotColor);
				m_annotLast = local;
				InvalidateAnnotationView();
			}
			else if (m_activeTool == AnnotToolArrow || m_activeTool == AnnotToolRect ||
				m_activeTool == AnnotToolEllipse)
			{
				m_previewRect.SetRect(m_annotStart.x, m_annotStart.y, local.x, local.y);
				InvalidateAnnotationView();
			}
		}
		CDialog::OnMouseMove(nFlags, point);
		return;
	}

	if (m_bDraw && m_bToolBarShown)
	{
		m_toolBar.HideBar();
		m_bToolBarShown = FALSE;
	}
	if (m_bDraw)
	{
		CRect prev = m_rectTracker.m_rect;
		m_rectTracker.m_rect.SetRect(m_startPt.x, m_startPt.y, point.x, point.y);
		CRect cur = m_rectTracker.m_rect;
		cur.NormalizeRect();
		CRect prevNorm = prev;
		prevNorm.NormalizeRect();
		CRect dirty = prevNorm;
		dirty.UnionRect(&dirty, &cur);
		dirty.InflateRect(2, 2);
		InvalidateRect(&dirty, FALSE);
	}

	CDialog::OnMouseMove(nFlags, point);
}

void CCatchScreenDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bLBtnDown = TRUE;
	const int nHitTest = m_rectTracker.HitTest(point);

	if (m_bFirstDraw && !m_bDraw && m_activeTool != AnnotToolNone && IsPointInSelection(point))
	{
		SyncAnnotationLayerToSelection();
		CPoint local;
		if (ClientToAnnot(point, local))
		{
			if (m_activeTool == AnnotToolText)
			{
				m_annotation.PushUndoSnapshot();
				CString text;
				if (PromptAnnotText(text) && !text.IsEmpty())
				{
					m_annotation.DrawTextAt(local.x, local.y, text, kAnnotTextHeight, kAnnotColor);
					InvalidateAnnotationView();
				}
			}
			else
			{
				m_bAnnotating = TRUE;
				m_annotStart = local;
				m_annotLast = local;
				m_previewRect.SetRect(local.x, local.y, local.x, local.y);
				if (m_activeTool == AnnotToolBrush)
					m_annotation.PushUndoSnapshot();
				SetCapture();
			}
		}
		CDialog::OnLButtonDown(nFlags, point);
		return;
	}

	if (!m_bDraw)
	{
		const bool onResizeHandle = (nHitTest >= CMyTracker::hitTopLeft &&
			nHitTest <= CMyTracker::hitLeft);
		const bool allowResize = (m_activeTool == AnnotToolNone);
		if (nHitTest < 0 || nHitTest == CMyTracker::hitMiddle || !onResizeHandle)
		{
			BeginSelectionAt(point);
		}
		else if (m_bFirstDraw && onResizeHandle && allowResize)
		{
			m_rectTracker.Track(this, point, TRUE);
			SyncAnnotationLayerToSelection();
			PositionToolBar();
			m_toolBar.ShowBar();
			m_bToolBarShown = TRUE;
			InvalidateAroundRect(m_rectTracker.m_rect);
		}
	}

	CDialog::OnLButtonDown(nFlags, point);
}

void CCatchScreenDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bAnnotating)
	{
		m_bAnnotating = FALSE;
		if (GetCapture() == this)
			ReleaseCapture();
		CPoint local;
		if (ClientToAnnot(point, local))
		{
			if (m_activeTool == AnnotToolBrush)
			{
				// already drawn during move
			}
			else if (m_activeTool == AnnotToolArrow)
			{
				m_annotation.PushUndoSnapshot();
				m_annotation.DrawArrow(m_annotStart.x, m_annotStart.y, local.x, local.y,
					kAnnotPenWidth, kAnnotColor);
			}
			else if (m_activeTool == AnnotToolRect)
			{
				m_annotation.PushUndoSnapshot();
				m_annotation.DrawRectangle(m_annotStart.x, m_annotStart.y, local.x, local.y,
					kAnnotPenWidth, kAnnotColor, FALSE);
			}
			else if (m_activeTool == AnnotToolEllipse)
			{
				m_annotation.PushUndoSnapshot();
				m_annotation.DrawEllipse(m_annotStart.x, m_annotStart.y, local.x, local.y,
					kAnnotPenWidth, kAnnotColor, FALSE);
			}
		}
		m_previewRect.SetRectEmpty();
		InvalidateAnnotationView();
		m_bLBtnDown = FALSE;
		CDialog::OnLButtonUp(nFlags, point);
		return;
	}

	const BOOL wasDrawing = m_bDraw;
	m_bLBtnDown = FALSE;
	m_bDraw = FALSE;
	if (GetCapture() == this)
		ReleaseCapture();
	if (wasDrawing && m_bFirstDraw)
		FinishSelectionIfValid(point);
	SyncAnnotationLayerToSelection();
	InvalidateAroundRect(m_rectTracker.m_rect);
	CDialog::OnLButtonUp(nFlags, point);
}

void CCatchScreenDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	int nHitTest;
	nHitTest = m_rectTracker.HitTest(point);

	//????????????????
	if (nHitTest == 8)
	{
		//????????????????,bSave ?TRUE,
		CopySelectionToClipboard(m_rectTracker.m_rect);
		PostQuitMessage(0);
	}

	CDialog::OnLButtonDblClk(nFlags, point);
}

void CCatchScreenDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_toolBar.HideBar();
	//InvalidateRgnWindow();
	CDialog::OnRButtonDown(nFlags, point);
}

void CCatchScreenDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_bLBtnDown = FALSE;
	if (m_bFirstDraw)
	{
		CancelCurrentSelection();
		InvalidateRect(NULL, FALSE);
	}
	else
	{
		//??????
		PostQuitMessage(0);
	}

	CDialog::OnRButtonUp(nFlags, point);
}

BOOL CCatchScreenDlg::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
	//??????????????????????
	BITMAP bmp;
	m_pBitmap->GetBitmap(&bmp);

	CDC dcCompatible;
	dcCompatible.CreateCompatibleDC(pDC);

	dcCompatible.SelectObject(m_pBitmap);

	CRect rect;
	GetClientRect(&rect);
	pDC->BitBlt(0, 0, rect.Width(), rect.Height(), &dcCompatible, 0, 0, SRCCOPY);
	return TRUE;
}

BOOL CCatchScreenDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	//????????????????????
	if (pWnd == this && m_rectTracker.SetCursor(this, nHitTest)
		&& !m_bDraw && m_bFirstDraw && m_activeTool == AnnotToolNone)
	{
		return TRUE;
	}

	//??????????
	SetCursor(m_hCursor);
	return TRUE;
}

void CCatchScreenDlg::ClearAnnotationLayer()
{
	m_annotation.Clear();
	m_annotationRect.SetRectEmpty();
}

void CCatchScreenDlg::SyncAnnotationLayerToSelection()
{
	CRect r = m_rectTracker.m_rect;
	r.NormalizeRect();
	if (r.IsRectEmpty())
	{
		ClearAnnotationLayer();
		return;
	}
	if (m_annotationRect == r && m_annotation.IsValid())
		return;
	if (!m_annotationRect.IsRectEmpty() && m_annotationRect != r)
		m_annotation.Clear();
	m_annotation.EnsureSize(r.Width(), r.Height());
	m_annotationRect = r;
}

BOOL CCatchScreenDlg::ClientToAnnot(CPoint client, CPoint& out) const
{
	CRect r = m_annotationRect;
	r.NormalizeRect();
	if (!r.PtInRect(client))
		return FALSE;
	out.x = client.x - r.left;
	out.y = client.y - r.top;
	return TRUE;
}

BOOL CCatchScreenDlg::IsPointInSelection(CPoint client) const
{
	CRect r = m_rectTracker.m_rect;
	r.NormalizeRect();
	return !r.IsRectEmpty() && r.PtInRect(client);
}

void CCatchScreenDlg::InvalidateAnnotationView()
{
	CRect r = m_annotationRect;
	r.NormalizeRect();
	if (r.IsRectEmpty())
		return;
	r.InflateRect(2, 2);
	InvalidateRect(&r, FALSE);
}

void CCatchScreenDlg::DrawPreviewShape(HDC hdc) const
{
	if (m_previewRect.IsRectEmpty() || m_annotationRect.IsRectEmpty())
		return;
	CRect r = m_previewRect;
	r.NormalizeRect();
	r.OffsetRect(m_annotationRect.left, m_annotationRect.top);

	HPEN pen = CreatePen(PS_DOT, 1, kAnnotColor);
	HGDIOBJ oldPen = SelectObject(hdc, pen);
	HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));
	if (m_activeTool == AnnotToolRect)
		Rectangle(hdc, r.left, r.top, r.right, r.bottom);
	else if (m_activeTool == AnnotToolEllipse)
		Ellipse(hdc, r.left, r.top, r.right, r.bottom);
	else if (m_activeTool == AnnotToolArrow)
	{
		CRect pr = m_previewRect;
		pr.NormalizeRect();
		const int x1 = m_annotStart.x + m_annotationRect.left;
		const int y1 = m_annotStart.y + m_annotationRect.top;
		const int x2 = pr.right + m_annotationRect.left;
		const int y2 = pr.bottom + m_annotationRect.top;
		MoveToEx(hdc, x1, y1, NULL);
		LineTo(hdc, x2, y2);
	}
	SelectObject(hdc, oldBrush);
	SelectObject(hdc, oldPen);
	DeleteObject(pen);
}

BOOL CCatchScreenDlg::PromptAnnotText(CString& text)
{
	CAnnotTextDlg dlg;
	dlg.m_text = text;
	if (dlg.DoModal() != IDOK)
		return FALSE;
	text = dlg.m_text;
	text.Trim();
	return TRUE;
}

void CCatchScreenDlg::FinishSelectionIfValid(const CPoint& endPt)
{
	const int dx = abs(endPt.x - m_startPt.x);
	const int dy = abs(endPt.y - m_startPt.y);
	if (dx <= 4 && dy <= 4)
		return;

	m_bFirstDraw = TRUE;
	if (GetAppSettings().copyAndExitAfterSelect)
	{
		CopySelectionToClipboard(m_rectTracker.m_rect);
		PostQuitMessage(0);
		return;
	}
	PositionToolBar();
	m_toolBar.ShowBar();
	m_bToolBarShown = TRUE;
}

HBITMAP CCatchScreenDlg::BuildSelectionBitmap(const CRect& clientRect) const
{
	CRect r = clientRect;
	r.NormalizeRect();
	if (r.IsRectEmpty() || !m_pBitmap)
		return NULL;

	const int nWidth = r.Width();
	const int nHeight = r.Height();
	if (nWidth <= 0 || nHeight <= 0)
		return NULL;

	HDC hScrDC = ::GetDC(NULL);
	if (!hScrDC)
		return NULL;
	HDC hMemDC = CreateCompatibleDC(hScrDC);
	if (!hMemDC)
	{
		::ReleaseDC(NULL, hScrDC);
		return NULL;
	}
	HBITMAP hBitmap = CreateCompatibleBitmap(hScrDC, nWidth, nHeight);
	if (!hBitmap)
	{
		DeleteDC(hMemDC);
		::ReleaseDC(NULL, hScrDC);
		return NULL;
	}
	HGDIOBJ hOld = SelectObject(hMemDC, hBitmap);

	CDC dcSrc;
	dcSrc.CreateCompatibleDC(CDC::FromHandle(hMemDC));
	dcSrc.SelectObject(m_pBitmap);
	BitBlt(hMemDC, 0, 0, nWidth, nHeight, dcSrc.m_hDC, r.left, r.top, SRCCOPY);

	if (m_annotation.IsValid() && m_annotationRect == r)
		m_annotation.DrawOn(hMemDC, 0, 0);

	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);
	::ReleaseDC(NULL, hScrDC);
	return hBitmap;
}

BOOL CCatchScreenDlg::CopySelectionToClipboard(const CRect& clientRect)
{
	HBITMAP hBitmap = BuildSelectionBitmap(clientRect);
	if (!hBitmap)
		return FALSE;

	if (!OpenClipboard())
	{
		DeleteObject(hBitmap);
		return FALSE;
	}
	EmptyClipboard();
	SetClipboardData(CF_BITMAP, hBitmap);
	CloseClipboard();
	return TRUE;
}

BOOL CCatchScreenDlg::SaveSelectionToFile(const CRect& clientRect)
{
	CRect r = clientRect;
	r.NormalizeRect();
	if (r.IsRectEmpty())
	{
		AfxMessageBox(L"\u8bf7\u5148\u6846\u9009\u6709\u6548\u533a\u57df\u3002");
		return FALSE;
	}

	CString dir = GetAppSettings().saveDirectory;
	dir.Trim();
	if (dir.IsEmpty())
	{
		AfxMessageBox(L"\u8bf7\u5728\u8bbe\u7f6e\u4e2d\u6307\u5b9a\u4fdd\u5b58\u56fe\u7247\u8def\u5f84\u3002");
		return FALSE;
	}
	if (dir[dir.GetLength() - 1] != _T('\\') && dir[dir.GetLength() - 1] != _T('/'))
		dir += _T('\\');

	CTime now = CTime::GetCurrentTime();
	CString path;
	path.Format(_T("%sscreenshot_%s.png"), (LPCTSTR)dir, (LPCTSTR)now.Format(_T("%Y%m%d_%H%M%S")));

	HBITMAP hBitmap = BuildSelectionBitmap(r);
	if (!hBitmap)
		return FALSE;

	Bitmap bmp(hBitmap, NULL);
	CLSID clsid;
	if (GetEncoderClsid(L"image/png", &clsid) < 0)
	{
		DeleteObject(hBitmap);
		AfxMessageBox(L"\u672a\u627e\u5230 PNG \u7f16\u7801\u5668\u3002");
		return FALSE;
	}
	Status st = bmp.Save(path, &clsid, NULL);
	DeleteObject(hBitmap);
	if (st != Ok)
	{
		CString msg;
		msg.Format(L"\u4fdd\u5b58\u5931\u8d25\uff1a%s", (LPCTSTR)path);
		AfxMessageBox(msg);
		return FALSE;
	}
	CopySelectionToClipboard(r);
	return TRUE;
}

// ???????, ?????????CSDN
// lpRect ???????????
HBITMAP CCatchScreenDlg::CopyScreenToBitmap(LPRECT lpRect, BOOL bSave)

{
	HDC       hScrDC, hMemDC;
	// ?????????????????
	HBITMAP    hBitmap, hOldBitmap;
	// ??????
	int       nX, nY, nX2, nY2;
	// ???????????
	int       nWidth, nHeight;

	// ????????????????
	if (IsRectEmpty(lpRect))
		return NULL;
	//?????????????????
	hScrDC = ::GetDC(NULL);
	if (!hScrDC)
		return NULL;
	hMemDC = CreateCompatibleDC(hScrDC);
	if (!hMemDC)
	{
		::ReleaseDC(NULL, hScrDC);
		return NULL;
	}
	nX = lpRect->left;
	nY = lpRect->top;
	nX2 = lpRect->right;
	nY2 = lpRect->bottom;

	//????????????????
	if (nX < 0)
		nX = 0;
	if (nY < 0)
		nY = 0;
	if (nX2 > m_nScreenWidth)
		nX2 = m_nScreenWidth;
	if (nY2 > m_nScreenHeight)
		nY2 = m_nScreenHeight;
	nWidth = nX2 - nX;
	nHeight = nY2 - nY;
	// ?????????????????????????????
	hBitmap = CreateCompatibleBitmap
		(hScrDC, nWidth, nHeight);
	// ????????????????????????
	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);
	// ??????????????????????????????????
	if (bSave)
	{
		//????????DC,??bSave???????????????????,?????????????????
		CDC dcCompatible;
		dcCompatible.CreateCompatibleDC(CDC::FromHandle(hMemDC));
		dcCompatible.SelectObject(m_pBitmap);

		BitBlt(hMemDC, 0, 0, nWidth, nHeight,
			dcCompatible, nX, nY, SRCCOPY);
	}
	else
	{
		BitBlt(hMemDC, 0, 0, nWidth, nHeight,
			hScrDC, m_nOriginX + nX, m_nOriginY + nY, SRCCOPY);
	}

	hBitmap = (HBITMAP)SelectObject(hMemDC, hOldBitmap);
	DeleteDC(hMemDC);
	::ReleaseDC(NULL, hScrDC);
	return hBitmap;
}

BOOL CCatchScreenDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	bool bHandle = true;
	HWND hWnd = m_toolBar.GetHWND();
	if (lParam == (LPARAM)m_toolBar.GetHWND())
	{
		const int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case DarkToolBar_CommandBase:
			if (m_annotation.Undo())
				InvalidateAnnotationView();
			break;
		case DarkToolBar_CommandBase + 1:
			m_activeTool = AnnotToolArrow;
			break;
		case DarkToolBar_CommandBase + 2:
			m_activeTool = AnnotToolRect;
			break;
		case DarkToolBar_CommandBase + 3:
			m_activeTool = AnnotToolEllipse;
			break;
		case DarkToolBar_CommandBase + 4:
			m_activeTool = AnnotToolBrush;
			break;
		case DarkToolBar_CommandBase + 5:
			m_activeTool = AnnotToolText;
			break;
		case DarkToolBar_CommandBase + 6:
			SaveSelectionToFile(m_rectTracker.m_rect);
			break;
		case DarkToolBar_CommandBase + 7:
			CancelCurrentSelection();
			InvalidateRect(NULL, FALSE);
			break;
		case DarkToolBar_CommandBase + 8:
			if (CopySelectionToClipboard(m_rectTracker.m_rect))
				PostQuitMessage(0);
			break;
		default:
			bHandle = false;
			break;
		}
		::SetFocus(hWnd);
	}
	if (bHandle == false)
	{
		return CDialog::OnCommand(wParam, lParam);
	}
	return TRUE;
}

////////////////////////////////// END OF FILE ///////////////////////////////////////