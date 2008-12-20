// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "MPSkinEditor.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(IDC_SAVE_SKIN, OnSaveSkin)
	ON_COMMAND(IDC_VIEW_OBJLIST_BAR, OnViewObjlistBar)
	ON_COMMAND(IDC_VIEW_PROPERTY_BAR, OnViewPropertyBar)
	ON_UPDATE_COMMAND_UI(IDC_MOUSE_POINTER, OnUpdatePaneMousePointer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_bTop = true;
	m_bImages = true;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndStatusBar.SetPaneText(1, _T("X: Y: "));

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	if (!m_wndObjListBar.Create(_T("Object List"), this, CSize(200, 200),
		TRUE, 123 + 2))//AFX_IDW_CONTROLBAR_FIRST + 33 + i))
	{
		ERRLOG0(_T("Failed to create m_wndLogBar\n"));
		return -1;      // fail to create
	}

	m_wndObjListBar.SetBarStyle(m_wndObjListBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	m_wndObjListBar.EnableDocking(CBRS_ALIGN_LEFT);


	if (!m_wndPropertyBar.Create(_T("Property"), this, CSize(200, 200),
        TRUE, 123 + 0))//AFX_IDW_CONTROLBAR_FIRST + 33 + i))
    {
        ERRLOG0(_T("Failed to create mybar\n"));
        return -1;      // fail to create
    }

    m_wndPropertyBar.SetBarStyle(m_wndPropertyBar.GetBarStyle() |
        CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
    m_wndPropertyBar.EnableDocking(CBRS_ALIGN_LEFT);


	DockControlBar(&m_wndObjListBar, AFX_IDW_DOCKBAR_LEFT);

	DockControlBar(&m_wndPropertyBar, AFX_IDW_DOCKBAR_LEFT);

	CSizingControlBar::GlobalLoadState(_T("BarState"));
	LoadBarState(_T("BarState"));

	// CMDITabs must be createt at last to ensure correct layout!!!
	// ------------------------------------------------------------
	DWORD dwStyle = (m_bTop ? MT_TOP : 0) | (m_bImages ? MT_IMAGES : 0);
	m_wndMDITabs.Create(this, dwStyle);

	ShowWindow(SW_SHOWMAXIMIZED);

	return 0;
}

void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
  CMDIFrameWnd::OnUpdateFrameTitle(bAddToTitle);

  m_wndMDITabs.Update(); // sync the mditabctrl with all views
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnDestroy() 
{
	CSizingControlBar::GlobalSaveState(_T("BarState"));
    SaveBarState(_T("BarState"));

	CMDIFrameWnd::OnDestroy();
}

void CMainFrame::OnSaveSkin() 
{
	g_skinPrj.Save();
/*	CFileDialog ofdlg(FALSE, 0, g_Profile.GetProfileString(_T("LastOpenSknFile"), _T("")), OFN_FILEMUSTEXIST, _T("Skin files(*.xml)|*.xml|All files (*.*)|*.*||"), this);
	if (ofdlg.DoModal() != IDOK)
		return;

	g_Profile.WriteProfileString(_T("LastOpenSknFile"), ofdlg.GetPathName());

	g_skinPrj.Save(ofdlg.GetPathName());*/
}

void CMainFrame::OnViewObjlistBar() 
{
	if (!m_wndObjListBar.IsVisible())
		ShowControlBar(&m_wndObjListBar, TRUE, FALSE);
	else
		ShowControlBar(&m_wndObjListBar, FALSE, FALSE);
}

void CMainFrame::OnViewPropertyBar() 
{
	if (!m_wndPropertyBar.IsVisible())
		ShowControlBar(&m_wndPropertyBar, TRUE, FALSE);
	else
		ShowControlBar(&m_wndPropertyBar, FALSE, FALSE);
}
