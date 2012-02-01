
// ID3v2TagRemover.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CID3v2TagRemoverApp:
// See ID3v2TagRemover.cpp for the implementation of this class
//

class CID3v2TagRemoverApp : public CWinAppEx
{
public:
	CID3v2TagRemoverApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CID3v2TagRemoverApp theApp;