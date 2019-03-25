/////////////////////////////////////////////////////////////////////////////
// Rulers. Written By Stefan Ungureanu (stefanu@usa.net)
//

#if !defined(AFX_RULER_H__CC6B5F05_3281_11D2_B4F2_0000E8DB069D__INCLUDED_)
#define AFX_RULER_H__CC6B5F05_3281_11D2_B4F2_0000E8DB069D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "DrawParameter.h"
/////////////////////////////////////////////////////////////////////////////
// Ruler stuff

// ruler types
#define RT_VERTICAL	  0
#define RT_HORIZONTAL 1

// hint information
#define RW_VSCROLL	  1
#define RW_HSCROLL	  2
#define RW_POSITION   3

#define RULER_SIZE    25
#define RULER_MARGIN	3

#define RID_SCROLLPOS	1
#define RID_MOUSEPOS	2
#define RID_DOCSIZE		4
#define RID_REALRANGE	8
#define RID_ZOOMFACTOR	16


struct stRULER_INFO {
    UINT   uMessage;
	UINT   uRID;
    CPoint ScrollPos;
    CPoint Pos;
    CSize  DocSize;
	stDoubleRange RealRange;
    float  fZoomFactor;
};

/////////////////////////////////////////////////////////////////////////////
// CRulerSplitterWnd window

class CRulerSplitterWnd : public CSplitterWnd
{
	DECLARE_DYNAMIC(CRulerSplitterWnd)
// Construction
public:
	CRulerSplitterWnd();
    BOOL CreateRulers(CFrameWnd* pParent, CCreateContext* pContext);
    void ShowRulers(BOOL bShow = TRUE, BOOL bSave = TRUE);
    void UpdateRulersInfo(stRULER_INFO stRulerInfo); 
    
// Attributes
private:
    BOOL m_bRulersVisible;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRulerSplitterWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CRulerSplitterWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CRulerSplitterWnd)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CRulerView view

class CRulerView : public CView
{
protected:
	CRulerView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CRulerView)

// Attributes
private:
	UINT   m_rulerType;
	CPoint m_scrollPos;
	CPoint m_lastPos;
    CSize  m_DocSize;
	stDoubleRange m_rangePoints;
    float  m_fZoomFactor;

// Operations
public:
	void SetRulerType(UINT rulerType = RT_HORIZONTAL)
    {
        m_rulerType   = rulerType;
    }
    void UpdateRulersInfo(stRULER_INFO stRulerInfo);
    
private:
    void DrawTicker(CDC* pDC, CRect rulerRect, int nFactor, int nBegin = 0, BOOL bShowPos = TRUE);
    void DrawCursorPos(CPoint NewPos);
	void DrawHRuler(CDC* pDC);
	void DrawVRuler(CDC* pDC, CRect rulerRect);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRulerView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CRulerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CRulerView)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CRulerCornerView view

class CRulerCornerView : public CView
{
protected:
	CRulerCornerView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CRulerCornerView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRulerCornerView)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnDraw(CDC* pDC);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CRulerCornerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CRulerCornerView)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_H__CC6B5F05_3281_11D2_B4F2_0000E8DB069D__INCLUDED_)
