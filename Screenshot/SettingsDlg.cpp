#include "stdafx.h"
#include "SettingsDlg.h"
#include "AppSettings.h"
#include "Resource.h"

CSettingsDlg::CSettingsDlg(CWnd* pParent)
	: CDialog(CSettingsDlg::IDD, pParent)
	, m_width(280)
	, m_height(88)
	, m_gray(128)
	, m_opacity(39)
{
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_WIN_W, m_width);
	DDX_Text(pDX, IDC_EDIT_WIN_H, m_height);
	DDX_Text(pDX, IDC_EDIT_MASK_GRAY, m_gray);
	DDX_Text(pDX, IDC_EDIT_MASK_OPACITY, m_opacity);
	DDV_MinMaxInt(pDX, m_width, 220, 800);
	DDV_MinMaxInt(pDX, m_height, 100, 600);
	DDV_MinMaxInt(pDX, m_gray, 0, 255);
	DDV_MinMaxInt(pDX, m_opacity, 0, 100);
}

BEGIN_MESSAGE_MAP(CSettingsDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDOK, &CSettingsDlg::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CSettingsDlg::OnInitDialog()
{
	const AppSettings& s = GetAppSettings();
	m_width = s.windowWidth;
	m_height = s.windowHeight;
	m_gray = s.maskGray;
	m_opacity = s.maskOpacity;
	m_brBg.CreateSolidBrush(RGB(30, 30, 30));

	CDialog::OnInitDialog();
	SetWindowText(_T("设置"));
	UpdateData(FALSE);
	return TRUE;
}

HBRUSH CSettingsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (nCtlColor == CTLCOLOR_DLG || nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkColor(RGB(30, 30, 30));
		pDC->SetTextColor(RGB(235, 235, 235));
		return (HBRUSH)m_brBg.GetSafeHandle();
	}
	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CSettingsDlg::OnBnClickedOk()
{
	if (!UpdateData(TRUE))
		return;
	AppSettings& s = GetAppSettings();
	s.windowWidth = m_width;
	s.windowHeight = m_height;
	s.maskGray = m_gray;
	s.maskOpacity = m_opacity;
	s.Clamp();
	s.Save();
	CDialog::OnOK();
}
