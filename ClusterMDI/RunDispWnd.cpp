// RunDispWnd.cpp : 实现文件
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



// CRunDispWnd 消息处理程序




void CRunDispWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CWnd::OnPaint()
	CRect clientRect;
	GetClientRect(&clientRect);

	CDC memDC;//
	memDC.CreateCompatibleDC(&dc);

	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, clientRect.Width(), clientRect.Height());//创建兼容位图，并指定宽高

	CBitmap *pOldBitmap = memDC.SelectObject(&bmp);//将位图选入内存上下文

	//4.调用算法显示
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

	// TODO:  在此添加您专用的创建代码
	return 0;
}
