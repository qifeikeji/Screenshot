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
	QueryVirtualScreen(&vsi);
	m_nOriginX = vsi.originX;
	m_nOriginY = vsi.originY;
	m_nScreenWidth = vsi.width;
	m_nScreenHeight = vsi.height;

	m_hBitmap = CaptureVirtualDesktop(vsi);
	if (!m_hBitmap)
		m_hBitmap = CaptureScreenRect(vsi);

	int bmpW = 0;
	int bmpH = 0;
	if (m_hBitmap && GetBitmapPixelSize(m_hBitmap, &bmpW, &bmpH))
	{
		m_nScreenWidth = bmpW;
		m_nScreenHeight = bmpH;
	}

	m_pBitmap = CBitmap::FromHandle(m_hBitmap);

	m_annotationRect.SetRectEmpty();
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
	InvalidateRect(NULL, FALSE);
}

void CCatchScreenDlg::PositionToolBar()
{
	CRect r = m_rectTracker.m_rect;
	r.NormalizeRect();
	m_toolBar.SetInsideSelectionClient(r);
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

		if (m_bFirstDraw)
		{
			m_rectTracker.Draw(&memDC);
			if (m_annotation.IsValid() && !m_annotationRect.IsRectEmpty())
				m_annotation.DrawOn(memDC.m_hDC, m_annotationRect.left, m_annotationRect.top);
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
	if (m_bDraw && m_bToolBarShown)
	{
		m_toolBar.HideBar();
		m_bToolBarShown = FALSE;
	}
	if (m_bDraw)
	{
		CRect prev = m_rectTracker.m_rect;
		m_rectTracker.m_rect.SetRect(m_startPt.x, m_startPt.y, point.x, point.y);
		CRect dirty = prev;
		dirty.UnionRect(&dirty, &m_rectTracker.m_rect);
		InvalidateAroundRect(dirty);
	}

	CDialog::OnMouseMove(nFlags, point);
}

void CCatchScreenDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bLBtnDown = TRUE;
	const int nHitTest = m_rectTracker.HitTest(point);

	if (!m_bDraw)
	{
		const bool onResizeHandle = (nHitTest >= CMyTracker::hitTopLeft &&
			nHitTest <= CMyTracker::hitLeft);
		if (nHitTest < 0 || nHitTest == CMyTracker::hitMiddle || !onResizeHandle)
		{
			BeginSelectionAt(point);
		}
		else if (m_bFirstDraw && onResizeHandle)
		{
			m_rectTracker.Track(this, point, TRUE);
			SyncAnnotationLayerToSelection();
			CopySelectionToClipboard(m_rectTracker.m_rect);
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
	const BOOL wasDrawing = m_bDraw;
	m_bLBtnDown = FALSE;
	m_bDraw = FALSE;
	if (wasDrawing && m_bFirstDraw)
	{
		const int dx = abs(point.x - m_startPt.x);
		const int dy = abs(point.y - m_startPt.y);
		if (dx > 4 || dy > 4)
		{
			CopySelectionToClipboard(m_rectTracker.m_rect);
			PositionToolBar();
			m_toolBar.ShowBar();
			m_bToolBarShown = TRUE;
		}
	}
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
	if (pWnd == this &&m_rectTracker.SetCursor(this, nHitTest)
		&& !m_bDraw &&m_bFirstDraw) //??????????????????????????
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
		AfxMessageBox(L"请先框选有效区域。");
		return FALSE;
	}

	CFileDialog dlg(FALSE, _T("png"), _T("screenshot.png"),
		OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY,
		_T("PNG ?? (*.png)|*.png|JPEG ?? (*.jpg;*.jpeg)|*.jpg;*.jpeg||"),
		this);
	if (dlg.DoModal() != IDOK)
		return FALSE;

	HBITMAP hBitmap = BuildSelectionBitmap(r);
	if (!hBitmap)
		return FALSE;

	Bitmap bmp(hBitmap, NULL);
	CLSID clsid;
	CString ext = dlg.GetFileExt();
	ext.MakeLower();
	LPCWSTR mime = (ext == _T("jpg") || ext == _T("jpeg")) ? L"image/jpeg" : L"image/png";
	if (GetEncoderClsid(mime, &clsid) < 0)
	{
		DeleteObject(hBitmap);
		AfxMessageBox(_T("???????????????"));
		return FALSE;
	}
	Status st = bmp.Save(dlg.GetPathName(), &clsid, NULL);
	DeleteObject(hBitmap);
	if (st != Ok)
	{
		AfxMessageBox(_T("????????"));
		return FALSE;
	}
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
	if(lParam == (LPARAM)m_toolBar.GetHWND())
	{
		int wmId  = LOWORD(wParam);
		switch(wmId)
		{
		case DarkToolBar_CommandBase:
		case DarkToolBar_CommandBase + 1:
		case DarkToolBar_CommandBase + 2:
			break;
		case DarkToolBar_CommandBase + 3:
			CancelCurrentSelection();
			InvalidateRect(NULL, FALSE);
			break;
		case DarkToolBar_CommandBase + 4:
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
		return CDialog::OnCommand(wParam,lParam);
	}
	return TRUE;
}

////////////////////////////////// END OF FILE ///////////////////////////////////////