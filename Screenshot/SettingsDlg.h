#pragma once

#include "resource.h"
#include "DarkButton.h"
#include "DarkMacInputHost.h"
#include "DarkMacCheckBox.h"

class CSettingsDlg : public CDialog
{
public:
	CSettingsDlg(CWnd* pParent = NULL);

	enum { IDD = IDD_SETTINGS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedOk();
	DECLARE_MESSAGE_MAP()

private:
	CBrush m_brBg;
	CDarkButton m_btnOk;
	CDarkButton m_btnCancel;
	CDarkMacInputHost m_hotKeyHost;
	CDarkMacInputHost m_saveDirHost;
	CDarkMacCheckBox m_checkCopyExit;
	CDarkMacCheckBox m_checkSingleMonitor;
};
