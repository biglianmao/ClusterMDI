// ChildFrm.h : interface of the CChildFrame class
//


#pragma once

#include "ruler.h"

class CChildFrame : public CBCGPMDIChildWnd
{
	DECLARE_DYNCREATE(CChildFrame)
public:
	CChildFrame();

// Attributes
public:

private:
// Operations
public:

// Overrides
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
};


class CChildFrameWithRuler : public CBCGPMDIChildWnd
{
	DECLARE_DYNCREATE(CChildFrameWithRuler)

public:
	CChildFrameWithRuler();

	// Attributes
public:
private:
	CRulerSplitterWnd m_Rulers;
	// Operations
public:
	void ShowRulers(BOOL bShow);
	void UpdateRulersInfo(stRULER_INFO stRulerInfo);

	// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	// Implementation
public:
	virtual ~CChildFrameWithRuler();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
public:
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
};