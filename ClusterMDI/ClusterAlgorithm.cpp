#include "stdafx.h"
#include "ClusterMDI.h"
#include "ClusterAlgorithm.h"

#include "DialogRun.h"

#include <stdlib.h>
#include <time.h>
#include <boost\foreach.hpp>


static DWORD g_Lock = 0;
boost::mutex m_mu;
static COLORREF crs[]
{
	RGB(50,50,50),
	RGB(255,0,0),
	RGB(0,255,0),
	RGB(0,0,255),
	RGB(200,0,0),
	RGB(0,200,0),
	RGB(0,0,200),
	RGB(150,0,0),
	RGB(0,150,0),
	RGB(0,0,150),
	RGB(100,0,0),
	RGB(0,100,0),
	RGB(0,0,100),
	RGB(50,0,0),
	RGB(0,50,0),
	RGB(0,0,50),
	RGB(0,0,0),
};

void CClusterAlgorithmBase::threadFun()
{
	while (!m_StopSignal)
	{
		step();
		m_pWnd->Invalidate();

		if (end())
			break;
			
		boost::this_thread::sleep(boost::posix_time::milliseconds(m_ms));
	}
	//OnPaint();
	if (!m_StopSignal)
	{
		m_pWndParent->PostMessage(WM_ALG_RUNCOMPLETE, 0, 0);
		m_RunStatus = RUN_STATUS_END;
	}
	return;
}

int CClusterAlgorithmBase::shuldDraw(DRAWTASKTYPE type)
{
	return m_bDoDrawTask[type];
}

bool CClusterAlgorithmBase::init(void * pParam)
{
	m_err = ERROR_NONE;
	m_RunStatus = RUN_STATUS_READY;

	return true;
}

bool CClusterAlgorithmBase::refresh()
{
	ClearDrawTask();
	return true;
}

void CClusterAlgorithmBase::run(int ms)
{
	//if (m_RunStatus == RUN_STATUS_STOP) return;
	m_ms = ms;
	m_RunStatus = RUN_STATUS_RUN;
	m_StopSignal = 0;
	m_pThread = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&CClusterAlgorithmBase::threadFun, this)));
	return;
}

void CClusterAlgorithmBase::pause()
{
	if (m_RunStatus != RUN_STATUS_RUN) return;
	m_StopSignal = 1;
	m_pThread->join();
	m_pThread.reset();
	m_RunStatus = RUN_STATUS_READY;
	return;
}

void CClusterAlgorithmBase::stop()
{
	if (m_RunStatus != RUN_STATUS_RUN) return;
	m_StopSignal = 1;
	m_pThread->join();
	m_pThread.reset();
	m_RunStatus = RUN_STATUS_END;
	return;
}

void CClusterAlgorithmBase::step()
{
	while (1)
	{
		runStatusMachine();
		if (!m_bSkipStatus[m_AlgStatus])
			break;
	}
}

void CClusterAlgorithmBase::doDraw(CDC * pDC, const CRect& rc)
{

	boost::mutex::scoped_lock lock(m_mu);

	//if (InterlockedCompareExchange(&g_Lock, 1, 0) == 1) return;
	bool bDrawProgress = GetDoDrawFlag(DRAW_TASK_PROGRESS);

	CRect clientRect(rc);
	CSize szCanvas;
	szCanvas.cx = rc.Width();
	szCanvas.cy = rc.Height();

	CRect coordinateRect(rc);
	if (bDrawProgress)
	{
		coordinateRect.bottom -= 10;
	}

	//��������ϵ
	DrawCoordinate(pDC, coordinateRect);

	//ִ�л滭����
	for (int i = 0;i < m_DrawTask.size();i++)
	{
		if(m_bDoDrawTask[m_DrawTask[i].type])
			ExcuteDrawTaskItem(pDC, szCanvas, m_rangeCanvas, m_DrawTask[i]);
	}
	//int index = 0;
	//for (DRAWTASK_T::iterator it = m_DrawTask.begin();it != m_DrawTask.end();it++)
	//{
	//	index++;
	//	if(m_bDoDrawTask[it->type])
	//		ExcuteDrawTaskItem(pDC, szCanvas, m_rangeCanvas, *it);
	//}

	if (bDrawProgress)
	{
		CRect prRc(0, szCanvas.cy - RUN_PANNEL_PROGRESS_HEIGHT, szCanvas.cx, szCanvas.cy);
		DrawAlgorithmProgress(pDC, prRc);
	}

	return;
}

void CClusterAlgorithmBase::DrawCoordinate(CDC * pDC, const CRect & rc)
{
	CRect clientRect(rc);

	//1.��䱳��
	CBrush brushBackground; // Must initialize!
	brushBackground.CreateSolidBrush(RUN_BACKGROUND);
	pDC->FillRect(&clientRect, &brushBackground);

	//2.������ϵ
	const int drawXStart = CANVAS_PIXEL_MARGIN;
	const int drawYStart = CANVAS_PIXEL_MARGIN;

	CRect rectCoordinate(rc);
	rectCoordinate.DeflateRect(drawXStart, drawYStart);

	DWORD cr = RGB(0, 0, 0);
	HGDIOBJ pOldBrush = pDC->SelectObject(GetStockObject(NULL_BRUSH));

	CPen pen;
	pen.CreatePen(PS_SOLID, 1, cr);
	CPen *pOldPen = pDC->SelectObject(&pen);
	pDC->Rectangle(rectCoordinate);

	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);

	//3.д����
	//3.1 ������������
	CFont vFont;
	CFont hFont;
	vFont.CreateFont(RUN_PANNEL_TEXT_SIZE, 0, 900, 900, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, VARIABLE_PITCH | FF_ROMAN, _T("Times New Roman"));
	hFont.CreateFont(RUN_PANNEL_TEXT_SIZE, 0, 000, 000, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, VARIABLE_PITCH | FF_ROMAN, _T("Times New Roman"));

	//3.2 д������

	//3.2.1 ׼������
	int iRulerLength;
	int iRulerStartMargin;
	int iRulerSideMargin;
	double Start;
	double End;
	int RegStart;
	int RegStep;
	int RegEnd;
	double step;
	int l, t, r, b, w, h;

	CFont* pOldFont = pDC->SelectObject(&hFont);
	int oldTextAlign = pDC->SetTextAlign(TA_CENTER | TA_TOP);
	int oldBkMode = pDC->SetBkMode(TRANSPARENT);

	//3.2.2 ��ʼд 
	iRulerLength = clientRect.Width() - 2 * RUN_PANNEL_CANVAS_PIXEL_MARGIN;
	iRulerStartMargin = RUN_PANNEL_CANVAS_PIXEL_MARGIN;
	iRulerSideMargin = RUN_PANNEL_CANVAS_PIXEL_MARGIN - 2;



	//���м䲿��
	Start = m_rangeCanvas.xStart;
	End = m_rangeCanvas.xEnd;
	RegStart = m_rangeCanvas.xRegStart;
	RegStep = m_rangeCanvas.xRegStep;
	RegEnd = m_rangeCanvas.xRegEnd;

#define LINE_COUNT 2.0
	step = iRulerLength / LINE_COUNT / (RegEnd - RegStart);
	//for (int i = 0;i <= 100;i++)
	for (int i = RegStart;i < RegEnd;i++)
	{
		int iOffset = i - RegStart;
		for (int k = 0;k < (int)LINE_COUNT + 1;k++)
		{
			int x, y, w, h;
			x = iRulerStartMargin + step*(iOffset * LINE_COUNT + k);
			if (k % 2 == 0)
				y = RUN_PANNEL_LINE_MARGIN_1;
			else y = RUN_PANNEL_LINE_MARGIN_2;
			w = 1;
			h = iRulerSideMargin - y;

			//if(i!=0 && i!=100)
			pDC->PatBlt(x, y, w, h, BLACKNESS);
		}
		//if (i!= RegStart)
		{
			double v = (i + 1)*pow(10, RegStep);
			TCHAR buffer[100];
			memset(buffer, 0, sizeof(TCHAR) * 100);
			_stprintf_s(buffer, _T("%g"), v);

			int x, y;
			x = iRulerStartMargin + step*(iOffset + 1) * LINE_COUNT;
			y = RUN_PANNEL_TEXT_MARGIN;
			pDC->TextOut(x, y, buffer);
		}
	}
	//©�˸�0������
	{
		double v = RegStart*pow(10, RegStep);
		TCHAR buffer[100];
		memset(buffer, 0, sizeof(TCHAR) * 100);
		_stprintf_s(buffer, _T("%g"), v);

		int x, y;
		x = iRulerStartMargin;
		y = RUN_PANNEL_TEXT_MARGIN;
		pDC->TextOut(x, y, buffer);
	}


	//3.3 ������
	pDC->SelectObject(&vFont);
	//pDC->SetTextAlign(TA_CENTER | TA_TOP);
	//pDC->SetBkMode(TRANSPARENT);

	//3.2.2 ��ʼд 
	iRulerLength = clientRect.Height() - 2 * RUN_PANNEL_CANVAS_PIXEL_MARGIN;
	iRulerStartMargin = RUN_PANNEL_CANVAS_PIXEL_MARGIN;
	iRulerSideMargin = RUN_PANNEL_CANVAS_PIXEL_MARGIN - 2;

	//���м䲿��
	Start = m_rangeCanvas.yStart;
	End = m_rangeCanvas.yEnd;
	RegStart = m_rangeCanvas.yRegStart;
	RegStep = m_rangeCanvas.yRegStep;
	RegEnd = m_rangeCanvas.yRegEnd;

#define LINE_COUNT 2.0
	step = iRulerLength / LINE_COUNT / (RegEnd - RegStart);
	//for (int i = 0;i <= 100;i++)
	for (int i = RegStart;i < RegEnd;i++)
	{
		int iOffset = i - RegStart;
		for (int k = 0;k < (int)LINE_COUNT + 1;k++)
		{
			int x, y, w, h;
			y = iRulerStartMargin + step*(iOffset * LINE_COUNT + k);
			if (k % 2 == 0)
				x = RUN_PANNEL_LINE_MARGIN_1;
			else x = RUN_PANNEL_LINE_MARGIN_2;
			h = 1;
			w = iRulerSideMargin - x;

			//if(i!=0 && i!=100)
			pDC->PatBlt(x, y, w, h, BLACKNESS);
		}
		//if (i!= RegStart)
		{
			double v = (i + 1)*pow(10, RegStep);
			TCHAR buffer[100];
			memset(buffer, 0, sizeof(TCHAR) * 100);
			_stprintf_s(buffer, _T("%g"), v);

			int x, y;
			y = iRulerStartMargin + step*(iOffset + 1) * LINE_COUNT;
			x = RUN_PANNEL_TEXT_MARGIN;
			pDC->TextOut(x, y, buffer);
		}
	}
	//©�˸�0������
	{
		double v = RegStart*pow(10, RegStep);
		TCHAR buffer[100];
		memset(buffer, 0, sizeof(TCHAR) * 100);
		_stprintf_s(buffer, _T("%g"), v);

		int x, y;
		y = iRulerStartMargin;
		x = RUN_PANNEL_TEXT_MARGIN;
		pDC->TextOut(x, y, buffer);
	}


	pDC->SelectObject(pOldFont);
	pDC->SetTextAlign(oldTextAlign);
	pDC->SetBkMode(oldBkMode);

	return;
}

bool CClusterAlgorithmBase::ExcuteDrawTaskItem(CDC* pDC, CSize& szCanvas, stDoubleRange& rangeCanvas, DRAWTASKITEM & item)
{
	/*
	DRAW_TASK_POINT,
	DRAW_TASK_SELECTED_POINT,
	DRAW_TASK_KSET,
	DRAW_TASK_SELECT_KSET,
	DRAW_TASK_MEASURE,
	DRAW_TASK_CENTOID,
	*/
	CSize drawSize;
	drawSize.cx = szCanvas.cx - RUN_PANNEL_CANVAS_PIXEL_MARGIN * 2;
	drawSize.cy = szCanvas.cy - RUN_PANNEL_CANVAS_PIXEL_MARGIN * 2;
	const int drawXStart = RUN_PANNEL_CANVAS_PIXEL_MARGIN;
	const int drawYStart = RUN_PANNEL_CANVAS_PIXEL_MARGIN;
	switch (item.type)
	{
	case DRAW_TASK_POINT:
	case DRAW_TASK_SELECTED_POINT:
	{
		const RAWPOINT_T& pt = m_RawPoints[item.wp];
		int X = drawXStart + (int)(drawSize.cx*(pt.first - rangeCanvas.xStart) / (rangeCanvas.xEnd - rangeCanvas.xStart));
		int Y = drawYStart + (int)(drawSize.cy*(pt.second - rangeCanvas.yStart) / (rangeCanvas.yEnd - rangeCanvas.yStart));
		DrawAPoint(pDC, X, Y, item.type - DRAW_TASK_POINT, RGB(50, 50, 50));
	}
	break;
	case DRAW_TASK_NONE:
		break;
	}
	return true;
}

void CClusterAlgorithmBase::PushDrawTask(DRAWTASKTYPE type, WPARAM wp, LPARAM lp)
{
	boost::mutex::scoped_lock lock(m_mu);

	DRAWTASKITEM drawItem;
	drawItem.type = type;
	drawItem.wp = wp;
	drawItem.lp = lp;

	m_DrawTask.push_back(drawItem);
	m_iDrawTaskCount++;

	//֪ͨ�����ػ�ͬһ�������߳���ִ��
	//if(m_bDoDrawTask[type])
	//	m_pWnd->Invalidate();

	return;
}

void CClusterAlgorithmBase::Redraw()
{
	m_pWnd->Invalidate();
	return;
}

void CClusterAlgorithmBase::PopDrawTask(int step)
{
	boost::mutex::scoped_lock lock(m_mu);
	ASSERT(step >= 0);
	//ASSERT(step <= m_DrawTask.size());
	ASSERT(step <= m_iDrawTaskCount);

	bool bInvalidate = false;
	for (int i = 0;i<step;i++)
	{
		if (m_bDoDrawTask[m_DrawTask.back().type])
			bInvalidate = true;
		m_DrawTask.pop_back();
	}

	m_iDrawTaskCount -= step;

	//if (bInvalidate)
	//	m_pWnd->Invalidate();

	return;
}

void CClusterAlgorithmBase::ClearDrawTask()
{
	PopDrawTask(m_iDrawTaskCount);
	//m_pWnd->Invalidate();

	return;
}

void CClusterAlgorithmBase::NotifyDataChanged()
{
	m_pWndParent->PostMessage(WM_ALG_DATACHANGE, 0, 0);
}

void CClusterAlgorithmBase::AddResultSet(LPCTSTR csTitle)
{
	ASSERT(::IsWindow(m_pWnd->GetSafeHwnd()));
	ASSERT(::IsWindow(m_pWndParent->GetSafeHwnd()));
	CDialogRun *pDlg = (CDialogRun *)m_pWndParent;
	//m_tabCtrl.InsertItem(0, _T("�����"));
	CBCGPGridCtrl *pGridCtrl = new CBCGPGridCtrl();
	//pGridCtrl->Create(WS_CHILD, rc, this, 1);

	pGridCtrl->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, CRect(0, 0, 100, 100), &pDlg->m_tabCtrl, IDC_STATIC_GRIDLOCATION);
	pGridCtrl->EnableMarkSortedColumn(FALSE);
	pGridCtrl->EnableHeader(TRUE, BCGP_GRID_HEADER_MOVE_ITEMS);
	pGridCtrl->EnableRowHeader(TRUE);
	pGridCtrl->EnableLineNumbers();
	pGridCtrl->SetClearInplaceEditOnEnter(FALSE);
	pGridCtrl->EnableInvertSelOnCtrl();
	pGridCtrl->SetScalingRange(0.1, 4.0);
	pGridCtrl->InsertColumn(0, _T("X"), 100);
	pGridCtrl->InsertColumn(1, _T("Y"), 100);
	pGridCtrl->SetReadOnly();


	pDlg->m_tabCtrl.AddTab(pGridCtrl, csTitle);

	m_Grids.push_back(pGridCtrl);
}

CClusterAlgorithmBase::CClusterAlgorithmBase(const RAWPOINTS_T& rawPoints, CWnd *pWnd) :
	m_RawPoints(rawPoints)
	, m_pWnd(pWnd)
{
	srand((unsigned)time(NULL));
	m_pWndParent = m_pWnd->GetParent();
	ASSERT(::IsWindow(m_pWnd->GetSafeHwnd()));
	ASSERT(::IsWindow(m_pWndParent->GetSafeHwnd()));
	m_err = ERROR_NONE;
	m_RunStatus = RUN_STATUS_NONE;

	int iSetp = 0;
	for (RAWPOINTS_T::const_iterator it = m_RawPoints.begin(); it != m_RawPoints.end();it++)
	{
		DRAWTASKITEM drawItem;
		drawItem.type = DRAW_TASK_POINT;
		drawItem.wp = iSetp++;
		drawItem.lp = 0;

		m_DrawTask.push_back(drawItem);
	}

	m_iDrawTaskCount = 0;

	BOOST_FOREACH(bool& var, m_bSkipStatus)
	{
		var = false;
	}
	BOOST_FOREACH(int& var, m_bDoDrawTask)
	{
		var = 1;
	}
	
	CaculateCanvasSize(m_rangeCanvas, rawPoints);
	FloorRealRange(m_rangeCanvas);
}


CClusterAlgorithmBase::~CClusterAlgorithmBase()
{
}

CClusterKmeans::CClusterKmeans(const RAWPOINTS_T & rawPoints, CWnd *pWnd) :
	CClusterAlgorithmBase(rawPoints, pWnd)
{
	setAlgorithmStatus(ALGORITHM_STATUS_NONE);

	//m_iDrawTaskCount = 0;
	//�������еĵ㲻�����κμ���
	m_K_Points_Set_Tags.clear();
	for (int i = 0;i < m_RawPoints.size();i++)
		m_K_Points_Set_Tags.push_back(-1);

	CaculateCanvasSize(m_rangeCanvas, rawPoints);
	FloorRealRange(m_rangeCanvas);

	iIndex = 0;
	iAlgorithmCount = 0;
}

CClusterKmeans::~CClusterKmeans()
{
}

bool CClusterKmeans::init(void * pParam)
{
	KMEANS_PARAM *pKmeansParam = (KMEANS_PARAM*)pParam;
	ASSERT(pKmeansParam);
	ASSERT(pKmeansParam->k >= 3);
	ASSERT(pKmeansParam->k == pKmeansParam->points.size());

	m_paramK = pKmeansParam->k;
	m_param_K_Points = pKmeansParam->points;
	m_param_Count = pKmeansParam->count;

	m_K_Weight_Points = pKmeansParam->points;

	iIndex = 0;
	iAlgorithmCount = 0;

	//�������еĵ㲻�����κμ���
	m_K_Points_Set_Tags.clear();
	for (int i = 0;i < m_RawPoints.size();i++)
		m_K_Points_Set_Tags.push_back(-1);

	setAlgorithmStatus(ALGORITHM_STATUS_INIT);

	//������˻������еĻ�ͼ����
	ClearDrawTask();
	//�����������ͼ����
	for (int i = 0;i < m_paramK;i++)
	{
		PushDrawTask(DRAW_TASK_KSET, i, 0);
	}

	//�޸Ľ����grid
	CDialogRun *pDlg = (CDialogRun *)m_pWndParent;
	pDlg->m_tabCtrl.SetRedraw(FALSE);
	pDlg->m_tabCtrl.SetActiveTab(1);
	//���ᱣ����grid�������
	int iReservedGrids = m_Grids.size() < m_paramK ? m_Grids.size() : m_paramK;
	for (int i = 0;i < iReservedGrids;i++)
	{
		m_Grids[i]->RemoveAll();
	}

	if (m_Grids.size() > m_paramK)
	{
		//ɾ�������
		for (int i = m_Grids.size();i > m_paramK;i--)
		{
			pDlg->m_tabCtrl.RemoveTab(i+2-1);
			CBCGPGridCtrl* pGridCtrl = m_Grids.back();
			m_Grids.pop_back();
			//pGridCtrl->DestroyWindow();
			delete pGridCtrl;
		}
	}
	else if (m_Grids.size() < m_paramK)
	{
		//����
		for (int i = m_Grids.size();i < m_paramK;i++)
		{
			CString str;
			str.Format(_T("�����%d"), i + 1);
			AddResultSet(str);
		}
	}
	else
	{
		//�ոպ�
	}
	pDlg->m_tabCtrl.SetRedraw(TRUE);
	pDlg->m_tabCtrl.Invalidate();

	//������ʾ
	RAWPOINTS_T::const_iterator it;
	m_pGridCtrl_Centoids->RemoveAll();
	for (it = m_param_K_Points.begin();it != m_param_K_Points.end(); it++)
	{
		CBCGPGridRow* pRow = m_pGridCtrl_Centoids->CreateRow(4);

		pRow->GetItem(0)->SetValue((double)(*it).first);
		pRow->GetItem(1)->SetValue((double)(*it).second);
		pRow->GetItem(2)->SetValue((int)0);

		int crSize = sizeof(crs) / sizeof(COLORREF);

#ifndef _BCGSUITE_INC_
		CBCGPGridColorItem* pColorItem = new CBCGPGridColorItem(crs[it- m_param_K_Points.begin()+1]);
		ASSERT_VALID(pColorItem);

		pColorItem->EnableOtherButton(_T("Other"));
#else
		CGridColorItem* pColorItem = new CGridColorItem(color);
#endif

		pRow->ReplaceItem(3, pColorItem);

		m_pGridCtrl_Centoids->AddRow(pRow);
	}

	return CClusterAlgorithmBase::init(pParam);
}

void CClusterKmeans::runStatusMachine()
{
	if (end())
	{
		ASSERT(FALSE);
		return;
	}
	//ASSERT(m_CallStack.size() != 0);
	//ASSERT(m_CallStack.size() == deep());
	switch (getAlgorithmStatus())
	{
	case ALGORITHM_STATUS_INIT:
		//����û����Ҫ�����,��Ϊ��һ���Ѿ���iIndex�趨����	
		//deep==1
		iIndex = 0;
		//deep==2
		iCurrentKSetIndex = 0;
		dbMinDistance = MAX_DISTANCE;
		iMinDistanceKSetIndex = 0;

		iAlgorithmCount = 0;
		//ֻ����ʾ��Ҫ

		//�ƽ�״̬
		setAlgorithmStatus(ALGORITHM_STATUS_KMEANS_SELECT_A_POINT);
		break;
	case ALGORITHM_STATUS_KMEANS_SELECT_A_POINT:
		//����û����Ҫ�����,��Ϊ��һ���Ѿ���iIndex�趨����
		//ֻ����ʾ��Ҫ
		PushDrawTask(DRAW_TASK_SELECTED_POINT, iIndex, 0);

		//�ƽ�״̬
		setAlgorithmStatus(ALGORITHM_STATUS_KMEANS_SELECT_A_KSET);
		break;
	case ALGORITHM_STATUS_KMEANS_SELECT_A_KSET:
		//����û����Ҫ�����,��Ϊ��һ���Ѿ���iCurrentKSetIndex�趨����
		//ֻ����ʾ��Ҫ
		PushDrawTask(DRAW_TASK_SELECT_KSET, iCurrentKSetIndex, 0);

		//�ƽ�״̬
		setAlgorithmStatus(ALGORITHM_STATUS_KMEANS_BEGINMEASURE);
		break;
	case ALGORITHM_STATUS_KMEANS_BEGINMEASURE:
		//
	{
		double newDistance = sqrt((m_RawPoints[iIndex].first - m_K_Weight_Points[iCurrentKSetIndex].first)*
			(m_RawPoints[iIndex].first - m_K_Weight_Points[iCurrentKSetIndex].first)
			+ (m_RawPoints[iIndex].second - m_K_Weight_Points[iCurrentKSetIndex].second)*
			(m_RawPoints[iIndex].second - m_K_Weight_Points[iCurrentKSetIndex].second)
		);

		if (newDistance < dbMinDistance)
		{
			dbMinDistance = newDistance;
			iMinDistanceKSetIndex = iCurrentKSetIndex;
		}
	}
	//ֻ����ʾ��Ҫ
	PushDrawTask(DRAW_TASK_MEASURE,
		iIndex,
		(LPARAM)iCurrentKSetIndex);

	//�ƽ�״̬
	setAlgorithmStatus(ALGORITHM_STATUS_KMEANS_ENDMEASURE);
	break;
	case ALGORITHM_STATUS_KMEANS_ENDMEASURE:
		//


		PopDrawTask(2); //��ѡ�м��϶�������
		iCurrentKSetIndex++;
		if (iCurrentKSetIndex == m_paramK)
		{
			//�ƽ�״̬
			//�˵�����������Ĳ�����ϣ�׼������ĳ������
			//PopDrawTask(3); //��ѡ�е��ѡ�м��϶�������
			setAlgorithmStatus(ALGORITHM_STATUS_KMEANS_ENDMEASURE_ALL);
		}
		else
		{
			//�ƽ�״̬
			setAlgorithmStatus(ALGORITHM_STATUS_KMEANS_SELECT_A_KSET);
		}
		//ֻ����ʾ��Ҫ
		break;
	case ALGORITHM_STATUS_KMEANS_ENDMEASURE_ALL:
	{
		PopDrawTask();
		//���������ѡ���ľ��༯��
		int oldKSetIndex = m_K_Points_Set_Tags[iIndex];
		m_K_Points_Set_Tags[iIndex] = iMinDistanceKSetIndex;

		{
			CBCGPGridRow* pRow = m_pGridCtrl_Centoids->GetRow(iMinDistanceKSetIndex);
			pRow->GetItem(2)->SetValue((pRow->GetItem(2)->GetValue().intVal +1));

			if (oldKSetIndex != -1)
			{
				CBCGPGridRow* pRow = m_pGridCtrl_Centoids->GetRow(oldKSetIndex);
				pRow->GetItem(2)->SetValue((pRow->GetItem(2)->GetValue().intVal - 1));
			}
		}
		//���µ�iIndex��grid��ʾ,��ԭ���Ľ����ɾ�������ӵ��½������
		if (oldKSetIndex != iMinDistanceKSetIndex)
		{
			//����ߵ��Ч�ʣ����������ܳ����⣬�ɴ�����ڼ���
			int count = m_RawPoints.size();
			if(oldKSetIndex != -1) m_Grids[oldKSetIndex]->RemoveAll();
			m_Grids[iMinDistanceKSetIndex]->RemoveAll();
			for (int i = 0;i < count;i++)
			{
				if (oldKSetIndex != -1 && m_K_Points_Set_Tags[i] == oldKSetIndex)
				{
					CBCGPGridRow* pRow = m_Grids[oldKSetIndex]->CreateRow(2);
					pRow->GetItem(0)->SetValue(m_RawPoints[i].first);
					pRow->GetItem(1)->SetValue(m_RawPoints[i].second);
					m_Grids[oldKSetIndex]->AddRow(pRow, FALSE);
				}
				if (m_K_Points_Set_Tags[i] == iMinDistanceKSetIndex)
				{
					CBCGPGridRow* pRow = m_Grids[iMinDistanceKSetIndex]->CreateRow(2);
					pRow->GetItem(0)->SetValue(m_RawPoints[i].first);
					pRow->GetItem(1)->SetValue(m_RawPoints[i].second);
					m_Grids[iMinDistanceKSetIndex]->AddRow(pRow, FALSE);
				}
			}
			if (oldKSetIndex != -1) m_Grids[oldKSetIndex]->AdjustLayout();
			m_Grids[iMinDistanceKSetIndex]->AdjustLayout();
			if (0)
			{
				if (oldKSetIndex >= 0)
				{
					int count = m_Grids[oldKSetIndex]->GetRowCount();
					for (int i = 0;i < count;i++)
					{
						CBCGPGridRow *pRow = m_Grids[oldKSetIndex]->GetRow(i);
						ASSERT(pRow);
						if (!pRow)
							break;
						if (pRow->GetData() == iIndex)
						{
							m_Grids[oldKSetIndex]->RemoveRow(i,FALSE);
							m_Grids[oldKSetIndex]->AdjustLayout();
							break;
						}
					}
				}

				//�򵥵����Ӽ��ɣ���ú�����˳��һ��
				CBCGPGridRow* pRow = m_Grids[iMinDistanceKSetIndex]->CreateRow(2);
				pRow->GetItem(0)->SetValue(m_RawPoints[iIndex].first);
				pRow->GetItem(1)->SetValue(m_RawPoints[iIndex].second);
				pRow->SetData(iIndex);
				m_Grids[iMinDistanceKSetIndex]->AddRow(pRow, FALSE);
				m_Grids[iMinDistanceKSetIndex]->AdjustLayout();
			}

		}

		dbMinDistance = MAX_DISTANCE;
		iMinDistanceKSetIndex = 0;
		//RecaculateCentoid(iMinDistanceKSetIndex);

		//��ʼ����һ��������д���
		iIndex++;
		iCurrentKSetIndex = 0;
		if (iIndex == m_RawPoints.size())
		{
			//�ƽ�״̬
			setAlgorithmStatus(ALGORITHM_STATUS_KMEANS_RECACULATE_CENTOID);
		}
		else
		{
			//�ƽ�״̬
			setAlgorithmStatus(ALGORITHM_STATUS_KMEANS_SELECT_A_POINT);
		}
		//ֻ����ʾ��Ҫ
		//�������ı��˼���,�������⴦����ʾ,ǰ���Ѿ����������е����ʾ����
		Redraw();
		break;

	}
	case ALGORITHM_STATUS_KMEANS_RECACULATE_CENTOID:

	{

		//���¼������е���ľ�������
		RAWPOINTS_T oldCentoid = m_K_Weight_Points;
		for (int i = 0;i < m_paramK; i++)
		{
			RecaculateCentoid(i);
		}

		//����grid��ʾ
		int count = m_pGridCtrl_Centoids->GetRowCount();
		ASSERT(m_K_Weight_Points.size() == count);

		for (int i = 0;i < count;i++)
		{
			CBCGPGridRow *pRow = m_pGridCtrl_Centoids->GetRow(i);

			pRow->GetItem(0)->SetValue(m_K_Weight_Points[i].first);
			pRow->GetItem(1)->SetValue(m_K_Weight_Points[i].second);
		}

		//����ƫ���
		double d = 0;
		for (int i = 0;i < m_paramK; i++)
		{
			d += sqrt(
				(m_K_Weight_Points[i].first - oldCentoid[i].first)*(m_K_Weight_Points[i].first - oldCentoid[i].first)
				+ (m_K_Weight_Points[i].second - oldCentoid[i].second)*(m_K_Weight_Points[i].second - oldCentoid[i].second)
			);
		}
		double *pd = (double*)new double;
		*pd = d;

		PushDrawTask(DRAW_TASK_CENTOID, iAlgorithmCount + 1, (LPARAM)pd);

		iAlgorithmCount++;
		if (iAlgorithmCount == m_param_Count || m_param_Count == 0 && d < 0.01)
		{
			//�ƽ�״̬
			setAlgorithmStatus(ALGORITHM_STATUS_COMPLETE);
		}
		else
		{
			iIndex = 0;
			iCurrentKSetIndex = 0;
			//�ƽ�״̬
			setAlgorithmStatus(ALGORITHM_STATUS_KMEANS_SELECT_A_POINT);
		}
	}

	//ֻ����ʾ��Ҫ
	//�������ı��˼���,�������⴦����ʾ,ǰ���Ѿ����������е����ʾ����
	//Redraw();
	break;
	default:
		ASSERT(FALSE);
		break;
	}

	return;
}

bool CClusterKmeans::initProp()
{
	CDialogRun *pDlg = (CDialogRun *)m_pWndParent;

	CBCGPProp* m_pGroupGeneral = new CBCGPProp(_T("K-means"));


	m_pPropK = new CBCGPProp(_T("����K"), 1, (_variant_t)3, _T("�趨K-means�㷨��������"));
	m_pPropK->EnableSpinControl(TRUE, 3, 10);
	m_pGroupGeneral->AddSubItem(m_pPropK);

	m_pPropRound = new CBCGPProp(_T("��������"), 8, (_variant_t)0, _T("�趨K-means�㷨����������0��ʾֱ�����ĵ���ƫ��С��0.01"));
	m_pPropRound->EnableSpinControl(TRUE, 0, 100);
	m_pGroupGeneral->AddSubItem(m_pPropRound);

	m_pPropRand = new CBCGPProp(_T("���ѡ������"), 2, (_variant_t)true, _T("�����ָ��ѡ������"));
	m_pGroupGeneral->AddSubItem(m_pPropRand);

	pDlg->m_wndPropList.AddProperty(m_pGroupGeneral);


	m_pPropMeasure = new CBCGPProp(_T("��ʾ��������"), 3, (_variant_t)true, _T("�㷨���й�������ʾ����"));
	pDlg->m_pPropGroup->AddSubItem(m_pPropMeasure);
	m_pPropCentoid = new CBCGPProp(_T("��ʾ����ƫ��"), 5, (_variant_t)true, _T("�㷨���й�������ʾÿ�ε�����������ƫ��ֵ"));
	pDlg->m_pPropGroup->AddSubItem(m_pPropCentoid);
	m_pPropProgress = new CBCGPProp(_T("��ʾ������"), 4, (_variant_t)true, _T("�㷨���й�������ʾִ�н���"));
	pDlg->m_pPropGroup->AddSubItem(m_pPropProgress);

	return true;
}

bool CClusterKmeans::initGrid()
{
	//AddResultSet(_T("ԭʼ����"));

	CDialogRun *pDlg = (CDialogRun *)m_pWndParent;
	//m_tabCtrl.InsertItem(0, _T("�����"));
	m_pGridCtrl_Orgi = new CBCGPGridCtrl();
	//pGridCtrl->Create(WS_CHILD, rc, this, 1);

	m_pGridCtrl_Orgi->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, CRect(0, 0, 100, 100), &pDlg->m_tabCtrl, IDC_STATIC_GRIDLOCATION);
	m_pGridCtrl_Orgi->EnableMarkSortedColumn(FALSE);
	m_pGridCtrl_Orgi->EnableHeader(TRUE, BCGP_GRID_HEADER_MOVE_ITEMS);
	m_pGridCtrl_Orgi->EnableRowHeader(TRUE);
	m_pGridCtrl_Orgi->EnableLineNumbers();
	m_pGridCtrl_Orgi->SetClearInplaceEditOnEnter(FALSE);
	m_pGridCtrl_Orgi->EnableInvertSelOnCtrl();
	m_pGridCtrl_Orgi->SetScalingRange(0.1, 4.0);
	m_pGridCtrl_Orgi->InsertColumn(0, _T("X"), 100);
	m_pGridCtrl_Orgi->InsertColumn(1, _T("Y"), 100);
	m_pGridCtrl_Orgi->SetReadOnly();


	pDlg->m_tabCtrl.AddTab(m_pGridCtrl_Orgi, _T("ԭʼ����"));



	RAWPOINTS_T::const_iterator it;
	for (it = m_RawPoints.begin();it != m_RawPoints.end(); it++)
	{
		CBCGPGridRow* pRow = m_pGridCtrl_Orgi->CreateRow(2);

		pRow->GetItem(0)->SetValue((double)(*it).first);
		pRow->GetItem(1)->SetValue((double)(*it).second);

		m_pGridCtrl_Orgi->AddRow(pRow);
	}


	m_pGridCtrl_Centoids = new CBCGPGridCtrl();
	//pGridCtrl->Create(WS_CHILD, rc, this, 1);

	m_pGridCtrl_Centoids->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, CRect(0, 0, 100, 100), &pDlg->m_tabCtrl, IDC_STATIC_GRIDLOCATION);
	m_pGridCtrl_Centoids->EnableMarkSortedColumn(FALSE);
	m_pGridCtrl_Centoids->EnableHeader(TRUE, BCGP_GRID_HEADER_MOVE_ITEMS);
	m_pGridCtrl_Centoids->EnableRowHeader(TRUE);
	m_pGridCtrl_Centoids->EnableLineNumbers();
	m_pGridCtrl_Centoids->SetClearInplaceEditOnEnter(FALSE);
	m_pGridCtrl_Centoids->EnableInvertSelOnCtrl();
	m_pGridCtrl_Centoids->SetScalingRange(0.1, 4.0);
	m_pGridCtrl_Centoids->InsertColumn(0, _T("X"), 100);
	m_pGridCtrl_Centoids->InsertColumn(1, _T("Y"), 100);
	m_pGridCtrl_Centoids->InsertColumn(2, _T("����"), 60);
	m_pGridCtrl_Centoids->InsertColumn(3, _T("��ɫ"), 60);
	m_pGridCtrl_Centoids->SetReadOnly();


	pDlg->m_tabCtrl.AddTab(m_pGridCtrl_Centoids, _T("�ʵ�ֵ"));

	//
	
	return true;
}

void CClusterKmeans::propChanged(UINT nID, CBCGPProp * pProp)
{
	CDialogRun *pDlg = (CDialogRun *)m_pWndParent;
	nID = pProp->GetID();

	if (nID == 5)
	{
		bool bShowCentoid = m_pPropCentoid->GetValue().boolVal;

		SetDoDrawFlag(DRAW_TASK_CENTOID, bShowCentoid);

		Redraw();
	}
	if (nID == 4)
	{
		bool bShowProgress = m_pPropProgress->GetValue().boolVal;

		SetDoDrawFlag(DRAW_TASK_PROGRESS, bShowProgress);

		Redraw();
	}
	if (nID == 3)
	{
		bool bShowMeasure = m_pPropMeasure->GetValue().boolVal;

		SetSkipStatus(ALGORITHM_STATUS_KMEANS_SELECT_A_KSET, !bShowMeasure);
		SetSkipStatus(ALGORITHM_STATUS_KMEANS_BEGINMEASURE, !bShowMeasure);
		SetSkipStatus(ALGORITHM_STATUS_KMEANS_ENDMEASURE, !bShowMeasure);
		//SetSkipStatus(ALGORITHM_STATUS_KMEANS_ENDMEASURE_ALL, !bShowMeasure);
		//SetDoDrawFlag(DRAW_TASK_SELECTED_POINT, bShowMeasure);
		SetDoDrawFlag(DRAW_TASK_SELECT_KSET, bShowMeasure);
		SetDoDrawFlag(DRAW_TASK_MEASURE, bShowMeasure);
	}
	if (nID == 1)
	{
		int iCount = pProp->GetValue().intVal;
		if (!m_pPropRand->GetValue().boolVal)
		{
			if (iCount > m_pPorps.size())
			{
				//����
				for (int i = m_pPorps.size();i < iCount;i++)
				{
					CString str;
					str.Format(_T("����%d"), i + 1);
					CBCGPProp* pGroupProp = new CBCGPProp(str, 0, TRUE);

					double X, Y;
					int iRandX, iRandY;
					iRandX = rand() % 100;
					iRandY = rand() % 100;

					X = m_rangeCanvas.xStart + iRandX*(m_rangeCanvas.xEnd - m_rangeCanvas.xStart) / 100;
					Y = m_rangeCanvas.yStart + iRandY*(m_rangeCanvas.yEnd - m_rangeCanvas.yStart) / 100;

					pGroupProp->AddSubItem(new CBCGPProp(_T("X"), (_variant_t)X, _T("X ���ֵ")));
					pGroupProp->AddSubItem(new CBCGPProp(_T("Y"), (_variant_t)Y, _T("Y ���ֵ")));

					//m_pGroupGeneral->AddSubItem(pGroupProp);
					pDlg->m_wndPropList.AddProperty(pGroupProp);

					m_pPorps.push_back(pGroupProp);
				}
			}
			else
			{
				//ɾ��
				while (m_pPorps.size() > iCount)
				{
					pDlg->m_wndPropList.DeleteProperty(m_pPorps.back());
					m_pPorps.pop_back();
				}
			}
			pDlg->m_wndPropList.AdjustLayout();
		}
	}
	if (nID == 2)
	{
		_variant_t vt = pProp->GetValue();

		if (vt.boolVal)
		{
			//ɾ������ָ�㼯��
			while (!m_pPorps.empty())
			{
				pDlg->m_wndPropList.DeleteProperty(m_pPorps.back());
				m_pPorps.pop_back();
			}
		}
		else
		{
			//�����ʵ㼯��
			int iCount = m_pPropK->GetValue().intVal;

			for (int i = 0;i < iCount;i++)
			{
				CString str;
				str.Format(_T("����%d"), i + 1);
				CBCGPProp* pGroupProp = new CBCGPProp(str, 0, TRUE);

				double X, Y;
				int iRandX, iRandY;
				iRandX = rand() % 100;
				iRandY = rand() % 100;

				X = m_rangeCanvas.xStart + iRandX*(m_rangeCanvas.xEnd - m_rangeCanvas.xStart) / 100;
				Y = m_rangeCanvas.yStart + iRandY*(m_rangeCanvas.yEnd - m_rangeCanvas.yStart) / 100;

				pGroupProp->AddSubItem(new CBCGPProp(_T("X"), (_variant_t)X, _T("X ���ֵ")));
				pGroupProp->AddSubItem(new CBCGPProp(_T("Y"), (_variant_t)Y, _T("Y ���ֵ")));

				//m_pGroupGeneral->AddSubItem(pGroupProp);
				pDlg->m_wndPropList.AddProperty(pGroupProp);

				m_pPorps.push_back(pGroupProp);
			}
			pDlg->m_wndPropList.AdjustLayout();
		}
	}
	return;
}

bool CClusterKmeans::getPropValue(void * pParam)
{
	CClusterKmeans::KMEANS_PARAM *pKmeansParam = (CClusterKmeans::KMEANS_PARAM *)pParam;
	pKmeansParam->k = m_pPropK->GetValue().intVal;
	pKmeansParam->count = m_pPropRound->GetValue().intVal;

	ASSERT(m_pPropRand->GetValue().boolVal || pKmeansParam->k == m_pPorps.size());

	if (!m_pPropRand->GetValue().boolVal)
	{
		//��prop

		for (int i = 0;i < pKmeansParam->k;i++)
		{
			double X, Y;

			X = m_pPorps[i]->GetSubItem(0)->GetValue().dblVal;
			Y = m_pPorps[i]->GetSubItem(1)->GetValue().dblVal;

			pKmeansParam->points.push_back(make_pair(X, Y));
		}
	}
	else
	{
		//�������
		for (int i = 0;i < pKmeansParam->k;i++)
		{
			double X, Y;
			int iRandX, iRandY;
			iRandX = rand() % 100;
			iRandY = rand() % 100;

			X = m_rangeCanvas.xStart + iRandX*(m_rangeCanvas.xEnd - m_rangeCanvas.xStart) / 100;
			Y = m_rangeCanvas.yStart + iRandY*(m_rangeCanvas.yEnd - m_rangeCanvas.yStart) / 100;

			pKmeansParam->points.push_back(make_pair(X, Y));
		}
	}
	return false;
}

void CClusterKmeans::getAlgoriStr(CString & str)
{
	str = _T("K-means�㷨");
}

bool CClusterKmeans::postRunComplete()
{
	return true;
}

bool CClusterKmeans::ExcuteDrawTaskItem(CDC* pDC, CSize& szCanvas, stDoubleRange& rangeCanvas, DRAWTASKITEM & item)
{
	bool bNeedBaseDraw = false;
	int crSize = sizeof(crs) / sizeof(COLORREF);

	COLORREF measureCr = RGB(150, 0, 0);
	COLORREF centoidCr = RGB(0, 0, 150);
	/*
	DRAW_TASK_POINT,
	DRAW_TASK_SELECTED_POINT,
	DRAW_TASK_KSET,
	DRAW_TASK_SELECT_KSET,
	DRAW_TASK_MEASURE,
	DRAW_TASK_CENTOID,
	*/
	CSize drawSize;
	drawSize.cx = szCanvas.cx - RUN_PANNEL_CANVAS_PIXEL_MARGIN * 2;
	drawSize.cy = szCanvas.cy - RUN_PANNEL_CANVAS_PIXEL_MARGIN * 2;

	const int drawXStart = RUN_PANNEL_CANVAS_PIXEL_MARGIN;
	const int drawYStart = RUN_PANNEL_CANVAS_PIXEL_MARGIN;
	switch (item.type)
	{
	case DRAW_TASK_POINT:
	case DRAW_TASK_SELECTED_POINT:
	{
		const RAWPOINT_T& pt = m_RawPoints[item.wp];
		int X = drawXStart + (int)(drawSize.cx*(pt.first - rangeCanvas.xStart) / (rangeCanvas.xEnd - rangeCanvas.xStart));
		int Y = drawYStart + (int)(drawSize.cy*(pt.second - rangeCanvas.yStart) / (rangeCanvas.yEnd - rangeCanvas.yStart));

		COLORREF cr = crs[(m_K_Points_Set_Tags[item.wp] + 1) % crSize];
		DrawAPoint(pDC, X, Y, item.type - DRAW_TASK_POINT, cr);
	}
	break;
	case DRAW_TASK_KSET:
	case DRAW_TASK_SELECT_KSET:
	{
		const RAWPOINT_T& pt = m_K_Weight_Points[item.wp];
		int X = drawXStart + (int)(drawSize.cx*(pt.first - rangeCanvas.xStart) / (rangeCanvas.xEnd - rangeCanvas.xStart));
		int Y = drawYStart + (int)(drawSize.cy*(pt.second - rangeCanvas.yStart) / (rangeCanvas.yEnd - rangeCanvas.yStart));

		COLORREF cr = crs[(item.wp + 1) % crSize];
		DrawAKSet(pDC, X, Y, item.type - DRAW_TASK_KSET, cr);
	}
	break;
	case DRAW_TASK_MEASURE:
	{
		const RAWPOINT_T& pt = m_RawPoints[item.wp];
		int ptX = drawXStart + (int)(drawSize.cx*(pt.first - rangeCanvas.xStart) / (rangeCanvas.xEnd - rangeCanvas.xStart));
		int ptY = drawYStart + (int)(drawSize.cy*(pt.second - rangeCanvas.yStart) / (rangeCanvas.yEnd - rangeCanvas.yStart));

		const RAWPOINT_T& set = m_K_Weight_Points[(int)item.lp];
		int setX = drawXStart + (int)(drawSize.cx*(set.first - rangeCanvas.xStart) / (rangeCanvas.xEnd - rangeCanvas.xStart));
		int setY = drawYStart + (int)(drawSize.cy*(set.second - rangeCanvas.yStart) / (rangeCanvas.yEnd - rangeCanvas.yStart));

		DrawMeasureLine(pDC, ptX, ptY, setX, setY, measureCr);
	}
	break;
	case DRAW_TASK_CENTOID:
	{
		int ptX = drawXStart + 10;
		int ptY = drawYStart + 30 * item.wp;
		double *p = (double*)item.lp;
		DrawCentoidInfo(pDC, ptX, ptY, item.wp, *p, centoidCr);
		//delete p;
	}
	break;
	default:
		bNeedBaseDraw = true;
	}

	if (bNeedBaseDraw)
		return CClusterAlgorithmBase::ExcuteDrawTaskItem(pDC, szCanvas, rangeCanvas, item);
	else
		return true;
}

bool CClusterKmeans::DrawAlgorithmProgress(CDC * pDC, CRect & rc)
{
	CString str;

	if (getAlgorithmStatus() == ALGORITHM_STATUS_NONE)
	{
		str.Format(_T("�㷨״̬δ��ʼ��"));
		DrawProgress(pDC, str, 0, rc);
	}
	else
	{
		str.Format(_T("��%d�ε��� %d/%d"), iAlgorithmCount + 1, iIndex,m_RawPoints.size());
		DrawProgress(pDC, str, iIndex * 100 / m_RawPoints.size(),rc);
	}
	return true;
}

double CClusterKmeans::MeasureDistance(RAWPOINT_T & pt1, RAWPOINT_T & pt2)
{
	double ret = (pt1.first - pt2.first) * (pt1.first - pt2.first)
		+ (pt1.second - pt2.second) * (pt1.second - pt2.second);
	return ret;
}

void CClusterKmeans::SelectPointIntoCSet(RAWPOINT_T& pt1, int KIndex)
{
}

void CClusterKmeans::RecaculateCentoid(int k)
{
	double X, Y;
	X = Y = 0;

	int count = m_RawPoints.size();
	int found = 0;
	for (int i = 0;i < count;i++)
	{
		if (m_K_Points_Set_Tags[i] == k)
		{
			X += m_RawPoints[i].first;
			Y += m_RawPoints[i].second;
			found++;
		}
	}

	if (found != 0)
	{
		m_K_Weight_Points[k].first = X / found;
		m_K_Weight_Points[k].second = Y / found;
	}
}

CClusterDbscan::CClusterDbscan(const RAWPOINTS_T & rawPoints, CWnd * pWnd) :
	CClusterAlgorithmBase(rawPoints, pWnd)
{
	setAlgorithmStatus(ALGORITHM_STATUS_NONE);

	//��ʼ���ɴ����
	m_listReach.clear();
	for (int i = 0;i < m_RawPoints.size();i++)
	{
		REACH_LIST_T reach_list;
		m_listReach.push_back(make_pair(-1, reach_list));
	}
	//��ʼ�����༯��
	m_listClusters.clear();

	//CaculateCanvasSize(m_rangeCanvas, rawPoints);
	//FloorRealRange(m_rangeCanvas);
}

CClusterDbscan::~CClusterDbscan()
{
}

bool CClusterDbscan::init(void * pParam)
{
	DBSCAN_PARAM *pDbscanParam = (DBSCAN_PARAM*)pParam;
	ASSERT(pDbscanParam);

	m_paramE = pDbscanParam->E;
	m_paramMinPts = pDbscanParam->MinPts;

	setAlgorithmStatus(ALGORITHM_STATUS_INIT);
	//������˻������еĻ�ͼ����
	ClearDrawTask();

	//��ʼ���ɴ����
	m_listReach.clear();
	for (int i = 0;i < m_RawPoints.size();i++)
	{
		REACH_LIST_T reach_list;
		m_listReach.push_back(make_pair(-1, reach_list));
	}
	//��ʼ�����༯��
	m_listClusters.clear();
	
	CDialogRun *pDlg = (CDialogRun *)m_pWndParent;

	m_pGridCtrl_ClusterCount->RemoveAll();
	m_pGridCtrl_Odd->RemoveAll();

	pDlg->m_tabCtrl.SetActiveTab(2);

	//for (int i = 0;i<m_Grids.size();i++)
	//{
	//	pDlg->m_tabCtrl.RemoveTab(i + 3);
	//	delete m_Grids[i];
	//}
	for (int i = m_Grids.size();i>0;i--)
	{
		pDlg->m_tabCtrl.RemoveTab(i + 3 - 1);
		delete m_Grids[i-1];
	}
	m_Grids.clear();

	return true;
}

void CClusterDbscan::runStatusMachine()
{
	if (end())
	{
		ASSERT(FALSE);
		return;
	}
	//ASSERT(m_CallStack.size() != 0);
	//ASSERT(m_CallStack.size() == deep());
	switch (getAlgorithmStatus())
	{
	case ALGORITHM_STATUS_INIT:
		//������㷨����;����...�ȵ�
		//�ƽ�״̬
		setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_INIT_MESURE);
		break;
	case ALGORITHM_STATUS_DBSCAN_INIT_MESURE:

		m_iIndex = 0;
		m_iEndIndex = 1;
		//�ƽ�״̬
		setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_SELECT_A_START_POINT);
		break;
	case ALGORITHM_STATUS_DBSCAN_SELECT_A_START_POINT:
	{
		if (m_iIndex == m_RawPoints.size())
		{
			setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_SELECT_ALL_START_POINT);
			break;
		}

		PushDrawTask(DRAW_TASK_SELECTED_POINT, m_iIndex, 0);

		m_iEndIndex = m_iIndex + 1;
		setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_SELECT_A_END_POINT);

		break;
	}
	case ALGORITHM_STATUS_DBSCAN_SELECT_A_END_POINT:
	{
		if (m_iEndIndex == m_RawPoints.size())
		{
			setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_SELECT_ALL_END_POINT);
			break;
		}
		PushDrawTask(DRAW_TASK_SELECTED_POINT, m_iEndIndex, 0);
		setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_BEGIN_MEASURE);

		break;
	}
	case ALGORITHM_STATUS_DBSCAN_BEGIN_MEASURE:
	{
		//����,���¿ɴ�����
		double dbDistance = sqrt((m_RawPoints[m_iIndex].first - m_RawPoints[m_iEndIndex].first)*
			(m_RawPoints[m_iIndex].first - m_RawPoints[m_iEndIndex].first)
			+ (m_RawPoints[m_iIndex].second - m_RawPoints[m_iEndIndex].second)*
			(m_RawPoints[m_iIndex].second - m_RawPoints[m_iEndIndex].second)
		);
		if (dbDistance < m_paramE)
		{
			m_listReach[m_iIndex].second.push_back(make_pair(m_iEndIndex, dbDistance));
			m_listReach[m_iEndIndex].second.push_back(make_pair(m_iIndex, dbDistance));
		}

		PushDrawTask(DRAW_TASK_MEASURE, m_iIndex, m_iEndIndex);
		setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_END_MEASURE);
		break;
	}
	case ALGORITHM_STATUS_DBSCAN_END_MEASURE:
	{
		PopDrawTask(2);
		m_iEndIndex++;
		setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_SELECT_A_END_POINT);

		break;
	}
	case ALGORITHM_STATUS_DBSCAN_SELECT_ALL_END_POINT:
	{
		PopDrawTask();
		m_iIndex++;
		setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_SELECT_A_START_POINT);
		break;
	}
	case ALGORITHM_STATUS_DBSCAN_SELECT_ALL_START_POINT:
	{
		setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_INIT_CLUSTER);

		break;
	}
	case ALGORITHM_STATUS_DBSCAN_INIT_CLUSTER:
	{
		m_iCoreIndex = 0;
		//�����еĺ��ĵ㻭����
		int iIndex = 0;
		BOOST_FOREACH(const POINT_REACH_ITEM_T &var, m_listReach)
		{			
			//����Ǻ��ĵ�
			if (var.second.size() >= m_paramMinPts)
			{
				PushDrawTask(DRAW_TASK_CORE, iIndex, 0);
			}
			iIndex++;
		}

		//m_listClusters.clear();
		setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_SELECT_A_CORE_POINT);

		break;
	}
	case ALGORITHM_STATUS_DBSCAN_SELECT_A_CORE_POINT:
	{
		//ѡ��һ��core point
		bool bFound = false;
		int iCoreIndex;
		for (int i = 0;i < m_RawPoints.size();i++)
		{
			//���û���ʹ�,���Ǻ��Ķ���(�ɴ�㳬����ֵ)
			if (m_listReach[i].first == -1 && m_listReach[i].second.size() >= m_paramMinPts)
			{
				bFound = true;
				iCoreIndex = i;
				break;
			}
		}

		//û���˺��ĵ�,�㷨����
		if (!bFound)
		{
			m_iCoreIndex = m_RawPoints.size();
			setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_SELECT_ALL_CORE_POINT);
			break;
		}
		m_iCoreIndex = iCoreIndex;

		PushDrawTask(DRAW_TASK_SELECTED_POINT, iCoreIndex, 0);

		//����һ������
		int iIndexCluster = m_listClusters.size();
		CLUSTER_ITEM_T newCluster;
		newCluster.push_back(iCoreIndex);
		m_listClusters.push_back(newCluster);


		//����һ��grid--��ʱ�������˻������
		//CString str;
		//str.Format(_T("�����%d"), m_listClusters.size());
		//AddResultSet(str);
		////���
		//CBCGPGridRow* pRow = m_Grids[iIndexCluster]->CreateRow(2);
		//pRow->GetItem(0)->SetValue(m_RawPoints[iCoreIndex].first);
		//pRow->GetItem(1)->SetValue(m_RawPoints[iCoreIndex].second);
		//m_Grids[iIndexCluster]->AddRow(pRow);

		//�޸Ļ�����ʾ�Ķ�Ӧ��--����

		CBCGPGridRow* pRow = m_pGridCtrl_ClusterCount->CreateRow(2);

		pRow->GetItem(1)->SetValue(1);

		int crSize = sizeof(crs) / sizeof(COLORREF);

#ifndef _BCGSUITE_INC_
		CBCGPGridColorItem* pColorItem = new CBCGPGridColorItem(crs[iIndexCluster+1]);
		ASSERT_VALID(pColorItem);

		pColorItem->EnableOtherButton(_T("Other"));
#else
		CGridColorItem* pColorItem = new CGridColorItem(color);
#endif

		pRow->ReplaceItem(0, pColorItem);

		m_pGridCtrl_ClusterCount->AddRow(pRow);


		//���÷��ʱ�־
		m_listReach[iCoreIndex].first = iIndexCluster;

		//��ѡ����corepoint�Ŀɴ�������
		m_ReachQueue.clear();
		BOOST_FOREACH(const REACH_ITEM_T &var, m_listReach[iCoreIndex].second)
		{
			//������Ŀɴ��δ�����ʹ���Ҳ����û�б���������۹�
			if (m_listReach[var.first].first == -1)
			{
				m_ReachQueue.push_back(make_pair(iCoreIndex, var.first));
				//m_listReach[var.first].first = iIndexCluster;
			}
		}

		setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_SELECT_A_REACH_POINT);

		break;
	}
	case ALGORITHM_STATUS_DBSCAN_SELECT_A_REACH_POINT:
	{
		//�Ӷ���ͷѡ���һ��δ���ʵĵ�,���������ڶ���ͷ�������Ķ�pop��ȥ
		pair<int, int> iPoint;
		while (!m_ReachQueue.empty())
		{
			iPoint = m_ReachQueue.front();

			if (m_listReach[iPoint.second].first == -1)
				break;
			else
				m_ReachQueue.pop_front();
		}


		//�ɴ���д����꣬
		if (m_ReachQueue.empty())
		{
			setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_SELECT_ALL_REACHE_POINT);
			break;
		}
		PushDrawTask(DRAW_TASK_SELECTED_POINT, iPoint.second, 0);
		setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_BEGIN_CLUSTER);
		break;
	}
	case ALGORITHM_STATUS_DBSCAN_BEGIN_CLUSTER:
	{
		pair<int, int> iPoint = m_ReachQueue.front();

		//�����ǰ�ɴ����Ǻ��ĵ�,
		//if (m_listReach[iPoint.second].second.size() >= m_paramMinPts)
		//{
		//	PushDrawTask(DRAW_TASK_CORE_PATH, iPoint.first, iPoint.second);
		//}

		//���÷��ʱ�־
		m_listReach[iPoint.second].first = m_listClusters.size() - 1;

		//���ӵ������--��ʱ�������ˣ��ڹ����߳��л������
		//CBCGPGridRow* pRow = m_Grids[m_listClusters.size() - 1]->CreateRow(2);
		//pRow->GetItem(0)->SetValue(m_RawPoints[iPoint.second].first);
		//pRow->GetItem(1)->SetValue(m_RawPoints[iPoint.second].second);
		//m_Grids[m_listClusters.size() - 1]->AddRow(pRow);
		//�޸Ļ�����ʾ

		CBCGPGridRow* pRow = m_pGridCtrl_ClusterCount->GetRow(m_listClusters.size() - 1);
		pRow->GetItem(1)->SetValue((pRow->GetItem(1)->GetValue().intVal + 1));

		//���������
		m_listClusters.back().push_back(iPoint.second);
		setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_END_CLUSTER);
		break;
	}
	case ALGORITHM_STATUS_DBSCAN_END_CLUSTER:
	{
		pair<int, int> iPoint = m_ReachQueue.front();

		PopDrawTask();
		//�����ǰ�ɴ����Ǻ��ĵ�,
		if (m_listReach[iPoint.second].second.size() >= m_paramMinPts)
		{
			//���Ϻ��ĵ㵽��·��
			PopDrawTask();
			PushDrawTask(DRAW_TASK_CORE_PATH, iPoint.first, iPoint.second);
			//Ϊ�����
			PushDrawTask(DRAW_TASK_NONE, 0, 0);

			BOOST_FOREACH(const REACH_ITEM_T &var, m_listReach[iPoint.second].second)
			{
				//������Ŀɴ��δ�����ʹ���Ҳ����û�б���������۹�
				if (m_listReach[var.first].first == -1)
				{
					m_ReachQueue.push_back(make_pair(iPoint.second, var.first));
					//m_listReach[var.first].first = true;
				}
			}
		}
		m_ReachQueue.pop_front();

		setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_SELECT_A_REACH_POINT);
		break;
	}
	case ALGORITHM_STATUS_DBSCAN_SELECT_ALL_REACHE_POINT:
	{
		//�ɴ���д����꣬ѡ����һ��corepoint
		PopDrawTask();

		//��ʾһ��������

		setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_SELECT_A_CORE_POINT);
		break;
	}
	case ALGORITHM_STATUS_DBSCAN_SELECT_ALL_CORE_POINT:
	{
		//�㷨����
		//���������뼯��
		int iIndex = 0;
		int count = 0;
		BOOST_FOREACH(const POINT_REACH_ITEM_T &var, m_listReach)
		{
			//����Ǻ��ĵ�
			if (var.first == -1)
			{
				CBCGPGridRow* pRow = m_pGridCtrl_Odd->CreateRow(2);

				pRow->GetItem(0)->SetValue(m_RawPoints[iIndex].first);
				pRow->GetItem(1)->SetValue(m_RawPoints[iIndex].second);

				m_pGridCtrl_Odd->AddRow(pRow);

				count++;
			}
			iIndex++;
		}

		CBCGPGridRow* pRow = m_pGridCtrl_ClusterCount->CreateRow(2);

		pRow->GetItem(1)->SetValue(count);

		int crSize = sizeof(crs) / sizeof(COLORREF);

#ifndef _BCGSUITE_INC_
		CBCGPGridColorItem* pColorItem = new CBCGPGridColorItem(crs[0]);
		ASSERT_VALID(pColorItem);

		pColorItem->EnableOtherButton(_T("Other"));
#else
		CGridColorItem* pColorItem = new CGridColorItem(color);
#endif

		pRow->ReplaceItem(0, pColorItem);

		m_pGridCtrl_ClusterCount->AddRow(pRow);

		setAlgorithmStatus(ALGORITHM_STATUS_COMPLETE);
		break;
	}
	default:
		break;
	}

	return;
}

bool CClusterDbscan::initProp()
{
	CDialogRun *pDlg = (CDialogRun *)m_pWndParent;

	CBCGPProp* m_pGroupGeneral = new CBCGPProp(_T("DBSCAN"));


	m_pPropE = new CBCGPProp(_T("����E"), 2, (_variant_t)5.0, _T("�趨DBSCAN�㷨����E"));
	m_pGroupGeneral->AddSubItem(m_pPropE);

	m_pPropMinPts = new CBCGPProp(_T("����MinPts"), 1, (_variant_t)5, _T("�趨DBSCAN�㷨����MinPts"));
	m_pPropMinPts->EnableSpinControl(TRUE, 3, 100);
	m_pGroupGeneral->AddSubItem(m_pPropMinPts);

	pDlg->m_wndPropList.AddProperty(m_pGroupGeneral);


	m_pPropMeasure = new CBCGPProp(_T("��ʾ��������"), 3, (_variant_t)true, _T("DBSCAN�㷨��һ����Ҫ�����������,��������ĵ�"));
	pDlg->m_pPropGroup->AddSubItem(m_pPropMeasure);

	m_pPropCore = new CBCGPProp(_T("���ĵ���ʾ��ʽ"),4, _T("E-Բ"), _T("1.����ʾ\n2.��ʾ���ĵ��ܶȿɴﷶΧ\n3.����СԲ��ʶ���ĵ�"));
	m_pPropCore->AddOption(_T("����ʾ"));
	m_pPropCore->AddOption(_T("E-Բ"));
	m_pPropCore->AddOption(_T("СԲ"));
	m_pPropCore->AllowEdit(FALSE);
	pDlg->m_pPropGroup->AddSubItem(m_pPropCore);
		
	m_pPropCorePath = new CBCGPProp(_T("��ʾ���ĵ�·��"), 5, (_variant_t)true, _T("��ʾ���ĵ�·��"));
	pDlg->m_pPropGroup->AddSubItem(m_pPropCorePath);
	m_pPropProgress = new CBCGPProp(_T("��ʾ������"), 6, (_variant_t)true, _T("�㷨���й�������ʾִ�н���"));
	pDlg->m_pPropGroup->AddSubItem(m_pPropProgress);

	return true;
}

bool CClusterDbscan::initGrid()
{
	//AddResultSet(_T("ԭʼ����"));

	CDialogRun *pDlg = (CDialogRun *)m_pWndParent;

	//ԭʼ����
	m_pGridCtrl_Orgi = new CBCGPGridCtrl();
	//pGridCtrl->Create(WS_CHILD, rc, this, 1);

	m_pGridCtrl_Orgi->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, CRect(0, 0, 100, 100), &pDlg->m_tabCtrl, IDC_STATIC_GRIDLOCATION);
	m_pGridCtrl_Orgi->EnableMarkSortedColumn(FALSE);
	m_pGridCtrl_Orgi->EnableHeader(TRUE, BCGP_GRID_HEADER_MOVE_ITEMS);
	m_pGridCtrl_Orgi->EnableRowHeader(TRUE);
	m_pGridCtrl_Orgi->EnableLineNumbers();
	m_pGridCtrl_Orgi->SetClearInplaceEditOnEnter(FALSE);
	m_pGridCtrl_Orgi->EnableInvertSelOnCtrl();
	m_pGridCtrl_Orgi->SetScalingRange(0.1, 4.0);
	m_pGridCtrl_Orgi->InsertColumn(0, _T("X"), 100);
	m_pGridCtrl_Orgi->InsertColumn(1, _T("Y"), 100);
	m_pGridCtrl_Orgi->SetReadOnly();


	pDlg->m_tabCtrl.AddTab(m_pGridCtrl_Orgi, _T("ԭʼ����"));

	//����㼯��,��ʼ���е�
	m_pGridCtrl_Odd = new CBCGPGridCtrl();
	//pGridCtrl->Create(WS_CHILD, rc, this, 1);

	m_pGridCtrl_Odd->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, CRect(0, 0, 100, 100), &pDlg->m_tabCtrl, IDC_STATIC_GRIDLOCATION);
	m_pGridCtrl_Odd->EnableMarkSortedColumn(FALSE);
	m_pGridCtrl_Odd->EnableHeader(TRUE, BCGP_GRID_HEADER_MOVE_ITEMS);
	m_pGridCtrl_Odd->EnableRowHeader(TRUE);
	m_pGridCtrl_Odd->EnableLineNumbers();
	m_pGridCtrl_Odd->SetClearInplaceEditOnEnter(FALSE);
	m_pGridCtrl_Odd->EnableInvertSelOnCtrl();
	m_pGridCtrl_Odd->SetScalingRange(0.1, 4.0);
	m_pGridCtrl_Odd->InsertColumn(0, _T("X"), 100);
	m_pGridCtrl_Odd->InsertColumn(1, _T("Y"), 100);
	m_pGridCtrl_Odd->SetReadOnly();

	pDlg->m_tabCtrl.AddTab(m_pGridCtrl_Odd, _T("�����"));

	//�����е�����ʼ���Ϻ�����㼯��
	RAWPOINTS_T::const_iterator it;
	for (it = m_RawPoints.begin();it != m_RawPoints.end(); it++)
	{
		CBCGPGridRow* pRow = m_pGridCtrl_Orgi->CreateRow(2);

		pRow->GetItem(0)->SetValue((double)(*it).first);
		pRow->GetItem(1)->SetValue((double)(*it).second);

		m_pGridCtrl_Orgi->AddRow(pRow);

		//pRow = m_pGridCtrl_Odd->CreateRow(2);

		//pRow->GetItem(0)->SetValue((double)(*it).first);
		//pRow->GetItem(1)->SetValue((double)(*it).second);

		//m_pGridCtrl_Odd->AddRow(pRow);
	}

	//���༯����ʾ

	m_pGridCtrl_ClusterCount = new CBCGPGridCtrl();
	//pGridCtrl->Create(WS_CHILD, rc, this, 1);

	m_pGridCtrl_ClusterCount->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, CRect(0, 0, 100, 100), &pDlg->m_tabCtrl, IDC_STATIC_GRIDLOCATION);
	m_pGridCtrl_ClusterCount->EnableMarkSortedColumn(FALSE);
	m_pGridCtrl_ClusterCount->EnableHeader(TRUE, BCGP_GRID_HEADER_MOVE_ITEMS);
	m_pGridCtrl_ClusterCount->EnableRowHeader(TRUE);
	m_pGridCtrl_ClusterCount->EnableLineNumbers();
	m_pGridCtrl_ClusterCount->SetClearInplaceEditOnEnter(FALSE);
	m_pGridCtrl_ClusterCount->EnableInvertSelOnCtrl();
	m_pGridCtrl_ClusterCount->SetScalingRange(0.1, 4.0);
	m_pGridCtrl_ClusterCount->InsertColumn(0, _T("��ɫ"), 100);
	m_pGridCtrl_ClusterCount->InsertColumn(1, _T("����"), 100);
	m_pGridCtrl_ClusterCount->SetReadOnly();

	pDlg->m_tabCtrl.AddTab(m_pGridCtrl_ClusterCount, _T("������"));


	//

	return true;
}

void CClusterDbscan::propChanged(UINT nID, CBCGPProp * pProp)
{
	CDialogRun *pDlg = (CDialogRun *)m_pWndParent;
	nID = pProp->GetID();

	if (nID == 3)
	{
		bool bShowMeasure = m_pPropMeasure->GetValue().boolVal;

		/*
		
	ALGORITHM_STATUS_DBSCAN_INIT_MESURE,
	ALGORITHM_STATUS_DBSCAN_SELECT_A_START_POINT,
	ALGORITHM_STATUS_DBSCAN_SELECT_A_END_POINT,
	ALGORITHM_STATUS_DBSCAN_BEGIN_MEASURE,
	ALGORITHM_STATUS_DBSCAN_END_MEASURE,
	ALGORITHM_STATUS_DBSCAN_SELECT_ALL_END_POINT,
	ALGORITHM_STATUS_DBSCAN_SELECT_ALL_START_POINT,

	ALGORITHM_STATUS_DBSCAN_INIT_CLUSTER,
	ALGORITHM_STATUS_DBSCAN_SELECT_A_CORE_POINT,
	ALGORITHM_STATUS_DBSCAN_SELECT_A_REACH_POINT,
	ALGORITHM_STATUS_DBSCAN_BEGIN_CLUSTER,
	ALGORITHM_STATUS_DBSCAN_END_CLUSTER,
	ALGORITHM_STATUS_DBSCAN_SELECT_ALL_REACHE_POINT,
	ALGORITHM_STATUS_DBSCAN_SELECT_ALL_CORE_POINT,
	*/
		SetSkipStatus(ALGORITHM_STATUS_DBSCAN_INIT_MESURE, !bShowMeasure);
		SetSkipStatus(ALGORITHM_STATUS_DBSCAN_SELECT_A_START_POINT, !bShowMeasure);
		SetSkipStatus(ALGORITHM_STATUS_DBSCAN_SELECT_A_END_POINT, !bShowMeasure);
		SetSkipStatus(ALGORITHM_STATUS_DBSCAN_BEGIN_MEASURE, !bShowMeasure);
		SetSkipStatus(ALGORITHM_STATUS_DBSCAN_END_MEASURE, !bShowMeasure);
		SetSkipStatus(ALGORITHM_STATUS_DBSCAN_SELECT_ALL_END_POINT, !bShowMeasure);
		SetSkipStatus(ALGORITHM_STATUS_DBSCAN_SELECT_ALL_START_POINT, !bShowMeasure);
		//SetSkipStatus(ALGORITHM_STATUS_KMEANS_ENDMEASURE_ALL, !bShowMeasure);
		//SetDoDrawFlag(DRAW_TASK_SELECTED_POINT, bShowMeasure);
		SetDoDrawFlag(DRAW_TASK_SELECTED_POINT, bShowMeasure);
		SetDoDrawFlag(DRAW_TASK_MEASURE, bShowMeasure);
	}

	if (nID == 4)
	{
		int bCoreType = m_pPropCore->GetSelectedOption();

		SetDoDrawFlag(DRAW_TASK_CORE, bCoreType);
		Redraw();
	}
	if (nID == 5)
	{
		bool bShowCorePath = m_pPropCorePath->GetValue().boolVal;

		SetDoDrawFlag(DRAW_TASK_CORE_PATH, bShowCorePath);
		Redraw();
	}

	if (nID == 6)
	{
		bool bShowProgress = m_pPropProgress->GetValue().boolVal;

		SetDoDrawFlag(DRAW_TASK_PROGRESS, bShowProgress);

		Redraw();
	}
}

bool CClusterDbscan::getPropValue(void * pParam)
{
	CClusterDbscan::DBSCAN_PARAM *pKmeansParam = (CClusterDbscan::DBSCAN_PARAM *)pParam;
	pKmeansParam->E = m_pPropE->GetValue().dblVal;
	pKmeansParam->MinPts = m_pPropMinPts->GetValue().intVal;
	
	return true;
}

void CClusterDbscan::getAlgoriStr(CString & str)
{
	str = _T("DBSCAN�㷨");
}

bool CClusterDbscan::postRunComplete()
{
	for (int i = 0;i < m_listClusters.size();i++)
	{
		CString str;
		str.Format(_T("�����%d"), i+1);
		AddResultSet(str);
		//���
		BOOST_FOREACH(int &var, m_listClusters[i])
		{
			CBCGPGridRow* pRow = m_Grids[i]->CreateRow(2);
			pRow->GetItem(0)->SetValue(m_RawPoints[var].first);
			pRow->GetItem(1)->SetValue(m_RawPoints[var].second);
			m_Grids[i]->AddRow(pRow);
		}
	}
	return true;
}

bool CClusterDbscan::ExcuteDrawTaskItem(CDC * pDC, CSize & szCanvas, stDoubleRange & rangeCanvas, DRAWTASKITEM & item)
{
	bool bNeedBaseDraw = false;
	int crSize = sizeof(crs) / sizeof(COLORREF);

	COLORREF measureCr = RGB(150, 0, 0);
	COLORREF centoidCr = RGB(0, 0, 150);
	/*
	DRAW_TASK_POINT,
	DRAW_TASK_SELECTED_POINT,
	DRAW_TASK_KSET,
	DRAW_TASK_SELECT_KSET,
	DRAW_TASK_MEASURE,
	DRAW_TASK_CENTOID,
	*/
	CSize drawSize;
	drawSize.cx = szCanvas.cx - RUN_PANNEL_CANVAS_PIXEL_MARGIN * 2;
	drawSize.cy = szCanvas.cy - RUN_PANNEL_CANVAS_PIXEL_MARGIN * 2;

	const int drawXStart = RUN_PANNEL_CANVAS_PIXEL_MARGIN;
	const int drawYStart = RUN_PANNEL_CANVAS_PIXEL_MARGIN;
	switch (item.type)
	{
	case DRAW_TASK_POINT:
	case DRAW_TASK_SELECTED_POINT:
	{
		const RAWPOINT_T& pt = m_RawPoints[item.wp];
		int X = drawXStart + (int)(drawSize.cx*(pt.first - rangeCanvas.xStart) / (rangeCanvas.xEnd - rangeCanvas.xStart));
		int Y = drawYStart + (int)(drawSize.cy*(pt.second - rangeCanvas.yStart) / (rangeCanvas.yEnd - rangeCanvas.yStart));

		COLORREF cr = crs[(m_listReach[item.wp].first + 1) % crSize];
		bool bIsCore = m_listReach[item.wp].second.size() >= m_paramMinPts;
		DrawAPoint(pDC, X, Y, item.type - DRAW_TASK_POINT, cr);
		//if(bIsCore)
		//	DrawAPoint(pDC, X, Y, 2, cr);
		//else
		//	DrawAPoint(pDC, X, Y, item.type - DRAW_TASK_POINT, cr);

		break;
	}
	case DRAW_TASK_CORE:
	{
		const RAWPOINT_T& pt = m_RawPoints[item.wp];
		int X = drawXStart + (int)(drawSize.cx*(pt.first - rangeCanvas.xStart) / (rangeCanvas.xEnd - rangeCanvas.xStart));
		int Y = drawYStart + (int)(drawSize.cy*(pt.second - rangeCanvas.yStart) / (rangeCanvas.yEnd - rangeCanvas.yStart));

		int W, H;
		int type = GetDoDrawFlag(DRAW_TASK_CORE);
		if (type == 1)
		{
			W = (int)(m_paramE * drawSize.cx / (rangeCanvas.xEnd - rangeCanvas.xStart));
			H = (int)(m_paramE * drawSize.cy / (rangeCanvas.yEnd - rangeCanvas.yStart));
		}
		else
		{
			W = 5;
			H = 5;
		}
		COLORREF cr = crs[(m_listReach[item.wp].first + 1) % crSize];
		DrawCore(pDC, X, Y, W, H, cr);
		break;
	}
	case DRAW_TASK_CORE_PATH:
	{
		const RAWPOINT_T& pt = m_RawPoints[item.wp];
		int ptX = drawXStart + (int)(drawSize.cx*(pt.first - rangeCanvas.xStart) / (rangeCanvas.xEnd - rangeCanvas.xStart));
		int ptY = drawYStart + (int)(drawSize.cy*(pt.second - rangeCanvas.yStart) / (rangeCanvas.yEnd - rangeCanvas.yStart));

		const RAWPOINT_T& set = m_RawPoints[(int)item.lp];
		int setX = drawXStart + (int)(drawSize.cx*(set.first - rangeCanvas.xStart) / (rangeCanvas.xEnd - rangeCanvas.xStart));
		int setY = drawYStart + (int)(drawSize.cy*(set.second - rangeCanvas.yStart) / (rangeCanvas.yEnd - rangeCanvas.yStart));

		COLORREF cr = crs[(m_listReach[item.wp].first + 1) % crSize];
		DrawCorePath(pDC, ptX, ptY, setX, setY, cr);
		break;
	}
	case DRAW_TASK_MEASURE:
	{
		const RAWPOINT_T& pt = m_RawPoints[item.wp];
		int ptX = drawXStart + (int)(drawSize.cx*(pt.first - rangeCanvas.xStart) / (rangeCanvas.xEnd - rangeCanvas.xStart));
		int ptY = drawYStart + (int)(drawSize.cy*(pt.second - rangeCanvas.yStart) / (rangeCanvas.yEnd - rangeCanvas.yStart));

		const RAWPOINT_T& set = m_RawPoints[(int)item.lp];
		int setX = drawXStart + (int)(drawSize.cx*(set.first - rangeCanvas.xStart) / (rangeCanvas.xEnd - rangeCanvas.xStart));
		int setY = drawYStart + (int)(drawSize.cy*(set.second - rangeCanvas.yStart) / (rangeCanvas.yEnd - rangeCanvas.yStart));

		DrawMeasureLine(pDC, ptX, ptY, setX, setY, measureCr);
	}
	default:
		bNeedBaseDraw = true;
	}

	if (bNeedBaseDraw)
		return CClusterAlgorithmBase::ExcuteDrawTaskItem(pDC, szCanvas, rangeCanvas, item);
	else
		return true;
}

bool CClusterDbscan::DrawAlgorithmProgress(CDC * pDC, CRect & rc)
{
	CString str;

	/*
	ALGORITHM_STATUS_NONE
	ALGORITHM_STATUS_INIT
	ALGORITHM_STATUS_COMPLETE,
	ALGORITHM_STATUS_COMPLETE_WITH_ERROR,

	ALGORITHM_STATUS_DBSCAN_INIT_MESURE,
	ALGORITHM_STATUS_DBSCAN_SELECT_A_START_POINT,
	ALGORITHM_STATUS_DBSCAN_SELECT_A_END_POINT,
	ALGORITHM_STATUS_DBSCAN_BEGIN_MEASURE,
	ALGORITHM_STATUS_DBSCAN_END_MEASURE,
	ALGORITHM_STATUS_DBSCAN_SELECT_ALL_END_POINT,
	ALGORITHM_STATUS_DBSCAN_SELECT_ALL_START_POINT,
	ALGORITHM_STATUS_DBSCAN_INIT_CLUSTER,
	ALGORITHM_STATUS_DBSCAN_SELECT_A_CORE_POINT,
	ALGORITHM_STATUS_DBSCAN_SELECT_A_REACH_POINT,
	ALGORITHM_STATUS_DBSCAN_BEGIN_CLUSTER,
	ALGORITHM_STATUS_DBSCAN_END_CLUSTER,
	ALGORITHM_STATUS_DBSCAN_SELECT_ALL_REACHE_POINT,
	ALGORITHM_STATUS_DBSCAN_SELECT_ALL_CORE_POINT,
	*/
	switch (getAlgorithmStatus())
	{
	case ALGORITHM_STATUS_NONE:
	{
		str.Format(_T("�㷨״̬δ��ʼ��"));
		DrawProgress(pDC, str, 0, rc);
		break;
	}
	case ALGORITHM_STATUS_INIT:
	{
		str.Format(_T("�㷨�ѳ�ʼ��"));
		DrawProgress(pDC, str,10, rc);
		break;
	}
	case ALGORITHM_STATUS_COMPLETE:
	{
		str.Format(_T("�㷨���н���"));
		DrawProgress(pDC, str, 100, rc);
		break;
	}
	case ALGORITHM_STATUS_DBSCAN_INIT_MESURE:
	case ALGORITHM_STATUS_DBSCAN_SELECT_A_START_POINT:
	case ALGORITHM_STATUS_DBSCAN_SELECT_A_END_POINT:
	case ALGORITHM_STATUS_DBSCAN_BEGIN_MEASURE:
	case ALGORITHM_STATUS_DBSCAN_END_MEASURE:
	case ALGORITHM_STATUS_DBSCAN_SELECT_ALL_END_POINT:
	case ALGORITHM_STATUS_DBSCAN_SELECT_ALL_START_POINT:
	{
		int count = m_RawPoints.size();
		int total = count*(count - 1) / 2;
		int cur = m_iIndex * (count - 1 + count - 1 - m_iIndex) / 2 + m_iEndIndex - m_iIndex;
		str.Format(_T("���������� %d/%d"),cur,total);
		DrawProgress(pDC, str, cur*100/total, rc);
		break;
	}
	case ALGORITHM_STATUS_DBSCAN_INIT_CLUSTER:
	case ALGORITHM_STATUS_DBSCAN_SELECT_A_CORE_POINT:
	case ALGORITHM_STATUS_DBSCAN_SELECT_A_REACH_POINT:
	case ALGORITHM_STATUS_DBSCAN_BEGIN_CLUSTER:
	case ALGORITHM_STATUS_DBSCAN_END_CLUSTER:
	case ALGORITHM_STATUS_DBSCAN_SELECT_ALL_REACHE_POINT:
	case ALGORITHM_STATUS_DBSCAN_SELECT_ALL_CORE_POINT:
	{
		int count = m_RawPoints.size();
		str.Format(_T("�������� %d/%d"), m_iCoreIndex, count);
		DrawProgress(pDC, str, m_iCoreIndex*100/count, rc);
		break;
	}
	default:
	{
		str.Format(_T("DBSCAN�����㷨"));
		DrawProgress(pDC, str,10, rc);
		break;
	}		
	}
	return true;
}
