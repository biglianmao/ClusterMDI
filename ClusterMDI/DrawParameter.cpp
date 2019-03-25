
#include "stdafx.h"
#include "ClusterMDI.h"
#include "DrawParameter.h"

bool CaculateCanvasSize(stDoubleRange& rangeSize, const RAWPOINTS_T& rawPoints)
{
	RAWPOINTS_T::const_iterator it;

	double	dbXStart;
	double	dbXEnd;
	double	dbYStart;
	double	dbYEnd;

	if (!rawPoints.empty())
	{
		it = rawPoints.begin();
		dbXStart = (*it).first;
		dbXEnd = (*it).first;
		dbYStart = (*it).second;
		dbYEnd = (*it).second;

		for (it = rawPoints.begin();it != rawPoints.end();it++)
		{
			if ((*it).first < dbXStart)	dbXStart = (*it).first;
			if ((*it).first > dbXEnd)	dbXEnd = (*it).first;

			if ((*it).second < dbYStart)	dbYStart = (*it).second;
			if ((*it).second > dbYEnd)	dbYEnd = (*it).second;
		}
	}
	else
	{
		dbXStart = 0;
		dbXEnd = 100;
		dbYStart = 0;
		dbYEnd = 100;
	}

	rangeSize.xStart = dbXStart;
	rangeSize.xEnd = dbXEnd;
	rangeSize.yStart = dbYStart;
	rangeSize.yEnd = dbYEnd;

	return true;
}


int VerifyCanvasSize(const stDoubleRange& rangeSize, const RAWPOINTS_T& rawPoints)
{
	int count = 0;
	RAWPOINTS_T::const_iterator it;

	for (it = rawPoints.begin();it != rawPoints.end();it++)
	{
		if ((*it).first < rangeSize.xStart || (*it).first > rangeSize.xEnd)
		{
			count++;
			break;
		}

		if ((*it).second < rangeSize.yStart || (*it).second > rangeSize.yEnd)
		{
			count++;
		}
	}

	return count;
}

void FloorRealRange(stDoubleRange& rangeSize)
{
	//修改按数量级靠拢
	double xLength = rangeSize.xEnd - rangeSize.xStart;
	if (xLength < 10) xLength = 10;
	int xRegRange = round(log10(xLength));
	int xRegStep = xRegRange - 1;
	//int xRegStart = ((xLength / 2 + rangeSize.xStart) / pow(10, xRegStep) - 5);
	int xRegStart = floor(rangeSize.xStart / pow(10, xRegStep));
	int xRegEnd = ceil(rangeSize.xEnd / pow(10, xRegStep));

	if (xRegEnd - xRegStart < 5)
	{
		xRegStart -= 10 - (xRegEnd - xRegStart) / 2;
		xRegEnd += 10 - (xRegEnd - xRegStart) / 2;
	}

	rangeSize.xRegStep = xRegStep;
	rangeSize.xRegStart = xRegStart;
	rangeSize.xRegEnd = xRegEnd;
	rangeSize.xStart = xRegStart * pow(10, xRegStep);
	rangeSize.xEnd = xRegEnd * pow(10, xRegStep);

	double yLength = rangeSize.yEnd - rangeSize.yStart;
	if (yLength < 10) yLength = 10;
	int yRegRange = round(log10(yLength));
	int yRegStep = yRegRange - 1;
	//int yRegStart = ((yLength / 2 + rangeSize.yStart) / pow(10, yRegStep) - 5);
	int yRegStart = floor(rangeSize.yStart / pow(10, yRegStep));
	int yRegEnd = ceil(rangeSize.yEnd / pow(10, yRegStep));

	if (yRegEnd - yRegStart < 5)
	{
		yRegStart -= 10 - (yRegEnd - yRegStart) / 2;
		yRegEnd += 10 - (yRegEnd - yRegStart) / 2;
	}

	rangeSize.yRegStep = yRegStep;
	rangeSize.yRegStart = yRegStart;
	rangeSize.yRegEnd = yRegEnd;
	rangeSize.yStart = yRegStart * pow(10, yRegStep);
	rangeSize.yEnd = yRegEnd * pow(10, yRegStep);

	return;
}

void DrawAPoint(CDC *pDC, int X, int Y, int type,DWORD cr)
{

	int xy[][2] = {
		{ 0 , -2 },
		{ 0 , -1 },
		{ 0 , 0 },
		{ 0 , 1 },
		{ 0 , 2 },
		{ -2 , 0 },
		{ -1 , 0 },
		{ 0 , 0 },
		{ 1 , 0 },
		{ 2 , 0 },
		{ -2 ,-2 },
		{ -1 ,-1 },
		{ 0 , 0 },
		{ 1 , 1 },
		{ 2 , 2 },
		{ 2 ,-2 },
		{ 1 ,-1 },
		{ 0 , 0 },
		{ -1 , 1 },
		{ -2 , 2 }
	};

	int count = sizeof(xy) / (sizeof(int) * 2);
	for (int i = 0;i < count;i++)
	{
		//DWORD cr = type == 0 ? POINT_COLOR_DRAW : POINT_COLOR_SELECT;
		//DWORD cr = POINT_COLOR_DRAW;
		pDC->SetPixel(X + xy[i][0], Y + xy[i][1], cr);
	}

	if (type == 1)
	{
		HGDIOBJ pOldBrush = pDC->SelectObject(GetStockObject(NULL_BRUSH));	
		CPen pen;
		pen.CreatePen(PS_SOLID, 2, cr);
		CPen *pOldPen = pDC->SelectObject(&pen);
		CRect rc = CRect(X - 4, Y - 4, X + 5, Y + 5);
		pDC->Ellipse(&rc);
		pDC->SelectObject(pOldPen);
		pDC->SelectObject(pOldBrush);
	}
	if (type == 2)
	{
		HGDIOBJ pOldBrush = pDC->SelectObject(GetStockObject(NULL_BRUSH));
		CPen pen;
		pen.CreatePen(PS_SOLID, 2, cr);
		CPen *pOldPen = pDC->SelectObject(&pen);
		CRect rc = CRect(X - 4, Y - 4, X + 5, Y + 5);
		pDC->Ellipse(&rc);
		pDC->SelectObject(pOldPen);
		pDC->SelectObject(pOldBrush);
	}
}

void DrawAKSet(CDC * pDC, int X, int Y, int type, DWORD cr)
{
	int iLineWidth = type == 0 ? 1 : 2;
	CPen pen;
	pen.CreatePen(PS_SOLID, iLineWidth, cr);
	CPen *pOldPen = pDC->SelectObject(&pen);
	pDC->MoveTo(X, Y - 20);
	pDC->LineTo(X, Y + 20);
	pDC->MoveTo(X-20, Y);
	pDC->LineTo(X+20, Y);
	pDC->SelectObject(pOldPen);

	return;
}

void DrawMeasureLine(CDC * pDC, int ptX, int ptY, int setX, int setY, DWORD cr)
{
	int iLineWidth = 1;
	CPen pen;
	pen.CreatePen(PS_DASH, iLineWidth, cr);
	CPen *pOldPen = pDC->SelectObject(&pen);
	pDC->MoveTo(ptX, ptY);
	pDC->LineTo(setX, setY);
	pDC->SelectObject(pOldPen);
	return;
}

void DrawCentoidInfo(CDC * pDC, int ptX, int ptY, int i, double v, DWORD cr)
{
	TCHAR buffer[100];
	memset(buffer, 0, sizeof(TCHAR) * 100);
	_stprintf_s(buffer, _T("第%d轮质心偏移：%g"), i,v);
	pDC->TextOut(ptX, ptY, buffer);
	return;
}

void DrawCore(CDC * pDC, int ptX, int ptY, int W, int H, DWORD cr)
{
	HGDIOBJ pOldBrush = pDC->SelectObject(GetStockObject(NULL_BRUSH));
	CPen pen;
	pen.CreatePen(PS_SOLID, 2, cr);
	CPen *pOldPen = pDC->SelectObject(&pen);
	CRect rc = CRect(ptX - W, ptY - H, ptX + W + 1, ptY + H + 1);
	pDC->Ellipse(&rc);
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
	return;
}

void DrawCorePath(CDC * pDC, int sX, int sY, int dX, int dY, DWORD cr)
{
	int iLineWidth = 1;
	CPen pen;
	pen.CreatePen(PS_SOLID, iLineWidth, cr);
	CPen *pOldPen = pDC->SelectObject(&pen);

	CPoint p1(sX, sY);
	CPoint p2(dX, dY);
	double theta = 2*3.1415926 * 45/ 360;//偏转45度
	double Px, Py, P1x, P1y, P2x, P2y;
	//以P2为原点得到向量P2P1（P）
	Px = p1.x - p2.x;
	Py = p1.y - p2.y;
	//向量P旋转theta角得到向量P1
	P1x = Px*cos(theta) - Py*sin(theta);
	P1y = Px*sin(theta) + Py*cos(theta);
	//向量P旋转-theta角得到向量P2
	P2x = Px*cos(-theta) - Py*sin(-theta);
	P2y = Px*sin(-theta) + Py*cos(-theta);
	//伸缩向量至制定长度
	double x1, x2;
	int length = 5;
	x1 = sqrt(P1x*P1x + P1y*P1y);
	P1x = P1x*length / x1;
	P1y = P1y*length / x1;
	x2 = sqrt(P2x*P2x + P2y*P2y);
	P2x = P2x*length / x2;
	P2y = P2y*length / x2;
	//平移变量到直线的末端
	P1x = P1x + p2.x;
	P1y = P1y + p2.y;
	P2x = P2x + p2.x;
	P2y = P2y + p2.y;



	pDC->MoveTo(p1.x, p1.y);
	pDC->LineTo(p2.x, p2.y);
	pDC->MoveTo(p2.x, p2.y);
	pDC->LineTo(P1x, P1y);
	pDC->MoveTo(p2.x, p2.y);
	pDC->LineTo(P2x, P2y);

	pDC->MoveTo(P1x, P1y);
	pDC->LineTo(P2x, P2y);


	pDC->SelectObject(pOldPen);
	return;
}

void DrawProgress(CDC * pDC, LPCTSTR pStr, int iProgress, const CRect & rc, 
	DWORD bgCr, DWORD prCr, DWORD textCr)
{
	CRect prRc(rc);
	prRc.right = prRc.left + (int)(iProgress * prRc.Width() / 100);
	pDC->FillSolidRect(rc, bgCr);
	pDC->FillSolidRect(prRc, prCr);

	//CFont hFont;
	//hFont.CreateFont(RUN_PANNEL_TEXT_SIZE, 0, 000, 000, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, VARIABLE_PITCH | FF_ROMAN, _T("Times New Roman"));

	//CFont* pOldFont = pDC->SelectObject(&hFont);
	UINT oldTextAlign = pDC->SetTextAlign(TA_CENTER | TA_TOP);
	int oldBkMode = pDC->SetBkMode(TRANSPARENT);
	COLORREF oldTextColor = pDC->SetTextColor(textCr);
	pDC->TextOut(rc.left + rc.Width()/2,rc.top+3,pStr);
	pDC->SetTextColor(oldTextColor);
	pDC->SetTextAlign(oldTextAlign);
	pDC->SetBkMode(oldBkMode);
	//pDC->SelectObject(pOldFont);
}

bool SearchPoint(RAWPOINTS_T& rawPoints, 
	const stDoubleRange& rangeSize,
	const CSize& canvas, 
	const CPoint& pt,
	stSelectPoint& selectPoint)
{
	RAWPOINTS_T::iterator it;
	stSelectPoint searchResult;
	searchResult.X = -100;
	searchResult.Y = -100;

	for (it = rawPoints.begin();it != rawPoints.end();it++)
	{

		int X = (int)(canvas.cx*((*it).first - rangeSize.xStart) / (rangeSize.xEnd - rangeSize.xStart));
		int Y = (int)(canvas.cy*((*it).second - rangeSize.yStart) / (rangeSize.yEnd - rangeSize.yStart));
/*
		if (pt.x == X && pt.y == Y)
		{
			selectPoint.X = X;
			selectPoint.Y = Y;
			selectPoint.XValue = (*it).first;
			selectPoint.YValue = (*it).second;

			return true;
		}*/

		if (abs(pt.x - X) <= 2 && abs(pt.y - Y) <= 2)
		{
			//bool bReplace = false;
			int newDistance = (pt.x - X)*(pt.x - X) + (pt.y - Y)*(pt.y - Y);
			int oldDistance = (searchResult.X - X)*(searchResult.X - X) + (searchResult.Y - Y)*(searchResult.Y - Y);
			if (newDistance < oldDistance || 
				newDistance == oldDistance && X+Y < searchResult.X+ searchResult.Y
				)
			{
				//bReplace = true;
				searchResult.it = it;
				searchResult.X = searchResult.targetX = X;
				searchResult.Y = searchResult.targetY = Y;
				searchResult.XValue = (*it).first;
				searchResult.YValue = (*it).second;
			}
		}
	}

	if (searchResult.X != -100)
	{
		selectPoint = searchResult;
		return true;
	}
	else
		return false;
}

void SendInvalidateRect(CScrollView *pWnd, const stSelectPoint& selectPoint)
{
	CClientDC dc(pWnd);
	pWnd->OnPrepareDC(&dc);

	CPoint pt(selectPoint.X, selectPoint.Y);
	CRect rc(pt, pt);
	rc.InflateRect(5, 5);
	rc.OffsetRect(CANVAS_PIXEL_MARGIN, CANVAS_PIXEL_MARGIN);

	dc.LPtoDP(&rc);
	pWnd->InvalidateRect(&rc, FALSE);
}

void DrawNet(CDC *pDC, CPoint ptTopLeftInPage, CPoint ptBottomRightInPage)
{

	DWORD cr = NET_COLOR;
	HGDIOBJ pOldBrush = pDC->SelectObject(GetStockObject(NULL_BRUSH));

	ptTopLeftInPage.Offset(CANVAS_PIXEL_MARGIN, CANVAS_PIXEL_MARGIN);
	ptBottomRightInPage.Offset(CANVAS_PIXEL_MARGIN, CANVAS_PIXEL_MARGIN);

	CPen pen;
	pen.CreatePen(PS_DOT, 1, cr);
	CPen *pOldPen = pDC->SelectObject(&pen);
	pDC->Rectangle(CRect(ptTopLeftInPage, ptBottomRightInPage));

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
}


CRect CombinePointToRect(CPoint pt1, CPoint pt2)
{
	int l, t, r, b;
	if (pt1.x < pt2.x)
	{
		l = pt1.x;
		r = pt2.x;
	}
	else
	{
		r = pt1.x;
		l = pt2.x;
	}

	if (pt1.y < pt2.y)
	{
		t = pt1.y;
		b = pt2.y;
	}
	else
	{
		b = pt1.y;
		t = pt2.y;
	}
	CRect rc(l, t, r, b);
	//rc.InflateRect(10, 10);
	return rc;
}