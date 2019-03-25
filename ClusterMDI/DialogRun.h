#pragma once

#include "RunDispWnd.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "ClusterAlgorithm.h"
// CDialogRun 对话框

class CDialogRun : public CDialog
{
	DECLARE_DYNAMIC(CDialogRun)
private:
	int m_selAlgorithm;
	const RAWPOINTS_T& m_rawPoints;
	CClusterAlgorithmBase* m_pAlgorithmObj;
	CRunDispWnd m_wndDisp;
	//vector<CBCGPGridCtrl*> m_Grids;
	CBCGPProp* m_pPropInterval;
	int m_iInterval;
public:
	CDialogRun(const RAWPOINTS_T& rawPoints,int selAlgorithm, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDialogRun();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_RUN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CBCGPTabWnd m_tabCtrl;
	CBCGPPropList m_wndPropList;
	CBCGPProp* m_pPropGroup;
	CBCGPButton m_btn[7];
	afx_msg void OnBnClickedButtonExit();
	afx_msg void OnBnClickedButtonRun();
	afx_msg void OnBnClickedButtonSetting();
	afx_msg void OnBnClickedButtonStep();
	afx_msg void OnBnClickedButtonPause();
	afx_msg void OnBnClickedButtonStop();
	afx_msg LRESULT OnRunComplete(WPARAM, LPARAM);
	afx_msg LRESULT OnPropertyChanged(WPARAM, LPARAM lp);

	void RetrieveData();
	void EnableBtn(DWORD btn);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBnClickedButtonRefresh();
};
