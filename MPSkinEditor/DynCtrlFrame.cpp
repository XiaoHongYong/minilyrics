// DynCtrlFrame.cpp : implementation of the CDynCtrlFrame class
//

#include "stdafx.h"
#include "MPSkinEditor.h"

#include "DynCtrlFrame.h"

#include "PropertyBar.h"
#include "SEPropertyListCtrl.h"

extern class CSkinEditorFrame		*g_pActiveFrameOld;

/////////////////////////////////////////////////////////////////////////////
// CDynCtrlFrame

IMPLEMENT_DYNCREATE(CDynCtrlFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(CDynCtrlFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CDynCtrlFrame)
		ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
		ON_WM_SETFOCUS()
		ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_CHILDACTIVATE()
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDynCtrlFrame construction/destruction

CDynCtrlFrame::CDynCtrlFrame()
{
	// TODO: add member initialization code here
	
}

CDynCtrlFrame::~CDynCtrlFrame()
{
}

BOOL CDynCtrlFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	
	cs.lpszClass = AfxRegisterWndClass(0);

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CDynCtrlFrame diagnostics

#ifdef _DEBUG
void CDynCtrlFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CDynCtrlFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDynCtrlFrame message handlers
void CDynCtrlFrame::OnFileClose() 
{
	// To close the frame, just send a WM_CLOSE, which is the equivalent
	// choosing close from the system menu.

	SendMessage(WM_CLOSE);
}

int CDynCtrlFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CRect	rc;
	GetClientRect(&rc);
	// create a view to occupy the client area of the frame
	m_wndEdit.Create(ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL,
		rc, this, AFX_WS_DEFAULT_VIEW);
	rc.top += 3;
	rc.bottom -= 3;
	m_wndEdit.SetRect(&rc);
	m_wndEdit.SetMargins(10, 10);

	SetWindowLong(m_wndEdit.m_hWnd, GWL_EXSTYLE, GetWindowLong(m_wndEdit.m_hWnd, GWL_EXSTYLE) | WS_EX_CLIENTEDGE);

	// g_skinFactory.GetDynamicCmds()
	m_wndEdit.SetFont(CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT)));

	//
	// Set ctrl data
	//
	CDynamicCtrls &dc = g_skinFactory.GetDynamicCtrls();
	SXNode		*pNode = dc.GetData();
	CXMLWriter	xmlStream(false);
	tstringex	str;

	pNode->ToXML(xmlStream);
	Utf8ToUCS2(xmlStream.GetData(), xmlStream.GetLength(), str);
	m_wndEdit.SetWindowText(str.c_str());

	g_skinPrj.OnCreateSkinDataItem(this);

	return 0;
}

void CDynCtrlFrame::OnSetFocus(CWnd* pOldWnd) 
{
	CMDIChildWnd::OnSetFocus(pOldWnd);

	m_wndEdit.SetFocus();
}

void CDynCtrlFrame::OnSave()
{
	CString		str;
	CSimpleXML	xml;
	stringex	strUtf8;

	m_wndEdit.GetWindowText(str);
	str.Insert(0, _T("<?xml version=\"1.0\" encoding='utf-8'?>"));
	UCS2ToUtf8(str, str.GetLength(), strUtf8);
	if (xml.ParseData((LPVOID)strUtf8.c_str(), strUtf8.size()))
	{
		CDynamicCtrls &dc = g_skinFactory.GetDynamicCtrls();
		dc.FromXML(xml.m_pRoot);
	}
}

void CDynCtrlFrame::OnDestroy() 
{
	g_skinPrj.OnDestroySkinDataItem(this);

	CMDIChildWnd::OnDestroy();
}

void CDynCtrlFrame::OnSize(UINT nType, int cx, int cy) 
{
	CMDIChildWnd::OnSize(nType, cx, cy);
	
	if (m_wndEdit.m_hWnd)
	{
		CRect	rc;
		GetClientRect(&rc);
		m_wndEdit.MoveWindow(&rc);
	}
}

void CDynCtrlFrame::OnChildActivate() 
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

void CDynCtrlFrame::OnSysCommand(UINT nID, LPARAM lParam) 
{
	if (nID == SC_CLOSE)
		return;

	CMDIChildWnd::OnSysCommand(nID, lParam);
}
