#pragma once

#include "DrawParameter.h"

// CDialogCanvasSize 对话框

class CDialogCanvasSize : public CDialog
{
	DECLARE_DYNAMIC(CDialogCanvasSize)

public:
	CDialogCanvasSize(stDoubleRange& range, RAWPOINTS_T& points,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDialogCanvasSize();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CANVAS_SIZE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	DWORD m_XScreen;
	DWORD m_YScreen;
	double m_dbXStart;
	double m_dbYStart;
	double m_dbXEnd;
	double m_dbYEnd;

	const stDoubleRange& m_range;
	RAWPOINTS_T& m_points;
	afx_msg void OnBnClickedOk();
};
