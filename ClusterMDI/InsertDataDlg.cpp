// InsertDataDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ClusterMDI.h"

#include "InsertDataDlg.h"
#include "afxdialogex.h"

#include <stdlib.h>
#include <time.h>

// CInsertDataDlg 对话框

IMPLEMENT_DYNAMIC(CInsertDataDlg, CDialog)

CInsertDataDlg::CInsertDataDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG1, pParent)
	, m_iCount(100)
	, m_iXStart(0)
	, m_iXEnd(100)
	, m_iYStart(0)
	, m_iYEnd(100)
	, m_PreWnd(m_RawPoints)
{
	m_PreWnd.SetParameter(m_iXStart, m_iXEnd, m_iYStart, m_iYEnd);
	srand((unsigned)time(NULL));
	m_bDrawFill = false;
}

CInsertDataDlg::~CInsertDataDlg()
{
}

void CInsertDataDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT3, m_iCount);
	DDX_Text(pDX, IDC_EDIT1, m_iXStart);
	DDX_Text(pDX, IDC_EDIT2, m_iXEnd);
	DDX_Text(pDX, IDC_EDIT4, m_iYStart);
	DDX_Text(pDX, IDC_EDIT5, m_iYEnd);
	DDX_Control(pDX, IDC_COMBO1, m_combXDistribution);
	DDX_Control(pDX, IDC_COMBO2, m_combYDistribution);
}


BEGIN_MESSAGE_MAP(CInsertDataDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, &CInsertDataDlg::OnBnClickedButton1)
END_MESSAGE_MAP()

double RandSj(int start, int end)
{
	double ret;

	ret = rand() / (double)(RAND_MAX);
	ret = start + (end - start)*ret;

	return ret;
}

double RandZs(int start, int end)
{
	double d1, d2;

	while (1)
	{
		d1 = rand();
		d2 = rand() / (double)(RAND_MAX);

		double e = exp(-d1/10000);
		if (d2 < e)
			break;
	}

	double ret = start + (end - start)*d1 / RAND_MAX;

	return ret;
}
double RandZt(int start, int end)
{
	double d1,d2;

	while (1)
	{
		d1 = rand() / ((double)(RAND_MAX) / 6) -3.0;
		d2 = rand() / (double)(RAND_MAX);

		double e = exp(-d1*d1/2) / sqrt(2*3.1415);
		if (d2 < e)
			break;
	}

	double ret = start + (end - start)/6.0*(d1+3.0);

	return ret;
}
// CInsertDataDlg 消息处理程序


void CInsertDataDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码

	UpdateData(TRUE);


	CWaitCursor wait;

	typedef double(*DistributionFun_t)(int, int);

	DistributionFun_t fun[3] = { RandSj ,RandZt ,RandZs };

	m_RawPoints.clear();
	for (int i = 0;i < m_iCount;i++)
	{
		double X, Y;
		X = fun[m_combXDistribution.GetCurSel()](m_iXStart, m_iXEnd);
		Y = fun[m_combYDistribution.GetCurSel()](m_iYStart, m_iYEnd);
		m_RawPoints.push_back(make_pair(X, Y));
	}

	m_wndGrid.RemoveAll();

	RAWPOINTS_T::const_iterator it;
	for (it = m_RawPoints.begin();it != m_RawPoints.end(); it++)
	{
		CBCGPGridRow* pRow = m_wndGrid.CreateRow(m_wndGrid.GetColumnCount());

		pRow->GetItem(0)->SetValue((double)(*it).first);
		pRow->GetItem(1)->SetValue((double)(*it).second);

		m_wndGrid.AddRow(pRow,FALSE);
	}
	m_wndGrid.AdjustLayout();

	m_PreWnd.SetParameter(m_iXStart, m_iXEnd, m_iYStart, m_iYEnd);
	m_PreWnd.Invalidate();
}


BOOL CInsertDataDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	m_combXDistribution.InsertString(0, _T("随机分布"));
	m_combXDistribution.InsertString(1, _T("正态分布"));
	m_combXDistribution.InsertString(2, _T("指数分布"));
	m_combXDistribution.SetCurSel(0);
	m_combYDistribution.InsertString(0, _T("随机分布"));
	m_combYDistribution.InsertString(1, _T("正态分布"));
	m_combYDistribution.InsertString(2, _T("指数分布"));
	m_combYDistribution.SetCurSel(0);


	CRect rectGrid;
	CWnd *pWndGridLocation = GetDlgItem(IDC_STATIC_GRIDLOCATION);
	pWndGridLocation->GetClientRect(&rectGrid);
	pWndGridLocation->MapWindowPoints(this, &rectGrid);

	m_wndGrid.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, rectGrid, this, IDC_STATIC_GRIDLOCATION);


	m_wndGrid.EnableMarkSortedColumn(FALSE);
	m_wndGrid.EnableHeader(TRUE, BCGP_GRID_HEADER_MOVE_ITEMS);
	m_wndGrid.EnableRowHeader(TRUE);
	m_wndGrid.EnableLineNumbers();
	m_wndGrid.SetClearInplaceEditOnEnter(FALSE);
	m_wndGrid.EnableInvertSelOnCtrl();
	m_wndGrid.SetScalingRange(0.1, 4.0);
	m_wndGrid.InsertColumn(0, _T("X"), 100);
	m_wndGrid.InsertColumn(1, _T("Y"), 100);
	m_wndGrid.SetReadOnly();

	CRect rectPreview;
	CWnd *pWndPreview = GetDlgItem(IDC_STATIC_PREVIEW);
	pWndPreview->GetClientRect(&rectPreview);
	pWndPreview->MapWindowPoints(this, &rectPreview);
	m_PreWnd.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER, rectPreview, this, IDC_STATIC_PREVIEW);

	if (m_bDrawFill)
	{
		CWnd *pEdit;

		pEdit = GetDlgItem(IDC_EDIT1);
		pEdit->EnableWindow(FALSE);
		pEdit = GetDlgItem(IDC_EDIT2);
		pEdit->EnableWindow(FALSE);
		pEdit = GetDlgItem(IDC_EDIT4);
		pEdit->EnableWindow(FALSE);
		pEdit = GetDlgItem(IDC_EDIT5);
		pEdit->EnableWindow(FALSE);

		m_iCount = 20;
	}
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
