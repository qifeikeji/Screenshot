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
	SetDlgItemText(IDC_STATIC_SAVE_DIR, L"\u4fdd\u5b58\u56fe\u7247\u8def\u5f84");
	SetDlgItemText(IDC_CHECK_COPY_EXIT, L"\u6846\u9009\u540e\u76f4\u63a5\u590d\u5236\u5230\u526a\u8d34\u677f\u5e76\u9000\u51fa\u622a\u56fe");
	SetDlgItemText(IDC_CHECK_SINGLE_MONITOR, L"\u5355\u663e\u793a\u5668\u622a\u56fe\uff08\u5feb\u6377\u952e\u65f6\u53ea\u622a\u9f20\u6807\u6240\u5728\u663e\u793a\u5668\uff09");
	SetDlgItemText(IDC_EDIT_SAVE_DIR, s.saveDirectory);
	CheckDlgButton(IDC_CHECK_COPY_EXIT, s.copyAndExitAfterSelect ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_CHECK_SINGLE_MONITOR, s.singleMonitorCapture ? BST_CHECKED : BST_UNCHECKED);

	WORD hk = 0;
	HotKeyToHotKeyCtrl(s.hotkeyModifiers, s.hotkeyVk, hk);
	if (CWnd* pHot = GetDlgItem(IDC_HOTKEY_SCREENSHOT))
		pHot->SendMessage(HKM_SETHOTKEY, hk, 0);

	m_hotKeyHost.Create(this, CRect(92, 12, 280, 40), IDC_HOTKEY_SCREENSHOT);
	m_saveDirHost.Create(this, CRect(92, 46, 280, 74), IDC_EDIT_SAVE_DIR);

	m_checkCopyExit.SubclassDlgItem(IDC_CHECK_COPY_EXIT, this);
	m_checkSingleMonitor.SubclassDlgItem(IDC_CHECK_SINGLE_MONITOR, this);

	m_btnOk.SubclassDlgItem(IDOK, this);
	m_btnCancel.SubclassDlgItem(IDCANCEL, this);
	m_btnOk.SetAccent(true);
	m_btnCancel.SetAccent(false);

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

	AppSettings& cfg = GetAppSettings();
	cfg.hotkeyModifiers = mod;
	cfg.hotkeyVk = vk;
	cfg.copyAndExitAfterSelect = (IsDlgButtonChecked(IDC_CHECK_COPY_EXIT) == BST_CHECKED);
	cfg.singleMonitorCapture = (IsDlgButtonChecked(IDC_CHECK_SINGLE_MONITOR) == BST_CHECKED);
	GetDlgItemText(IDC_EDIT_SAVE_DIR, cfg.saveDirectory);
	cfg.saveDirectory.Trim();
	cfg.Clamp();
	cfg.Save();
	CDialog::OnOK();
}
