#pragma once

#define DarkToolBar_CommandBase 600

class CDarkToolBar : public CWnd
{
public:
	CDarkToolBar();
	virtual ~CDarkToolBar();

	BOOL Create(CWnd* pParent);
	void SetShowPlaceScreen(int screenX, int screenY);
	void ShowBar();
	void HideBar();
	HWND GetHWND() const { return m_hWnd; }

protected:
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()

private:
	int HitTest(CPoint pt) const;

	HIMAGELIST m_hImageList;
	int m_hoverIndex;
	int m_pressedIndex;
	bool m_trackingLeave;
	static const int kButtonCount = 9;
	static const int kButtonSize = 28;
	static const int kPadding = 6;
};
