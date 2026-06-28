#if !defined(AFX_CATCHSCREENDLG_H__536FDBC8_7DB2_4BEF_8943_70DBE8AD845F__INCLUDED_)
#define AFX_CATCHSCREENDLG_H__536FDBC8_7DB2_4BEF_8943_70DBE8AD845F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 

#include "Resource.h"
#include "MyEdit.h"
#ifndef MYTRACKER_
#include "MyTracker.h"
#endif
#include "DarkToolBar.h"
#include "AnnotationLayer.h"
#include "TextAnnotOverlay.h"

enum AnnotTool
{
	AnnotToolNone = 0,
	AnnotToolArrow,
	AnnotToolRect,
	AnnotToolEllipse,
	AnnotToolBrush,
	AnnotToolText
};

struct CatchScreenEditUndoSnapshot
{
	HBITMAP hAnnot = NULL;
	int annotCx = 0;
	int annotCy = 0;
	std::vector<TextAnnotBlock> texts;
};

class CCatchScreenDlg : public CDialog
{
public:
	CCatchScreenDlg(CWnd* pParent = NULL);	
	enum { IDD = IDD_DIALOGFORIMG };

	CMyEdit m_tipEdit;
	CDarkToolBar m_toolBar;

	int m_nOriginX;
	int m_nOriginY;
	int m_nScreenWidth;
	int m_nScreenHeight;

	BOOL m_bLBtnDown;
	BOOL m_bDraw;
	BOOL m_bFirstDraw;
	BOOL m_bQuit;
	BOOL m_bToolBarShown;
	CPoint m_startPt;
	
	CMyTracker m_rectTracker;
	HCURSOR m_hCursor;
	CBitmap* m_pBitmap;
	HBITMAP m_hBitmap;
	
	CAnnotationLayer m_annotation;
	CRect m_annotationRect;
	CRect m_rectPrevDrag;

	AnnotTool m_activeTool;
	COLORREF m_annotColor;
	int m_annotPenWidth;
	BOOL m_bAnnotating;
	CPoint m_annotStart;
	CPoint m_annotLast;
	CRect m_previewRect;

	CTextAnnotOverlay m_textOverlay;
	CFont m_textAnnotFont;
	int m_editingTextIndex;
	int m_dragTextIndex;
	CPoint m_dragTextGrabOffset;
	int m_textCaretPos;
	BOOL m_bCaretVisible;
	BOOL m_bPendingReselect;
	CPoint m_pendingReselectPt;
	static const UINT kTextCaretTimerId = 6001;
	static const int kReselectDragThresholdPx = 10;

	std::vector<CatchScreenEditUndoSnapshot> m_editUndoStack;
	void PushEditUndoSnapshot();
	BOOL PerformEditUndo();
	void ClearEditUndoStack();

	bool HasValidSelection() const;
	void InvalidateTextBlockRegion(const CRect& blockRect);

public:
	HBITMAP CopyScreenToBitmap(LPRECT lpRect, BOOL bSave = FALSE);
	HBITMAP BuildSelectionBitmap(const CRect& clientRect) const;
	BOOL CopySelectionToClipboard(const CRect& clientRect);
	BOOL SaveSelectionToFile(const CRect& clientRect);
	void SyncAnnotationLayerToSelection();
	void ClearAnnotationLayer();
	void InvalidateAroundRect(const CRect& area);
	void InvalidateSelectionFrame(const CRect& rect);
	void InvalidateSelectionFrameUnion(const CRect& a, const CRect& b);
	CRect SelectionFrameInvalidRect(CRect rect) const;
	void PositionToolBar();
	void BeginSelectionAt(CPoint point);
	void CancelCurrentSelection();

	BOOL ClientToAnnot(CPoint client, CPoint& out) const;
	BOOL IsPointInSelection(CPoint client) const;
	void InvalidateAnnotationView();
	void DrawPreviewShape(HDC hdc) const;
	void DrawSelectionSizeLabels(CDC& dc);
	BOOL PromptAnnotText(CString& text);
	void FinishSelectionIfValid(const CPoint& endPt);
	void OpenSaveFolder();
	void ClearTextOverlay();
	void EndTextEdit(BOOL commit);
	void BeginTextEdit(int index);
	void SyncTextBlockLayout(int index);
	void DrawTextOverlay(CDC& dc) const;
	void DrawOneTextBlock(CDC& dc, const TextAnnotBlock& block, bool editing, int caretPos, bool caretVisible) const;
	void CompositeTextOverlay(HDC hdc, const CRect& selectionClient) const;
	void GetCaretClientPoint(CDC& dc, const CRect& textRc, const CString& text, int caretPos, CPoint& out) const;
	void InsertCharAtCaret(TCHAR ch);
	void DeleteCharBeforeCaret();
	int MaxTextWrapWidth() const;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
};

#endif 
