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
	m_rectTracker.m_nStyle = CMyTracker::resizeMiddle | CMyTracker::solidLine;
	m_rectTracker.m_rect.SetRect(-1, -2, -3, -4);
	//??????????
	m_rectTracker.SetRectColor(RGB(10, 100, 130));

	m_hCursor = AfxGetApp()->LoadCursor(IDC_CURSOR1);

	m_bDraw = FALSE;
	m_bFirstDraw = FALSE;
	m_bQuit = FALSE;
	m_bNeedShowMsg = FALSE;
	m_startPt = 0;

	m_nOriginX = GetSystemMetrics(SM_XVIRTUALSCREEN);
	m_nOriginY = GetSystemMetrics(SM_YVIRTUALSCREEN);
	m_nScreenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	m_nScreenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	CRect rect(0, 0, m_nScreenWidth, m_nScreenHeight);
	m_hBitmap = CopyScreenToBitmap(&rect);
	m_pBitmap = CBitmap::FromHandle(m_hBitmap);

	m_annotationRect.SetRectEmpty();
	//??????????????? m_rgn
	m_rgn.CreateRectRgn(0, 0, 50, 50);
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
	ON_WM_RBUTTONUP()
	ON_WM_CTLCOLOR()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCatchScreenDlg message handlers

BOOL CCatchScreenDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	//?????????????????????
	SetWindowPos(&wndTopMost, m_nOriginX, m_nOriginY, m_nScreenWidth, m_nScreenHeight, SWP_SHOWWINDOW);

	//??????????????
	CRect rect;
	m_tipEdit.GetWindowRect(&rect);
	m_tipEdit.MoveWindow(10, 10, rect.Width(), rect.Height());


	m_toolBar.CreateToolBar(m_hWnd);
	m_toolBar.RemoveChildStyle();
	::MoveWindow(m_toolBar.GetHWND(),300,300,230,30,FALSE);


	UpdateTipString();
	SetEidtWndText();

	((CScreenshotApp *)AfxGetApp())->m_hwndDlg = AfxGetMainWnd()->GetSafeHwnd();
	return TRUE;
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
	else  // ??????????????
	{
		CPaintDC dc(this);

		CDC dcCompatible;

		dcCompatible.CreateCompatibleDC(&dc);
		RECT rect;
		::GetClientRect(m_hWnd, &rect);
		HBITMAP hBitmap = ::CreateCompatibleBitmap(dc.m_hDC,rect.right-rect.left,rect.bottom-rect.top);
		::SelectObject(dcCompatible.m_hDC,hBitmap);

		HBRUSH s_hBitmapBrush = CreatePatternBrush(m_hBitmap); 
		::FillRect(dcCompatible.m_hDC,&rect,s_hBitmapBrush);

		//?????????????????
		if (m_bNeedShowMsg && m_bFirstDraw)
		{
			CRect rect = m_rectTracker.m_rect;
			DrawMessage(rect, &dcCompatible);
		}

		//????????????
		if (m_bFirstDraw)
		{
			m_rectTracker.Draw(&dcCompatible);
		}

		if (m_bFirstDraw && m_annotation.IsValid() && !m_annotationRect.IsRectEmpty())
		{
			m_annotation.DrawOn(dcCompatible.m_hDC, m_annotationRect.left, m_annotationRect.top);
		}

		Gdiplus::Graphics graphics(dcCompatible);

		HRGN hgn1 = CreateRectRgn(m_rectTracker.m_rect.left,m_rectTracker.m_rect.top,
			m_rectTracker.m_rect.right,m_rectTracker.m_rect.bottom);
		Region region1(hgn1);

		HRGN hgn2 = CreateRectRgn(rect.left,rect.top,
			rect.right,rect.bottom);
		Region region2(hgn2);

		region2.Exclude(&region1);

		SolidBrush  solidBrush(Color(100, 128, 128, 128));
		graphics.FillRegion(&solidBrush,&region2);

		DeleteObject(hgn1);
		DeleteObject(hgn2);

		dc.BitBlt(0,0,rect.right, rect.bottom,&dcCompatible,0,0,SRCCOPY);
		DeleteObject(hBitmap);
		DeleteObject(s_hBitmapBrush);

		//CDialog::OnPaint();
	}
}

void CCatchScreenDlg::OnCancel()
{
	if (m_bFirstDraw)
	{
		//??????????????
		m_bFirstDraw = FALSE;
		m_bDraw = FALSE;
		m_rectTracker.m_rect.SetRect(-1, -1, -1, -1);
		ClearAnnotationLayer();
		InvalidateRgnWindow();
	}
	else
	{
		CDialog::OnCancel();
	}
}

void CCatchScreenDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if(m_bLBtnDown)
		m_toolBar.HideToolBar();
	else
		m_toolBar.ShowToolBar();
	if (m_bDraw)
	{
		//???????????????,????????
		m_rectTracker.m_rect.SetRect(m_startPt.x + 4, m_startPt.y + 4, point.x, point.y);
		InvalidateRgnWindow();
	}
	
	//??????????????????,???????MouseMove???
	CRect rect;
	m_tipEdit.GetWindowRect(&rect);
	if (rect.PtInRect(point))
		m_tipEdit.SendMessage(WM_MOUSEMOVE);

	UpdateMousePointRGBString();
	SetEidtWndText();

	CDialog::OnMouseMove(nFlags, point);
}

void CCatchScreenDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_bLBtnDown = TRUE;
	int nHitTest;
	nHitTest = m_rectTracker.HitTest(point);
	//???????????
	if (nHitTest < 0)
	{
		if (!m_bFirstDraw)
		{
			//???????????
			m_startPt = point;
			m_bDraw = TRUE;
			m_bFirstDraw = TRUE;
			//????????????????????????
			m_rectTracker.m_rect.SetRect(point.x, point.y, point.x + 4, point.y + 4);

			//?????????????????????
			if (m_bFirstDraw)
				m_bNeedShowMsg = TRUE;
			UpdateTipString();
			SetEidtWndText();
			InvalidateRgnWindow();
		}
	}
	else
	{
		//?????????????????????
		m_bNeedShowMsg = TRUE;
		if (m_bFirstDraw)
		{
			//?????????,Track?????????????????,???????,?????CRectTracker???????
			m_rectTracker.Track(this, point, TRUE);
			SyncAnnotationLayerToSelection();
			InvalidateRgnWindow();
		}
	}

	CDialog::OnLButtonDown(nFlags, point);
}

void CCatchScreenDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bLBtnDown = FALSE;
	m_bNeedShowMsg = FALSE;
	m_bDraw = FALSE;
	UpdateTipString();
	SetEidtWndText();
	m_toolBar.SetShowPlace(m_rectTracker.m_rect.right,m_rectTracker.m_rect.bottom);
	SyncAnnotationLayerToSelection();

	InvalidateRgnWindow();
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
	m_toolBar.HideToolBar();
	//InvalidateRgnWindow();
	CDialog::OnRButtonDown(nFlags, point);
}

void CCatchScreenDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_bLBtnDown = FALSE;
	if (m_bFirstDraw)
	{
		//???????????????????????
		m_bFirstDraw = FALSE;
		//???????????
		m_rectTracker.m_rect.SetRect(-1, -1, -1, -1);
		ClearAnnotationLayer();
		UpdateTipString();
		SetEidtWndText();
		InvalidateRgnWindow();
	}
	else
	{
		//??????
		PostQuitMessage(0);
	}

	CDialog::OnRButtonUp(nFlags, point);
}

HBRUSH CCatchScreenDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	//?????????????????????
	if (pWnd->GetDlgCtrlID() == IDC_EDIT1)
	{
		pDC->SetTextColor(RGB(247,76,128));
	}

	return hbr;
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

	HDC hScrDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	if (!hScrDC)
		return NULL;
	HDC hMemDC = CreateCompatibleDC(hScrDC);
	if (!hMemDC)
	{
		DeleteDC(hScrDC);
		return NULL;
	}
	HBITMAP hBitmap = CreateCompatibleBitmap(hScrDC, nWidth, nHeight);
	if (!hBitmap)
	{
		DeleteDC(hMemDC);
		DeleteDC(hScrDC);
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
	DeleteDC(hScrDC);
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
		AfxMessageBox(_T("?????????????"));
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
	hScrDC = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);

	//??????????????????????????????????
	hMemDC = CreateCompatibleDC(hScrDC);
	// ??????????????
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
	//?????????????
	//??? 
	DeleteDC(hScrDC);
	DeleteDC(hMemDC);
	return hBitmap;
}

// ?????????????
void CCatchScreenDlg::UpdateTipString()
{
	CString strTemp;
	if (!m_bDraw && !m_bFirstDraw)
	{
		strTemp = _T("\r\n\r\n?????????????????????\r\n\r\n??ESC?????????????");
	}
	else 
		if (m_bDraw && m_bFirstDraw)
	{
		strTemp = _T("\r\n\r\n?????????????????????\r\n\r\n??ESC?????");
	}
	else if (!m_bDraw && m_bFirstDraw)
	{
		CString sudami(_T("\r\n\r\n?????????????????????\r\n\r\n??????????????????"));
		strTemp = _T("\r\n\r\n?????????????????????");

		strTemp += sudami;
	}
	m_strEditTip = strTemp;
}

// ?????????????
void CCatchScreenDlg::DrawMessage(CRect &inRect, CDC * pDC)
{
	//?????????????????????
	const int space = 3;

	//???????????????
	CPoint pt;
	CPen pen(PS_SOLID, 1, RGB(47, 79, 79));
	CPen *pOldPen;
	pOldPen = pDC->SelectObject(&pen);

	//pDC->SetTextColor(RGB(147,147,147));
	CFont font;
	CFont * pOldFont;
	font.CreatePointFont(90, _T("????"));
	pOldFont = pDC->SelectObject(&font);

	//?????????????
	GetCursorPos(&pt);
	ScreenToClient(&pt);
	int OldBkMode;
	OldBkMode = pDC->SetBkMode(TRANSPARENT);

	TEXTMETRIC tm;
	int charHeight;
	CSize size;
	int	lineLength;
	pDC->GetTextMetrics(&tm);
	charHeight = tm.tmHeight + tm.tmExternalLeading;
	size = pDC->GetTextExtent(_T("????????  "), _tcslen(_T("????????  ")));
	lineLength = size.cx;

	//?????????, ????????????????
	CRect rect(pt.x + space, pt.y - charHeight * 6 - space, pt.x + lineLength + space, pt.y - space);

	//???????????
	CRect rectTemp;
	//?????????????????????????????
	if ((pt.x + rect.Width()) >= m_nScreenWidth)
	{
		//?????????????????
		rectTemp = rect;
		rectTemp.left = rect.left - rect.Width() - space * 2;
		rectTemp.right = rect.right - rect.Width() - space * 2;;
		rect = rectTemp;
	}

	if ((pt.y - rect.Height()) <= 0)
	{
		//?????????????????
		rectTemp = rect;
		rectTemp.top = rect.top + rect.Height() + space * 2;;
		rectTemp.bottom = rect.bottom + rect.Height() + space * 2;;
		rect = rectTemp;
	}

	//??????????????
	CBrush * pOldBrush;
	pOldBrush = pDC->SelectObject(CBrush::FromHandle((HBRUSH)GetStockObject(NULL_BRUSH)));

	pDC->Rectangle(rect);
	rect.top += 2;
	//??????????????
	CRect outRect(rect.left, rect.top, rect.left + lineLength, rect.top + charHeight);
	CString string(_T("????????"));
	pDC->DrawText(string, outRect, DT_CENTER);

	outRect.SetRect(rect.left, rect.top + charHeight, rect.left + lineLength, charHeight + rect.top + charHeight);
	string.Format(_T("(%d,%d)"), inRect.left, inRect.top);
	pDC->DrawText(string, outRect, DT_CENTER);


	outRect.SetRect(rect.left, rect.top + charHeight * 2, rect.left + lineLength, charHeight + rect.top + charHeight * 2);
	string = _T("????????");
	pDC->DrawText(string, outRect, DT_CENTER);

	outRect.SetRect(rect.left, rect.top + charHeight * 3, rect.left + lineLength, charHeight + rect.top + charHeight * 3);
	string.Format(_T("(%d,%d)"), inRect.Width(), inRect.Height());
	pDC->DrawText(string, outRect, DT_CENTER);

	outRect.SetRect(rect.left, rect.top + charHeight * 4, rect.left + lineLength, charHeight + rect.top + charHeight * 4);
	string = _T("???????");
	pDC->DrawText(string, outRect, DT_CENTER);

	outRect.SetRect(rect.left, rect.top + charHeight * 5, rect.left + lineLength, charHeight + rect.top + charHeight * 5);
	string.Format(_T("(%d,%d)"), pt.x, pt.y);
	pDC->DrawText(string, outRect, DT_CENTER);

	pDC->SetBkMode(OldBkMode);
	pDC->SelectObject(pOldFont);
	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
}

// ?????????
void CCatchScreenDlg::InvalidateRgnWindow()
{
	//?????????????????
	CRect rect1;
	GetWindowRect(rect1);

	//???????????
	CRect rect2;
	m_tipEdit.GetWindowRect(rect2);

	CRgn rgn1, rgn2;
	rgn1.CreateRectRgnIndirect(rect1);
	rgn2.CreateRectRgnIndirect(rect2);

	//???????????,????????????????
	//m_rgn.CombineRgn(&rgn1, &rgn2, RGN_DIFF);

	// ????ToolBar?????
	CRect rect3;
	::GetWindowRect(m_toolBar.GetHWND(),&rect3);
	CRgn rgn3;
	rgn3.CreateRectRgnIndirect(rect3);

	CRgn rgnTemp;
	rgnTemp.CreateRectRgn(0, 0, 50, 50);
	rgnTemp.CombineRgn(&rgn1, &rgn2, RGN_DIFF);
	m_rgn.CombineRgn(&rgnTemp, &rgn3, RGN_DIFF);

	InvalidateRgn(&m_rgn);
}

void CCatchScreenDlg::UpdateMousePointRGBString()
{
	static CString strOld("");

	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	COLORREF color;
	CClientDC dc(this);
	color = dc.GetPixel(pt);
	BYTE rValue, gValue, bValue;
	rValue = GetRValue(color);
	gValue = GetGValue(color);
	bValue = GetBValue(color);

	//?????????????
	CString string;
	string.Format(_T("\r\n\r\n?????????RGB(%d,%d,%d)"), rValue, gValue, bValue);

	//?????????????????RGB?,???????????????
	if (strOld != string)
	{
		m_strRgb = string;
	}
	strOld = string;
}

void  CCatchScreenDlg::SetEidtWndText()
{
	m_tipEdit.SetWindowText(this->GetEditText());
}

CString CCatchScreenDlg::GetEditText()
{
	CString str;
	str.Append(m_strRgb);
	str.Append(m_strEditTip);
	return str;
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
		case MyToolBar_ID:
			AfxMessageBox(_T("????"));
			break;
		case MyToolBar_ID+1:
			AfxMessageBox(_T("???"));
			break;
		case MyToolBar_ID +2:
			AfxMessageBox(_T("????"));
			break;
		case MyToolBar_ID +3:
			AfxMessageBox(_T("??????"));
			break;
		case MyToolBar_ID +4:
			AfxMessageBox(_T("????"));
			break;
		case MyToolBar_ID +5:
			AfxMessageBox(_T("????"));
			break;
		case MyToolBar_ID +6:
			SaveSelectionToFile(m_rectTracker.m_rect);
			break;
		case MyToolBar_ID +7:
			PostQuitMessage(0);
			break;
		case MyToolBar_ID +8:
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