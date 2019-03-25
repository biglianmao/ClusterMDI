// ClusterMDIDoc.cpp : implementation of the CClusterMDIDoc class
//

#include "stdafx.h"
#include "ClusterMDI.h"

#include "MainFrm.h"
#include "ClusterMDIDoc.h"
#include "ClusterMDIView.h"
#include "GridView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CClusterMDIDoc

IMPLEMENT_DYNCREATE(CClusterMDIDoc, CDocument)

BEGIN_MESSAGE_MAP(CClusterMDIDoc, CDocument)
END_MESSAGE_MAP()


// CClusterMDIDoc construction/destruction

CClusterMDIDoc::CClusterMDIDoc()
{
	// TODO: add one-time construction code here

}

CClusterMDIDoc::~CClusterMDIDoc()
{
}

//BOOL CClusterMDIDoc::OnNewDocument()
//{
//	if (!CDocument::OnNewDocument())
//		return FALSE;
//
//	// TODO: add reinitialization code here
//	// (SDI documents will reuse this document)
//
//	return TRUE;
//}




// CClusterMDIDoc serialization

void CClusterMDIDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
		ar << m_RawPoints.size();
		RAWPOINTS_T::const_iterator it;
		for (it = m_RawPoints.begin();it != m_RawPoints.end(); it++)
		{
			ar << (*it).first << (*it).second;
		}

	}
	else
	{
		// TODO: add loading code here
		m_RawPoints.clear();
		size_t sz;
		ar >> sz;

		for (int i = 0;i < sz;i++)
		{
			double x, y;
			ar >> x;
			ar >> y;

			m_RawPoints.push_back(make_pair(x, y));
		}

	}
}


// CClusterMDIDoc diagnostics

#ifdef _DEBUG
void CClusterMDIDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CClusterMDIDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CClusterMDIDoc commands


//void CClusterMDIDoc::OnCloseDocument()
//{
//	// TODO: 在此添加专用代码和/或调用基类
//
//	CDocument::OnCloseDocument();
//}


BOOL CClusterMDIDoc::SaveModified()
{
	// TODO: 在此添加专用代码和/或调用基类
	CMainFrame *pMainFrame = (CMainFrame *)theApp.m_pMainWnd;
	CMDIChildWnd* pActiveFrame = pMainFrame->MDIGetActive();	
	CView* pCurView = pActiveFrame->GetActiveView();
	if (pCurView->IsKindOf(RUNTIME_CLASS(CGridView)))
	{
		((CGridView*)pCurView)->SaveData();
	}

	return CDocument::SaveModified();
}


//BOOL CClusterMDIDoc::OnSaveDocument(LPCTSTR lpszPathName)
//{
//	// TODO: 在此添加专用代码和/或调用基类
//	//CMainFrame *pMainFrame = (CMainFrame *)theApp.m_pMainWnd;
//	//CMDIChildWnd* pActiveFrame = pMainFrame->MDIGetActive();
//	//CView* pCurView = pActiveFrame->GetActiveView();
//	//if (pCurView->IsKindOf(RUNTIME_CLASS(CGridView)))
//	//{
//	//	((CGridView*)pCurView)->SaveData();
//	//}
//
//	return CDocument::OnSaveDocument(lpszPathName);
//}


void CClusterMDIDoc::SetTitle(LPCTSTR lpszTitle)
{
	// TODO: 在此添加专用代码和/或调用基类

	CDocument::SetTitle(lpszTitle);
}
