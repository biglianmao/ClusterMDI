// ChildFrm.cpp : implementation of the CChildFrame class
//
#include "stdafx.h"
#include "ClusterMDI.h"

#include "ChildFrm.h"

#include "ClusterMDIDoc.h"
#include "ClusterMDIView.h"
#include "GridView.h"
#include "MainFrm.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildFrame

IMPLEMENT_DYNCREATE(CChildFrame, CBCGPMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrame, CBCGPMDIChildWnd)
	ON_WM_MDIACTIVATE()
//	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CChildFrame construction/destruction

CChildFrame::CChildFrame()
{
	// TODO: add member initialization code here
}

CChildFrame::~CChildFrame()
{
}


BOOL CChildFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs
	if( !CBCGPMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	cs.style = WS_CHILD | WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION 
		| FWS_ADDTOTITLE | WS_THICKFRAME | WS_MAXIMIZE;

	return TRUE;
}


// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrame::AssertValid() const
{
	CBCGPMDIChildWnd::AssertValid();
}

void CChildFrame::Dump(CDumpContext& dc) const
{
	CBCGPMDIChildWnd::Dump(dc);
}

#endif //_DEBUG


// CChildFrame message handlers



// CChildFrameWithRuler

IMPLEMENT_DYNCREATE(CChildFrameWithRuler, CBCGPMDIChildWnd)

BEGIN_MESSAGE_MAP(CChildFrameWithRuler, CBCGPMDIChildWnd)
	ON_WM_CLOSE()
	ON_WM_MDIACTIVATE()
END_MESSAGE_MAP()


// CChildFrameWithRuler construction/destruction

CChildFrameWithRuler::CChildFrameWithRuler()
{
	// TODO: add member initialization code here
}

CChildFrameWithRuler::~CChildFrameWithRuler()
{
}

BOOL CChildFrameWithRuler::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs
	if (!CBCGPMDIChildWnd::PreCreateWindow(cs))
		return FALSE;

	cs.style = WS_CHILD | WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION
		| FWS_ADDTOTITLE | WS_THICKFRAME | WS_MAXIMIZE;

	return TRUE;
}

// CChildFrame diagnostics

#ifdef _DEBUG
void CChildFrameWithRuler::AssertValid() const
{
	CBCGPMDIChildWnd::AssertValid();
}

void CChildFrameWithRuler::Dump(CDumpContext& dc) const
{
	CBCGPMDIChildWnd::Dump(dc);
}

#endif //_DEBUG
// CChildFrameWithRuler message handlers


void CChildFrameWithRuler::ShowRulers(BOOL bShow)
{
	m_Rulers.ShowRulers(bShow);
}

void CChildFrameWithRuler::UpdateRulersInfo(stRULER_INFO stRulerInfo)
{
	m_Rulers.UpdateRulersInfo(stRulerInfo);
}

BOOL CChildFrameWithRuler::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (!m_Rulers.CreateRulers(this, pContext)) {
		TRACE("Error creation of rulers\n");
		return CBCGPMDIChildWnd::OnCreateClient(lpcs, pContext);
	}

	return TRUE;

	//return CChildFrame::OnCreateClient(lpcs, pContext);
}


void CChildFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CBCGPMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

	CGridView* pView = (CGridView*)GetActiveView();
	if (!pView)
		return;
	// TODO: 在此处添加消息处理程序代码
	if (!bActivate)
	{
		pView->SaveData();
	}
	else
	{
		pView->RetrieveData();

	}

}



void CChildFrameWithRuler::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
{
	CBCGPMDIChildWnd::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);

	// TODO: 在此处添加消息处理程序代码
	CMainFrame *pMainFrame = (CMainFrame *)theApp.GetMainWnd();
	if (bActivate)
	{
		CList<UINT, UINT> lstHiddenCommands;
		lstHiddenCommands.AddHead(ID_EDIT_INSERTROW);
		lstHiddenCommands.AddHead(ID_EDIT_DELETEROW);
		lstHiddenCommands.AddHead(ID_EDIT_FILL);
		lstHiddenCommands.AddHead(ID_EDIT_EXPORT);
		lstHiddenCommands.AddHead(ID_EDIT_IMPORT);
		lstHiddenCommands.AddHead(ID_EDIT_SORT);
		pMainFrame->SetHiddenCommands(lstHiddenCommands);

		pMainFrame->ShowControlBar(&pMainFrame->m_wndDrawInfo, TRUE, FALSE, FALSE);
	}
	else
	{
		CList<UINT, UINT> lstHiddenCommands;
		lstHiddenCommands.AddHead(ID_EDIT_ARROW);
		lstHiddenCommands.AddHead(ID_EDIT_PEN);
		lstHiddenCommands.AddHead(ID_EDIT_DRAWFILE);
		lstHiddenCommands.AddHead(ID_EDIT_HAND);
		lstHiddenCommands.AddHead(ID_EDIT_DELETE);
		lstHiddenCommands.AddHead(ID_EDIT_COPY);
		lstHiddenCommands.AddHead(ID_EDIT_ZOOMIN);
		lstHiddenCommands.AddHead(ID_EDIT_ZOOMOUT);
		lstHiddenCommands.AddHead(ID_EDIT_FITSIZE);
		CMainFrame *pMainFrame = (CMainFrame *)theApp.GetMainWnd();
		pMainFrame->SetHiddenCommands(lstHiddenCommands);

		pMainFrame->ShowControlBar(&pMainFrame->m_wndDrawInfo, FALSE, FALSE, FALSE);
	}
	//CClusterMDIView* pView = (CClusterMDIView*)GetActiveView();
	//if (!pView)
	//	return;
	//if (!bActivate)
	//{
	//	//pView->SaveData();
	//}
	//else
	//{
	//	pView->ClearViewBeforSwitch();
	//}
	//if (pTargetFrame->GetActiveView()->IsKindOf(RUNTIME_CLASS(CClusterMDIView)))
	//{
	//	CClusterMDIView* pClusterView = (CClusterMDIView*)pTargetFrame->GetActiveView();
	//	pClusterView->ClearViewBeforSwitch();
	//}
}


void CChildFrameWithRuler::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	// TODO: 在此添加专用代码和/或调用基类

	//CBCGPMDIChildWnd::OnUpdateFrameTitle(bAddToTitle);
	//CBCGPMDIChildWnd::OnUpdateFrameTitle(FALSE);

	CDocument* pDocument = GetActiveDocument();
	if (true)
	{
		TCHAR szText[256 + _MAX_PATH];
		if (pDocument == NULL)
			Checked::tcsncpy_s(szText, _countof(szText), m_strTitle, _TRUNCATE);
		else
			Checked::tcsncpy_s(szText, _countof(szText), pDocument->GetTitle(), _TRUNCATE);

		// set title if changed, but don't remove completely
		AfxSetWindowText(m_hWnd, szText);
	}

}


void CChildFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	// TODO: 在此添加专用代码和/或调用基类

	//CBCGPMDIChildWnd::OnUpdateFrameTitle(bAddToTitle);

	CDocument* pDocument = GetActiveDocument();
	if (true)
	{
		TCHAR szText[256 + _MAX_PATH];
		if (pDocument == NULL)
			Checked::tcsncpy_s(szText, _countof(szText), m_strTitle, _TRUNCATE);
		else
			Checked::tcsncpy_s(szText, _countof(szText), pDocument->GetTitle(), _TRUNCATE);

		// set title if changed, but don't remove completely
		AfxSetWindowText(m_hWnd, szText);
	}
}
