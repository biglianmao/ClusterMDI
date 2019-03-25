// DialogBarDrawInfo.cpp : 实现文件
//

#include "stdafx.h"
#include "ClusterMDI.h"
#include "DialogBarDrawInfo.h"
#include "DrawParameter.h"

// CDialogBarDrawInfo

IMPLEMENT_DYNAMIC(CDialogBarDrawInfo, CBCGPDialogBar)

CDialogBarDrawInfo::CDialogBarDrawInfo()
{
	//EnableLayout();
	m_selectedPoints = NULL;
}

CDialogBarDrawInfo::~CDialogBarDrawInfo()
{
}


BEGIN_MESSAGE_MAP(CDialogBarDrawInfo, CBCGPDialogBar)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()


void CDialogBarDrawInfo::DoDataExchange(CDataExchange* pDX)
{
	CBCGPDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogBarDrawInfo)
	DDX_Text(pDX, IDC_EDIT1, m_dbMouseX);
	DDX_Text(pDX, IDC_EDIT2, m_dbMouseY);
	//}}AFX_DATA_MAP
}

// CDialogBarDrawInfo 消息处理程序





int CDialogBarDrawInfo::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBCGPDialogBar::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码

	CRect rectGrid(0,0,100,100);

	m_wndGrid.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, rectGrid, this, IDC_GRID);

	m_wndGrid.EnableMarkSortedColumn(FALSE);
	m_wndGrid.EnableHeader(TRUE, BCGP_GRID_HEADER_MOVE_ITEMS);
	m_wndGrid.EnableRowHeader(TRUE);
	m_wndGrid.EnableLineNumbers();
	m_wndGrid.SetClearInplaceEditOnEnter(FALSE);
	m_wndGrid.EnableInvertSelOnCtrl();
	m_wndGrid.SetScalingRange(0.1, 4.0);
	m_wndGrid.InsertColumn(0, _T("X"), 60);
	m_wndGrid.InsertColumn(1, _T("Y"), 60);
	m_wndGrid.SetReadOnly();
	
	//CBCGPStaticLayout* pLayout = (CBCGPStaticLayout*)GetLayout();
	//if (pLayout != NULL)
	//{
	//	pLayout->AddAnchor(IDC_GRID, CBCGPStaticLayout::e_MoveTypeNone, CBCGPStaticLayout::e_SizeTypeBoth);
	//	
	//}
	return 0;
}


void CDialogBarDrawInfo::OnSize(UINT nType, int cx, int cy)
{
	CBCGPDialogBar::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	CRect rectGrid;
	CWnd *pWndGridLocation = GetDlgItem(IDC_STATIC_GRID);
	if (!pWndGridLocation) return;
	pWndGridLocation->GetClientRect(&rectGrid);
	pWndGridLocation->MapWindowPoints(this, &rectGrid);

	rectGrid.right = cx - 20;
	rectGrid.bottom = cy - 20;

	m_wndGrid.MoveWindow(rectGrid);
}


void CDialogBarDrawInfo::AdjustControlsLayout()
{
	CBCGPDialogBar::AdjustControlsLayout();

	CRect rectGrid;
	CRect rectClient;

	GetClientRect(rectClient);
	
	CWnd *pWndGridLocation = GetDlgItem(IDC_STATIC_GRID);
	if (!pWndGridLocation) return;
	pWndGridLocation->GetClientRect(&rectGrid);
	pWndGridLocation->MapWindowPoints(this, &rectGrid);

	rectGrid.bottom = rectClient.bottom - 20;

	//m_wndGrid.MoveWindow(rectGrid);
	//m_wndGrid.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}


bool CDialogBarDrawInfo::RetrieveData()
{
	CBCGPGridCtrl* pGridCtrl = &m_wndGrid;
	pGridCtrl->RemoveAll();

	TRACE(_T("******%d\n"), m_selectedPoints->size());

	SELECT_POINT::const_iterator it;
	for (it = m_selectedPoints->begin();it != m_selectedPoints->end(); it++)
	{
		CBCGPGridRow* pRow = pGridCtrl->CreateRow(pGridCtrl->GetColumnCount());

		pRow->GetItem(0)->SetValue((double)it->XValue);
		pRow->GetItem(1)->SetValue((double)it->YValue);

		pGridCtrl->AddRow(pRow);
	}

	return true;
}

