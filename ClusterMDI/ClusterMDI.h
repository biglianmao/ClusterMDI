// ClusterMDI.h : main header file for the ClusterMDI application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CClusterMDIApp:
// See ClusterMDI.cpp for the implementation of this class
//

class CClusterMDIApp : public CBCGPWinApp
{
public:
	CClusterMDIApp();

	// Override from CBCGPWorkspace
	virtual void PreLoadState ();

	CMultiDocTemplate* m_pTemplateGridView;
	CMultiDocTemplate* m_pTemplateDrawView;
// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	virtual BOOL SaveAllModified();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
//	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
	afx_msg void OnFileNew();
	afx_msg void OnFileOpen();
	bool CloseAllDoc();
};

extern CClusterMDIApp theApp;

#include <utility>
#include <vector>
#include <list>
#include <stdlib.h>
using namespace std;
typedef vector<pair<double, double>> RAWPOINTS_T;
typedef pair<double, double> RAWPOINT_T;

#define WM_MOUSE_XY	WM_USER+1
#define WM_ALG_RUNCOMPLETE	WM_USER+3
#define WM_ALG_DATACHANGE	WM_USER+4

#include "DrawParameter.h"