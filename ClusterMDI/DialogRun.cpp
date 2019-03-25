// DialogRun.cpp : 实现文件
//

#include "stdafx.h"
#include "ClusterMDI.h"
#include "DialogRun.h"
#include "afxdialogex.h"

#include <boost/timer.hpp>

#define BTN_INIT	1
#define BTN_RUN		2
#define BTN_STEP	4
#define BTN_PAUSE	8
#define BTN_STOP	16
#define BTN_REFRESH	32
#define BTN_EXIT	64
// CDialogRun 对话框

IMPLEMENT_DYNAMIC(CDialogRun, CDialog)

CDialogRun::CDialogRun(const RAWPOINTS_T& rawPoints, int selAlgorithm,CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_RUN, pParent)
	, m_wndDisp(rawPoints)
	, m_rawPoints(rawPoints)
{
	m_selAlgorithm = selAlgorithm;
	m_pAlgorithmObj == NULL;
	m_iInterval = 30;
}

CDialogRun::~CDialogRun()
{
}

void CDialogRun::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_TAB1, m_tabCtrl);
	DDX_Control(pDX, IDC_BUTTON_RUN, m_btn[0]);
	DDX_Control(pDX, IDC_BUTTON_STEP, m_btn[1]);
	DDX_Control(pDX, IDC_BUTTON_PAUSE, m_btn[2]);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_btn[3]);
	DDX_Control(pDX, IDC_BUTTON_REFRESH, m_btn[4]);
	DDX_Control(pDX, IDC_BUTTON_EXIT, m_btn[5]);
	DDX_Control(pDX, IDC_BUTTON_SETTING, m_btn[6]);
}


BEGIN_MESSAGE_MAP(CDialogRun, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_EXIT, &CDialogRun::OnBnClickedButtonExit)
	ON_BN_CLICKED(IDC_BUTTON_RUN, &CDialogRun::OnBnClickedButtonRun)
	ON_BN_CLICKED(IDC_BUTTON_SETTING, &CDialogRun::OnBnClickedButtonSetting)
	ON_BN_CLICKED(IDC_BUTTON_STEP, &CDialogRun::OnBnClickedButtonStep)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CDialogRun::OnBnClickedButtonPause)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CDialogRun::OnBnClickedButtonStop)
	ON_MESSAGE(WM_ALG_RUNCOMPLETE, OnRunComplete)
	ON_REGISTERED_MESSAGE(BCGM_PROPERTY_CHANGED, &CDialogRun::OnPropertyChanged)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, &CDialogRun::OnBnClickedButtonRefresh)
END_MESSAGE_MAP()


// CDialogRun 消息处理程序


BOOL CDialogRun::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CRect rectDisp;
	CWnd *pwndDispDummy = GetDlgItem(IDC_STATIC_DISP_LOCATION);
	pwndDispDummy->GetClientRect(&rectDisp);
	pwndDispDummy->MapWindowPoints(this, &rectDisp);
	m_wndDisp.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, rectDisp, this, IDC_STATIC_DISP_LOCATION);

	if (m_selAlgorithm == 0) //kmeans
	{
		m_pAlgorithmObj = new CClusterKmeans(m_rawPoints, &m_wndDisp);
		m_wndDisp.m_pAlgoriObj = m_pAlgorithmObj;
	}
	else if(m_selAlgorithm == 1) //dbscan
	{
		m_pAlgorithmObj = new CClusterDbscan(m_rawPoints, &m_wndDisp);
		m_wndDisp.m_pAlgoriObj = m_pAlgorithmObj;
	}
	else
	{
		AfxMessageBox(_T("不支持的算法"));
		CDialog::OnOK();
		return FALSE;
	}


	HICON hIcon[14];
	hIcon[0] = theApp.LoadIcon(IDI_ICON_RUN);
	hIcon[1] = theApp.LoadIcon(IDI_ICON_STEP);
	hIcon[2] = theApp.LoadIcon(IDI_ICON_PAUSE);
	hIcon[3] = theApp.LoadIcon(IDI_ICON_STOP);
	hIcon[4] = theApp.LoadIcon(IDI_ICON_REFRESH);
	hIcon[5] = theApp.LoadIcon(IDI_ICON_EXIT);
	hIcon[6] = theApp.LoadIcon(IDI_ICON_SETTING);
	hIcon[7] = theApp.LoadIcon(IDI_ICON_RUN2);
	hIcon[8] = theApp.LoadIcon(IDI_ICON_STEP2);
	hIcon[9] = theApp.LoadIcon(IDI_ICON_PAUSE2);
	hIcon[10] = theApp.LoadIcon(IDI_ICON_STOP2);
	hIcon[11] = theApp.LoadIcon(IDI_ICON_REFRESH2);
	hIcon[12] = theApp.LoadIcon(IDI_ICON_EXIT2);
	hIcon[13] = theApp.LoadIcon(IDI_ICON_SETTING2);

	for (int i = 0;i < 7;i++)
	{
		m_btn[i].SetImage(hIcon[i+7],TRUE, hIcon[i + 7], hIcon[i]);
		m_btn[i].SetWindowText(_T(""));
		m_btn[i].SizeToContent();
	}
	
	CRect rectTab;
	CWnd *pwndTabDummy = GetDlgItem(IDC_STATIC_TAB_LOACTION);
	pwndTabDummy->GetClientRect(&rectTab);
	pwndTabDummy->MapWindowPoints(this, &rectTab);
	m_tabCtrl.Create(CBCGPTabWnd::STYLE_3D_VS2005, rectTab, this, ID_TAB_CONTROL);

	CRect rectPropList;
	CWnd *pwndPropDummy = GetDlgItem(IDC_STATIC_HH);
	pwndPropDummy->GetClientRect(&rectPropList);
	pwndPropDummy->MapWindowPoints(this, &rectPropList);

	m_wndPropList.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, rectPropList, this, (UINT)-1);

	m_wndPropList.EnableHeaderCtrl();
	m_wndPropList.EnableDesciptionArea();
	m_wndPropList.SetVSDotNetLook(TRUE);
	m_wndPropList.MarkModifiedProperties(TRUE);
	m_wndPropList.EnableToolBar();
	m_wndPropList.EnableSearchBox(TRUE, _T("Search"));


	m_pPropGroup = new CBCGPProp(_T("运行参数"));
	m_pPropInterval = new CBCGPProp(_T("运行间隔（ms）"), (_variant_t)30, _T("设定算法连续运行间隔，单位毫秒"));
	m_pPropInterval->EnableSpinControl(TRUE, 10, 1000);
	m_pPropGroup->AddSubItem(m_pPropInterval);
	m_wndPropList.AddProperty(m_pPropGroup);

	
	m_pAlgorithmObj->initProp();
	m_pAlgorithmObj->initGrid();


	EnableBtn(BTN_INIT | BTN_EXIT);

	CString str;
	m_pAlgorithmObj->getAlgoriStr(str);
	SetWindowText(str);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CDialogRun::OnBnClickedButtonExit()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialog::OnOK();
}


void CDialogRun::OnBnClickedButtonRun()
{
	// TODO: 在此添加控件通知处理程序代码
	//AfxMessageBox(_T("ok"));
	m_iInterval = m_pPropInterval->GetValue().intVal;
	m_pAlgorithmObj->run(m_iInterval);
	EnableBtn(BTN_PAUSE|BTN_STOP);
}


void CDialogRun::OnBnClickedButtonSetting()
{
	// TODO: 在此添加控件通知处理程序代码
	m_iInterval = m_pPropInterval->GetValue().intVal;

	if (m_selAlgorithm == 0) //kmeans
	{
		CClusterKmeans::KMEANS_PARAM kmeans;
		m_pAlgorithmObj->getPropValue(&kmeans);
		m_pAlgorithmObj->init(&kmeans);
	}
	else if (m_selAlgorithm == 1) //dbscan
	{
		CClusterDbscan::DBSCAN_PARAM dbscan;
		m_pAlgorithmObj->getPropValue(&dbscan);
		m_pAlgorithmObj->init(&dbscan);
	}


	EnableBtn(BTN_RUN | BTN_STEP | BTN_EXIT | BTN_REFRESH);
}


void CDialogRun::OnBnClickedButtonStep()
{
	// TODO: 在此添加控件通知处理程序代码

	m_pAlgorithmObj->step();
	if (m_pAlgorithmObj->end())
	{
		EnableBtn(BTN_REFRESH | BTN_EXIT);
	}
}


void CDialogRun::OnBnClickedButtonPause()
{
	// TODO: 在此添加控件通知处理程序代码
	m_pAlgorithmObj->stop();
	EnableBtn(BTN_RUN | BTN_STEP | BTN_EXIT | BTN_REFRESH);
	//if (m_pAlgorithmObj->end())
	//{
	//	EnableBtn(BTN_REFRESH | BTN_EXIT);
	//}
	//else
	//{
	//	EnableBtn(BTN_RUN | BTN_STEP | BTN_EXIT | BTN_REFRESH);
	//}
}


void CDialogRun::OnBnClickedButtonStop()
{
	// TODO: 在此添加控件通知处理程序代码
	m_pAlgorithmObj->stop();
	EnableBtn(BTN_REFRESH | BTN_EXIT);
}

LRESULT CDialogRun::OnRunComplete(WPARAM, LPARAM)
{
	AfxMessageBox(_T("算法运行结束"));
	EnableBtn(BTN_REFRESH | BTN_EXIT);
	m_pAlgorithmObj->postRunComplete();
	m_wndDisp.Invalidate();
	return TRUE;
}

LRESULT CDialogRun::OnPropertyChanged(WPARAM wp, LPARAM lp)
{
	m_pAlgorithmObj->propChanged(wp, (CBCGPProp*)lp);
	return TRUE;
}


void CDialogRun::RetrieveData()
{
	return;
}

void CDialogRun::EnableBtn(DWORD btn)
{
	CWnd *pWnd;
	pWnd = GetDlgItem(IDC_BUTTON_SETTING);
	pWnd->EnableWindow(btn & BTN_INIT);
	pWnd = GetDlgItem(IDC_BUTTON_RUN);
	pWnd->EnableWindow(btn & BTN_RUN);
	pWnd = GetDlgItem(IDC_BUTTON_STEP);
	pWnd->EnableWindow(btn & BTN_STEP);
	pWnd = GetDlgItem(IDC_BUTTON_PAUSE);
	pWnd->EnableWindow(btn & BTN_PAUSE);
	pWnd = GetDlgItem(IDC_BUTTON_STOP);
	pWnd->EnableWindow(btn & BTN_STOP);
	pWnd = GetDlgItem(IDC_BUTTON_REFRESH);
	pWnd->EnableWindow(btn & BTN_REFRESH);
	pWnd = GetDlgItem(IDC_BUTTON_EXIT);
	pWnd->EnableWindow(btn & BTN_EXIT);
}


void CDialogRun::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialog::OnOK();
}


void CDialogRun::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialog::OnCancel();
}


void CDialogRun::OnBnClickedButtonRefresh()
{
	// TODO: 在此添加控件通知处理程序代码
	m_pAlgorithmObj->refresh();
	EnableBtn(BTN_INIT | BTN_EXIT);
}
