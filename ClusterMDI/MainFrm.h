// MainFrm.h : interface of the CMainFrame class
//


#pragma once

#include "WorkSpaceBar.h"
#include "WorkSpaceBar2.h"
#include "OutputBar.h"
#include "PropertiesViewBar.h"
#include "DialogBarDrawInfo.h"

class CMainFrame : public CBCGPMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:

// Operations
public:

	void SetHiddenCommands(CList<UINT, UINT>& lstHiddenCommands);
// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CDialogBarDrawInfo m_wndDrawInfo;
protected:  // control bar embedded members

	CList<UINT, UINT>	m_lstHiddenCommands;

	CBCGPStatusBar			m_wndStatusBar;
	CBCGPMenuBar			m_wndMenuBar;
	CBCGPToolBar			m_wndToolBar;
	//CWorkSpaceBar			m_wndWorkSpace;
	//CWorkSpaceBar2			m_wndWorkSpace2;
	//COutputBar				m_wndOutput;
	//CBCGPPropBar			m_wndPropGrid;

	CMDIChildWnd* m_pDrawViewFrame;
	CMDIChildWnd* m_pGridViewFrame;
// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg LRESULT OnToolbarReset(WPARAM,LPARAM);
	afx_msg void OnWindowManager();
	afx_msg void OnClose();
	afx_msg void OnUpdateDrawOrList(CCmdUI *pCmdUI);
	afx_msg void OnDrawOrList(UINT nID);
	afx_msg LRESULT OnMouseXY(WPARAM, LPARAM);
	afx_msg void OnAlgorithmCombo();
	DECLARE_MESSAGE_MAP()

	virtual CBCGPMDIChildWnd* CreateDocumentWindow (LPCTSTR lpcszDocName, CObject* /*pObj*/);
public:
	afx_msg void OnFileNew();
	afx_msg void OnFileOpen();
	afx_msg void OnRunBegin();
	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);
	afx_msg void OnUpdateRunBegin(CCmdUI *pCmdUI);
};
