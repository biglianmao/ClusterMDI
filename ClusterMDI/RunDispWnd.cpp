// RunDispWnd.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ClusterMDI.h"
#include "RunDispWnd.h"


// CRunDispWnd

IMPLEMENT_DYNAMIC(CRunDispWnd, CWnd)

void CRunDispWnd::SetAlgorithmObj(CClusterAlgorithmBase * pAlgoriObj)
{
	m_pAlgoriObj = pAlgoriObj;
	Invalidate();
}

CRunDispWnd::CRunDispWnd(const RAWPOINTS_T& rawPoints):
	m_RawPoints(rawPoints)
{
	m_pAlgoriObj = NULL;
	//CaculateCanvasSize(m_rangeCanvas, rawPoints);
	//FloorRealRange(m_rangeCanvas);
}

CRunDispWnd::~CRunDispWnd()
{
}


BEGIN_MESSAGE_MAP(CRunDispWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
END_MESSAGE_MAP()



// CRunDispWnd ��Ϣ�������




void CRunDispWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: �ڴ˴������Ϣ����������
					   // ��Ϊ��ͼ��Ϣ���� CWnd::OnPaint()
	CRect clientRect;
	GetClientRect(&clientRect);

	CDC memDC;//
	memDC.CreateCompatibleDC(&dc);

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, clientRect.Width(), clientRect.Height());//��������λͼ����ָ�����

	CBitmap *pOldBitmap = memDC.SelectObject(&bmp);//��λͼѡ���ڴ�������

	//4.�����㷨��ʾ
	if (m_pAlgoriObj)
		m_pAlgoriObj->doDraw(&memDC, clientRect);

	//5. restore DC settings
	dc.BitBlt(0, 0, clientRect.Width(), clientRect.Height(), &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(pOldBitmap);
	bmp.DeleteObject();
	memDC.DeleteDC();
}


int CRunDispWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  �ڴ������ר�õĴ�������
	return 0;
}
