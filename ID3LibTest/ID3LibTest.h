// ID3LibTest.h : main header file for the ID3LIBTEST application
//

#if !defined(AFX_ID3LIBTEST_H__0DC65F21_69C0_4D07_8E4A_0D39743248E3__INCLUDED_)
#define AFX_ID3LIBTEST_H__0DC65F21_69C0_4D07_8E4A_0D39743248E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CID3LibTestApp:
// See ID3LibTest.cpp for the implementation of this class
//

class CID3LibTestApp : public CWinApp
{
public:
	CID3LibTestApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CID3LibTestApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CID3LibTestApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ID3LIBTEST_H__0DC65F21_69C0_4D07_8E4A_0D39743248E3__INCLUDED_)
