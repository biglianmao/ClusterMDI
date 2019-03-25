#pragma once


// CPreviewRawPointsWnd

class CPreviewRawPointsWnd : public CWnd
{
	DECLARE_DYNAMIC(CPreviewRawPointsWnd)

public:
	CPreviewRawPointsWnd(const RAWPOINTS_T& RawPoints);
	virtual ~CPreviewRawPointsWnd();

protected:

	int m_iXStart;
	int m_iXEnd;
	int m_iYStart;
	int m_iYEnd;
	const RAWPOINTS_T& m_RawPoints;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	void SetParameter(int XStart, int XEnd, int YStart, int YEnd);
};


