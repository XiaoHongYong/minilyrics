#if !defined(AFX_DLGTREE_H__1817D967_9836_44AA_8B2B_62210DC02156__INCLUDED_)
#define AFX_DLGTREE_H__1817D967_9836_44AA_8B2B_62210DC02156__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgTree.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgTree dialog

class CDlgTree : public CDialog
{
// Construction
public:
	CDlgTree(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgTree)
	enum { IDD = IDD_DBG_TREE };
	CTreeCtrl	m_tree;
	//}}AFX_DATA

	DWORD		m_dwTreeId;

	void AddTree(DWORD dwTreeId, DWORD dwParentId, DWORD CurId, LPCTSTR szName);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgTree)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgTree)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGTREE_H__1817D967_9836_44AA_8B2B_62210DC02156__INCLUDED_)
