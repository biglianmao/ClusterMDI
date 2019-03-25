#pragma once


// CDialogBarDrawInfo

class CDialogBarDrawInfo : public CBCGPDialogBar
{
	DECLARE_DYNAMIC(CDialogBarDrawInfo)
private:

	CBCGPGridCtrl m_wndGrid;
public:
	SELECT_POINT* m_selectedPoints;
	bool RetrieveData();
	double m_dbMouseX;
	double m_dbMouseY;
public:
	CDialogBarDrawInfo();
	virtual ~CDialogBarDrawInfo();

	enum { IDD = IDD_DIALOGBAR_DRAW_INFO};
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void AdjustControlsLayout();
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};


