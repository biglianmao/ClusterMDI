#pragma once

#define INITIAL_CANVAS_SCREEN_SIZE_X	1200
#define INITIAL_CANVAS_SCREEN_SIZE_Y	1200
#define CANVAS_PIXEL_MARGIN	20

#define LINE_MARGIN_1	2
#define LINE_MARGIN_2	6
#define LINE_MARGIN_3	12
#define TEXT_MARGIN		2

#define ZOOM_MIN	0.1
#define ZOOM_MAX	2.0

#define POINT_COLOR_DRAW	0xff0000
#define POINT_COLOR_SELECT	0x0000ff
#define NET_COLOR	0x0000ff

#define RUN_BACKGROUND	RGB(255, 255, 255)

#define RUN_PANNEL_CANVAS_PIXEL_MARGIN	20
#define RUN_PANNEL_PROGRESS_HEIGHT	20
#define RUN_PANNEL_TEXT_SIZE	10
#define RUN_PANNEL_TEXT_MARGIN	3
#define RUN_PANNEL_LINE_MARGIN_1	13
#define RUN_PANNEL_LINE_MARGIN_2	16

#define PROGRESS_BACKGROUND	RGB(200, 200, 200)
#define PROGRESS_BAR	RGB(150, 40, 40)
#define PROGRESS_TEXT	RGB(255, 0, 0)

struct stDoubleRange
{
	double xStart;
	double xEnd;
	int xRegStart;
	int xRegStep;
	int xRegEnd;
	double yStart;
	double yEnd;
	int yRegStart;
	int yRegStep;
	int yRegEnd;
};

struct stSelectPoint
{
	int X;
	int Y;
	double XValue;
	double YValue;
	int targetX;
	int targetY;
	RAWPOINTS_T::iterator it;
};

typedef vector<stSelectPoint> SELECT_POINT;

bool CaculateCanvasSize(stDoubleRange& rangeSize, const RAWPOINTS_T& rawPoints);
int VerifyCanvasSize(const stDoubleRange& rangeSize, const RAWPOINTS_T& rawPoints);

void FloorRealRange(stDoubleRange& rangeSize);
void DrawAPoint(CDC *pDC, int X, int Y, int type=0,DWORD cr = POINT_COLOR_DRAW);
void DrawAKSet(CDC *pDC, int X, int Y, int type = 0, DWORD cr = POINT_COLOR_DRAW);
void DrawMeasureLine(CDC *pDC, int ptX, int ptY, int setX, int setY, DWORD cr = POINT_COLOR_DRAW);
void DrawCentoidInfo(CDC *pDC, int ptX, int ptY, int i, double v, DWORD cr = POINT_COLOR_DRAW);
void DrawCore(CDC *pDC, int ptX, int ptY, int W, int H, DWORD cr = POINT_COLOR_DRAW);
void DrawCorePath(CDC *pDC, int sX, int sY, int dX, int dY, DWORD cr = POINT_COLOR_DRAW);
void DrawProgress(CDC *pDC, LPCTSTR pStr, int iProgress, const CRect& rc, 
	DWORD bgCr = PROGRESS_BACKGROUND, DWORD prCr = PROGRESS_BAR, DWORD textCr = PROGRESS_TEXT);

bool SearchPoint(RAWPOINTS_T& rawPoints,
	const stDoubleRange& rangeSize,
	const CSize& canvas,
	const CPoint& pt,
	stSelectPoint& selectPoint);


void SendInvalidateRect(CScrollView *pWnd, const stSelectPoint& selectPoint);
void DrawNet(CDC *pDC, CPoint ptTopLeftInPage, CPoint ptBottomRightInPage);

CRect CombinePointToRect(CPoint pt1, CPoint pt2);