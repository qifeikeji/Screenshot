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
	CPoint m_startPt;
	
	CMyTracker m_rectTracker;
	HCURSOR m_hCursor;
	CBitmap* m_pBitmap;
	HBITMAP m_hBitmap;
	
	CAnnotationLayer m_annotation;
	CRect m_annotationRect;
	CRect m_rectPrevDrag;

public:
	HBITMAP CopyScreenToBitmap(LPRECT lpRect, BOOL bSave = FALSE);
	HBITMAP BuildSelectionBitmap(const CRect& clientRect) const;
	BOOL CopySelectionToClipboard(const CRect& clientRect);
	BOOL SaveSelectionToFile(const CRect& clientRect);
	void SyncAnnotationLayerToSelection();
	void ClearAnnotationLayer();
	void InvalidateAroundRect(const CRect& area);
	void PositionToolBar();
	void BeginSelectionAt(CPoint point);
	void CancelCurrentSelection();

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

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
};

#endif 
