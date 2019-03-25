// PreviewRawPointsWnd.cpp : 实现文件
//

#include "stdafx.h"
#include "ClusterMDI.h"
#include "PreviewRawPointsWnd.h"


// CPreviewRawPointsWnd

IMPLEMENT_DYNAMIC(CPreviewRawPointsWnd, CWnd)

CPreviewRawPointsWnd::CPreviewRawPointsWnd(const RAWPOINTS_T& RawPoints)
	:m_RawPoints(RawPoints)
{
	if (m_iXEnd > m_iXStart)
	{
		int t = m_iXEnd;
		m_iXEnd = m_iXStart;
		m_iXStart = t;
	}
	if (m_iYEnd > m_iYStart)
	{
		int t = m_iYEnd;
		m_iYEnd = m_iYStart;
		m_iYStart = t;
	}
}

CPreviewRawPointsWnd::~CPreviewRawPointsWnd()
{
}


BEGIN_MESSAGE_MAP(CPreviewRawPointsWnd, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CPreviewRawPointsWnd 消息处理程序


#define CL_BACKGROUND	RGB(255, 255, 255)
#define CL_RULER	0xEBEBEB

#define MARGIN	5
#define RULER_WIDTH	10

void CPreviewRawPointsWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 在此处添加消息处理程序代码
					   // 不为绘图消息调用 CWnd::OnPaint()

	CRect rc;
	GetClientRect(&rc);

	CBrush brushBackground; // Must initialize!
	brushBackground.CreateSolidBrush(CL_BACKGROUND);
	dc.FillRect(&rc,&brushBackground);

	//计算XY的数量级
	int LevelX, LevelY;
	LevelX = (m_iXEnd - m_iXStart)/10;
	
	//画标尺
	//CRect rcRuler;
	//rcRuler.left = MARGIN;
	//rcRuler.top = MARGIN + RULER_WIDTH;
	//rcRuler.right = MARGIN + RULER_WIDTH;
	//rcRuler.bottom = rc.bottom - MARGIN;
	//CBrush brushRuler; // Must initialize!
	//brushRuler.CreateSolidBrush(CL_RULER);
	//dc.FillRect(rcRuler, &brushRuler);

	//rcRuler.left = MARGIN + RULER_WIDTH;
	//rcRuler.top = MARGIN;
	//rcRuler.right = rc.right - MARGIN;
	//rcRuler.bottom = MARGIN + RULER_WIDTH;
	//dc.FillRect(rcRuler, &brushRuler);

	CPen penRuler;
	penRuler.CreatePen(PS_SOLID, 2, (COLORREF)0x000000);
	CPen *pOldPen = dc.SelectObject(&penRuler);

	dc.MoveTo(MARGIN + RULER_WIDTH, 0);
	dc.LineTo(MARGIN + RULER_WIDTH, rc.bottom);
	dc.LineTo(MARGIN + RULER_WIDTH -5, rc.bottom-5);
	dc.MoveTo(MARGIN + RULER_WIDTH, rc.bottom);
	dc.LineTo(MARGIN + RULER_WIDTH +5, rc.bottom-5);

	dc.MoveTo(0,MARGIN + RULER_WIDTH);
	dc.LineTo(rc.right, MARGIN + RULER_WIDTH);
	dc.LineTo(rc.right-5,MARGIN + RULER_WIDTH -5);
	dc.MoveTo(rc.right, MARGIN + RULER_WIDTH);
	dc.LineTo(rc.right - 5, MARGIN + RULER_WIDTH + 5);


	const int drawXStart = MARGIN + RULER_WIDTH;
	const int drawXEnd = rc.right - MARGIN;
	const int drawYStart = MARGIN + RULER_WIDTH;
	const int drawYEnd = rc.bottom - MARGIN;
	RAWPOINTS_T::const_iterator it;
	for (it = m_RawPoints.begin();it != m_RawPoints.end(); it++)
	{
		int X = drawXStart + (int)((drawXEnd - drawXStart)*((*it).first - m_iXStart) / (m_iXEnd - m_iXStart));
		int Y = drawYStart + (int)((drawYEnd - drawYStart)*((*it).second - m_iYStart) / (m_iYEnd - m_iYStart));

		dc.SetPixel(X, Y, 0xff0000);
		dc.SetPixel(X-1, Y, 0xff0000);
		dc.SetPixel(X+1, Y, 0xff0000);
		dc.SetPixel(X, Y-1, 0xff0000);
		dc.SetPixel(X, Y+1, 0xff0000);
	}
}


void CPreviewRawPointsWnd::SetParameter(int XStart, int XEnd, int YStart, int YEnd)
{
	m_iXEnd = XEnd;
	m_iXStart = XStart;
	m_iYStart = YStart;
	m_iYEnd = YEnd;
}
