#pragma once
#include "afxwin.h"

#include "PreviewRawPointsWnd.h"
// CInsertDataDlg �Ի���

class CInsertDataDlg : public CDialog
{
	DECLARE_DYNAMIC(CInsertDataDlg)

public:
	CInsertDataDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CInsertDataDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	bool m_bDrawFill;
	int m_iCount;
	int m_iXStart;
	int m_iXEnd;
	int m_iYStart;
	int m_iYEnd;
	CComboBox m_combXDistribution;
	CComboBox m_combYDistribution;
	CBCGPGridCtrl m_wndGrid;	
	RAWPOINTS_T m_RawPoints;

	CPreviewRawPointsWnd m_PreWnd;

	afx_msg void OnBnClickedButton1();
	virtual BOOL OnInitDialog();
};
