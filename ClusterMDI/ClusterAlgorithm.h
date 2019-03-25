#pragma once

#include <boost\thread.hpp>
#include <boost\bind.hpp>
#include <boost\smart_ptr.hpp>

typedef enum _enumAlgorithmError
{
	ERROR_NONE = 0,
	ERROR_RAWPOINTS,
	ERROR_PARMETER,
	ERROR_UNKOWN,
	ERROR_RUN_COMPLETE
}ALGORITHMERROR;

typedef enum _enumRunStatus
{
	RUN_STATUS_NONE = 0,
	RUN_STATUS_READY,
	RUN_STATUS_RUN,
	RUN_STATUS_END
}RUNSTATUS;

typedef enum _enumAlgorithmStatus
{
	ALGORITHM_STATUS_NONE=0,
	ALGORITHM_STATUS_INIT,
	ALGORITHM_STATUS_COMPLETE,
	ALGORITHM_STATUS_COMPLETE_WITH_ERROR,
	//K-MEANS
	ALGORITHM_STATUS_KMEANS_SELECT_A_POINT,
	ALGORITHM_STATUS_KMEANS_SELECT_A_KSET,
	ALGORITHM_STATUS_KMEANS_BEGINMEASURE,
	ALGORITHM_STATUS_KMEANS_ENDMEASURE,
	ALGORITHM_STATUS_KMEANS_ENDMEASURE_ALL, 
	ALGORITHM_STATUS_KMEANS_RECACULATE_CENTOID,
	ALGORITHM_STATUS_KMEANS_,

	//DBSCAN
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

	ALGORITHM_STATUS_MAX
	
}ALGORITHMSTATUS;

typedef enum _enumDrawTaskType
{
	DRAW_TASK_NONE = 0,
	DRAW_TASK_PROGRESS,
	DRAW_TASK_POINT,
	DRAW_TASK_SELECTED_POINT,
	DRAW_TASK_KSET,
	DRAW_TASK_SELECT_KSET,
	DRAW_TASK_MEASURE,
	DRAW_TASK_CENTOID,
	DRAW_TASK_CORE,
	DRAW_TASK_CORE_PATH,
	DRAW_TASK_MAX
}DRAWTASKTYPE;

typedef struct _stru_DrawTaskItem
{
	DRAWTASKTYPE type;
	WPARAM  wp;
	LPARAM  lp;
}DRAWTASKITEM;
typedef vector<DRAWTASKITEM> DRAWTASK_T;
typedef struct _stru_Kmeans_CallStack_Item
{
	//deep==1
	int iIndex;
	int iCount;

	//deep==2
	int iCurrentKSetIndex;
	double dbMinDistance;
	int iMinDistanceKSetIndex;
}KMEANS_CALLSTACK_ITEM;

#define MAX_DISTANCE	1e100
#define MAX_STEPS  100000

class CClusterAlgorithmBase
{
public:
	// 线程函数
	void threadFun();
private:
	ALGORITHMERROR m_err;
	DRAWTASK_T m_DrawTask;
	CWnd *m_pWnd;
	int m_StopSignal;
	boost::shared_ptr<boost::thread> m_pThread;
	int m_ms;

	RUNSTATUS m_RunStatus;
	ALGORITHMSTATUS m_AlgStatus;
	bool m_bSkipStatus[ALGORITHM_STATUS_MAX];
	int  m_bDoDrawTask[DRAW_TASK_MAX];
protected:
	CSize m_szCanvas;
	stDoubleRange m_rangeCanvas;
	CWnd *m_pWndParent;
	const RAWPOINTS_T& m_RawPoints;

	int m_iDrawTaskCount;
	vector<CBCGPGridCtrl*> m_Grids;
	void setLastError(ALGORITHMERROR err) { m_err = err; };
	void setAlgorithmStatus(ALGORITHMSTATUS status) { m_AlgStatus = status; };

	int shuldDraw(DRAWTASKTYPE type);
	virtual void runStatusMachine() = 0;
public:
	ALGORITHMERROR getLastError() { return m_err; };
	LPCTSTR translateError(ALGORITHMERROR err)
	{
		TCHAR *errMsg[] = {
			_T("everything is OK!"),
			_T("rawPoints data is error!"),
			_T("parameter is error!"),
			_T("something is error!"),
			_T("algorithm is successful over!"),
			_T("not defined error!"),
		};
		if (err<0 || err > ERROR_RUN_COMPLETE)
			err = (ALGORITHMERROR)(ERROR_RUN_COMPLETE + 1);
		return errMsg[err];
	};
	
	RUNSTATUS getRunStatus() { return  m_RunStatus; }
	ALGORITHMSTATUS getAlgorithmStatus() { return m_AlgStatus; };

	void SetDoDrawFlag(DRAWTASKTYPE type, int val = 0) { m_bDoDrawTask[type] = val; }
	void SetSkipStatus(ALGORITHMSTATUS status, bool val = true) { m_bSkipStatus[status] = val; }
	int GetDoDrawFlag(DRAWTASKTYPE type) { return  m_bDoDrawTask[type]; }
public:
	virtual bool init(void *pParam);
	virtual bool refresh();
	void run(int ms);
	void pause();
	void stop();
	virtual void step();
	virtual bool end() { return m_AlgStatus == ALGORITHM_STATUS_COMPLETE;}
	virtual bool initProp() = 0;
	virtual bool initGrid() = 0;
	virtual void propChanged(UINT nID, CBCGPProp* pProp) = 0;
	virtual bool getPropValue(void *pParam) = 0;
	void doDraw(CDC* pDC, const CRect& clientRect);

	virtual void getAlgoriStr(CString &str) = 0;

	virtual bool postRunComplete() = 0;
private:
	void DrawCoordinate(CDC* pDC, const CRect& rc);
protected:
	virtual bool ExcuteDrawTaskItem(CDC* pDC, CSize& szCanvas, stDoubleRange& rangeCanvas, DRAWTASKITEM &item);
	virtual bool DrawAlgorithmProgress(CDC* pDC, CRect& rc) = 0;
	void PushDrawTask(DRAWTASKTYPE type,WPARAM wp,LPARAM lp);
	void Redraw();
	void PopDrawTask(int step = 1);
	void ClearDrawTask();
	void NotifyDataChanged();
	void AddResultSet(LPCTSTR csTitle);

public:
	CClusterAlgorithmBase(const RAWPOINTS_T& rawPoints,CWnd *pWnd);
	~CClusterAlgorithmBase();

};


class CClusterKmeans : public CClusterAlgorithmBase
{
public:
	typedef struct _stru_KMEANS_PARAM
	{
		int k;
		RAWPOINTS_T points;
		int count;
	}KMEANS_PARAM;
private:
	//KMEANS_CALLSTACK_ITEM m_CallStack;
	//ALGORITHMSTATUS m_AlgStatus;

	//deep==1
	int iIndex;
	//int iCount;

	//deep==2
	int iCurrentKSetIndex;
	double dbMinDistance;
	int iMinDistanceKSetIndex;

	int iAlgorithmCount;

	CBCGPProp* m_pPropK;
	CBCGPProp* m_pPropRand;
	CBCGPProp* m_pPropRound;
	CBCGPProp* m_pPropMeasure;
	CBCGPProp* m_pPropCentoid;
	CBCGPProp* m_pPropProgress;
	vector<CBCGPProp*> m_pPorps;
	CBCGPGridCtrl* m_pGridCtrl_Orgi;
	CBCGPGridCtrl* m_pGridCtrl_Centoids;
	stDoubleRange m_rangeCanvas;
public:
	int m_paramK;
	RAWPOINTS_T m_param_K_Points;
	int m_param_Count;

	RAWPOINTS_T m_K_Weight_Points;	//聚类集合的重心
	vector<int> m_K_Points_Set_Tags; //对应rawPoints集合，指示所处于的聚类集

private:
	double	MeasureDistance(RAWPOINT_T& pt1, RAWPOINT_T& pt2);
	void	SelectPointIntoCSet(RAWPOINT_T& pt1, int KIndex);
	void	RecaculateCentoid(int k);
	virtual void runStatusMachine();
public:
	CClusterKmeans(const RAWPOINTS_T& rawPoints, CWnd *pWnd);
	~CClusterKmeans();

	virtual bool init(void *pParam);

	virtual bool initProp();
	virtual bool initGrid();
	virtual void propChanged(UINT nID, CBCGPProp* pProp);
	virtual bool getPropValue(void *pParam);
	virtual void getAlgoriStr(CString &str);
	virtual bool postRunComplete();
protected:
	virtual bool ExcuteDrawTaskItem(CDC* pDC, CSize& szCanvas, stDoubleRange& rangeCanvas, DRAWTASKITEM &item);
	virtual bool DrawAlgorithmProgress(CDC* pDC, CRect& rc);
};

typedef pair<int, double> REACH_ITEM_T;
typedef vector<REACH_ITEM_T> REACH_LIST_T;
typedef pair<int, REACH_LIST_T> POINT_REACH_ITEM_T;
typedef vector<POINT_REACH_ITEM_T> POINT_REACH_LIST_T;
typedef vector<int> CLUSTER_ITEM_T;
typedef vector<CLUSTER_ITEM_T> CLUSTER_LIST_T;

class CClusterDbscan : public CClusterAlgorithmBase
{
public:
	typedef struct _stru_DBSCAN_PARAM
	{
		double E;
		int MinPts;
	}DBSCAN_PARAM;
private:
	//KMEANS_CALLSTACK_ITEM m_CallStack;
	//
	//stDoubleRange m_rangeCanvas;

	int m_iIndex;
	int m_iEndIndex;
	int m_iCoreIndex;
	
	CBCGPProp* m_pPropE;
	CBCGPProp* m_pPropMinPts;
	CBCGPProp* m_pPropMeasure;
	CBCGPProp* m_pPropCore;
	CBCGPProp* m_pPropCorePath;
	CBCGPProp* m_pPropProgress;
	CBCGPGridCtrl* m_pGridCtrl_Orgi;
	CBCGPGridCtrl* m_pGridCtrl_Odd;
	CBCGPGridCtrl* m_pGridCtrl_ClusterCount;
	vector<CBCGPGridCtrl*> m_listClustersGrid;

	POINT_REACH_LIST_T m_listReach;
	CLUSTER_LIST_T m_listClusters;

	list<pair<int,int>> m_ReachQueue;

public:
	double m_paramE;
	int m_paramMinPts;
private:

public:
	CClusterDbscan(const RAWPOINTS_T& rawPoints, CWnd *pWnd);
	~CClusterDbscan();

	virtual bool init(void *pParam);

	virtual bool initProp();
	virtual bool initGrid();
	virtual void propChanged(UINT nID, CBCGPProp* pProp);
	virtual bool getPropValue(void *pParam);
	virtual void getAlgoriStr(CString &str);
	virtual bool postRunComplete();
protected:
	virtual bool ExcuteDrawTaskItem(CDC* pDC, CSize& szCanvas, stDoubleRange& rangeCanvas, DRAWTASKITEM &item);
	virtual void runStatusMachine();
	virtual bool DrawAlgorithmProgress(CDC* pDC, CRect& rc);
};
