#pragma once


// CGridView ��ͼ

class CGridView : public CBCGPGridView
{
	DECLARE_DYNCREATE(CGridView)

protected:
	CGridView();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
	virtual ~CGridView();

	// Attributes
public:
	CClusterMDIDoc* GetDocument() const;
public:
	//virtual void OnDraw(CDC* pDC);      // ��д�Ի��Ƹ���ͼ
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	afx_msg void OnInsertRow();
	afx_msg void OnDeleteRow();

	bool RetrieveData();
	bool SaveData();
	afx_msg void OnInsertData();
	afx_msg void OnExport();
	afx_msg void OnImport();
	afx_msg void OnSort();
	virtual void OnActivateFrame(UINT nState, CFrameWnd* pDeactivateFrame);
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
//	afx_msg void OnClose();
};



#ifndef _DEBUG  // debug version 
inline CClusterMDIDoc* CGridView::GetDocument() const
{
	return reinterpret_cast<CClusterMDIDoc*>(m_pDocument);
}
#endif