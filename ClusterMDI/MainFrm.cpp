// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "ClusterMDI.h"

#include "MainFrm.h"
#include "ClusterMDIDoc.h"
#include "ClusterMDIView.h"
#include "GridView.h"

#include "DialogRun.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CBCGPMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CBCGPMDIFrameWnd)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_WINDOW_MANAGER, OnWindowManager)
	ON_REGISTERED_MESSAGE(BCGM_RESETTOOLBAR, OnToolbarReset)
	ON_UPDATE_COMMAND_UI_RANGE(ID_EDIT_DRAW, ID_EDIT_LIST, OnUpdateDrawOrList)
	ON_COMMAND_RANGE(ID_EDIT_DRAW, ID_EDIT_LIST, OnDrawOrList)
	//ON_COMMAND(ID_FILE_NEW, &CMainFrame::OnFileNew)
	//ON_COMMAND(ID_FILE_OPEN, &CMainFrame::OnFileOpen)
	ON_MESSAGE(WM_MOUSE_XY, OnMouseXY)
	ON_COMMAND(ID_RUN_ALGORITHM_COMBO, OnAlgorithmCombo)
	ON_CBN_SELENDOK(ID_RUN_ALGORITHM_COMBO, OnAlgorithmCombo)
	ON_COMMAND(ID_RUN_BEGIN, &CMainFrame::OnRunBegin)
	ON_UPDATE_COMMAND_UI(ID_RUN_BEGIN, &CMainFrame::OnUpdateRunBegin)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	m_pDrawViewFrame = NULL;
	m_pGridViewFrame = NULL;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBCGPMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CBCGPToolBar::EnableQuickCustomization ();


	// Menu will not take the focus on activation:
	CBCGPPopupMenu::SetForceMenuFocus (FALSE);

	if (!m_wndMenuBar.Create (this))
	{
		TRACE0("Failed to create menubar\n");
		return -1;      // fail to create
	}

	m_wndMenuBar.SetBarStyle(m_wndMenuBar.GetBarStyle() | CBRS_SIZE_DYNAMIC);

	// Detect color depth. 256 color toolbars can be used in the
	// high or true color modes only (bits per pixel is > 8):
	CClientDC dc (this);
	BOOL bIsHighColor = dc.GetDeviceCaps (BITSPIXEL) > 8;

	UINT uiToolbarHotID = bIsHighColor ? IDB_TOOLBAR256 : 0;

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		/*| CBRS_GRIPPER | CBRS_FLYBY*/ | CBRS_TOOLTIPS | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME, 0, 0, FALSE, 0, 0, uiToolbarHotID))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
/*
	const int nComboboxWidth = globalUtils.ScaleByDPI(100);
	CBCGPToolbarComboBoxButton* pcomboButton = new CBCGPToolbarComboBoxButton(ID_RUN_ALGORITHM_COMBO,
#ifdef _BCGSUITE_INC_
		GetCmdMgr()->GetCmdImage(ID_RUN_ALGORITHM_COMBO, FALSE),
#else
		CImageHash::GetImageOfCommand(ID_RUN_ALGORITHM_COMBO, FALSE),
#endif
		CBS_DROPDOWNLIST, nComboboxWidth);
	pcomboButton->AddItem(_T("K-means"));
	pcomboButton->AddItem(_T("DBSCAN"));
	int iRet = m_wndToolBar.ReplaceButton(ID_RUN_ALGORITHM_COMBO, *pcomboButton);
	int nIndex = m_wndToolBar.CommandToIndex(ID_RUN_ALGORITHM_COMBO);
	m_wndToolBar.InvalidateButton(nIndex);
	TRACE(_T("=====%d"),iRet);*/

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// Load control bar icons:
	CBCGPToolBarImages imagesWorkspace;
	imagesWorkspace.SetImageSize (CSize (16, 16));
	imagesWorkspace.Load (IDB_WORKSPACE);
	imagesWorkspace.SmoothResize(globalData.GetRibbonImageScale());

	const int nPaneSize = globalUtils.ScaleByDPI(200);


	if (!m_wndDrawInfo.Create(_T("DrawInfo"), this, FALSE, MAKEINTRESOURCE(CDialogBarDrawInfo::IDD),
		/*CBRS_GRIPPER |*/ CBRS_SIZE_DYNAMIC | CBRS_ALIGN_LEFT | CBRS_TOOLTIPS | CBRS_FLYBY, AFX_IDW_DIALOGBAR + 10,
		CBRS_BCGP_REGULAR_TABS, CBRS_BCGP_FLOAT | /*CBRS_BCGP_CLOSE | */CBRS_BCGP_RESIZE | CBRS_BCGP_AUTOHIDE))

	{
		TRACE0("Failed to create color chooser bar\n");
		return -1;      // fail to create
	}
	//if (!m_wndWorkSpace.Create (_T("View 1"), this, CRect (0, 0, nPaneSize, nPaneSize),
	//	TRUE, ID_VIEW_WORKSPACE,
	//	WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT))
	//{
	//	TRACE0("Failed to create Workspace bar\n");
	//	return -1;      // fail to create
	//}

	//m_wndWorkSpace.SetIcon (imagesWorkspace.ExtractIcon (0), FALSE);

	//if (!m_wndWorkSpace2.Create (_T("View 2"), this, CRect (0, 0, nPaneSize, nPaneSize),
	//	TRUE, ID_VIEW_WORKSPACE2,
	//	WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	//{
	//	TRACE0("Failed to create Workspace bar 2\n");
	//	return -1;      // fail to create
	//}

	//m_wndWorkSpace2.SetIcon (imagesWorkspace.ExtractIcon (1), FALSE);


	//const int nOutputPaneSize = globalUtils.ScaleByDPI(150);

	//if (!m_wndOutput.Create (_T("Output"), this, CSize (nOutputPaneSize, nOutputPaneSize),
	//	TRUE /* Has gripper */, ID_VIEW_OUTPUT,
	//	WS_CHILD | WS_VISIBLE | CBRS_BOTTOM))
	//{
	//	TRACE0("Failed to create output bar\n");
	//	return -1;      // fail to create
	//}
	//m_wndOutput.SetIcon (imagesWorkspace.ExtractIcon (2), FALSE);

	//if (!m_wndPropGrid.Create (_T("Properties"), this, CRect (0, 0, nPaneSize, nPaneSize),
	//	TRUE, 
	//	ID_VIEW_PROPERTIES,
	//	WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
	//{
	//	TRACE0("Failed to create Properties Bar\n");
	//	return FALSE;		// fail to create
	//}

	//m_wndPropGrid.SetIcon (imagesWorkspace.ExtractIcon (3), FALSE);

	CString strMainToolbarTitle;
	strMainToolbarTitle.LoadString (IDS_MAIN_TOOLBAR);
	m_wndToolBar.SetWindowText (strMainToolbarTitle);
	//m_wndToolBar.EnableCustomizeButton(TRUE, (UINT)-1, _T("More Items"));

	m_lstHiddenCommands.AddHead(ID_EDIT_ARROW);
	m_lstHiddenCommands.AddHead(ID_EDIT_PEN);
	m_lstHiddenCommands.AddHead(ID_EDIT_DRAWFILE);
	m_lstHiddenCommands.AddHead(ID_EDIT_HAND);
	m_lstHiddenCommands.AddHead(ID_EDIT_DELETE);
	m_lstHiddenCommands.AddHead(ID_EDIT_COPY);
	m_lstHiddenCommands.AddHead(ID_EDIT_ZOOMIN);
	m_lstHiddenCommands.AddHead(ID_EDIT_ZOOMOUT);
	m_lstHiddenCommands.AddHead(ID_EDIT_FITSIZE);
	SetHiddenCommands(m_lstHiddenCommands);
	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	
	// Cjg：去掉下面这两行可以把菜单条和工具条的停靠功能去掉
	//m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	//m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndDrawInfo.EnableDocking(CBRS_ALIGN_ANY);
	//m_wndWorkSpace.EnableDocking(CBRS_ALIGN_RIGHT);
	//m_wndWorkSpace2.EnableDocking(CBRS_ALIGN_ANY);
	//m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
	//m_wndPropGrid.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	//EnableAutoHideBars(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndMenuBar);
	DockControlBar(&m_wndToolBar);
	DockControlBar(&m_wndDrawInfo,AFX_IDW_DOCKBAR_RIGHT);
	//DockControlBar(&m_wndWorkSpace);
	//m_wndWorkSpace2.AttachToTabWnd (&m_wndWorkSpace, BCGP_DM_STANDARD, FALSE, NULL);
	//DockControlBar(&m_wndOutput);
	//DockControlBar(&m_wndPropGrid);
	//ShowControlBar(&m_wndDrawInfo, TRUE, FALSE, TRUE);
	// Enable windows manager:
	//EnableWindowsDialog (ID_WINDOW_MANAGER, IDS_WINDOWS_MANAGER, TRUE);

	// Enable windows navigator (activated by Ctrl+Tab/Ctrl+Shift+Tab):
	//EnableWindowsNavigator();

	RecalcLayout();
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CBCGPMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	//cs.style &= ~FWS_ADDTOTITLE;
	m_strTitle = _T("聚类算法演示");
	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CBCGPMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CBCGPMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers




LRESULT CMainFrame::OnToolbarReset(WPARAM wp,LPARAM)
{
	// TODO: reset toolbar with id = (UINT) wp to its initial state:
	//
	 UINT uiToolBarId = (UINT) wp;
	 if (uiToolBarId == IDR_MAINFRAME)
	 {
		 const int nComboboxWidth = globalUtils.ScaleByDPI(100);
		 CBCGPToolbarComboBoxButton comboButton(ID_RUN_ALGORITHM_COMBO,
#ifdef _BCGSUITE_INC_
			 GetCmdMgr()->GetCmdImage(ID_RUN_ALGORITHM_COMBO, FALSE),
#else
			 CImageHash::GetImageOfCommand(ID_RUN_ALGORITHM_COMBO, FALSE),
#endif
			 CBS_DROPDOWNLIST, nComboboxWidth);
		 comboButton.AddItem(_T("K-means"));
		 comboButton.AddItem(_T("DBSCAN"));
		 comboButton.SelectItem(0);
		 int iRet = m_wndToolBar.ReplaceButton(ID_RUN_ALGORITHM_COMBO, comboButton);
		 //int nIndex = m_wndToolBar.CommandToIndex(ID_RUN_ALGORITHM_COMBO);
		 //m_wndToolBar.InvalidateButton(nIndex);
		 TRACE(_T("=====%d"), iRet);
	 }

//	const int nComboboxWidth = globalUtils.ScaleByDPI(100);
//	CBCGPToolbarComboBoxButton* pcomboButton = new CBCGPToolbarComboBoxButton(ID_RUN_ALGORITHM_COMBO,
//#ifdef _BCGSUITE_INC_
//		GetCmdMgr()->GetCmdImage(ID_RUN_ALGORITHM_COMBO, FALSE),
//#else
//		CImageHash::GetImageOfCommand(ID_RUN_ALGORITHM_COMBO, FALSE),
//#endif
//		CBS_DROPDOWNLIST, nComboboxWidth);
//	pcomboButton->AddItem(_T("K-means"));
//	pcomboButton->AddItem(_T("DBSCAN"));
//	int iRet = m_wndToolBar.ReplaceButton(ID_RUN_ALGORITHM_COMBO, *pcomboButton);
//	int nIndex = m_wndToolBar.CommandToIndex(ID_RUN_ALGORITHM_COMBO);
//	//m_wndToolBar.InvalidateButton(nIndex);
//	TRACE(_T("=====%d"), iRet);
//

	return 0;
}

void CMainFrame::OnWindowManager() 
{
	ShowWindowsDialog ();
}

CBCGPMDIChildWnd* CMainFrame::CreateDocumentWindow (LPCTSTR lpcszDocName, CObject* /*pObj*/)
{
	if (lpcszDocName != NULL && lpcszDocName [0] != '\0')
	{
		CDocument* pDoc = AfxGetApp()->OpenDocumentFile (lpcszDocName);

		if (pDoc != NULL)
		{
			POSITION pos = pDoc->GetFirstViewPosition();

			if (pos != NULL)
			{
				CView* pView = pDoc->GetNextView (pos);
				if (pView == NULL)
				{
					return NULL;
				}

				return DYNAMIC_DOWNCAST (CBCGPMDIChildWnd, pView->GetParent ());
			}   
		}
	}

	return NULL;
}

void CMainFrame::OnClose() 
{
	SaveMDIState (theApp.GetRegSectionPath ());
	CBCGPMDIFrameWnd::OnClose();
}


void CMainFrame::OnUpdateDrawOrList(CCmdUI * pCmdUI)
{
	CMDIChildWnd* pActiveFrame = MDIGetActive();
	if (!pActiveFrame)
	{
		pCmdUI->SetCheck(FALSE);
		pCmdUI->Enable(FALSE);
		return;
	}

	CView* pCurView = pActiveFrame->GetActiveView();
	BOOL bCheck;
	if (pCmdUI->m_nID == ID_EDIT_DRAW && pCurView->IsKindOf(RUNTIME_CLASS(CClusterMDIView))
		|| pCmdUI->m_nID == ID_EDIT_LIST && pCurView->IsKindOf(RUNTIME_CLASS(CGridView))
		)
	{
		bCheck = TRUE;
	}
	else
	{
		bCheck = FALSE;
	}
	pCmdUI->SetCheck(bCheck);

	return;
}

void CMainFrame::OnDrawOrList(UINT nID)
{
	CMDIChildWnd* pActiveFrame = MDIGetActive();

	if (!pActiveFrame)
		return;

	CView* pCurView = pActiveFrame->GetActiveView();
	CDocument* pDoc = pActiveFrame->GetActiveDocument();
	CFrameWnd* pCurFrame = pCurView->GetParentFrame();

	CRuntimeClass *pTargetViewClass = NULL;
	CMultiDocTemplate* pTargetDocTemplate = NULL;

	
	switch (nID)
	{
	case ID_EDIT_DRAW:
		//pTargetView = RUNTIME_CLASS(CclusterView);
		pTargetViewClass = RUNTIME_CLASS(CClusterMDIView);
		pTargetDocTemplate = theApp.m_pTemplateDrawView;
		break;

	case ID_EDIT_LIST:
		//pTargetView = RUNTIME_CLASS(CClusterFormView);
		pTargetViewClass = RUNTIME_CLASS(CGridView);
		pTargetDocTemplate = theApp.m_pTemplateGridView;
		break;
	}

	if (pCurView->IsKindOf(pTargetViewClass))
	{
		// ok!
		return;
	}

	CMDIChildWnd *pTargetFrame = NULL;
	CView* pTargetView = NULL;
	POSITION pos = pDoc->GetFirstViewPosition();
	while (pos != NULL)
	{
		pTargetView = pDoc->GetNextView(pos);
		if (pTargetView->IsKindOf(pTargetViewClass))
		{
			pTargetFrame = (CMDIChildWnd*)pTargetView->GetParentFrame();
			break;
		}
	}
	//pCurFrame->ShowWindow(SW_HIDE);
	if (!pTargetFrame)
	{
		pTargetFrame = (CMDIChildWnd*)pTargetDocTemplate->CreateNewFrame(pDoc,NULL);
		pTargetDocTemplate->InitialUpdateFrame(pTargetFrame, pDoc);
	}
	else
	{
		::SendMessage(m_hWndMDIClient, WM_SETREDRAW, 0, 0);
		pTargetFrame->MDIActivate();
		pTargetFrame->ShowWindow(SW_MAXIMIZE);
		::SendMessage(m_hWndMDIClient, WM_SETREDRAW, 1, 0);
		//pTargetFrame->UpdateWindow();
		::RedrawWindow(m_hWndMDIClient, NULL, NULL, RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);

		if (pTargetView->IsKindOf(RUNTIME_CLASS(CClusterMDIView)))
		{
			//绘图界面切换过来后需要初始化它的工具状态
			//并且由于在列表编辑状态可能添加了新的数据点有可能超出它当前的现实范围
			//在这里进行处理

			//CClusterMDIView* pClusterView = (CClusterMDIView*)pTargetView;
			CClusterMDIView* pClusterView = DYNAMIC_DOWNCAST(CClusterMDIView, pTargetView);
			ASSERT_VALID(pClusterView);
			pClusterView->ClearViewBeforSwitch();
		}
	}
	//if (pFrame)
	//{
	//	pFrame->MDIActivate();
	//	pFrame->UpdateWindow();
	//}
}


void CMainFrame::OnFileNew()
{
	// TODO: 在此添加命令处理程序代码

	CDocument* pDocument = theApp.m_pTemplateDrawView->CreateNewDocument();
	if (pDocument == NULL)
	{
		TRACE(traceAppMsg, 0, "CDocTemplate::CreateNewDocument returned NULL.\n");
		AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
		return;
	}

	BOOL bAutoDelete = pDocument->m_bAutoDelete;
	pDocument->m_bAutoDelete = FALSE;   // don't destroy if something goes wrong
	m_pDrawViewFrame = (CMDIChildWnd*)theApp.m_pTemplateDrawView->CreateNewFrame(pDocument, NULL);
	m_pDrawViewFrame->InitialUpdateFrame(pDocument, TRUE);
	m_pGridViewFrame = (CMDIChildWnd*)theApp.m_pTemplateGridView->CreateNewFrame(pDocument, NULL);
	m_pGridViewFrame->InitialUpdateFrame(pDocument, TRUE);
	pDocument->m_bAutoDelete = bAutoDelete;
	//if (m_pDrawViewFrame == NULL || m_pGridViewFrame == NULL)
	//{
	//	AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
	//	delete pDocument;       // explicit delete on error
	//	return;
	//}
	// create a new document - with default document name
	theApp.m_pTemplateDrawView->SetDefaultTitle(pDocument);

	if (!pDocument->OnNewDocument())
	{
		// user has be alerted to what failed in OnNewDocument
		TRACE(traceAppMsg, 0, "CDocument::OnNewDocument returned FALSE.\n");
		m_pDrawViewFrame->DestroyWindow();
		m_pGridViewFrame->DestroyWindow();
		return;
	}
	//m_pTemplateDrawView->InitialUpdateFrame(m_pDrawViewFrame, pDocument, TRUE);
	//if (lpszPathName == NULL)
	//{

	//	// it worked, now bump untitled count
	//	m_nUntitledCount++;
	//}
	//else
	//{
	//	// open an existing document
	//	CWaitCursor wait;
	//	if (!pDocument->OnOpenDocument(lpszPathName))
	//	{
	//		// user has be alerted to what failed in OnOpenDocument
	//		TRACE(traceAppMsg, 0, "CDocument::OnOpenDocument returned FALSE.\n");
	//		pFrame->DestroyWindow();
	//		return NULL;
	//	}
	//	pDocument->SetPathName(lpszPathName, bAddToMRU);
	//	pDocument->OnDocumentEvent(CDocument::onAfterOpenDocument);
	//}

	//InitialUpdateFrame(pFrame, pDocument, bMakeVisible);
	return ;
}


void CMainFrame::OnFileOpen()
{
	// TODO: 在此添加命令处理程序代码
}


LRESULT CMainFrame::OnMouseXY(WPARAM wp, LPARAM lp)
{
	if (lp)
	{
		double *p = (double*)lp;
		m_wndDrawInfo.m_dbMouseX = p[0];
		m_wndDrawInfo.m_dbMouseY = p[1];
		m_wndDrawInfo.UpdateData(FALSE);

		delete p;
	}

	if (wp)
	{
		m_wndDrawInfo.RetrieveData();
	}

	return 0;
}


void CMainFrame::OnAlgorithmCombo()
{
	CBCGPToolbarComboBoxButton* pCombobox = DYNAMIC_DOWNCAST(CBCGPToolbarComboBoxButton,
		m_wndToolBar.GetButton(m_wndToolBar.CommandToIndex(ID_RUN_ALGORITHM_COMBO)));

	ASSERT_VALID(pCombobox);

	int iSel = pCombobox->GetCurSel();

}


void CMainFrame::SetHiddenCommands(CList<UINT, UINT>& lstHiddenCommands)
{
	for (int i = 0; i < m_wndToolBar.GetCount(); i++)
	{
		CBCGPToolbarButton* pButton = m_wndToolBar.GetButton(i);
		ASSERT_VALID(pButton);

		const BOOL bIsVisible = lstHiddenCommands.Find(pButton->m_nID) == NULL;

		pButton->SetVisible(bIsVisible);

		if (!bIsVisible && i > 0)
		{
			CBCGPToolbarButton* pPrevButton = m_wndToolBar.GetButton(i - 1);
			ASSERT_VALID(pPrevButton);

			if (pPrevButton->m_nStyle & TBBS_SEPARATOR)
			{
				pPrevButton->SetVisible(FALSE);
			}
		}
	}

	m_wndToolBar.AdjustLayout();

	m_lstHiddenCommands.RemoveAll();
	m_lstHiddenCommands.AddTail((CList<UINT, UINT>*)&lstHiddenCommands);
}

void CMainFrame::OnRunBegin()
{
	// TODO: 在此添加命令处理程序代码


	CMDIChildWnd* pActiveFrame = MDIGetActive();
	ASSERT(pActiveFrame);
	CView* pCurView = pActiveFrame->GetActiveView();
	CClusterMDIDoc* pDoc = (CClusterMDIDoc*)(pActiveFrame->GetActiveDocument());
	ASSERT(pCurView);
	ASSERT(pDoc);

	CBCGPToolbarComboBoxButton* pCombobox = DYNAMIC_DOWNCAST(CBCGPToolbarComboBoxButton,
		m_wndToolBar.GetButton(m_wndToolBar.CommandToIndex(ID_RUN_ALGORITHM_COMBO)));

	ASSERT_VALID(pCombobox);

	int iSel = pCombobox->GetCurSel();
	
	CDialogRun dlg(pDoc->m_RawPoints, iSel);
	dlg.DoModal();
}


void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	// TODO: 在此添加专用代码和/或调用基类

	CBCGPMDIFrameWnd::OnUpdateFrameTitle(bAddToTitle);
}


void CMainFrame::OnUpdateRunBegin(CCmdUI *pCmdUI)
{
	// TODO: 在此添加命令更新用户界面处理程序代码
	CMDIChildWnd* pActiveFrame = MDIGetActive();
	pCmdUI->Enable(pActiveFrame!=NULL);
}
