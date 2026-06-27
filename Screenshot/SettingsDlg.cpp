#include "stdafx.h"
#include "SettingsDlg.h"
#include "AppSettings.h"
#include "resource.h"

#ifndef HKM_SETHOTKEY
#define HKM_SETHOTKEY (WM_USER + 1)
#endif
#ifndef HKM_GETHOTKEY
#define HKM_GETHOTKEY (WM_USER + 2)
#endif

CSettingsDlg::CSettingsDlg(CWnd* pParent)
	: CDialog(CSettingsDlg::IDD, pParent)
{
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSettingsDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDOK, &CSettingsDlg::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CSettingsDlg::OnInitDialog()
{
	const AppSettings& s = GetAppSettings();
	m_brBg.CreateSolidBrush(RGB(30, 30, 30));

	if (!CDialog::OnInitDialog())
		return FALSE;

	SetWindowText(L"\u8bbe\u7f6e");
	SetDlgItemText(IDC_STATIC_HOTKEY_LABEL, L"\u622a\u56fe\u5feb\u6377\u952e");

	WORD hk = 0;
	HotKeyToHotKeyCtrl(s.hotkeyModifiers, s.hotkeyVk, hk);
	if (CWnd* pHot = GetDlgItem(IDC_HOTKEY_SCREENSHOT))
		pHot->SendMessage(HKM_SETHOTKEY, hk, 0);

	CenterWindow(GetParent());
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
	CWnd* pHot = GetDlgItem(IDC_HOTKEY_SCREENSHOT);
	if (!pHot)
	{
		CDialog::OnOK();
		return;
	}

	const DWORD dw = (DWORD)pHot->SendMessage(HKM_GETHOTKEY, 0, 0);
	WORD hk = MAKEWORD(LOBYTE(dw), HIBYTE(dw));
	UINT mod = 0;
	UINT vk = 0;
	HotKeyFromHotKeyCtrl(hk, mod, vk);
	if (vk == 0)
	{
		AfxMessageBox(L"\u8bf7\u8bbe\u7f6e\u6709\u6548\u7684\u5feb\u6377\u952e\u3002");
		return;
	}
	if (mod == 0)
	{
		AfxMessageBox(L"\u5feb\u6377\u952e\u9700\u5305\u542b Ctrl\u3001Alt \u6216 Shift \u7b49\u7ec4\u5408\u952e\u3002");
		return;
	}

	AppSettings& s = GetAppSettings();
	s.hotkeyModifiers = mod;
	s.hotkeyVk = vk;
	s.Clamp();
	s.Save();
	CDialog::OnOK();
}
