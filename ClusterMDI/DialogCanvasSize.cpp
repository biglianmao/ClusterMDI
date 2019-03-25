// DialogCanvasSize.cpp : 实现文件
//

#include "stdafx.h"
#include "ClusterMDI.h"
#include "DialogCanvasSize.h"
#include "afxdialogex.h"


// CDialogCanvasSize 对话框

IMPLEMENT_DYNAMIC(CDialogCanvasSize, CDialog)

CDialogCanvasSize::CDialogCanvasSize(stDoubleRange& range, RAWPOINTS_T& points,CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DIALOG_CANVAS_SIZE, pParent)
	, m_XScreen(0)
	, m_YScreen(0)
	, m_dbXStart(0)
	, m_dbYStart(0)
	, m_dbXEnd(0)
	, m_dbYEnd(0)
	, m_range(range)
	, m_points(points)
{

}

CDialogCanvasSize::~CDialogCanvasSize()
{
}

void CDialogCanvasSize::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_XScreen);
	DDX_Text(pDX, IDC_EDIT6, m_YScreen);
	DDX_Text(pDX, IDC_EDIT2, m_dbXStart);
	DDX_Text(pDX, IDC_EDIT4, m_dbYStart);
	DDX_Text(pDX, IDC_EDIT3, m_dbXEnd);
	DDX_Text(pDX, IDC_EDIT5, m_dbYEnd);
}


BEGIN_MESSAGE_MAP(CDialogCanvasSize, CDialog)
	ON_BN_CLICKED(IDOK, &CDialogCanvasSize::OnBnClickedOk)
END_MESSAGE_MAP()


// CDialogCanvasSize 消息处理程序


void CDialogCanvasSize::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	stDoubleRange range;
	range.xStart = m_dbXStart;
	range.xEnd = m_dbXEnd;
	range.yStart = m_dbYStart;
	range.yEnd = m_dbYEnd;
	FloorRealRange(range);
	if (VerifyCanvasSize(range, m_points) > 0)
	{
		AfxMessageBox(_T("现有点超出取值范围"));
		return;
	}
	CDialog::OnOK();
}
