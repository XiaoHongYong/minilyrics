/////////////////////////////////////////////////////////////
// My Library like mfc
#ifndef _OUR_SDK_WINDOWABOUT_2001_4_25_HEADER_INCLUDED
#define _OUR_SDK_WINDOWABOUT_2001_4_25_HEADER_INCLUDED

////////////////////////////////////////////////////////////
//
BOOL IsTopmostWindow(HWND hWnd);

//
// remove it's WS_EX_APPWINDOW AND ADD WS_EX_TOOLWINDOW
BOOL ShowAsToolWindow(HWND hWnd);

// add it's WS_EX_APPWINDOW AND remove WS_EX_TOOLWINDOW
BOOL ShowAsAppWindow(HWND hWnd);

BOOL IsToolWindow(HWND hWnd);

void TopmostWindow(HWND hwnd, BOOL bTopmost);

// PURPOSE:
//      Activate the windows
void ActivateWindow(HWND hWnd);

BOOL IsChildWnd(HWND hWnd);

BOOL MoveWindowSafely(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint);

bool EnableDlgItem(HWND hWnd, int nIDItem, bool bEnable);
bool ShowDlgItem(HWND hWnd, int nIDItem, int nCmdShow);
#define SetDlgItemFocus(hWnd, nIDItem)	::SetFocus(::GetDlgItem(hWnd, nIDItem))

#ifndef _WINCE_

BOOL BrowserForFolder(HWND hWnd, LPCTSTR szTitle, LPTSTR szPath, LPCTSTR szRootFoler = NULL);

#if !defined(AC_SRC_ALPHA)
#define AC_SRC_ALPHA                0x01
#endif

BOOL WinAlphaBlend(
  HDC hdcDest,                 // handle to destination DC
  int nXOriginDest,            // x-coord of upper-left corner
  int nYOriginDest,            // y-coord of upper-left corner
  int nWidthDest,              // destination width
  int nHeightDest,             // destination height
  HDC hdcSrc,                  // handle to source DC
  int nXOriginSrc,             // x-coord of upper-left corner
  int nYOriginSrc,             // y-coord of upper-left corner
  int nWidthSrc,               // source width
  int nHeightSrc,              // source height
  BLENDFUNCTION blendFunction  // alpha-blending function
);

/*******************************************************************/
// 下面的几行是Win2000中的透明窗口定义用的
#define WS_EX_LAYERED           0x00080000

#define LWA_COLORKEY            0x00000001
#define LWA_ALPHA               0x00000002


#define ULW_COLORKEY            0x00000001
#define ULW_ALPHA               0x00000002
#define ULW_OPAQUE              0x00000004

// COMMENT:
//		判断系统是否支持半透明窗口
//		只有Windows2000或以上才支持
BOOL IsLayeredWndSupported();

//	COMMENT:
//		sets the opacity and transparency color key of a layered window
//	INPUT:
//		crKey,		specifies the color key
//		bAlpha,		value for the blend function, When bAlpha is 0, 
//					the window is completely transparent. 
//					When bAlpha is 255, the window is opaque. 
//		dwFlags		Specifies an action to take. This parameter can be one or 
//					more of the following values. Value Meaning 
//					LWA_COLORKEY Use crKey as the transparency color.  
//					LWA_ALPHA Use bAlpha to determine the opacity of the layered window 
BOOL SetLayeredWindow(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);

BOOL SetLayeredWindowAttributes(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);

BOOL UpdateLayeredWindow(
	HWND hwnd,             // handle to layered window
	HDC hdcDst,            // handle to screen DC
	POINT *pptDst,         // new screen position
	SIZE *psize,           // new size of the layered window
	HDC hdcSrc,            // handle to surface DC
	POINT *pptSrc,         // layer position
	COLORREF crKey,        // color key
	BLENDFUNCTION *pblend, // blend function
	DWORD dwFlags          // options
);

void UnSetLayeredWindow(HWND hWnd);
#endif

/*******************************************************************/


#endif  //_OUR_SDK_WINDOWABOUT_2001_4_25_HEADER_INCLUDED