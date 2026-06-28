#pragma once

#define DarkToolBar_CommandBase 600

enum DarkToolBarButton
{
	DarkToolBarBtnUndo = 0,
	DarkToolBarBtnArrow,
	DarkToolBarBtnRect,
	DarkToolBarBtnEllipse,
	DarkToolBarBtnBrush,
	DarkToolBarBtnText,
	DarkToolBarBtnSave,
	DarkToolBarBtnFolder,
	DarkToolBarBtnColorRed,
	DarkToolBarBtnColorYellow,
	DarkToolBarBtnColorBlue,
	DarkToolBarBtnColorWhite,
	DarkToolBarBtnThickSmall,
	DarkToolBarBtnThickLarge,
	DarkToolBarBtnExit,
	DarkToolBarBtnFinish,
	DarkToolBarBtnCount
};

class CDarkToolBar : public CWnd
{
public:
	CDarkToolBar();
	virtual ~CDarkToolBar();

	BOOL Create(CWnd* pParent);
	void SetBelowSelectionClient(const CRect& selectionClient, const CRect& parentClient);
	void ShowBar();
	void HideBar();
	HWND GetHWND() const { return m_hWnd; }

	void SetSelectedColorIndex(int index);
	void SetSelectedThicknessIndex(int index);
	int GetSelectedColorIndex() const { return m_selectedColorIndex; }
	int GetSelectedThicknessIndex() const { return m_selectedThicknessIndex; }

protected:
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()

private:
	int HitTest(CPoint pt) const;
	CRect ButtonRect(int index) const;
	void InvalidateHoverRegion(int prevIndex, int newIndex);
	void InvalidateButton(int index);
	int ToolbarWidth() const;
	int ToolbarHeight() const;

	HIMAGELIST m_hImageList;
	int m_hoverIndex;
	int m_pressedIndex;
	int m_selectedColorIndex;
	int m_selectedThicknessIndex;
	bool m_trackingLeave;
	static const int kRow1Count = 8;
	static const int kRow2Count = 8;
	static const int kButtonSize = 28;
	static const int kPadding = 6;
	static const int kRowGap = 4;
};
