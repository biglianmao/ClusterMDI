// GridView.cpp : 实现文件
//

#include "stdafx.h"
#include "ClusterMDI.h"
#include "ClusterMDIDoc.h"
#include "GridView.h"

#include "InsertDataDlg.h"

#include <algorithm>
#include <fstream>
// CGridView

IMPLEMENT_DYNCREATE(CGridView, CBCGPGridView)

CGridView::CGridView()
{

}

CGridView::~CGridView()
{
}

BEGIN_MESSAGE_MAP(CGridView, CBCGPGridView)
	ON_COMMAND(ID_EDIT_INSERTROW, &CGridView::OnInsertRow)
	ON_COMMAND(ID_EDIT_DELETEROW, &CGridView::OnDeleteRow)
	ON_COMMAND(ID_EDIT_FILL, &CGridView::OnInsertData)
	ON_COMMAND(ID_EDIT_EXPORT, &CGridView::OnExport)
	ON_COMMAND(ID_EDIT_IMPORT, &CGridView::OnImport)
	ON_COMMAND(ID_EDIT_SORT, &CGridView::OnSort)
//	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CGridView 绘图

//void CGridView::OnDraw(CDC* pDC)
//{
//	CDocument* pDoc = GetDocument();
//	// TODO:  在此添加绘制代码
//}


// CGridView 诊断

#ifdef _DEBUG
void CGridView::AssertValid() const
{
	CView::AssertValid();
}

#ifndef _WIN32_WCE
void CGridView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif
CClusterMDIDoc* CGridView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CClusterMDIDoc)));
	return (CClusterMDIDoc*)m_pDocument;
}
#endif //_DEBUG


// CGridView 消息处理程序


void CGridView::OnInitialUpdate()
{
	TRACE(_T("\n")__FUNCTION__ _T("\n"));

	CBCGPGridView::OnInitialUpdate();

	// TODO: 在此添加专用代码和/或调用基类
	CBCGPGridCtrl* pGridCtrl = GetGridCtrl();

	//pGridCtrl->RemoveAll();
	//pGridCtrl->DeleteAllColumns();

	pGridCtrl->EnableMarkSortedColumn(FALSE);
	pGridCtrl->EnableHeader(TRUE, BCGP_GRID_HEADER_MOVE_ITEMS);
	pGridCtrl->EnableRowHeader(TRUE);
	pGridCtrl->EnableLineNumbers();
	pGridCtrl->SetClearInplaceEditOnEnter(FALSE);
	pGridCtrl->EnableInvertSelOnCtrl();
	//pGridCtrl->SetWholeRowSel(TRUE);
	pGridCtrl->SetScalingRange(0.1, 4.0);
	
	pGridCtrl->InsertColumn(0, _T("X"), 100);
	pGridCtrl->InsertColumn(1, _T("Y"), 100);

	RetrieveData();
/*
	for (int nRow = 0; nRow < 100; nRow++)
	{
		CBCGPGridRow* pRow = pGridCtrl->CreateRow(pGridCtrl->GetColumnCount());

		for (int i = 0; i < pGridCtrl->GetColumnCount(); i++)
		{
			pRow->GetItem(i)->SetValue((long)((nRow + 1) * (i + 1)));
		}

		pGridCtrl->AddRow(pRow, FALSE);
	}*/

}


void CGridView::OnInsertRow()
{
	// TODO: 在此添加命令处理程序代码

	CBCGPGridCtrl* pGridCtrl = GetGridCtrl();
	CBCGPGridItemID SelID = pGridCtrl->GetCurSelItemID();

	if (SelID.IsRow())
	{
		CBCGPGridRow* pRow = pGridCtrl->CreateRow(pGridCtrl->GetColumnCount());

		for (int i = 0; i < pGridCtrl->GetColumnCount(); i++)
		{
			pRow->GetItem(i)->SetValue((double)(0));
		}

		pGridCtrl->InsertRowAfter(SelID.m_nRow, pRow, TRUE);
	}
	else
	{
		CBCGPGridRow* pRow = pGridCtrl->CreateRow(pGridCtrl->GetColumnCount());

		for (int i = 0; i < pGridCtrl->GetColumnCount(); i++)
		{
			pRow->GetItem(i)->SetValue((double)(0));
		}

		pGridCtrl->AddRow(pRow, TRUE);
	}


	CClusterMDIDoc *pDoc = GetDocument();
	pDoc->SetModifiedFlag(TRUE);
}


void CGridView::OnDeleteRow()
{
	// TODO: 在此添加命令处理程序代码
	CBCGPGridCtrl* pGridCtrl = GetGridCtrl();
	CBCGPGridItemID SelID = pGridCtrl->GetCurSelItemID();

	vector<int> selRows;

	int count = pGridCtrl->GetRowCount();
	for (int i = 0;i < count;i++)
	{
		CBCGPGridRow *pRow = pGridCtrl->GetRow(i);

		if (pRow->IsSelected())
			selRows.push_back(i);
	}

	if (selRows.size())
	{
		CString s = _T("确定要删除行:");
		for (vector<int>::iterator it = selRows.begin(); it != selRows.end();it++)
		{
			CString temp;
			temp.Format(_T("%d"), *it+1);
			s += temp;
			s += _T(",");
		}

		if (AfxMessageBox(s,MB_YESNO|MB_ICONEXCLAMATION) == IDYES)
		{
			for (vector<int>::reverse_iterator it = selRows.rbegin(); it != selRows.rend();it++)
			{
				pGridCtrl->RemoveRow(*it);
			}
		}

		CClusterMDIDoc *pDoc = GetDocument();
		pDoc->SetModifiedFlag(TRUE);
	}
	else
	{
		AfxMessageBox(_T("请先选中行"));
	}
}

bool CGridView::RetrieveData()
{
	CClusterMDIDoc *pDoc = GetDocument();
	CBCGPGridCtrl* pGridCtrl = GetGridCtrl();

	pGridCtrl->RemoveAll();


	RAWPOINTS_T::const_iterator it;
	for (it = pDoc->m_RawPoints.begin();it != pDoc->m_RawPoints.end(); it++)
	{
		CBCGPGridRow* pRow = pGridCtrl->CreateRow(pGridCtrl->GetColumnCount());

		pRow->GetItem(0)->SetValue((double)(*it).first);
		pRow->GetItem(1)->SetValue((double)(*it).second);

		pGridCtrl->AddRow(pRow);
	}

	return true;
}


bool CGridView::SaveData()
{
	CClusterMDIDoc *pDoc = GetDocument();
	CBCGPGridCtrl* pGridCtrl = GetGridCtrl();

	pDoc->m_RawPoints.clear();

	int count = pGridCtrl->GetRowCount();
	for (int i = 0;i < count;i++)
	{
		CBCGPGridRow *pRow = pGridCtrl->GetRow(i);

		double x = pRow->GetItem(0)->GetValue().dblVal;
		double y = pRow->GetItem(1)->GetValue().dblVal;

		pDoc->m_RawPoints.push_back(make_pair(x, y));
	}

	return true;
}


void CGridView::OnInsertData()
{
	// TODO: 在此添加命令处理程序代码
	CInsertDataDlg dlg;
	if (dlg.DoModal() == IDOK && !dlg.m_RawPoints.empty())
	{
		CBCGPGridCtrl* pGridCtrl = GetGridCtrl();
		CBCGPGridItemID SelID = pGridCtrl->GetCurSelItemID();

		for (RAWPOINTS_T::iterator it = dlg.m_RawPoints.begin(); it != dlg.m_RawPoints.end();it++)
		{
			if (SelID.IsRow())
			{
				CBCGPGridRow* pRow = pGridCtrl->CreateRow(pGridCtrl->GetColumnCount());

				pRow->GetItem(0)->SetValue((double)(*it).first);
				pRow->GetItem(1)->SetValue((double)(*it).second);

				pGridCtrl->InsertRowAfter(SelID.m_nRow, pRow, TRUE);
			}
			else
			{
				CBCGPGridRow* pRow = pGridCtrl->CreateRow(pGridCtrl->GetColumnCount());

				pRow->GetItem(0)->SetValue((double)(*it).first);
				pRow->GetItem(1)->SetValue((double)(*it).second);

				pGridCtrl->AddRow(pRow, TRUE);
			}
		}

		CClusterMDIDoc *pDoc = GetDocument();
		pDoc->SetModifiedFlag(TRUE);
	}
}


void CGridView::OnExport()
{
	// TODO: 在此添加命令处理程序代码
	TCHAR *pFilter = _T("csv文件|*.csv|所有文件|*.*||");
	CFileDialog dlg(FALSE, _T("csv"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, pFilter, this);

	if (dlg.DoModal() == IDOK)
	{
		CString szFileName = dlg.GetPathName();
		//MessageBox(szFileName);

		ofstream of(szFileName, ios::trunc);
		//of << "asdfasdfadsa";


		CString exportStr;
		CBCGPGridCtrl* pGridCtrl = GetGridCtrl();
		CBCGPGridRange range(0,0,1,pGridCtrl->GetRowCount()-1);
		pGridCtrl->ExportRangeToText(exportStr, range, 1);

		LPWSTR pwstr = exportStr.GetBuffer();
		size_t count = exportStr.GetLength();
		char *pstr = new char[count + 1];
		memset(pstr, 0, count + 1);
		wcstombs_s(&count,pstr, count+1, pwstr, count);
		exportStr.ReleaseBuffer();
		of << pstr;

		delete pstr;
	}
}


void CGridView::OnImport()
{
	// TODO: 在此添加命令处理程序代码

	CBCGPGridCtrl* pGridCtrl = GetGridCtrl();
	CBCGPGridItemID SelID = pGridCtrl->GetCurSelItemID();

	TCHAR *pFilter = _T("csv文件|*.csv|所有文件|*.*||");
	CFileDialog dlg(TRUE, _T("csv"), NULL, OFN_DONTADDTORECENT, pFilter, this);

	if (dlg.DoModal() == IDOK)
	{
		CString szFileName = dlg.GetPathName();
		//MessageBox(szFileName);

		ifstream f(szFileName, ios::in);
		//of << "asdfasdfadsa";
		int offset = 0;
		do {
			char buf[128];
			f >> buf;

			double X, Y;
			sscanf_s(buf, "%lf;%lf", &X, &Y);

			CBCGPGridRow* pRow = pGridCtrl->CreateRow(pGridCtrl->GetColumnCount());
			pRow->GetItem(0)->SetValue(X);
			pRow->GetItem(1)->SetValue(Y);
			if (SelID.IsRow())
			{
				pGridCtrl->InsertRowAfter(SelID.m_nRow + offset++, pRow, TRUE);
			}
			else
			{
				pGridCtrl->AddRow(pRow, TRUE);
			}
		} while (f.good());

	}
}

bool compare(const pair<double, double> &a, const pair<double, double> &b)
{
	if (a.first < b.first)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CGridView::OnSort()
{
	// TODO: 在此添加命令处理程序代码

	CBCGPGridCtrl* pGridCtrl = GetGridCtrl();
	pGridCtrl->Sort(0);
}


void CGridView::OnActivateFrame(UINT nState, CFrameWnd* pDeactivateFrame)
{
	// TODO: 在此添加专用代码和/或调用基类
	TRACE(_T("\n*************active\n"));
	CBCGPGridView::OnActivateFrame(nState, pDeactivateFrame);
}


void CGridView::OnUpdate(CView* pSender, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	// TODO: 在此添加专用代码和/或调用基类
	if(pSender)
		TRACE(_T("%s"), pSender->GetRuntimeClass()->m_lpszClassName);
	return;
}


//void CGridView::OnClose()
//{
//	// TODO: 在此添加消息处理程序代码和/或调用默认值
//
//	CBCGPGridView::OnClose();
//}
