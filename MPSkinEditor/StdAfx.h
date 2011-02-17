// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__4CFAC8BC_F312_4AF3_A484_6B6E5734E35C__INCLUDED_)
#define AFX_STDAFX_H__4CFAC8BC_F312_4AF3_A484_6B6E5734E35C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _WIN32_WINNT	0x0400

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


#include <assert.h>

#define WM_MOUSEWHEEL                   0x020A

#include "../base/base.h"
#include "../Utils/Utils.h"
#include "../gfx/gfx.h"
#include "../widget/widget.h"
#include "../Skin/MLSkin.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__4CFAC8BC_F312_4AF3_A484_6B6E5734E35C__INCLUDED_)
