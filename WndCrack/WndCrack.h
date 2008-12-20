// WndCrack.h : main header file for the WNDCRACK application
//

#if !defined(AFX_WNDCRACK_H__8407EAC4_B4A2_11D4_9F4C_52544C98EE4B__INCLUDED_)
#define AFX_WNDCRACK_H__8407EAC4_B4A2_11D4_9F4C_52544C98EE4B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CWndCrackApp:
// See WndCrack.cpp for the implementation of this class
//

class CWndCrackApp : public CWinApp
{
public:
	CWndCrackApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWndCrackApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CWndCrackApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WNDCRACK_H__8407EAC4_B4A2_11D4_9F4C_52544C98EE4B__INCLUDED_)
