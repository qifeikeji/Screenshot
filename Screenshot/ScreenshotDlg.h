#pragma once

#include "Resource.h"
#include "DarkButton.h"

class CScreenshotDlg : public CDialog
{
public:
	CScreenshotDlg(CWnd* pParent = NULL);

	enum { IDD = IDD_SCREENSHOT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnDestroy();
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnBnClickedBtnSettings();

	void StartScreenshot();
	void RegisterScreenshotHotKey();
	void UnregisterScreenshotHotKey();

private:
	HICON m_hIcon;
	CBrush m_brBackground;
	CDarkButton m_btnStart;
	CDarkButton m_btnSettings;
	static const UINT kHotKeyId = 1;
	void ApplyWindowSizeFromSettings();
	void SaveWindowSizeToSettings();
	void LayoutButtons(int cx, int cy);
};
