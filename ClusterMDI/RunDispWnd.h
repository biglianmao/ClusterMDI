#pragma once

// CRunDispWnd
#include "ClusterAlgorithm.h"

class CRunDispWnd : public CWnd
{
	DECLARE_DYNAMIC(CRunDispWnd)
private:
	//CSize m_szCanvas;
	//stDoubleRange m_rangeCanvas;
protected:
	const RAWPOINTS_T& m_RawPoints;
public:
	CClusterAlgorithmBase *m_pAlgoriObj;
	void SetAlgorithmObj(CClusterAlgorithmBase *pAlgoriObj);
	CRunDispWnd(const RAWPOINTS_T& rawPoints);
	virtual ~CRunDispWnd();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};


