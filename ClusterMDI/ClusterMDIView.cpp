// ClusterMDIView.cpp : implementation of the CClusterMDIView class
//

#include "stdafx.h"
#include "ClusterMDI.h"

#include "ClusterMDIDoc.h"
#include "ClusterMDIView.h"

#include "MainFrm.h"
#include "ChildFrm.h"

#include "DialogCanvasSize.h"

#include "InsertDataDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CClusterMDIView

IMPLEMENT_DYNCREATE(CClusterMDIView, CBCGPScrollView)

BEGIN_MESSAGE_MAP(CClusterMDIView, CBCGPScrollView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CBCGPScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CBCGPScrollView::OnFilePrint)
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
	ON_REGISTERED_MESSAGE(BCGM_CHANGEVISUALMANAGER, OnChangeVisualManager)
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_SETCURSOR()
	ON_UPDATE_COMMAND_UI_RANGE(ID_EDIT_ARROW, ID_EDIT_HAND, OnUpdateTool)
	ON_COMMAND_RANGE(ID_EDIT_ARROW, ID_EDIT_HAND, OnTool)
	ON_COMMAND(ID_EDIT_FITSIZE, &CClusterMDIView::OnEditFitsize)
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_COMMAND(ID_EDIT_ZOOMIN, &CClusterMDIView::OnEditZoomin)
	ON_COMMAND(ID_EDIT_ZOOMOUT, &CClusterMDIView::OnEditZoomout)
	ON_WM_LBUTTONUP()
	ON_COMMAND(ID_EDIT_DELETE, &CClusterMDIView::OnEditDelete)
	ON_COMMAND(ID_EDIT_COPY, &CClusterMDIView::OnEditCopy)
//	ON_WM_MOUSEHWHEEL()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

// CClusterMDIView construction/destruction

CClusterMDIView::CClusterMDIView()
{
	// TODO: add construction code here
	m_flZoomRate = 1.0;

	m_ToolType = enum_ToolType::TOOL_ARROW;

	HCURSOR hc;
	hc = LoadCursor(NULL, IDC_ARROW);
	m_HCs[0] = hc;

	hc = theApp.LoadCursor(IDC_CURSOR_CROSS);
	m_HCs[1] = hc;

	hc = theApp.LoadCursor(IDC_CURSOR_MAGIC);
	m_HCs[2] = hc;

	hc = theApp.LoadCursor(IDC_CURSOR_HAND);
	m_HCs[3] = hc;

	//hc = theApp.LoadCursor(IDC_CURSOR_SELECT);
	//m_HCs[3] = hc;

	m_bCaptured = FALSE;

	m_bArrowHasMove = m_bArrowHit = m_bArrowDrawNet = FALSE;
}

CClusterMDIView::~CClusterMDIView()
{
}

BOOL CClusterMDIView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CBCGPScrollView::PreCreateWindow(cs);
}

// CClusterMDIView drawing

void CClusterMDIView::OnDraw(CDC* pDC)
{
	CClusterMDIDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
/*
	CRect invalidateRect;
	int iRet = pDC->GetClipBox(&invalidateRect);
	HRGN hRgn = CreateRectRgnIndirect(&invalidateRect);*/
	
	//NULLREGION;
	/*
#define ERROR               0
#define NULLREGION          1
#define SIMPLEREGION        2
#define COMPLEXREGION       3
#define RGN_ERROR ERROR
*/
	//TRACE(_T("******%d(%d,%d,%d,%d)\n"), iRet, invalidateRect.left, invalidateRect.top, invalidateRect.Width(), invalidateRect.Height());
	CRect drawRect;
	CPoint memViewOrg;
	CPoint postDrawDCOrg;

	if (pDC->GetClipBox(&drawRect))
	{
		memViewOrg = drawRect.TopLeft();
		postDrawDCOrg = memViewOrg;
		pDC->LPtoDP(&postDrawDCOrg);
	}
	else
	{
		GetClientRect(&drawRect);
		memViewOrg = GetScrollPosition();
		postDrawDCOrg.x = postDrawDCOrg.y = 0;
	}

	//TRACE(_T("*******%d-%d-%d-%d******\n"), 
	//	drawRect.left, drawRect.top, drawRect.right, drawRect.bottom);
	
	CDC memDC;//
	memDC.CreateCompatibleDC(pDC);
	memDC.SetViewportOrg(-memViewOrg);
	
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC, drawRect.Width(), drawRect.Height());//��������λͼ����ָ�����

	CBitmap *pOldBitmap = memDC.SelectObject(&bmp);//��λͼѡ���ڴ�������
	// TODO: add draw code for native data here

	
	
	//TRACE(_T("*******%d-%d-%d-%d******\n"), clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
	memDC.FillSolidRect(drawRect, ::GetSysColor(COLOR_INACTIVECAPTION));

	CRect pageRect(CPoint(CANVAS_PIXEL_MARGIN, CANVAS_PIXEL_MARGIN),m_szCanvas);
	memDC.FillSolidRect(pageRect, RGB(255, 255, 255));

	//����Ӱ
	//DWORD cr = ::GetSysColor(COLOR_INACTIVECAPTION);
	//HGDIOBJ pOldBrush = pDC->SelectObject(GetStockObject(NULL_BRUSH));

	//pageRect.InflateRect(10, 10);
	//for (int i = 0;i < 5;i++)
	//{
	//	CPen pen;
	//	pen.CreatePen(PS_SOLID, 2, cr);
	//	CPen *pOldPen = pDC->SelectObject(&pen);
	//	pDC->Rectangle(pageRect);
	//	pDC->SelectObject(pOldPen);

	//	pageRect.DeflateRect(2, 2);
	//	cr = RGB(GetRValue(cr)*3/4, GetGValue(cr) * 3 / 4, GetBValue(cr) * 3 / 4);
	//}
	//pDC->SelectObject(pOldBrush);

	//2.�������ĵ��
	const int drawXStart = CANVAS_PIXEL_MARGIN;
	const int drawYStart = CANVAS_PIXEL_MARGIN;
	RAWPOINTS_T::const_iterator it;
	for (it = pDoc->m_RawPoints.begin();it != pDoc->m_RawPoints.end(); it++)
	{
		//1.�쿴�Ƿ���ѡ�м�����

		bool bFound = false;
		for (SELECT_POINT::const_iterator sit = m_selectedPoints.begin();
			sit != m_selectedPoints.end();sit++)
		{
			if (sit->it == it)
			{
				bFound = true;
				break;
			}
		}

		//2.�����ѡ�м����в���Ҫ��
		if (bFound)
		{
			continue;
		}

		int X = drawXStart + (int)(m_szCanvas.cx*((*it).first - m_rangeCanvas.xStart) / (m_rangeCanvas.xEnd - m_rangeCanvas.xStart));
		int Y = drawYStart + (int)(m_szCanvas.cy*((*it).second - m_rangeCanvas.yStart) / (m_rangeCanvas.yEnd - m_rangeCanvas.yStart));

		DrawAPoint(&memDC, X, Y);
	}

	//3����ѡ�еĵ��
	SELECT_POINT::const_iterator sit;
	for (sit = m_selectedPoints.begin();sit != m_selectedPoints.end();sit++)
	{
		int X = sit->X + drawXStart;
		int Y = sit->Y + drawYStart;
		DrawAPoint(&memDC, X, Y,1, POINT_COLOR_SELECT);
	}

	//4�������Ҫ������
	if (m_bArrowDrawNet)
	{
		DrawNet(&memDC, m_ptOrgPointInPage, m_ptForwardPointInPage);
	}


	pDC->SetViewportOrg(postDrawDCOrg);
	pDC->BitBlt(0, 0, drawRect.Width(), drawRect.Height(), &memDC, drawRect.left, drawRect.top, SRCCOPY);

	memDC.SelectObject(pOldBitmap);
	bmp.DeleteObject();
	memDC.DeleteDC();
	return;

	CBrush brush; // Must initialize!
	brush.CreateSolidBrush(RGB(255, 0, 0));
	CBrush* pTempBrush = NULL;
	pTempBrush = (CBrush*)pDC->SelectObject(brush);

	CPoint p1, p2, p3;
	p1.x = 50;
	p1.y = 0;
	p2.x = 0;
	p2.y = 100;
	p3.x = 100;
	p3.y = 100;

	pDC->BeginPath();
	pDC->MoveTo(p1);
	pDC->LineTo(p2);
	pDC->LineTo(p3);
	pDC->LineTo(p1);
	pDC->EndPath();
	pDC->FillPath();

	CRect rc(100, 100, 300, 300);
	pDC->FillRect(rc, &brush);

	pDC->SelectObject(pTempBrush);
}

void CClusterMDIView::OnInitialUpdate()
{
	CBCGPScrollView::OnInitialUpdate();
	CClusterMDIDoc* pDoc = GetDocument();
	
	CaculateCanvasSize(m_rangeCanvas, pDoc->m_RawPoints);
	FloorRealRange(m_rangeCanvas);

	m_szCanvas.cx = INITIAL_CANVAS_SCREEN_SIZE_X;
	m_szCanvas.cy = INITIAL_CANVAS_SCREEN_SIZE_Y;
	CSize sizeTotal = m_szCanvas + CSize(CANVAS_PIXEL_MARGIN*2, CANVAS_PIXEL_MARGIN*2);
	// TODO: calculate the total size of this view
	//sizeTotal.cx = sizeTotal.cy = 1000;
	SetScrollSizes(MM_TEXT, sizeTotal);

	UpdateRulersInfo(RW_POSITION, GetScrollPosition());

	CMainFrame* pMainFrame = (CMainFrame*)theApp.m_pMainWnd;
	pMainFrame->m_wndDrawInfo.m_selectedPoints = &m_selectedPoints;
}


// CClusterMDIView printing

void CClusterMDIView::OnFilePrintPreview()
{
	BCGPPrintPreview (this);
}

BOOL CClusterMDIView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CClusterMDIView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CClusterMDIView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CClusterMDIView diagnostics

#ifdef _DEBUG
void CClusterMDIView::AssertValid() const
{
	CBCGPScrollView::AssertValid();
}

void CClusterMDIView::Dump(CDumpContext& dc) const
{
	CBCGPScrollView::Dump(dc);
}

CClusterMDIDoc* CClusterMDIView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CClusterMDIDoc)));
	return (CClusterMDIDoc*)m_pDocument;
}
#endif //_DEBUG


// CClusterMDIView message handlers

LRESULT CClusterMDIView::OnPrintClient(WPARAM wp, LPARAM lp)
{
	if ((lp & PRF_CLIENT) == PRF_CLIENT)
	{
		CDC* pDC = CDC::FromHandle((HDC) wp);
		ASSERT_VALID(pDC);
		
		OnDraw(pDC);
	}
	
	return 0;
}

LRESULT CClusterMDIView::OnChangeVisualManager(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return 0;
}

void CClusterMDIView::UpdateRulersInfo(int nMessage, CPoint ScrollPos, CPoint Pos)
{

	CChildFrameWithRuler* pFrame = (CChildFrameWithRuler*)GetParentFrame();

	stRULER_INFO pRulerInfo;
	pRulerInfo.uMessage = nMessage;
	pRulerInfo.ScrollPos = ScrollPos;
	pRulerInfo.Pos = Pos;
	pRulerInfo.DocSize = m_szCanvas;
	pRulerInfo.RealRange = m_rangeCanvas;
	pRulerInfo.fZoomFactor = m_flZoomRate;

	pFrame->UpdateRulersInfo(pRulerInfo);
}


void CClusterMDIView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	//Invalidate();

	CScrollView::OnVScroll(nSBCode, nPos, pScrollBar);
	UpdateRulersInfo(RW_VSCROLL, GetScrollPosition());
}


void CClusterMDIView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	//Invalidate();

	CScrollView::OnHScroll(nSBCode, nPos, pScrollBar);
	UpdateRulersInfo(RW_HSCROLL, GetScrollPosition());
}

BOOL CClusterMDIView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	//HCURSOR hc = LoadCursor(NULL, IDC_HAND);
	SetCursor(m_HCs[m_ToolType]);
	return TRUE;
	return CBCGPScrollView::OnSetCursor(pWnd, nHitTest, message);
}

void CClusterMDIView::OnUpdateTool(CCmdUI * pCmdUI)
{
	pCmdUI->SetCheck(pCmdUI->m_nID - ID_EDIT_ARROW == (int)m_ToolType);
}

void CClusterMDIView::OnTool(UINT nID)
{
	if (m_ToolType == TOOL_ARROW && nID != ID_EDIT_ARROW)
	{
		//1.ȡ��ѡ�еĵ�
		if (!m_selectedPoints.empty())
		{
			for (SELECT_POINT::const_iterator it = m_selectedPoints.begin();
				it != m_selectedPoints.end();it++)
			{
				SendInvalidateRect(this, *it);
			}
			m_selectedPoints.clear();
			UpdateWindow();
			::PostMessage(AfxGetMainWnd()->m_hWnd, WM_MOUSE_XY, 1, 0);

		}
	}

	switch (nID)
	{
	case ID_EDIT_ARROW:
		m_ToolType = enum_ToolType::TOOL_ARROW;
		break;
	case ID_EDIT_PEN:
		m_ToolType = enum_ToolType::TOOL_PEN;
		break;
	case ID_EDIT_HAND:
		m_ToolType = enum_ToolType::TOOL_HAND;
		break;
	case ID_EDIT_DRAWFILE:
		m_ToolType = TOOL_DRAWFILL;
		break;
	}


}

void CClusterMDIView::OnEditFitsize()
{
	// TODO: �ڴ���������������
	CClusterMDIDoc* pDoc = GetDocument();
	CDialogCanvasSize dlg(m_rangeCanvas, pDoc->m_RawPoints);
	dlg.m_XScreen = m_szCanvas.cx;
	dlg.m_YScreen = m_szCanvas.cy;
	dlg.m_dbXStart = m_rangeCanvas.xStart;
	dlg.m_dbXEnd = m_rangeCanvas.xEnd;
	dlg.m_dbYStart = m_rangeCanvas.yStart;
	dlg.m_dbYEnd = m_rangeCanvas.yEnd;

	if (dlg.DoModal() == IDOK)
	{
		m_szCanvas.cx = dlg.m_XScreen;
		m_szCanvas.cy = dlg.m_YScreen;
		m_rangeCanvas.xStart = dlg.m_dbXStart;
		m_rangeCanvas.xEnd = dlg.m_dbXEnd;
		m_rangeCanvas.yStart = dlg.m_dbYStart;
		m_rangeCanvas.yEnd = dlg.m_dbYEnd;


		CSize sizeTotal = m_szCanvas + CSize(CANVAS_PIXEL_MARGIN * 2, CANVAS_PIXEL_MARGIN * 2);
		SetScrollSizes(MM_TEXT, sizeTotal);
		UpdateRulersInfo(0, GetScrollPosition());
	}
}



//BOOL CClusterMDIView::OnScrollBy(CSize sizeScroll, BOOL bDoScroll)
//{
//	// TODO: �ڴ����ר�ô����/����û���
//	TRACE(__FUNCTION__ _T("\n"));
//	return CBCGPScrollView::OnScrollBy(sizeScroll, bDoScroll);
//}


BOOL CClusterMDIView::OnScrollBy(CSize sizeScroll, BOOL bDoScroll)
{
	// TODO: �ڴ����ר�ô����/����û���
	TRACE(__FUNCTION__ _T("\n"));
	return CBCGPScrollView::OnScrollBy(sizeScroll, bDoScroll);
}


void CClusterMDIView::OnSize(UINT nType, int cx, int cy)
{
	CBCGPScrollView::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������
	if (m_nMapMode == MM_TEXT)
	{
		UpdateRulersInfo(0, GetScrollPosition());
		//UpdateRulersInfo(RT_HORIZONTAL, GetScrollPosition());
	}
}


void CClusterMDIView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CClusterMDIDoc* pDoc = GetDocument();
	bool bNeedUpdateWindow = false;
	//1.ˢ�±���α�
	UpdateRulersInfo(RW_POSITION, GetScrollPosition(), point);

	//2����������inpage
	CClientDC dc(this);
	OnPrepareDC(&dc);
	CPoint pt(point);
	dc.DPtoLP(&pt);
	pt.Offset(-CANVAS_PIXEL_MARGIN, -CANVAS_PIXEL_MARGIN);

	//3��ˢ��bar��Ϣ
	CRect pageRect(CPoint(0, 0), m_szCanvas);
	if (pageRect.PtInRect(pt))
	{
		double X, Y;
		X = m_rangeCanvas.xStart + pt.x * (m_rangeCanvas.xEnd - m_rangeCanvas.xStart) / m_szCanvas.cx;
		Y = m_rangeCanvas.yStart + pt.y * (m_rangeCanvas.yEnd - m_rangeCanvas.yStart) / m_szCanvas.cy;
		double* pDouble = new double[2];
		pDouble[0] = X;
		pDouble[1] = Y;
		::PostMessage(AfxGetMainWnd()->m_hWnd, WM_MOUSE_XY, 0, (LPARAM)pDouble);
	}
	//4�����û�а���������ֱ������
	if (!nFlags & MK_LBUTTON)
		goto FUN_END;

	if (m_ToolType == TOOL_HAND) {
		SetScrollPos(SB_HORZ, m_RefScroll.x - point.x + m_RefPoint.x);
		SetScrollPos(SB_VERT, m_RefScroll.y - point.y + m_RefPoint.y);
		Invalidate(FALSE);
		UpdateRulersInfo(RW_VSCROLL, GetScrollPosition());
		UpdateRulersInfo(RW_HSCROLL, GetScrollPosition());
	}
	if (m_ToolType == TOOL_DRAWFILL) 
	{
		//-1.���ǰ��û��capture,˵��û�н�������down-move-up����,�˳� 
		if (!m_bCaptured)
			goto FUN_END;

		//0. �����������hasMove
		m_bArrowHasMove = true;

		//bNeedUpdateWindow = true;
		//1.��һ�ַ���ֱ����draw�ﻭ
		CPoint oldPt = m_ptForwardPointInPage;
		oldPt.Offset(CANVAS_PIXEL_MARGIN, CANVAS_PIXEL_MARGIN);
		m_ptForwardPointInPage = pt;
		CPoint tempPt1 = m_ptOrgPointInPage;
		CPoint tempPt2 = m_ptForwardPointInPage;
		tempPt1.Offset(CANVAS_PIXEL_MARGIN, CANVAS_PIXEL_MARGIN);
		tempPt2.Offset(CANVAS_PIXEL_MARGIN, CANVAS_PIXEL_MARGIN);
		CRect rc1 = CombinePointToRect(tempPt1, tempPt2);
		CRect rc2 = CombinePointToRect(tempPt1, oldPt);
		rc1 |= rc2;
		//rc.InflateRect(1, 1);
		InvalidateRect(rc1, FALSE);
		//2.�ڶ��ַ�������/�ػ�

	}

	if (m_ToolType == TOOL_ARROW)
	{

		TRACE(__FUNCTION__ _T("+++(%d,%d)%d,%d,%d\n"),
			pt.x, pt.y,
			m_bArrowHasMove, m_bArrowDrawNet, m_bArrowHit);
		//-1.���ǰ��û��capture,˵��û�н�������down-move-up����,�˳� 
		if (!m_bCaptured)
			goto FUN_END;

		//0. �����������hasMove
		m_bArrowHasMove = true;

		//1�����hiton��������ק
		if (m_bArrowHit)
		{
			ASSERT(!m_selectedPoints.empty());
			//1.����ѡ�е�ľ���;m_rcSelected

			//2.�õ��еĵ��������͵�ǰ���������ƫ��
			CRect rcTarget = m_rcSelected;
			rcTarget.OffsetRect(pt.x- m_itHitOnPoint->targetX,pt.y- m_itHitOnPoint->targetY);

			if (rcTarget.left < 0)
				rcTarget.OffsetRect(-rcTarget.left,0);
			if (rcTarget.top < 0)
				rcTarget.OffsetRect(0, -rcTarget.top);
			if (rcTarget.right > m_szCanvas.cx)
				rcTarget.OffsetRect(m_szCanvas.cx -rcTarget.right, 0);
			if (rcTarget.bottom < 0)
				rcTarget.OffsetRect(0,m_szCanvas.cy-rcTarget.bottom);

			int offsetX, offsetY;
			offsetX = rcTarget.left - m_rcSelected.left;
			offsetY = rcTarget.top - m_rcSelected.top;

			TRACE(_T("====%d,%d\n"), offsetX, offsetY);
			//3.��ѡ�м����е����е�ƫ���ƶ�λ��

			for (SELECT_POINT::iterator it = m_selectedPoints.begin();it != m_selectedPoints.end();it++)
			{
				SendInvalidateRect(this, *it);
				it->X = it->targetX + offsetX;
				it->Y = it->targetY + offsetY;

				//����ƫ�ƺ�ı��ֵ: ע���������ط�,selected���Ϻ�ԭ����
				double XValue, YValue;
				XValue = m_rangeCanvas.xStart +
					it->X * (m_rangeCanvas.xEnd - m_rangeCanvas.xStart) / m_szCanvas.cx;
				YValue = m_rangeCanvas.yStart +
					it->Y * (m_rangeCanvas.yEnd - m_rangeCanvas.yStart) / m_szCanvas.cy;
				it->XValue = it->it->first = XValue;
				it->YValue = it->it->second = YValue;

				SendInvalidateRect(this, *it);
			}
			//ѡ�񼯺Ϸ����˸ı�֪ͨbar
			::PostMessage(AfxGetMainWnd()->m_hWnd, WM_MOUSE_XY, 1, 0);

			pDoc->SetModifiedFlag(TRUE);
		}
		else  //2.���û��hiton ����������
		{
			if(!m_bArrowDrawNet)
				m_bArrowDrawNet = true;
			//bNeedUpdateWindow = true;
			//1.��һ�ַ���ֱ����draw�ﻭ
			CPoint oldPt = m_ptForwardPointInPage;
			oldPt.Offset(CANVAS_PIXEL_MARGIN, CANVAS_PIXEL_MARGIN);
			m_ptForwardPointInPage = pt;
			CPoint tempPt1 = m_ptOrgPointInPage;
			CPoint tempPt2 = m_ptForwardPointInPage;
			tempPt1.Offset(CANVAS_PIXEL_MARGIN, CANVAS_PIXEL_MARGIN);
			tempPt2.Offset(CANVAS_PIXEL_MARGIN, CANVAS_PIXEL_MARGIN);
			CRect rc1 = CombinePointToRect(tempPt1, tempPt2);
			CRect rc2 = CombinePointToRect(tempPt1, oldPt);
			rc1 |= rc2;
			//rc.InflateRect(1, 1);
			InvalidateRect(rc1, FALSE);
			//2.�ڶ��ַ�������/�ػ�

			if (CaculateNewSelected(rc2, rc1))
			{
				//ѡ�񼯺Ϸ����˸ı�֪ͨbar
				::PostMessage(AfxGetMainWnd()->m_hWnd, WM_MOUSE_XY, 1, 0);
			}
		}

	}
FUN_END:

	UpdateWindow();
	CBCGPScrollView::OnMouseMove(nFlags, point);
}


void CClusterMDIView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (nChar == VK_ESCAPE)
	{
		m_ToolType = TOOL_ARROW;
		SetCursor(m_HCs[m_ToolType]);
	}
	CBCGPScrollView::OnKeyDown(nChar, nRepCnt, nFlags);
}


BOOL CClusterMDIView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	return FALSE;
	return CBCGPScrollView::OnEraseBkgnd(pDC);
}


void CClusterMDIView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	/*
	
	TOOL_ARROW =0,
	TOOL_PEN =1,
	TOOL_HAND =2,
	TOOL_SELECT =3
	*/
	//SetFocus();
	
	CClusterMDIDoc* pDoc = GetDocument();
	CRect pageRect(CPoint(CANVAS_PIXEL_MARGIN, CANVAS_PIXEL_MARGIN), m_szCanvas);
	CPoint origPoint = point;

	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);

	//if (m_ToolType != TOOL_ARROW)
	//{
	//	//1.ȡ��ѡ�еĵ�
	//	for (SELECT_POINT::const_iterator it = m_selectedPoints.begin();
	//		it != m_selectedPoints.end();it++)
	//	{
	//		SendInvalidateRect(this, *it);
	//	}
	//	m_selectedPoints.clear();
	//	UpdateWindow();
	//	::PostMessage(AfxGetMainWnd()->m_hWnd, WM_MOUSE_XY, 1, 0);
	//}

	switch (m_ToolType)
	{
	case TOOL_ARROW:

		TRACE(__FUNCTION__ _T("+++(%d,%d)%d,%d,%d\n"),
			point.x, point.y,
			m_bArrowHasMove, m_bArrowDrawNet, m_bArrowHit);

		if (!pageRect.PtInRect(point))
		{
			break;
		}
		//else
		{
			point.Offset(-CANVAS_PIXEL_MARGIN, -CANVAS_PIXEL_MARGIN);
			bool bNeedUpdateWindow = false;
			//-1.���������capture
			if (!m_bCaptured)
			{
				SetCapture();
				m_bCaptured = true;
			}
			//0.��¼���µ�����
			m_ptOrgPointInPage = m_ptForwardPointInPage = point;

			//1.���Ȳ���hiton״̬
			stSelectPoint searchPoint;
			bool bHitOnPoint = SearchPoint(pDoc->m_RawPoints, m_rangeCanvas, m_szCanvas, point, searchPoint);
			if (!bHitOnPoint)
			{
				//1.ȡ��ѡ�еĵ�
				for (SELECT_POINT::const_iterator it = m_selectedPoints.begin();
					it != m_selectedPoints.end();it++)
				{
					SendInvalidateRect(this, *it);
					bNeedUpdateWindow = true;
				}
				m_selectedPoints.clear();
			}
			else
			{
				//0.���������hiton
				m_bArrowHit = true;
				//1.�ж��Ƿ�����ǰѡ�еĵ�
				bool bIsAlreadySelected = false;
				for (SELECT_POINT::iterator it = m_selectedPoints.begin();it != m_selectedPoints.end();it++)
				{
					if (searchPoint.X == it->X && searchPoint.Y == it->Y)
					{
						//���µ��е���ĵ�����,������������ʼλ��
						m_itHitOnPoint = it;
						//it->targetX = it->X;
						//it->targetY = it->Y;
						bIsAlreadySelected = true;
						break;
					}
				}

				//2.�������ѡ�еĵ��֮һ��ȡ��ԭ��ѡ��
				if (!bIsAlreadySelected)
				{
					for (SELECT_POINT::const_iterator it = m_selectedPoints.begin();it != m_selectedPoints.end();it++)
					{
						SendInvalidateRect(this, *it);
					}
					m_selectedPoints.clear();

					//2.1.�µ������
					m_selectedPoints.push_back(searchPoint);
					m_itHitOnPoint = m_selectedPoints.end()-1;
					SendInvalidateRect(this, searchPoint);
					bNeedUpdateWindow = true;
				}

				//2.1������ѡ�е�������е��ԭ��λ�ü�����,ǰ��Ӧ�ö��Ѿ�����,
				//		����ʡȥ��һ��
				for (SELECT_POINT::iterator it = m_selectedPoints.begin();it != m_selectedPoints.end();it++)
				{
					it->targetX = it->X;
					it->targetY = it->Y;
				}

				//3.����ѡ�е�ľ���;
				ASSERT(!m_selectedPoints.empty());
				SELECT_POINT::const_iterator it = m_selectedPoints.begin();
				m_rcSelected = CRect(it->X, it->Y, it->X, it->Y);
				for (it = m_selectedPoints.begin();
					it != m_selectedPoints.end();it++)
				{
					if (it->X < m_rcSelected.left)
						m_rcSelected.left = it->X;
					if (it->X > m_rcSelected.right)
						m_rcSelected.right = it->X;
					if (it->Y < m_rcSelected.top)
						m_rcSelected.top = it->Y;
					if (it->Y > m_rcSelected.bottom)
						m_rcSelected.bottom = it->Y;
				}

				//m_rcSelected.OffsetRect(-m_rcSelected.TopLeft());
			}
			if (bNeedUpdateWindow)
			{
				UpdateWindow();
				::PostMessage(AfxGetMainWnd()->m_hWnd, WM_MOUSE_XY, 1, 0);
			}
		}
		break;
	case TOOL_PEN:
		if (!pageRect.PtInRect(point))
		{
			break;
		}
		point.Offset(-CANVAS_PIXEL_MARGIN, -CANVAS_PIXEL_MARGIN);

		double X, Y;
		X = m_rangeCanvas.xStart + point.x * (m_rangeCanvas.xEnd - m_rangeCanvas.xStart) / m_szCanvas.cx;
		Y = m_rangeCanvas.yStart + point.y * (m_rangeCanvas.yEnd - m_rangeCanvas.yStart) / m_szCanvas.cy;
		pDoc->m_RawPoints.push_back(make_pair(X, Y));
		{
			//Invalidate(FALSE);
			CRect rc(point, point);
			rc.InflateRect(5, 5);
			rc.OffsetRect(CANVAS_PIXEL_MARGIN, CANVAS_PIXEL_MARGIN);

			dc.LPtoDP(&rc);
			InvalidateRect(&rc, FALSE);
		}

		pDoc->SetModifiedFlag(TRUE);
		//DPtoLP(clientRect);
		break;
	case TOOL_HAND:
		m_RefScroll = GetScrollPosition();
		m_RefPoint = origPoint;
		break;
	case TOOL_DRAWFILL:
		if (!pageRect.PtInRect(point))
		{
			break;
		}
		{
			point.Offset(-CANVAS_PIXEL_MARGIN, -CANVAS_PIXEL_MARGIN);
			bool bNeedUpdateWindow = false;
			//-1.���������capture
			if (!m_bCaptured)
			{
				SetCapture();
				m_bCaptured = true;
			}
			//0.��¼���µ�����
			m_ptOrgPointInPage = m_ptForwardPointInPage = point;
			m_bArrowDrawNet = true;

			if (bNeedUpdateWindow)
			{
				UpdateWindow();
				::PostMessage(AfxGetMainWnd()->m_hWnd, WM_MOUSE_XY, 1, 0);
			}
		}
		break;
	}

	CBCGPScrollView::OnLButtonDown(nFlags, point);
}


void CClusterMDIView::OnEditZoomin()
{
	// TODO: �ڴ���������������
	int cx, cy;
	cx = m_szCanvas.cx;
	cy = m_szCanvas.cy;

	cx = ceil(cx / 100.0) * 100;
	cy = ceil(cy / 100.0) * 100;
	if (cx < 2000) cx += 100;
	if (cy < 2000) cy += 100;
	m_szCanvas.cx = cx;
	m_szCanvas.cy = cy;

	CSize sizeTotal = m_szCanvas + CSize(CANVAS_PIXEL_MARGIN * 2, CANVAS_PIXEL_MARGIN * 2);
	SetScrollSizes(MM_TEXT, sizeTotal);
	UpdateRulersInfo(0, GetScrollPosition());

	//�޸�ѡ�е������
	SELECT_POINT::iterator sit;
	for (sit = m_selectedPoints.begin();sit != m_selectedPoints.end();sit++)
	{
		sit->X = /*CANVAS_PIXEL_MARGIN + */
			(int)(m_szCanvas.cx*(sit->XValue - m_rangeCanvas.xStart) / (m_rangeCanvas.xEnd - m_rangeCanvas.xStart));
		sit->Y = /*CANVAS_PIXEL_MARGIN +*/
			(int)(m_szCanvas.cy*(sit->YValue - m_rangeCanvas.yStart) / (m_rangeCanvas.yEnd - m_rangeCanvas.yStart));
		
	}

	Invalidate(FALSE);
}


void CClusterMDIView::OnEditZoomout()
{
	// TODO: �ڴ���������������
	int cx, cy;
	cx = m_szCanvas.cx;
	cy = m_szCanvas.cy;

	cx = ceil(cx / 100.0) * 100;
	cy = ceil(cy / 100.0) * 100;
	if (cx > 200) cx -= 100;
	if (cy > 200) cy -= 100;
	m_szCanvas.cx = cx;
	m_szCanvas.cy = cy;

	CSize sizeTotal = m_szCanvas + CSize(CANVAS_PIXEL_MARGIN * 2, CANVAS_PIXEL_MARGIN * 2);
	SetScrollSizes(MM_TEXT, sizeTotal);
	UpdateRulersInfo(0, GetScrollPosition());

	//�޸�ѡ�е������
	SELECT_POINT::iterator sit;
	for (sit = m_selectedPoints.begin();sit != m_selectedPoints.end();sit++)
	{
		sit->X = /*CANVAS_PIXEL_MARGIN +*/
			(int)(m_szCanvas.cx*(sit->XValue - m_rangeCanvas.xStart) / (m_rangeCanvas.xEnd - m_rangeCanvas.xStart));
		sit->Y = /*CANVAS_PIXEL_MARGIN +*/
			(int)(m_szCanvas.cy*(sit->YValue - m_rangeCanvas.yStart) / (m_rangeCanvas.yEnd - m_rangeCanvas.yStart));

	}

	Invalidate(FALSE);
}


void CClusterMDIView::ClearViewBeforSwitch()
{
	CClusterMDIDoc* pDoc = GetDocument();

	m_ToolType = TOOL_ARROW;

	m_selectedPoints.clear();
	double* pDouble = new double[2];
	pDouble[0] = 0;
	pDouble[1] = 0;
	::PostMessage(AfxGetMainWnd()->m_hWnd, WM_MOUSE_XY, 1, (LPARAM)pDouble);

	if (VerifyCanvasSize(m_rangeCanvas, pDoc->m_RawPoints) > 0)
	{
		AfxMessageBox(_T("���е㳬��ȡֵ��Χ"));
		CaculateCanvasSize(m_rangeCanvas, pDoc->m_RawPoints);
		FloorRealRange(m_rangeCanvas);
		UpdateRulersInfo(RW_POSITION, GetScrollPosition());
	}
}


void CClusterMDIView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CClusterMDIDoc* pDoc = GetDocument();
	bool bNeedUpdateWindow = false;
	
	//1����������inpage
	CClientDC dc(this);
	OnPrepareDC(&dc);
	CPoint pt(point);
	dc.DPtoLP(&pt);
	pt.Offset(-CANVAS_PIXEL_MARGIN, -CANVAS_PIXEL_MARGIN);

	if (m_ToolType == TOOL_DRAWFILL)
	{
		//-1.���capture,
		if (m_bCaptured)
		{
			ReleaseCapture();
			m_bCaptured = false;
		}
		else
		{
			goto FUN_END;
		}
		//1.���û���ƶ���,������е�״̬
		if (!m_bArrowHasMove)
		{
			m_bArrowHasMove = false;
			m_bArrowDrawNet = false;

			goto FUN_END;
		}

		//C
		CInsertDataDlg dlg;
		dlg.m_bDrawFill = true;

		double dbXStart, dbXEnd, dbYStart, dbYEnd;
		dbXStart = m_rangeCanvas.xStart + m_ptOrgPointInPage.x * (m_rangeCanvas.xEnd - m_rangeCanvas.xStart) / m_szCanvas.cx;
		dbYStart = m_rangeCanvas.yStart + m_ptOrgPointInPage.y * (m_rangeCanvas.yEnd - m_rangeCanvas.yStart) / m_szCanvas.cy;
		dbXEnd = m_rangeCanvas.xStart + m_ptForwardPointInPage.x * (m_rangeCanvas.xEnd - m_rangeCanvas.xStart) / m_szCanvas.cx;
		dbYEnd = m_rangeCanvas.yStart + m_ptForwardPointInPage.y * (m_rangeCanvas.yEnd - m_rangeCanvas.yStart) / m_szCanvas.cy;

		int iXStart = round(dbXStart < dbXEnd ? dbXStart : dbXEnd);
		int iXEnd = round(dbXStart < dbXEnd ? dbXEnd : dbXStart);
		int iYStart = round(dbYStart < dbYEnd ? dbYStart : dbYEnd);
		int iYEnd = round(dbYStart < dbYEnd ? dbYEnd : dbYStart);
		dlg.m_iXStart = iXStart;
		dlg.m_iXEnd = iXEnd;
		dlg.m_iYStart = iYStart;
		dlg.m_iYEnd = iYEnd;
		dlg.m_iCount = 20;

		if (dlg.DoModal() == IDOK && !dlg.m_RawPoints.empty())
		{
			pDoc->m_RawPoints.insert(pDoc->m_RawPoints.end(), dlg.m_RawPoints.begin(), dlg.m_RawPoints.end());
			
			pDoc->SetModifiedFlag(TRUE);
		}

		CPoint tempPt1 = m_ptOrgPointInPage;
		CPoint tempPt2 = m_ptForwardPointInPage;
		tempPt1.Offset(CANVAS_PIXEL_MARGIN, CANVAS_PIXEL_MARGIN);
		tempPt2.Offset(CANVAS_PIXEL_MARGIN, CANVAS_PIXEL_MARGIN);
		CRect rc1 = CombinePointToRect(tempPt1, tempPt2);
		//rc.InflateRect(1, 1);


		m_bArrowHasMove = false;
		m_bArrowDrawNet = false;

		InvalidateRect(rc1, FALSE);
		
	}
	if (m_ToolType == TOOL_ARROW)
	{
		TRACE(__FUNCTION__ _T("+++(%d,%d)%d,%d,%d\n"), 
			pt.x,pt.y,
			m_bArrowHasMove, m_bArrowDrawNet, m_bArrowHit);
		//-1.���capture,
		if (m_bCaptured)
		{
			ReleaseCapture();
			m_bCaptured = false;
		}
		else
		{
			goto FUN_END;
		}
		//1.���û���ƶ���,������е�״̬
		if (!m_bArrowHasMove)
		{
			m_bArrowHasMove = false;
			m_bArrowHit = false;
			m_bArrowDrawNet = false;

			goto FUN_END;
		}

		//ReleaseCapture();

		//2.�������ק
		if (m_bArrowHit)
		{

		}
		//else  //3.���������
		if(m_bArrowDrawNet)
		{
			//1.��һ�ַ���ֱ����draw�ﻭ
			CPoint oldPt = m_ptForwardPointInPage;
			oldPt.Offset(CANVAS_PIXEL_MARGIN, CANVAS_PIXEL_MARGIN);
			m_ptForwardPointInPage = pt;
			CPoint tempPt1 = m_ptOrgPointInPage;
			CPoint tempPt2 = m_ptForwardPointInPage;
			tempPt1.Offset(CANVAS_PIXEL_MARGIN, CANVAS_PIXEL_MARGIN);
			tempPt2.Offset(CANVAS_PIXEL_MARGIN, CANVAS_PIXEL_MARGIN);
			CRect rc1 = CombinePointToRect(tempPt1, tempPt2);
			CRect rc2 = CombinePointToRect(tempPt1, oldPt);
			rc1 |= rc2;
			//rc.InflateRect(1, 1);
			InvalidateRect(rc1, FALSE);
			//2.�ڶ��ַ�������/�ػ�

		}

		m_bArrowHasMove = false;
		m_bArrowHit = false;
		m_bArrowDrawNet = false;
	}

FUN_END:
	CBCGPScrollView::OnLButtonUp(nFlags, point);
}


bool CClusterMDIView::CaculateNewSelected(CRect& orgRc, CRect& nowRC)
{
	CClusterMDIDoc* pDoc = GetDocument();
	RAWPOINTS_T& rawPoints(pDoc->m_RawPoints);

	CRect nowRCInPage = nowRC;
	nowRCInPage.OffsetRect(-CANVAS_PIXEL_MARGIN, -CANVAS_PIXEL_MARGIN);

	int count = 0;

	//���￼�Ǽ򵥵����Ϊ�˱ܿ���Ҫ��ѡ�񼯺����Ƴ�ȡ��ѡ���

	//���Ƴ�û�б�ѡ���

	for (SELECT_POINT::const_iterator sit = m_selectedPoints.begin();
		sit != m_selectedPoints.end();sit++)
	{
		if (!nowRCInPage.PtInRect(CPoint(sit->X, sit->Y)))
		{
			SendInvalidateRect(this, *sit);
			sit = m_selectedPoints.erase(sit);
			count++;
			if (sit == m_selectedPoints.end())
				break;
		}
	}

	const int drawXStart = CANVAS_PIXEL_MARGIN;
	const int drawYStart = CANVAS_PIXEL_MARGIN;
	RAWPOINTS_T::const_iterator it;
	for (RAWPOINTS_T::iterator it = pDoc->m_RawPoints.begin();it != pDoc->m_RawPoints.end(); it++)
	{
		int X = /*drawXStart + */(int)(m_szCanvas.cx*((*it).first - m_rangeCanvas.xStart) / (m_rangeCanvas.xEnd - m_rangeCanvas.xStart));
		int Y = /*drawYStart + */(int)(m_szCanvas.cy*((*it).second - m_rangeCanvas.yStart) / (m_rangeCanvas.yEnd - m_rangeCanvas.yStart));

		
		if (nowRCInPage.PtInRect(CPoint(X, Y)))
		{
			bool bFound = false;
			for (SELECT_POINT::const_iterator sit = m_selectedPoints.begin();
				sit != m_selectedPoints.end();sit++)
			{
				if (sit->it == it)
				{
					bFound = true;
					break;
				}
			}

			if (!bFound)
			{
				stSelectPoint searchResult;
				searchResult.it = it;
				searchResult.X = X;
				searchResult.Y = Y;
				searchResult.XValue = it->first;
				searchResult.YValue = it->second;

				m_selectedPoints.push_back(searchResult);

				SendInvalidateRect(this, searchResult);

				count++;
			}
		}
	}

	if (count > 0)
	{
		return true;
	}
	else
		return false;
}


void CClusterMDIView::OnEditDelete()
{
	// TODO: �ڴ���������������
	CClusterMDIDoc* pDoc = GetDocument();
	RAWPOINTS_T& rawPoints(pDoc->m_RawPoints);
		//1.ȡ��ѡ�еĵ�
	if (!m_selectedPoints.empty())
	{

		CString s = _T("ȷ��Ҫɾ����:");
		s.Format(_T("ȷ��Ҫɾ��%d����"), m_selectedPoints.size());

		if (AfxMessageBox(s, MB_YESNO | MB_ICONEXCLAMATION) != IDYES)
		{
			return;
		}

		RAWPOINTS_T::iterator raw_it = rawPoints.begin();
		RAWPOINTS_T::iterator over_it = rawPoints.begin();
	
		for (;raw_it != rawPoints.end();raw_it++)
		{
			bool bFound = false;
			for (SELECT_POINT::const_iterator sel_it = m_selectedPoints.begin();
				sel_it != m_selectedPoints.end();sel_it++)
			{
				if (raw_it == sel_it->it)
				{
					bFound = true;
					break;
				}
			}

			if (!bFound)
			{
				//if(over_it != raw_it)
					*over_it = *raw_it;
				over_it++;
			}
			else
			{

			}
		}
		rawPoints.erase(over_it, rawPoints.end());

		for (SELECT_POINT::const_iterator sel_it = m_selectedPoints.begin();
			sel_it != m_selectedPoints.end();sel_it++)
		{
			SendInvalidateRect(this, *sel_it);
		}
		m_selectedPoints.clear();
		UpdateWindow();
		::PostMessage(AfxGetMainWnd()->m_hWnd, WM_MOUSE_XY, 1, 0);
		pDoc->SetModifiedFlag(TRUE);
	}
	else
	{
		AfxMessageBox(_T("����ѡ�е�"));
	}
}


void CClusterMDIView::OnEditCopy()
{
	// TODO: �ڴ���������������
	CClusterMDIDoc* pDoc = GetDocument();
	RAWPOINTS_T& rawPoints(pDoc->m_RawPoints);
	//1.ȡ��ѡ�еĵ�
	if (!m_selectedPoints.empty())
	{
		//1.�ѵ�ǰ��Ļ�����Ͻ���Ϊ����λ��
		//��������inpage
		CPoint pt(0, 0);
		CClientDC dc(this);
		OnPrepareDC(&dc);
		dc.DPtoLP(&pt);
		pt.Offset(-CANVAS_PIXEL_MARGIN, -CANVAS_PIXEL_MARGIN);
		if (pt.x < 0 || pt.x>m_szCanvas.cx)
		{
			pt.x = 0;
		}

		if (pt.y < 0 || pt.y>m_szCanvas.cy)
		{
			pt.y = 0;
		}

		//����ѡ�е㼯�Ͼ���
		m_rcSelected = CRect(m_selectedPoints.begin()->X, m_selectedPoints.begin()->Y, 
			m_selectedPoints.begin()->X, m_selectedPoints.begin()->Y);
		for (SELECT_POINT::const_iterator it = m_selectedPoints.begin();
			it != m_selectedPoints.end();it++)
		{
			if (it->X < m_rcSelected.left)
				m_rcSelected.left = it->X;
			if (it->X > m_rcSelected.right)
				m_rcSelected.right = it->X;
			if (it->Y < m_rcSelected.top)
				m_rcSelected.top = it->Y;
			if (it->Y > m_rcSelected.bottom)
				m_rcSelected.bottom = it->Y;
		}
		//2.�õ��еĵ��������͵�ǰ���������ƫ��
		CRect rcTarget = m_rcSelected;
		rcTarget.OffsetRect(pt.x - m_rcSelected.left, pt.y - m_rcSelected.top);

		if (rcTarget.left < 0)
			rcTarget.OffsetRect(-rcTarget.left, 0);
		if (rcTarget.top < 0)
			rcTarget.OffsetRect(0, -rcTarget.top);
		if (rcTarget.right > m_szCanvas.cx)
			rcTarget.OffsetRect(m_szCanvas.cx - rcTarget.right, 0);
		if (rcTarget.bottom < 0)
			rcTarget.OffsetRect(0, m_szCanvas.cy - rcTarget.bottom);

		int offsetX, offsetY;
		offsetX = pt.x - m_rcSelected.left;
		offsetY = pt.y - m_rcSelected.top;

		//����ƫ�ƺ�ı��ֵ
		double XValue, YValue;
		XValue = offsetX * (m_rangeCanvas.xEnd - m_rangeCanvas.xStart) / m_szCanvas.cx;
		YValue = offsetY * (m_rangeCanvas.yEnd - m_rangeCanvas.yStart) / m_szCanvas.cy;

		TRACE(_T("====%d,%d\n"), offsetX, offsetY);
		SELECT_POINT newSelectedPoints;
		int oldSize = rawPoints.size();
		//3.��ѡ�м����е����е����ƫ�ƣ�����һ�ݷ���rawpoint��
		for (SELECT_POINT::iterator it = m_selectedPoints.begin();it != m_selectedPoints.end();it++)
		{
			SendInvalidateRect(this, *it);

			rawPoints.push_back(make_pair(it->XValue+ XValue,it->YValue+ YValue));

			stSelectPoint st;
			st.X = it->X + offsetX;
			st.Y = it->Y + offsetY;
			st.XValue = it->XValue + XValue;
			st.YValue = it->YValue + YValue;
			newSelectedPoints.push_back(st);

			SendInvalidateRect(this, st);
		}

		//�������µ�ѡ�񼯺ϵ�it
		for (SELECT_POINT::iterator it = newSelectedPoints.begin();it != newSelectedPoints.end();it++)
		{
			it->it = rawPoints.begin() + oldSize++;
		}
		//����ѡ�񼯺ϵ�ֵ������ѡ�񼯺���
		m_selectedPoints.clear();
		m_selectedPoints.insert(m_selectedPoints.end(),newSelectedPoints.begin(), newSelectedPoints.end());
		//ѡ�񼯺Ϸ����˸ı�֪ͨbar
		::PostMessage(AfxGetMainWnd()->m_hWnd, WM_MOUSE_XY, 1, 0);
		pDoc->SetModifiedFlag(TRUE);
	}
	else
	{
		AfxMessageBox(_T("����ѡ�е�"));
	}
}



BOOL CClusterMDIView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	if (zDelta > 0)
		OnEditZoomin();
	else
		OnEditZoomout();
	return TRUE;
	//return CBCGPScrollView::OnMouseWheel(nFlags, zDelta, pt);
}
