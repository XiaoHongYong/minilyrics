// ObjListBar.cpp : implementation file
//

#include "stdafx.h"
#include "MPSkinEditor.h"
#include "ObjListBar.h"

#define IDC_SKIN_OBJ_LIST		101

CTreeCtrl	*CObjListBar::ms_pwndTreeObj = NULL;

CTreeCtrl *GetSkinUIObjTreeCtrl()
{
	return CObjListBar::ms_pwndTreeObj;
}


/////////////////////////////////////////////////////////////////////////////
// CObjListBar

CObjListBar::CObjListBar()
{
}

CObjListBar::~CObjListBar()
{
}


BEGIN_MESSAGE_MAP(CObjListBar, CSizingControlBar)
	//{{AFX_MSG_MAP(CObjListBar)
    ON_WM_CREATE()
    ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_NOTIFY(TVN_SELCHANGED, IDC_SKIN_OBJ_LIST, OnSelchangeSkinObjList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CObjListBar message handlers

int CObjListBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CSizingControlBar::OnCreate(lpCreateStruct) == -1)
        return -1;

//	LoadState(_T("ObjListBar"));
	ms_pwndTreeObj = &m_wndTreeObj;

	CRect	rc;
	GetClientRect(&rc);
	m_wndTreeObj.Create(TVS_LINESATROOT | TVS_FULLROWSELECT | TVS_HASLINES | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | WS_VISIBLE | WS_TABSTOP | WS_CHILD | WS_BORDER, rc, this, IDC_SKIN_OBJ_LIST);
	m_wndTreeObj.ShowWindow(SW_SHOW);

    return 0;
}

void CObjListBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	CSizingControlBar::OnUpdateCmdUI(pTarget, bDisableIfNoHndler);

	UpdateDialogControls(pTarget, bDisableIfNoHndler);
}

void CObjListBar::OnSize(UINT nType, int cx, int cy) 
{
    CSizingControlBar::OnSize(nType, cx, cy);

    // TODO: Add your message handler code here
    CRect rc;
    GetClientRect(rc);

	if (m_wndTreeObj.GetSafeHwnd())
		m_wndTreeObj.MoveWindow(rc);
}

void CObjListBar::OnDestroy() 
{
	CSizingControlBar::OnDestroy();
}

void CObjListBar::OnSelchangeSkinObjList(NMHDR* pNMHDR, LRESULT* pResult)
{
	CMDIFrameWnd *pFame = (CMDIFrameWnd *)AfxGetMainWnd();
	CWnd *pChild = pFame->GetActiveFrame();
	if (pChild)
	{
		pChild->SendMessage(WM_COMMAND, IDC_SKIN_OBJ_LIST_SEL_CHANGED);
	}
}
