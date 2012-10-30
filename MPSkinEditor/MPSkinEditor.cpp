// MPSkinEditor.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "MPSkinEditor.h"

#include "MainFrm.h"
#include "SkinEditorFrame.h"
#include "DynCmdFrame.h"
#include "DynCtrlFrame.h"
#include "DlgOpenSkin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMPSkinEditorApp

BEGIN_MESSAGE_MAP(CMPSkinEditorApp, CWinApp)
	//{{AFX_MSG_MAP(CMPSkinEditorApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(IDC_OPEN_SKIN, OnOpenSkin)
	ON_COMMAND(IDC_SKIN_WND_NEW, OnSkinWndNew)
	ON_COMMAND(IDC_SKIN_WND_DUP, OnSkinWndDup)
	ON_COMMAND(IDC_SKIN_WND_DEL, OnSkinWndDel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMPSkinEditorApp construction

CMPSkinEditorApp::CMPSkinEditorApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMPSkinEditorApp object

CMPSkinEditorApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CMPSkinEditorApp initialization

BOOL CMPSkinEditorApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	InitBaseFrameWork(AfxGetInstanceHandle(), _T("ZPSkinEditorLog.txt"), _T("ZPSkinEditor.ini"), _T("ZPSkinEditor"));

/*
	{
		CSimpleXML	xml;
		xml.ParseFile(_T("c:\\1.txt"));
	}
	{
		CSimpleXML	xml;
		xml.ParseFile(_T("c:\\2.txt"));
	}
	{
		CSimpleXML	xml;
		xml.ParseFile(_T("c:\\3.txt"));
	}
	{
		CSimpleXML	xml;
		xml.ParseFile(_T("c:\\4.txt"));
	}
*/

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));


	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object.

	CMDIFrameWnd* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;

	// create main MDI frame window
	if (!pFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;

	// try to load shared MDI menus and accelerator table
	//TODO: add additional member variables and load calls for
	//	additional menu types your application may need. 

	HINSTANCE hInst = AfxGetResourceHandle();
	m_hMDIMenu  = ::LoadMenu(hInst, MAKEINTRESOURCE(IDR_MPSKINTYPE));
	m_hMDIAccel = ::LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_MPSKINTYPE));
	m_hDynCmdMenu  = ::LoadMenu(hInst, MAKEINTRESOURCE(IDR_DYNCMD));



	// The main window has been initialized, so show and update it.
	pFrame->ShowWindow(m_nCmdShow);
	pFrame->UpdateWindow();

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMPSkinEditorApp message handlers

int CMPSkinEditorApp::ExitInstance() 
{
	//TODO: handle additional resources you may have added
	if (m_hMDIMenu != NULL)
		FreeResource(m_hMDIMenu);
	if (m_hMDIAccel != NULL)
		FreeResource(m_hMDIAccel);
	if (m_hDynCmdMenu != NULL)
		FreeResource(m_hDynCmdMenu);

	return CWinApp::ExitInstance();
}

void CMPSkinEditorApp::OnFileNew() 
{
}



/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CMPSkinEditorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CMPSkinEditorApp message handlers


void CMPSkinEditorApp::OnOpenSkin() 
{
	tstring		strSkinFile;
	int			nRet;
/*	CFileDialog ofdlg(TRUE, 0, g_Profile.GetString(_T("LastOpenSknFile"), _T("")), OFN_FILEMUSTEXIST, _T("Skin files(*.xml)|*.xml|All files (*.*)|*.*||"), m_pMainWnd);

	if (ofdlg.DoModal() != IDOK)
		return;

	strSkinFile = ofdlg.GetPathName();
	g_Profile.WriteString(_T("LastOpenSknFile"), strSkinFile.c_str());*/

	CDlgOpenSkin	dlg;

	if (dlg.DoModal() != IDOK)
		return;

	CSkinFileXML		*pskinFile;
	vector<tstring>		vSkinWndNames;
	pskinFile = g_skinFactory.GetSkinFile();
	CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);

	// close opened skins.
	CMDIChildWnd *pChild = pFrame->MDIGetActive();
	while (pChild)
	{
		pChild->DestroyWindow();
		pChild = pFrame->MDIGetActive();
	}
	g_skinPrj.Close();

	nRet = g_skinFactory.OpenSkinEx(dlg.m_strSelSkin.c_str());
	if (nRet != ERR_OK)
	{
		AfxMessageBox(Error2Str(nRet));
		return;
	}

	pskinFile->EnumSkinWnd(vSkinWndNames);

	for (int i = 0; i < vSkinWndNames.size(); i++)
	{
		// create a new MDI child window
		CSkinEditorFrame *pSEFrame = (CSkinEditorFrame*)pFrame->CreateNewChild(
			RUNTIME_CLASS(CSkinEditorFrame), IDR_MPSKINTYPE, m_hMDIMenu, m_hMDIAccel);

		pSEFrame->m_wndSkin.OpenSkin(vSkinWndNames[i].c_str());
		pSEFrame->m_wndSkin.SetMainAppWnd(false);
		pSEFrame->SetTitle(vSkinWndNames[i].c_str());
		AfxSetWindowText(pSEFrame->m_hWnd, vSkinWndNames[i].c_str());
		pSEFrame->ShowWindow(SW_MAXIMIZE);
		pSEFrame->OnUIObjFocusChanged();
		pSEFrame->UpdateSkinObjList();
	}

	// Open Dynamic command window
	pFrame->CreateNewChild(
			RUNTIME_CLASS(CDynCmdFrame), IDR_DYNCMD, m_hDynCmdMenu, m_hMDIAccel);

	// Open Dynamic control window
	pFrame->CreateNewChild(
			RUNTIME_CLASS(CDynCtrlFrame), IDR_DYNCTRL, m_hDynCmdMenu, m_hMDIAccel);

	pFrame->GetMDITab()->Update();
}

void CMPSkinEditorApp::OnSkinWndNew() 
{
	tstring		strName;
	tstring		strNamePrefix = _T("Untitled Window");
	CSkinFileXML *pSkinFile = g_skinFactory.GetSkinFile();
	CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);

	strName = strNamePrefix;
	for (int i = 1; !pSkinFile->CanUseSkinWndName(strName.c_str()); i++)
		strName = strNamePrefix + CStrPrintf(_T(" %d"), i).c_str();

	CSkinEditorFrame *pSEFrame = (CSkinEditorFrame*)pFrame->CreateNewChild(
		RUNTIME_CLASS(CSkinEditorFrame), IDR_MPSKINTYPE, m_hMDIMenu, m_hMDIAccel);

	pSEFrame->m_wndSkin.NewSkin(strName.c_str());
	pSEFrame->m_wndSkin.SetMainAppWnd(false);
	pSEFrame->SetTitle(strName.c_str());
	AfxSetWindowText(pSEFrame->m_hWnd, strName.c_str());
	pSEFrame->ShowWindow(SW_MAXIMIZE);
	pSEFrame->OnUIObjFocusChanged();
	pSEFrame->UpdateSkinObjList();

	pSkinFile->NewSkinWndNode(strName.c_str());

	pFrame->GetMDITab()->Update();
}

void CMPSkinEditorApp::OnSkinWndDup() 
{
	tstring		strName;
	tstring		strNamePrefix = _T("Copy Of ");
	CSkinFileXML *pSkinFile = g_skinFactory.GetSkinFile();
	CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
	CMDIChildWnd *child = pFrame->MDIGetActive();

	if (child->IsKindOf(&CSkinEditorFrame::classCSkinEditorFrame))
	{
		CSkinEditorFrame *pSEFrameSrc = (CSkinEditorFrame*)child;

		strName = strNamePrefix += pSEFrameSrc->m_wndSkin.GetSkinWndName();
		for (int i = 1; !pSkinFile->CanUseSkinWndName(strName.c_str()); i++)
			strName = strNamePrefix + CStrPrintf(_T(" %d"), i).c_str();

		CSkinEditorFrame *pSEFrame = (CSkinEditorFrame*)pFrame->CreateNewChild(
			RUNTIME_CLASS(CSkinEditorFrame), IDR_MPSKINTYPE, m_hMDIMenu, m_hMDIAccel);

		pSEFrame->m_wndSkin.CopyFrom(strName.c_str(), &(pSEFrameSrc->m_wndSkin));
		pSEFrame->m_wndSkin.SetMainAppWnd(false);
		pSEFrame->SetTitle(strName.c_str());
		AfxSetWindowText(pSEFrame->m_hWnd, strName.c_str());
		pSEFrame->ShowWindow(SW_MAXIMIZE);
		pSEFrame->OnUIObjFocusChanged();
		pSEFrame->UpdateSkinObjList();

		pSkinFile->NewSkinWndNode(strName.c_str());
	}

	pFrame->GetMDITab()->Update();
}

void CMPSkinEditorApp::OnSkinWndDel() 
{
	CMainFrame* pFrame = STATIC_DOWNCAST(CMainFrame, m_pMainWnd);
	CMDIChildWnd *child = pFrame->MDIGetActive();

	if (child->IsKindOf(&CSkinEditorFrame::classCSkinEditorFrame))
	{
		CSkinEditorFrame *pSEFrameSrc = (CSkinEditorFrame*)child;
		tstring			strName;

		strName = pSEFrameSrc->m_wndSkin.GetSkinWndName();

		pSEFrameSrc->MDIDestroy();

		g_skinFactory.GetSkinFile()->RemoveSkinWnd(strName.c_str());

		pFrame->GetMDITab()->Update();
	}
}
