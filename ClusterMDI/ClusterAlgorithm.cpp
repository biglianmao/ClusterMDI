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

	//绘制坐标系
	DrawCoordinate(pDC, coordinateRect);

	//执行绘画任务
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

	//1.填充背景
	CBrush brushBackground; // Must initialize!
	brushBackground.CreateSolidBrush(RUN_BACKGROUND);
	pDC->FillRect(&clientRect, &brushBackground);

	//2.画坐标系
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

	//3.写坐标
	//3.1 创建两种字体
	CFont vFont;
	CFont hFont;
	vFont.CreateFont(RUN_PANNEL_TEXT_SIZE, 0, 900, 900, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, VARIABLE_PITCH | FF_ROMAN, _T("Times New Roman"));
	hFont.CreateFont(RUN_PANNEL_TEXT_SIZE, 0, 000, 000, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, VARIABLE_PITCH | FF_ROMAN, _T("Times New Roman"));

	//3.2 写横坐标

	//3.2.1 准备坐标
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

	//3.2.2 开始写 
	iRulerLength = clientRect.Width() - 2 * RUN_PANNEL_CANVAS_PIXEL_MARGIN;
	iRulerStartMargin = RUN_PANNEL_CANVAS_PIXEL_MARGIN;
	iRulerSideMargin = RUN_PANNEL_CANVAS_PIXEL_MARGIN - 2;



	//画中间部分
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
	//漏了个0，补上
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


	//3.3 纵坐标
	pDC->SelectObject(&vFont);
	//pDC->SetTextAlign(TA_CENTER | TA_TOP);
	//pDC->SetBkMode(TRANSPARENT);

	//3.2.2 开始写 
	iRulerLength = clientRect.Height() - 2 * RUN_PANNEL_CANVAS_PIXEL_MARGIN;
	iRulerStartMargin = RUN_PANNEL_CANVAS_PIXEL_MARGIN;
	iRulerSideMargin = RUN_PANNEL_CANVAS_PIXEL_MARGIN - 2;

	//画中间部分
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
	//漏了个0，补上
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

	//通知窗口重画同一调整到线程中执行
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
	//m_tabCtrl.InsertItem(0, _T("结果集"));
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
	//设置所有的点不属于任何集合
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

	//设置所有的点不属于任何集合
	m_K_Points_Set_Tags.clear();
	for (int i = 0;i < m_RawPoints.size();i++)
		m_K_Points_Set_Tags.push_back(-1);

	setAlgorithmStatus(ALGORITHM_STATUS_INIT);

	//清除出了基类所有的绘图任务
	ClearDrawTask();
	//将质心推入绘图任务
	for (int i = 0;i < m_paramK;i++)
	{
		PushDrawTask(DRAW_TASK_KSET, i, 0);
	}

	//修改结果集grid
	CDialogRun *pDlg = (CDialogRun *)m_pWndParent;
	pDlg->m_tabCtrl.SetRedraw(FALSE);
	pDlg->m_tabCtrl.SetActiveTab(1);
	//将会保留的grid内容清空
	int iReservedGrids = m_Grids.size() < m_paramK ? m_Grids.size() : m_paramK;
	for (int i = 0;i < iReservedGrids;i++)
	{
		m_Grids[i]->RemoveAll();
	}

	if (m_Grids.size() > m_paramK)
	{
		//删掉多余的
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
		//增加
		for (int i = m_Grids.size();i < m_paramK;i++)
		{
			CString str;
			str.Format(_T("结果集%d"), i + 1);
			AddResultSet(str);
		}
	}
	else
	{
		//刚刚好
	}
	pDlg->m_tabCtrl.SetRedraw(TRUE);
	pDlg->m_tabCtrl.Invalidate();

	//质心显示
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
		//这里没有需要处理的,因为上一步已经把iIndex设定好了	
		//deep==1
		iIndex = 0;
		//deep==2
		iCurrentKSetIndex = 0;
		dbMinDistance = MAX_DISTANCE;
		iMinDistanceKSetIndex = 0;

		iAlgorithmCount = 0;
		//只有显示需要

		//推进状态
		setAlgorithmStatus(ALGORITHM_STATUS_KMEANS_SELECT_A_POINT);
		break;
	case ALGORITHM_STATUS_KMEANS_SELECT_A_POINT:
		//这里没有需要处理的,因为上一步已经把iIndex设定好了
		//只有显示需要
		PushDrawTask(DRAW_TASK_SELECTED_POINT, iIndex, 0);

		//推进状态
		setAlgorithmStatus(ALGORITHM_STATUS_KMEANS_SELECT_A_KSET);
		break;
	case ALGORITHM_STATUS_KMEANS_SELECT_A_KSET:
		//这里没有需要处理的,因为上一步已经把iCurrentKSetIndex设定好了
		//只有显示需要
		PushDrawTask(DRAW_TASK_SELECT_KSET, iCurrentKSetIndex, 0);

		//推进状态
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
	//只有显示需要
	PushDrawTask(DRAW_TASK_MEASURE,
		iIndex,
		(LPARAM)iCurrentKSetIndex);

	//推进状态
	setAlgorithmStatus(ALGORITHM_STATUS_KMEANS_ENDMEASURE);
	break;
	case ALGORITHM_STATUS_KMEANS_ENDMEASURE:
		//


		PopDrawTask(2); //将选中集合都弹出来
		iCurrentKSetIndex++;
		if (iCurrentKSetIndex == m_paramK)
		{
			//推进状态
			//此点儿和所有质心测量完毕，准备加入某个集合
			//PopDrawTask(3); //将选中点和选中集合都弹出来
			setAlgorithmStatus(ALGORITHM_STATUS_KMEANS_ENDMEASURE_ALL);
		}
		else
		{
			//推进状态
			setAlgorithmStatus(ALGORITHM_STATUS_KMEANS_SELECT_A_KSET);
		}
		//只有显示需要
		break;
	case ALGORITHM_STATUS_KMEANS_ENDMEASURE_ALL:
	{
		PopDrawTask();
		//将点儿加入选定的聚类集合
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
		//更新第iIndex个grid显示,从原来的结果集删除并增加到新结果集中
		if (oldKSetIndex != iMinDistanceKSetIndex)
		{
			//想提高点儿效率，发现这样总出问题，干脆清空在加载
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

				//简单的增加即可，这好和增加顺序一致
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

		//开始对下一个点儿进行处理
		iIndex++;
		iCurrentKSetIndex = 0;
		if (iIndex == m_RawPoints.size())
		{
			//推进状态
			setAlgorithmStatus(ALGORITHM_STATUS_KMEANS_RECACULATE_CENTOID);
		}
		else
		{
			//推进状态
			setAlgorithmStatus(ALGORITHM_STATUS_KMEANS_SELECT_A_POINT);
		}
		//只有显示需要
		//这个点儿改变了集合,不用特殊处理显示,前面已经加入了所有点的显示任务
		Redraw();
		break;

	}
	case ALGORITHM_STATUS_KMEANS_RECACULATE_CENTOID:

	{

		//重新计算所有点儿的聚类质心
		RAWPOINTS_T oldCentoid = m_K_Weight_Points;
		for (int i = 0;i < m_paramK; i++)
		{
			RecaculateCentoid(i);
		}

		//更新grid显示
		int count = m_pGridCtrl_Centoids->GetRowCount();
		ASSERT(m_K_Weight_Points.size() == count);

		for (int i = 0;i < count;i++)
		{
			CBCGPGridRow *pRow = m_pGridCtrl_Centoids->GetRow(i);

			pRow->GetItem(0)->SetValue(m_K_Weight_Points[i].first);
			pRow->GetItem(1)->SetValue(m_K_Weight_Points[i].second);
		}

		//计算偏离度
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
			//推进状态
			setAlgorithmStatus(ALGORITHM_STATUS_COMPLETE);
		}
		else
		{
			iIndex = 0;
			iCurrentKSetIndex = 0;
			//推进状态
			setAlgorithmStatus(ALGORITHM_STATUS_KMEANS_SELECT_A_POINT);
		}
	}

	//只有显示需要
	//这个点儿改变了集合,不用特殊处理显示,前面已经加入了所有点的显示任务
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


	m_pPropK = new CBCGPProp(_T("参数K"), 1, (_variant_t)3, _T("设定K-means算法聚类数量"));
	m_pPropK->EnableSpinControl(TRUE, 3, 10);
	m_pGroupGeneral->AddSubItem(m_pPropK);

	m_pPropRound = new CBCGPProp(_T("迭代次数"), 8, (_variant_t)0, _T("设定K-means算法迭代次数。0表示直到质心迭代偏移小于0.01"));
	m_pPropRound->EnableSpinControl(TRUE, 0, 100);
	m_pGroupGeneral->AddSubItem(m_pPropRound);

	m_pPropRand = new CBCGPProp(_T("随机选择质心"), 2, (_variant_t)true, _T("随机或指定选择质心"));
	m_pGroupGeneral->AddSubItem(m_pPropRand);

	pDlg->m_wndPropList.AddProperty(m_pGroupGeneral);


	m_pPropMeasure = new CBCGPProp(_T("显示测量过程"), 3, (_variant_t)true, _T("算法运行过程中显示测量"));
	pDlg->m_pPropGroup->AddSubItem(m_pPropMeasure);
	m_pPropCentoid = new CBCGPProp(_T("显示迭代偏移"), 5, (_variant_t)true, _T("算法运行过程中显示每次迭代聚类质心偏移值"));
	pDlg->m_pPropGroup->AddSubItem(m_pPropCentoid);
	m_pPropProgress = new CBCGPProp(_T("显示进度条"), 4, (_variant_t)true, _T("算法运行过程中显示执行进度"));
	pDlg->m_pPropGroup->AddSubItem(m_pPropProgress);

	return true;
}

bool CClusterKmeans::initGrid()
{
	//AddResultSet(_T("原始数据"));

	CDialogRun *pDlg = (CDialogRun *)m_pWndParent;
	//m_tabCtrl.InsertItem(0, _T("结果集"));
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


	pDlg->m_tabCtrl.AddTab(m_pGridCtrl_Orgi, _T("原始数据"));



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
	m_pGridCtrl_Centoids->InsertColumn(2, _T("计数"), 60);
	m_pGridCtrl_Centoids->InsertColumn(3, _T("颜色"), 60);
	m_pGridCtrl_Centoids->SetReadOnly();


	pDlg->m_tabCtrl.AddTab(m_pGridCtrl_Centoids, _T("质点值"));

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
				//增加
				for (int i = m_pPorps.size();i < iCount;i++)
				{
					CString str;
					str.Format(_T("质心%d"), i + 1);
					CBCGPProp* pGroupProp = new CBCGPProp(str, 0, TRUE);

					double X, Y;
					int iRandX, iRandY;
					iRandX = rand() % 100;
					iRandY = rand() % 100;

					X = m_rangeCanvas.xStart + iRandX*(m_rangeCanvas.xEnd - m_rangeCanvas.xStart) / 100;
					Y = m_rangeCanvas.yStart + iRandY*(m_rangeCanvas.yEnd - m_rangeCanvas.yStart) / 100;

					pGroupProp->AddSubItem(new CBCGPProp(_T("X"), (_variant_t)X, _T("X 随机值")));
					pGroupProp->AddSubItem(new CBCGPProp(_T("Y"), (_variant_t)Y, _T("Y 随机值")));

					//m_pGroupGeneral->AddSubItem(pGroupProp);
					pDlg->m_wndPropList.AddProperty(pGroupProp);

					m_pPorps.push_back(pGroupProp);
				}
			}
			else
			{
				//删除
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
			//删除几个指点集合
			while (!m_pPorps.empty())
			{
				pDlg->m_wndPropList.DeleteProperty(m_pPorps.back());
				m_pPorps.pop_back();
			}
		}
		else
		{
			//增加质点集合
			int iCount = m_pPropK->GetValue().intVal;

			for (int i = 0;i < iCount;i++)
			{
				CString str;
				str.Format(_T("质心%d"), i + 1);
				CBCGPProp* pGroupProp = new CBCGPProp(str, 0, TRUE);

				double X, Y;
				int iRandX, iRandY;
				iRandX = rand() % 100;
				iRandY = rand() % 100;

				X = m_rangeCanvas.xStart + iRandX*(m_rangeCanvas.xEnd - m_rangeCanvas.xStart) / 100;
				Y = m_rangeCanvas.yStart + iRandY*(m_rangeCanvas.yEnd - m_rangeCanvas.yStart) / 100;

				pGroupProp->AddSubItem(new CBCGPProp(_T("X"), (_variant_t)X, _T("X 随机值")));
				pGroupProp->AddSubItem(new CBCGPProp(_T("Y"), (_variant_t)Y, _T("Y 随机值")));

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
		//读prop

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
		//随机生成
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
	str = _T("K-means算法");
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
		str.Format(_T("算法状态未初始化"));
		DrawProgress(pDC, str, 0, rc);
	}
	else
	{
		str.Format(_T("第%d次迭代 %d/%d"), iAlgorithmCount + 1, iIndex,m_RawPoints.size());
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

	//初始化可达矩阵
	m_listReach.clear();
	for (int i = 0;i < m_RawPoints.size();i++)
	{
		REACH_LIST_T reach_list;
		m_listReach.push_back(make_pair(-1, reach_list));
	}
	//初始化聚类集合
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
	//清除出了基类所有的绘图任务
	ClearDrawTask();

	//初始化可达矩阵
	m_listReach.clear();
	for (int i = 0;i < m_RawPoints.size();i++)
	{
		REACH_LIST_T reach_list;
		m_listReach.push_back(make_pair(-1, reach_list));
	}
	//初始化聚类集合
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
		//检查下算法条件;点数...等等
		//推进状态
		setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_INIT_MESURE);
		break;
	case ALGORITHM_STATUS_DBSCAN_INIT_MESURE:

		m_iIndex = 0;
		m_iEndIndex = 1;
		//推进状态
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
		//计算,更新可达链表
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
		//将所有的核心点画出来
		int iIndex = 0;
		BOOST_FOREACH(const POINT_REACH_ITEM_T &var, m_listReach)
		{			
			//如果是核心点
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
		//选择一个core point
		bool bFound = false;
		int iCoreIndex;
		for (int i = 0;i < m_RawPoints.size();i++)
		{
			//如果没访问过,且是核心对象(可达点超过阈值)
			if (m_listReach[i].first == -1 && m_listReach[i].second.size() >= m_paramMinPts)
			{
				bFound = true;
				iCoreIndex = i;
				break;
			}
		}

		//没有了核心点,算法结束
		if (!bFound)
		{
			m_iCoreIndex = m_RawPoints.size();
			setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_SELECT_ALL_CORE_POINT);
			break;
		}
		m_iCoreIndex = iCoreIndex;

		PushDrawTask(DRAW_TASK_SELECTED_POINT, iCoreIndex, 0);

		//生成一个聚类
		int iIndexCluster = m_listClusters.size();
		CLUSTER_ITEM_T newCluster;
		newCluster.push_back(iCoreIndex);
		m_listClusters.push_back(newCluster);


		//插入一个grid--暂时不增加了会出问题
		//CString str;
		//str.Format(_T("结果集%d"), m_listClusters.size());
		//AddResultSet(str);
		////添加
		//CBCGPGridRow* pRow = m_Grids[iIndexCluster]->CreateRow(2);
		//pRow->GetItem(0)->SetValue(m_RawPoints[iCoreIndex].first);
		//pRow->GetItem(1)->SetValue(m_RawPoints[iCoreIndex].second);
		//m_Grids[iIndexCluster]->AddRow(pRow);

		//修改汇总显示的对应行--增加

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


		//设置访问标志
		m_listReach[iCoreIndex].first = iIndexCluster;

		//将选出的corepoint的可达点入队列
		m_ReachQueue.clear();
		BOOST_FOREACH(const REACH_ITEM_T &var, m_listReach[iCoreIndex].second)
		{
			//如果它的可达点未被访问过，也就是没有被其他点儿聚过
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
		//从队列头选择第一个未访问的点,让它保持在队列头，其他的都pop出去
		pair<int, int> iPoint;
		while (!m_ReachQueue.empty())
		{
			iPoint = m_ReachQueue.front();

			if (m_listReach[iPoint.second].first == -1)
				break;
			else
				m_ReachQueue.pop_front();
		}


		//可达队列处理完，
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

		//如果当前可达点儿是核心点,
		//if (m_listReach[iPoint.second].second.size() >= m_paramMinPts)
		//{
		//	PushDrawTask(DRAW_TASK_CORE_PATH, iPoint.first, iPoint.second);
		//}

		//设置访问标志
		m_listReach[iPoint.second].first = m_listClusters.size() - 1;

		//增加到结果集--暂时不增加了，在工作线程中会出问题
		//CBCGPGridRow* pRow = m_Grids[m_listClusters.size() - 1]->CreateRow(2);
		//pRow->GetItem(0)->SetValue(m_RawPoints[iPoint.second].first);
		//pRow->GetItem(1)->SetValue(m_RawPoints[iPoint.second].second);
		//m_Grids[m_listClusters.size() - 1]->AddRow(pRow);
		//修改汇总显示

		CBCGPGridRow* pRow = m_pGridCtrl_ClusterCount->GetRow(m_listClusters.size() - 1);
		pRow->GetItem(1)->SetValue((pRow->GetItem(1)->GetValue().intVal + 1));

		//放入聚类中
		m_listClusters.back().push_back(iPoint.second);
		setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_END_CLUSTER);
		break;
	}
	case ALGORITHM_STATUS_DBSCAN_END_CLUSTER:
	{
		pair<int, int> iPoint = m_ReachQueue.front();

		PopDrawTask();
		//如果当前可达点儿是核心点,
		if (m_listReach[iPoint.second].second.size() >= m_paramMinPts)
		{
			//画上核心点到达路径
			PopDrawTask();
			PushDrawTask(DRAW_TASK_CORE_PATH, iPoint.first, iPoint.second);
			//为了填空
			PushDrawTask(DRAW_TASK_NONE, 0, 0);

			BOOST_FOREACH(const REACH_ITEM_T &var, m_listReach[iPoint.second].second)
			{
				//如果它的可达点未被访问过，也就是没有被其他点儿聚过
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
		//可达队列处理完，选择下一个corepoint
		PopDrawTask();

		//显示一个聚类结果

		setAlgorithmStatus(ALGORITHM_STATUS_DBSCAN_SELECT_A_CORE_POINT);
		break;
	}
	case ALGORITHM_STATUS_DBSCAN_SELECT_ALL_CORE_POINT:
	{
		//算法结束
		//把奇异点加入集合
		int iIndex = 0;
		int count = 0;
		BOOST_FOREACH(const POINT_REACH_ITEM_T &var, m_listReach)
		{
			//如果是核心点
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


	m_pPropE = new CBCGPProp(_T("参数E"), 2, (_variant_t)5.0, _T("设定DBSCAN算法参数E"));
	m_pGroupGeneral->AddSubItem(m_pPropE);

	m_pPropMinPts = new CBCGPProp(_T("参数MinPts"), 1, (_variant_t)5, _T("设定DBSCAN算法参数MinPts"));
	m_pPropMinPts->EnableSpinControl(TRUE, 3, 100);
	m_pGroupGeneral->AddSubItem(m_pPropMinPts);

	pDlg->m_wndPropList.AddProperty(m_pGroupGeneral);


	m_pPropMeasure = new CBCGPProp(_T("显示测量过程"), 3, (_variant_t)true, _T("DBSCAN算法第一步需要测量距离矩阵,并计算核心点"));
	pDlg->m_pPropGroup->AddSubItem(m_pPropMeasure);

	m_pPropCore = new CBCGPProp(_T("核心点显示方式"),4, _T("E-圆"), _T("1.不显示\n2.显示核心点密度可达范围\n3.仅用小圆标识核心点"));
	m_pPropCore->AddOption(_T("不显示"));
	m_pPropCore->AddOption(_T("E-圆"));
	m_pPropCore->AddOption(_T("小圆"));
	m_pPropCore->AllowEdit(FALSE);
	pDlg->m_pPropGroup->AddSubItem(m_pPropCore);
		
	m_pPropCorePath = new CBCGPProp(_T("显示核心点路径"), 5, (_variant_t)true, _T("显示核心点路径"));
	pDlg->m_pPropGroup->AddSubItem(m_pPropCorePath);
	m_pPropProgress = new CBCGPProp(_T("显示进度条"), 6, (_variant_t)true, _T("算法运行过程中显示执行进度"));
	pDlg->m_pPropGroup->AddSubItem(m_pPropProgress);

	return true;
}

bool CClusterDbscan::initGrid()
{
	//AddResultSet(_T("原始数据"));

	CDialogRun *pDlg = (CDialogRun *)m_pWndParent;

	//原始集合
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


	pDlg->m_tabCtrl.AddTab(m_pGridCtrl_Orgi, _T("原始数据"));

	//奇异点集合,初始所有点
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

	pDlg->m_tabCtrl.AddTab(m_pGridCtrl_Odd, _T("奇异点"));

	//蒋所有点加入初始集合和奇异点集合
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

	//聚类集合显示

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
	m_pGridCtrl_ClusterCount->InsertColumn(0, _T("颜色"), 100);
	m_pGridCtrl_ClusterCount->InsertColumn(1, _T("计数"), 100);
	m_pGridCtrl_ClusterCount->SetReadOnly();

	pDlg->m_tabCtrl.AddTab(m_pGridCtrl_ClusterCount, _T("聚类结果"));


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
	str = _T("DBSCAN算法");
}

bool CClusterDbscan::postRunComplete()
{
	for (int i = 0;i < m_listClusters.size();i++)
	{
		CString str;
		str.Format(_T("结果集%d"), i+1);
		AddResultSet(str);
		//添加
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
		str.Format(_T("算法状态未初始化"));
		DrawProgress(pDC, str, 0, rc);
		break;
	}
	case ALGORITHM_STATUS_INIT:
	{
		str.Format(_T("算法已初始化"));
		DrawProgress(pDC, str,10, rc);
		break;
	}
	case ALGORITHM_STATUS_COMPLETE:
	{
		str.Format(_T("算法运行结束"));
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
		str.Format(_T("距离矩阵测量 %d/%d"),cur,total);
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
		str.Format(_T("聚类生成 %d/%d"), m_iCoreIndex, count);
		DrawProgress(pDC, str, m_iCoreIndex*100/count, rc);
		break;
	}
	default:
	{
		str.Format(_T("DBSCAN聚类算法"));
		DrawProgress(pDC, str,10, rc);
		break;
	}		
	}
	return true;
}
