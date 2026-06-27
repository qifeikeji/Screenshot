#include "stdafx.h"
#include "SettingsDlg.h"
#include "AppSettings.h"
#include "Resource.h"

CSettingsDlg::CSettingsDlg(CWnd* pParent)
	: CDialog(CSettingsDlg::IDD, pParent)
{
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HOTKEY_SCREENSHOT, m_hotKey);
}

BEGIN_MESSAGE_MAP(CSettingsDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDOK, &CSettingsDlg::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CSettingsDlg::OnInitDialog()
{
	const AppSettings& s = GetAppSettings();
	m_brBg.CreateSolidBrush(RGB(30, 30, 30));

	CDialog::OnInitDialog();
	SetWindowText(_T("设置"));

	WORD hk = 0;
	HotKeyToHotKeyCtrl(s.hotkeyModifiers, s.hotkeyVk, hk);
	m_hotKey.SetHotKey(LOBYTE(hk), HIBYTE(hk));
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
	DWORD dw = m_hotKey.GetHotKey();
	WORD hk = MAKEWORD(LOBYTE(dw), HIBYTE(dw));
	UINT mod = 0;
	UINT vk = 0;
	HotKeyFromHotKeyCtrl(hk, mod, vk);
	if (vk == 0)
	{
		AfxMessageBox(_T("请设置有效的快捷键。"));
		return;
	}
	if (mod == 0)
	{
		AfxMessageBox(_T("快捷键需包含 Ctrl、Alt 或 Shift 等组合键。"));
		return;
	}

	AppSettings& s = GetAppSettings();
	s.hotkeyModifiers = mod;
	s.hotkeyVk = vk;
	s.Clamp();
	s.Save();
	CDialog::OnOK();
}
