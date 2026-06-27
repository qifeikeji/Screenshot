#pragma once

class CDarkMacInputHost : public CWnd
{
public:
	CDarkMacInputHost();

	BOOL Create(CWnd* pParent, const CRect& rect, UINT childCtrlId);

protected:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()

private:
	CBrush m_brInner;
	UINT m_childId;
};
