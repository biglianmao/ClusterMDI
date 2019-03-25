// ClusterMDIDoc.h : interface of the CClusterMDIDoc class
//


#pragma once


class CClusterMDIDoc : public CDocument
{
protected: // create from serialization only
	CClusterMDIDoc();
	DECLARE_DYNCREATE(CClusterMDIDoc)

// Attributes
public:

	RAWPOINTS_T m_RawPoints;
// Operations
public:

// Overrides
	public:
//	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CClusterMDIDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
//	virtual void OnCloseDocument();
	virtual BOOL SaveModified();
//	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void SetTitle(LPCTSTR lpszTitle);
};


