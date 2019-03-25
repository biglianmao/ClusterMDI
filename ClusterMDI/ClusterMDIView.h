// ClusterMDIView.h : interface of the CClusterMDIView class
//


#pragma once
#include "DrawParameter.h"
typedef enum {
	TOOL_ARROW =0,
	TOOL_PEN =1,
	TOOL_DRAWFILL = 2,
	TOOL_HAND =3,
	TOOL_SELECT =4
}enum_ToolType;
typedef enum {
	ARROW_LB_DOWN,
	ARROW_HAVE_MOVE,
	ARROW_HIT_POINT,

	NONE
}enum_ToolStatus;
class CClusterMDIView : public CBCGPScrollView
{
protected: // create from serialization only
	CClusterMDIView();
	DECLARE_DYNCREATE(CClusterMDIView)

// Attributes
public:
	CClusterMDIDoc* GetDocument() const;
private:
	enum_ToolType m_ToolType;
	CPoint m_ptOrgPointInPage;
	CPoint m_ptForwardPointInPage;

	//RAWPOINTS_T m_selectedPoints;
	SELECT_POINT m_selectedPoints;
	//Arrow
	BOOL m_bCaptured;
	bool m_bArrowHasMove;
	bool m_bArrowHit;
	bool m_bArrowDrawNet;
	//拖动需要记下的数据
	SELECT_POINT::iterator m_itHitOnPoint;
	CRect  m_rcSelected;   //选中点的rc,left,top=0,0;


	HCURSOR m_HCs[4];

	float m_flZoomRate;
	CSize m_szCanvas;
	stDoubleRange m_rangeCanvas;

	CPoint       m_RefScroll;
	CPoint       m_RefPoint;
// Operations
public:
private:
	void     UpdateRulersInfo(int nMessage, CPoint ScrollPos, CPoint Pos = CPoint(0, 0));
// Overrides
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CClusterMDIView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg LRESULT OnPrintClient(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnChangeVisualManager(WPARAM wParam, LPARAM lParam);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnUpdateTool(CCmdUI *pCmdUI);
	afx_msg void OnTool(UINT nID);
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnEditFitsize();
//	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);
//	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);
	virtual BOOL OnScrollBy(CSize sizeScroll, BOOL bDoScroll = TRUE);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnEditZoomin();
	afx_msg void OnEditZoomout();
	void ClearViewBeforSwitch();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	bool CaculateNewSelected(CRect& orgRc, CRect& nowRC);
	afx_msg void OnEditDelete();
	afx_msg void OnEditCopy();
//	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
};

#ifndef _DEBUG  // debug version in ClusterMDIView.cpp
inline CClusterMDIDoc* CClusterMDIView::GetDocument() const
   { return reinterpret_cast<CClusterMDIDoc*>(m_pDocument); }
#endif

