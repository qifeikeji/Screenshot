#pragma once

class CDarkHotKeyFrame : public CWnd
{
public:
	CDarkHotKeyFrame();
	BOOL Create(CWnd* pParent, const CRect& rect, UINT hotKeyCtrlId);

protected:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
};
