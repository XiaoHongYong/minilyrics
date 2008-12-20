// PropertyBar.cpp : implementation file
//

#include "stdafx.h"
#include "MPSkinEditor.h"
#include "PropertyBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropertyBar

CSEPropertyListCtrl	*CPropertyBar::ms_pwndProperty = NULL;

CPropertyBar::CPropertyBar()
{
}

CPropertyBar::~CPropertyBar()
{
}


BEGIN_MESSAGE_MAP(CPropertyBar, CSizingControlBar)
	//{{AFX_MSG_MAP(CPropertyBar)
    ON_WM_CREATE()
    ON_WM_SIZE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPropertyBar message handlers

int CPropertyBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CSizingControlBar::OnCreate(lpCreateStruct) == -1)
        return -1;

//	LoadState("PropertyBar");

	///////////////////////////////////////
	// Create the list box font
	m_ListBoxFont.CreateFont( 14, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, 
							 CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH | FF_SWISS,
							 _T("arial"));	

	///////////////////////////////////////
	// Create the Control
	ms_pwndProperty = new CSEPropertyListCtrl();
	// ms_pwndProperty->SubclassDlgItem(ID_PROPERTYLIST, this);
	CRect	rc;
	GetClientRect(&rc);
	ms_pwndProperty->Create(WS_CHILD|WS_BORDER|WS_VISIBLE|LBS_NOTIFY|WS_VSCROLL|WS_HSCROLL|LBS_HASSTRINGS|LBS_OWNERDRAWFIXED, rc, this, 1000);
	
	///////////////////////////////////////
	// Set some attributes (Completly Optional)
	ms_pwndProperty->SetFont(&m_ListBoxFont);
/*	ms_pwndProperty->SetBkColor(RGB(214,227,239));
	ms_pwndProperty->SetTextColor(RGB(74,109,132));
	ms_pwndProperty->SetTextHighlightColor(RGB(80,80,80));
	ms_pwndProperty->SetHighlightColor(RGB(246,246,220));
	ms_pwndProperty->SetPropertyBkColor(RGB(255,255,255));
	ms_pwndProperty->SetPropertyTextColor(RGB(0,0,192));
	ms_pwndProperty->SetBoldSelection(TRUE);
	ms_pwndProperty->SetLineStyle(RGB(74,109,132), PS_SOLID);*/

/*
    if (!m_wndChild.Create(_T(_T("Paste")),
		WS_CHILD|WS_VISIBLE,
		CRect(0,0,0,0), this, ID_EDIT_PASTE))
        return -1;
*/
/*
	CRect	rc;
	GetClientRect(&rc);
	if (!m_wndChild.Create(LBS_HASSTRINGS | LBS_MULTICOLUMN | LBS_NOTIFY | LBS_OWNERDRAWFIXED, rc, this, ID_EDIT_PASTE))
		return -1;
	m_wndChild.ShowWindow(SW_SHOW);

    // older versions of Windows* (NT 3.51 for instance)
    // fail with DEFAULT_GUI_FONT

    if (!m_font.CreateStockObject(DEFAULT_GUI_FONT))
        if (!m_font.CreatePointFont(80, _T("MS Sans Serif")))
            return -1;

    m_wndChild.SetFont(&m_font);
	ms_pwndProperty = &m_wndChild;
*/

	ms_pwndProperty = ms_pwndProperty;

    return 0;
}

void CPropertyBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	CSizingControlBar::OnUpdateCmdUI(pTarget, bDisableIfNoHndler);

	UpdateDialogControls(pTarget, bDisableIfNoHndler);
}

void CPropertyBar::OnSize(UINT nType, int cx, int cy) 
{
    CSizingControlBar::OnSize(nType, cx, cy);

    // TODO: Add your message handler code here
    CRect rc;
    GetClientRect(rc);

	if (ms_pwndProperty)
		ms_pwndProperty->MoveWindow(rc);
//    m_wndChild.MoveWindow(rc);
}

void CPropertyBar::OnDestroy() 
{
	CSizingControlBar::OnDestroy();
	
	ms_pwndProperty = NULL;
}
