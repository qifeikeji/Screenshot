#include "stdafx.h"
#include "Screenshot.h"
#include "ScreenshotDlg.h"
#include "CatchScreenDlg.h"
#include "AppSettings.h"
#include "SettingsDlg.h"

#ifdef _DEBUG
#endif

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
	ON_BN_CLICKED(IDC_BTN_START, &CScreenshotDlg::OnBnClickedBtnStart)
	ON_BN_CLICKED(IDC_BTN_SETTINGS, &CScreenshotDlg::OnBnClickedBtnSettings)
END_MESSAGE_MAP()

void CScreenshotDlg::ApplyWindowSizeFromSettings()
{
	const AppSettings& s = GetAppSettings();
	SetWindowPos(NULL, 0, 0, s.windowWidth, s.windowHeight, SWP_NOMOVE | SWP_NOZORDER);
	CenterWindow();
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
	return TRUE;
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
	CCatchScreenDlg dlg;
	dlg.DoModal();
	::ShowWindow(hWndMain, SW_SHOW);
	return 0;
}

void CScreenshotDlg::OnBnClickedBtnStart()
{
	::ShowWindow(m_hWnd, SW_HIDE);
	AfxBeginThread(SccreenShot_Thread, (LPVOID)GetSafeHwnd());
}

void CScreenshotDlg::OnBnClickedBtnSettings()
{
	CSettingsDlg dlg(this);
	if (dlg.DoModal() == IDOK)
		ApplyWindowSizeFromSettings();
}
