#pragma once

class CDarkButton : public CButton
{
public:
	CDarkButton();

	void SetAccent(bool accent = true);

protected:
	virtual void PreSubclassWindow();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

private:
	void DrawButton(CDC& dc, const CRect& rc, bool hover, bool pressed);
	bool m_hover;
	bool m_pressed;
	bool m_tracking;
	bool m_accent;
};
