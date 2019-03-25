// ClusterMDI.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ClusterMDI.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "ClusterMDIDoc.h"
#include "ClusterMDIView.h"
#include "GridView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int  iMaxUserToolbars		= 10;
const UINT uiFirstUserToolBarId	= AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId	= uiFirstUserToolBarId + iMaxUserToolbars - 1;

// CClusterMDIApp

BEGIN_MESSAGE_MAP(CClusterMDIApp, CBCGPWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	//ON_COMMAND(ID_FILE_NEW, CBCGPWinApp::OnFileNew)
	//ON_COMMAND(ID_FILE_OPEN, CBCGPWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CBCGPWinApp::OnFilePrintSetup)
	ON_COMMAND(ID_FILE_NEW, &CClusterMDIApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CClusterMDIApp::OnFileOpen)
END_MESSAGE_MAP()


// CClusterMDIApp construction

CClusterMDIApp::CClusterMDIApp()
{
	CBCGPVisualManagerVS2012::m_bAutoGrayscaleImages = FALSE;

	AddVisualTheme(BCGP_VISUAL_THEME_OFFICE_2000, ID_VIEW_APPLOOK_2000);
	AddVisualTheme(BCGP_VISUAL_THEME_OFFICE_XP, ID_VIEW_APPLOOK_XP);
	AddVisualTheme(BCGP_VISUAL_THEME_OFFICE_2003, ID_VIEW_APPLOOK_2003);
	AddVisualTheme(BCGP_VISUAL_THEME_VS_2005, ID_VIEW_APPLOOK_VS2005);
	AddVisualTheme(BCGP_VISUAL_THEME_VS_2008, ID_VIEW_APPLOOK_VS2008);
	AddVisualTheme(BCGP_VISUAL_THEME_VS_2010, ID_VIEW_APPLOOK_VS2010);
	AddVisualTheme(BCGP_VISUAL_THEME_VS_2013_LIGHT, ID_VIEW_APPLOOK_VS2013_LIGHT);
	AddVisualTheme(BCGP_VISUAL_THEME_VS_2013_DARK, ID_VIEW_APPLOOK_VS2013_DARK);
	AddVisualTheme(BCGP_VISUAL_THEME_VS_2013_BLUE, ID_VIEW_APPLOOK_VS2013_BLUE);
	AddVisualTheme(BCGP_VISUAL_THEME_DEFAULT, ID_VIEW_APPLOOK_WINDOWS_NATIVE);
	AddVisualTheme(BCGP_VISUAL_THEME_OFFICE_2007_BLUE, ID_VIEW_APPLOOK_2007);
	AddVisualTheme(BCGP_VISUAL_THEME_OFFICE_2007_BLACK, ID_VIEW_APPLOOK_2007_1);
	AddVisualTheme(BCGP_VISUAL_THEME_OFFICE_2007_SILVER, ID_VIEW_APPLOOK_2007_2);
	AddVisualTheme(BCGP_VISUAL_THEME_OFFICE_2007_AQUA, ID_VIEW_APPLOOK_2007_3);
	AddVisualTheme(BCGP_VISUAL_THEME_OFFICE_2010_BLUE, ID_VIEW_APPLOOK_2010_1);
	AddVisualTheme(BCGP_VISUAL_THEME_OFFICE_2010_BLACK, ID_VIEW_APPLOOK_2010_2);
	AddVisualTheme(BCGP_VISUAL_THEME_OFFICE_2010_SILVER, ID_VIEW_APPLOOK_2010_3);
	AddVisualTheme(BCGP_VISUAL_THEME_SCENIC, ID_VIEW_APPLOOK_SCENIC);
	AddVisualTheme(BCGP_VISUAL_THEME_OFFICE_2013_WHITE, ID_VIEW_APPLOOK_2013_1);
	AddVisualTheme(BCGP_VISUAL_THEME_OFFICE_2013_GRAY, ID_VIEW_APPLOOK_2013_2);
	AddVisualTheme(BCGP_VISUAL_THEME_OFFICE_2013_DARK_GRAY, ID_VIEW_APPLOOK_2013_3);
	AddVisualTheme(BCGP_VISUAL_THEME_OFFICE_2016_COLORFUL, ID_VIEW_APPLOOK_2016_1);
	AddVisualTheme(BCGP_VISUAL_THEME_OFFICE_2016_DARK_GRAY, ID_VIEW_APPLOOK_2016_2);
	AddVisualTheme(BCGP_VISUAL_THEME_OFFICE_2016_WHITE, ID_VIEW_APPLOOK_2016_3);
	AddVisualTheme(BCGP_VISUAL_THEME_OFFICE_2016_BLACK, ID_VIEW_APPLOOK_2016_4);

	SetVisualTheme(BCGP_VISUAL_THEME_VS_2013_BLUE);

	CBCGPToolbarOptions toolbarOptions;

	toolbarOptions.m_nViewToolbarsMenuEntryID = ID_VIEW_TOOLBARS;
	toolbarOptions.m_nCustomizeCommandID = ID_VIEW_CUSTOMIZE;
	toolbarOptions.m_strCustomizeCommandLabel = _T("Customize...");
	toolbarOptions.m_nUserToolbarCommandIDFirst = uiFirstUserToolBarId;
	toolbarOptions.m_nUserToolbarCommandIDLast = uiLastUserToolBarId;
	toolbarOptions.m_strToolbarCustomIconsPath = _T(".\\UserImages.png");

	//SetToolbarOptions(toolbarOptions);

	// Remove this line if you don't need themed product dialogs:
	globalData.m_bUseVisualManagerInBuiltInDialogs = TRUE;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CClusterMDIApp object

CClusterMDIApp theApp;


// CClusterMDIApp initialization

BOOL CClusterMDIApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CBCGPWinApp::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("BCGP AppWizard-Generated Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)

	SetRegistryBase(_T("Settings"));

	// Initialize all Managers for usage. They are automatically constructed
	// if not yet present
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	
	m_pTemplateDrawView = new CMultiDocTemplate(IDR_ClusterMDITYPE,
		RUNTIME_CLASS(CClusterMDIDoc),
		//RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CChildFrameWithRuler), // custom MDI child frame
		RUNTIME_CLASS(CClusterMDIView));
	if (!m_pTemplateDrawView)
		return FALSE;
	AddDocTemplate(m_pTemplateDrawView);
	m_pTemplateGridView = new CMultiDocTemplate(IDR_ClusterMDITYPE,
		RUNTIME_CLASS(CClusterMDIDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CGridView));
	if (!m_pTemplateGridView)
		return FALSE;
	//AddDocTemplate(m_pTemplateGridView);
	CBCGPPopupMenu::EnableMenuSound(FALSE);
	EnableTaskBarInteraction(FALSE);
	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;
	// call DragAcceptFiles only if there's a suffix
	//  In an MDI app, this should occur immediately after setting m_pMainWnd
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);


	if (cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
	{
		if (!pMainFrame->LoadMDIState(GetRegSectionPath()) || 
			DYNAMIC_DOWNCAST(CMDIChildWnd, pMainFrame->GetActiveFrame()) == NULL)
		{
			if (!ProcessShellCommand(cmdInfo))
				return FALSE;
		}
	}
	else
	{
		// Dispatch commands specified on the command line
		if (!ProcessShellCommand(cmdInfo))
			return FALSE;
	}
	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}

// CClusterMDIApp message handlers

int CClusterMDIApp::ExitInstance() 
{
	return CBCGPWinApp::ExitInstance();
}

// CAboutDlg dialog used for App About

class CAboutDlg : public CBCGPDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };
	CBCGPURLLinkButton m_btnURL;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CBCGPDialog(CAboutDlg::IDD)
{
	EnableVisualManagerStyle(TRUE, TRUE);
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CBCGPDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMPANY_URL, m_btnURL);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CBCGPDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CClusterMDIApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CClusterMDIApp message handlers


void CClusterMDIApp::PreLoadState()
{

	GetContextMenuManager()->AddMenu(_T("My menu"), IDR_CONTEXT_MENU);

	// TODO: add another context menus here
}

BOOL CClusterMDIApp::SaveAllModified()
{
	if (!CBCGPWinApp::SaveAllModified())
	{
		return FALSE;
	}
	
	CBCGPMDIFrameWnd* pMainFrame = DYNAMIC_DOWNCAST(CBCGPMDIFrameWnd, m_pMainWnd);
	if (pMainFrame != NULL)
	{
		pMainFrame->SaveMDIState(GetRegSectionPath());
	}
	
	return TRUE;
}



//CDocument* CClusterMDIApp::OpenDocumentFile(LPCTSTR lpszFileName)
//{
//	// TODO: 在此添加专用代码和/或调用基类
//	m_pTemplateDrawView->CloseAllDocuments(TRUE);
//
//	return CBCGPWinApp::OpenDocumentFile(lpszFileName);
//}


void CClusterMDIApp::OnFileNew()
{
	// TODO: 在此添加命令处理程序代码
	//ON_COMMAND(ID_FILE_NEW, CBCGPWinApp::OnFileNew)
	CloseAllDoc();
	return CBCGPWinApp::OnFileNew();
}


void CClusterMDIApp::OnFileOpen()
{
	// TODO: 在此添加命令处理程序代码
	//ON_COMMAND(ID_FILE_OPEN, CBCGPWinApp::OnFileOpen)
	CloseAllDoc();
	return CBCGPWinApp::OnFileOpen();
}


bool CClusterMDIApp::CloseAllDoc()
{
	POSITION posDoc = m_pTemplateDrawView->GetFirstDocPosition();
	while (posDoc)
	{
		//遍历文档模板对应的文档
		CDocument *pDoc = m_pTemplateDrawView->GetNextDoc(posDoc);
		if (pDoc)
		{
			if (!pDoc->SaveModified())
				return false;

			// shut it down
			pDoc->OnCloseDocument();
		}
	}
}
