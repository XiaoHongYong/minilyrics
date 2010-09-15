// ID3LibTest.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ID3LibTest.h"
#include "ID3LibTestDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CID3LibTestApp

BEGIN_MESSAGE_MAP(CID3LibTestApp, CWinApp)
	//{{AFX_MSG_MAP(CID3LibTestApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CID3LibTestApp construction

CID3LibTestApp::CID3LibTestApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CID3LibTestApp object

CID3LibTestApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CID3LibTestApp initialization

BOOL CID3LibTestApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	InitBaseFrameWork(AfxGetInstanceHandle(), NULL, _T("Id3libTest.ini"), _T("ID3libTest"));

	CID3LibTestDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
