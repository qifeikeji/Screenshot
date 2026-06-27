#include "stdafx.h"
#include "Screenshot.h"
#include "ScreenshotDlg.h"
#include "CatchScreenDlg.h"
#include "AppSettings.h"
#include "SettingsDlg.h"
#include "ScreenCapture.h"

CScreenshotDlg::CScreenshotDlg(CWnd* pParent)
	: CDialog(CScreenshotDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CScreenshotDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_START, m_btnStart);
	DDX_Control(pDX, IDC_BTN_SETTINGS, m_btnSettings);
}

BEGIN_MESSAGE_MAP(CScreenshotDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_DESTROY()
	ON_WM_HOTKEY()
	ON_BN_CLICKED(IDC_BTN_START, &CScreenshotDlg::OnBnClickedBtnStart)
	ON_BN_CLICKED(IDC_BTN_SETTINGS, &CScreenshotDlg::OnBnClickedBtnSettings)
END_MESSAGE_MAP()

void CScreenshotDlg::ApplyWindowSizeFromSettings()
{
	const AppSettings& s = GetAppSettings();
	SetWindowPos(NULL, 0, 0, s.windowWidth, s.windowHeight, SWP_NOMOVE | SWP_NOZORDER);
	CenterWindow();
	CRect rc;
	GetClientRect(&rc);
	LayoutButtons(rc.Width(), rc.Height());
}

void CScreenshotDlg::SaveWindowSizeToSettings()
{
	CRect wr;
	GetWindowRect(&wr);
	AppSettings& s = GetAppSettings();
	s.windowWidth = wr.Width();
	s.windowHeight = wr.Height();
	s.Clamp();
	s.Save();
}

void CScreenshotDlg::LayoutButtons(int cx, int cy)
{
	const int btnW = 96;
	const int btnH = 26;
	const int gap = 16;
	const int totalW = btnW * 2 + gap;
	const int x0 = (cx - totalW) / 2;
	const int y0 = (cy - btnH) / 2;
	const int x = x0 >= 8 ? x0 : 8;
	const int y = y0 >= 8 ? y0 : 8;
	if (m_btnStart.GetSafeHwnd())
		m_btnStart.SetWindowPos(NULL, x, y, btnW, btnH, SWP_NOZORDER);
	if (m_btnSettings.GetSafeHwnd())
		m_btnSettings.SetWindowPos(NULL, x + btnW + gap, y, btnW, btnH, SWP_NOZORDER);
}

BOOL CScreenshotDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	m_brBackground.CreateSolidBrush(RGB(28, 28, 30));
	m_btnStart.SetAccent(true);
	m_btnSettings.SetAccent(false);
	ApplyWindowSizeFromSettings();
	m_btnStart.SetWindowText(L"\u622a\u56fe");
	m_btnSettings.SetWindowText(L"\u8bbe\u7f6e");
	SetWindowText(L"\u622a\u56fe\u5de5\u5177");
	RegisterScreenshotHotKey();
	return TRUE;
}

void CScreenshotDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	if (nType != SIZE_MINIMIZED && cx > 0 && cy > 0)
		LayoutButtons(cx, cy);
}

void CScreenshotDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	lpMMI->ptMinTrackSize.x = 220;
	lpMMI->ptMinTrackSize.y = 72;
	CDialog::OnGetMinMaxInfo(lpMMI);
}

void CScreenshotDlg::OnDestroy()
{
	UnregisterScreenshotHotKey();
	SaveWindowSizeToSettings();
	CDialog::OnDestroy();
}

void CScreenshotDlg::RegisterScreenshotHotKey()
{
	UnregisterScreenshotHotKey();
	const AppSettings& s = GetAppSettings();
	if (!RegisterHotKey(m_hWnd, kHotKeyId, s.hotkeyModifiers, s.hotkeyVk))
	{
		// 可能被占用，静默失败；用户可在设置中更换
	}
}

void CScreenshotDlg::UnregisterScreenshotHotKey()
{
	UnregisterHotKey(m_hWnd, kHotKeyId);
}

void CScreenshotDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
	if (nHotKeyId == kHotKeyId)
		StartScreenshot();
	CDialog::OnHotKey(nHotKeyId, nKey1, nKey2);
}

void CScreenshotDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		dc.DrawIcon((rect.Width() - cxIcon + 1) / 2, (rect.Height() - cyIcon + 1) / 2, m_hIcon);
	}
	else
	{
		CPaintDC dc(this);
		CRect rc;
		GetClientRect(&rc);
		dc.FillSolidRect(&rc, RGB(28, 28, 30));
	}
}

HCURSOR CScreenshotDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

HBRUSH CScreenshotDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkColor(RGB(28, 28, 30));
		pDC->SetTextColor(RGB(235, 235, 235));
		return (HBRUSH)m_brBackground.GetSafeHandle();
	}
	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CScreenshotDlg::OnEraseBkgnd(CDC* pDC)
{
	CRect rc;
	GetClientRect(&rc);
	pDC->FillSolidRect(&rc, RGB(28, 28, 30));
	return TRUE;
}

UINT SccreenShot_Thread(LPVOID lpParam)
{
	HWND hWndMain = (HWND)lpParam;
	::Sleep(80);
	CCatchScreenDlg dlg;
	dlg.DoModal();
	::ShowWindow(hWndMain, SW_SHOW);
	return 0;
}

void CScreenshotDlg::StartScreenshot()
{
	if (!IsWindowVisible())
		return;
	POINT cursor = {};
	GetCursorPos(&cursor);
	SetCaptureCursorAnchor(cursor);
	::ShowWindow(m_hWnd, SW_HIDE);
	AfxBeginThread(SccreenShot_Thread, (LPVOID)GetSafeHwnd());
}

void CScreenshotDlg::OnBnClickedBtnStart()
{
	StartScreenshot();
}

void CScreenshotDlg::OnBnClickedBtnSettings()
{
	CSettingsDlg dlg(this);
	const INT_PTR ret = dlg.DoModal();
	if (ret == IDOK)
		RegisterScreenshotHotKey();
}
