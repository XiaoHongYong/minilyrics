// ChildFrm.cpp : implementation of the CDynCmdFrame class
//

#include "stdafx.h"
#include "MPSkinEditor.h"

#include "DynCmdFrame.h"

#include "PropertyBar.h"
#include "SEPropertyListCtrl.h"

extern class CSkinEditorFrame		*g_pActiveFrameOld;

/////////////////////////////////////////////////////////////////////////////
// CDynCmdFrame

IMPLEMENT_DYNCREATE(CDynCmdFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CDynCmdFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CDynCmdFrame)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_WM_SETFOCUS()
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_CHILDACTIVATE()
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDynCmdFrame construction/destruction

CDynCmdFrame::CDynCmdFrame()
{
	// TODO: add member initialization code here
	
}

CDynCmdFrame::~CDynCmdFrame()
{
}

BOOL CDynCmdFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	// cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CDynCmdFrame diagnostics

#ifdef _DEBUG
void CDynCmdFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CDynCmdFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDynCmdFrame message handlers
void CDynCmdFrame::OnFileClose() 
{
	// To close the frame, just send a WM_CLOSE, which is the equivalent
	// choosing close from the system menu.

	SendMessage(WM_CLOSE);
}

int CDynCmdFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// CRect	rc;
	// GetClientRect(&rc);
	// m_wndGrid.Create(rc, this, 101);
	m_wndGrid.Create(CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST);
	m_wndGrid.SetColumnCount(3);
	m_wndGrid.SetRowCount(10);
	m_wndGrid.SetFixedRowCount(1);
	m_wndGrid.SetColumnWidth(0, 200);
	m_wndGrid.SetColumnWidth(1, 200);
	m_wndGrid.SetColumnWidth(2, 200);

	m_wndGrid.SetItemText(0, COL_ID, _T("ID"));
	m_wndGrid.SetItemText(0, COL_FUNC, _T("Function"));
	m_wndGrid.SetItemText(0, COL_PARAM, _T("Parameter"));

	CDynamicCmds &dc = g_skinFactory.GetDynamicCmds();
	CDynamicCmds::LIST_DYNCMD &listdc = dc.GetDataList();
	CDynamicCmds::LIST_DYNCMD::iterator it, itEnd;
	int		i = 1;
	itEnd = listdc.end();
	for (it = listdc.begin(); it != itEnd; ++it, i++)
	{
		CDynamicCmds::DynamicCmd	&item = *it;
		m_wndGrid.InsertRow(_T(""));
		m_wndGrid.SetItemText(i, COL_ID, item.strCmdId.c_str());
		m_wndGrid.SetItemText(i, COL_FUNC, item.strFunction.c_str());
		m_wndGrid.SetItemText(i, COL_PARAM, item.strParam.c_str());
	}

	g_skinPrj.OnCreateSkinDataItem(this);

	return 0;
}

void CDynCmdFrame::OnSetFocus(CWnd* pOldWnd) 
{
	CMDIChildWnd::OnSetFocus(pOldWnd);

	m_wndGrid.SetFocus();
}

void CDynCmdFrame::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
// 
// 	CRect		rc;
// 
// 	GetClientRect(&rc);
// 
// 	dc.FillSolidRect(&rc, RGB(255, 255, 255));
}

void CDynCmdFrame::OnSave()
{
	CDynamicCmds &dc = g_skinFactory.GetDynamicCmds();
	CDynamicCmds::LIST_DYNCMD &ldc = dc.GetDataList();
	CString		str;
	ldc.clear();

	for (int i  = 1; i < m_wndGrid.GetRowCount(); i++)
	{
		CDynamicCmds::LIST_DYNCMD::iterator it, itEnd;

		str = m_wndGrid.GetItemText(i, COL_ID);
		if (str.IsEmpty())
			continue;

		// is this command ()id existed?
		itEnd = ldc.end();
		for (it = ldc.begin(); it != itEnd; ++it)
		{
			CDynamicCmds::DynamicCmd	&item = *it;
			if (_tcsicmp(item.strCmdId.c_str(), str) == 0)
				break;
		}

		if (it == itEnd)
		{
			// add it
			CDynamicCmds::DynamicCmd	newitem;
			newitem.strCmdId = str;
			newitem.strFunction = m_wndGrid.GetItemText(i, COL_FUNC);
			newitem.strParam = m_wndGrid.GetItemText(i, COL_PARAM);
			ldc.push_back(newitem);
		}
	}
}

void CDynCmdFrame::OnDestroy() 
{
	g_skinPrj.OnDestroySkinDataItem(this);

	CMDIChildWnd::OnDestroy();
}

void CDynCmdFrame::OnChildActivate() 
{
	CMDIChildWnd::OnChildActivate();
	
	if (g_seSkinPropertyAdapter.IsSrcChanged(NULL, NULL))
	{
		CPropertyBar::ms_pwndProperty->ResetContent();
	}

	if (g_pActiveFrameOld != NULL)
	{
		CTreeCtrl		*pTree = GetSkinUIObjTreeCtrl();
		pTree->DeleteAllItems();
	}
}

void CDynCmdFrame::OnSysCommand(UINT nID, LPARAM lParam) 
{
	if (nID == SC_CLOSE)
		return;

	CMDIChildWnd::OnSysCommand(nID, lParam);
}
