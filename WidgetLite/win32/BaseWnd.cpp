// BaseWnd.cpp: implementation of the CBaseWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BaseWnd.h"

#define BASEWNDCLASSNAME		_T("CBaseWnd")

#define WM_MOUSEWHEEL                   0x020A

CMLHandleMap<HWND, CBaseWnd>		g_mapWnd;


LRESULT CALLBACK BaseWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// fetch out the CBaseWnd *
#ifdef _WINCE_
	if (uMsg == WM_CREATE)
#else
	if (uMsg == WM_CREATE || uMsg == WM_NCCREATE)
#endif
	{
		CREATESTRUCT	*cs = (CREATESTRUCT *)lParam;
		assert(cs->lpCreateParams != NULL);
		CBaseWnd		*pWnd;

		// stash pointer to self
		pWnd = (CBaseWnd *)cs->lpCreateParams;
		if (pWnd)
			pWnd->Attach(hWnd);
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)(cs->lpCreateParams));
	}
	else if (uMsg == WM_INITDIALOG)
	{
		CBaseWnd		*pWnd;

		pWnd = (CBaseWnd *)lParam;
		if (pWnd)
			pWnd->Attach(hWnd);

		// stash pointer to self
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)lParam);
	}

	// pass the messages into the BaseWnd
	CBaseWnd	*pBaseWnd = (CBaseWnd *)GetWindowLong(hWnd, GWL_USERDATA);

	if (pBaseWnd == NULL)
	{
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	else
	{
		// ASSERT(pBaseWnd->GetHandle());
		return pBaseWnd->WndProc(uMsg, wParam, lParam);
	}
}

#ifdef _WINCE_
BOOL MLRegisterWndClass(HINSTANCE hInstance, LPCTSTR szClassName)
{
	WNDCLASS wc;

	if (GetClassInfo(hInstance, szClassName, &wc))
		return TRUE;

	// regiszter pane class
	wc.style		= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc	= (WNDPROC)BaseWndProc;
	wc.cbClsExtra	= 0;
	wc.cbWndExtra	= 0;
	wc.hInstance	= hInstance;
	wc.hIcon		= NULL;
	wc.hCursor		= LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
	wc.hbrBackground= (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName	= NULL;
	wc.lpszClassName= szClassName;

	int r = RegisterClass(&wc);
	if (r == 0)
	{
		int res = GetLastError();
		if (res != ERROR_CLASS_ALREADY_EXISTS)
		{
			// LOG1(_T("Failed to register class, err %d"), res);
			return FALSE;
		}
	}

	return TRUE;
}
#else
BOOL MLRegisterWndClass(HINSTANCE hInstance, LPCTSTR szClassName)
{
	WNDCLASSEX wc;

	wc.cbSize = sizeof(wc);

	if (GetClassInfoEx(hInstance, szClassName, &wc))
		return TRUE;
	
	// regiszter pane class
	wc.style		= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc	= (WNDPROC)BaseWndProc;
	wc.cbClsExtra	= 0;
	wc.cbWndExtra	= 0;
	wc.hInstance	= hInstance;
	wc.hIcon		= NULL;
	wc.hIconSm		= NULL;
	wc.hCursor		= LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
	wc.hbrBackground= (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName	= NULL;
	wc.lpszClassName= szClassName;

	int r = RegisterClassEx(&wc);
	if (r == 0)
	{
		int res = GetLastError();
		if (res != ERROR_CLASS_ALREADY_EXISTS)
		{
			// LOG1(_T("Failed to register class, err %d"), res);
			return FALSE;
		}
	}

	return TRUE;
}
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBaseWnd::CBaseWnd()
{
	m_hWnd = NULL;
	m_WndSizeMode = (WndSizeMode)-1;
}

CBaseWnd::~CBaseWnd()
{

}


BOOL CBaseWnd::Create(LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent, DWORD dwStyle, int nID)
{
	return CreateEx(BASEWNDCLASSNAME, szCaption, x, y, nWidth, nHeight, pWndParent, dwStyle, nID);
}

BOOL CBaseWnd::CreateEx(LPCTSTR szClassName, LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent, DWORD dwStyle, DWORD dwExStyle, int nID)
{
	if (!MLRegisterWndClass(GetAppInstance(), szClassName))
		return FALSE;

	ASSERT(m_hWnd == NULL);

	// ´´½¨´°¿Ú
	m_hWnd = CreateWindowEx(
		dwExStyle,						// extend styles
		szClassName,			// window class name
		szCaption, 				// window title
		dwStyle,				// window style
		x, y,					// screen position
		nWidth, nHeight,		// width & height of window
		pWndParent ? pWndParent->m_hWnd : NULL,	// parent window
		(HMENU)nID,					// menu or window id
		GetAppInstance(),		// hInstance
		(LPVOID)this);			// this for window creation data

	return m_hWnd != NULL;
}

bool CBaseWnd::CreateForSkin(LPCTSTR szClassName, LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent, bool bToolWindow, bool bTopmost, bool bVisible)
{
	DWORD	dwStyle = DS_NOIDLEMSG | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_MAXIMIZEBOX;
	DWORD	dwExStyle = 0;
	if (bTopmost)
		dwExStyle |= WS_EX_TOPMOST;
	if (bToolWindow)
		dwExStyle |= WS_EX_TOOLWINDOW;
	if (bVisible)
		dwStyle |= WS_VISIBLE;

	// WS_THICKFRAME will cause window flashing on vista
	if (!CreateEx(szClassName, szCaption,
		x, y, nWidth, nHeight,
		pWndParent,
		dwStyle, dwExStyle))
		return false;

	return true;
}

bool CBaseWnd::Destroy()
{
	return Tobool(::DestroyWindow(m_hWnd));
}

void CBaseWnd::PostDestroy()
{
	PostMessage(m_hWnd, WM_CLOSE, 0, 0);
}

void CBaseWnd::OnClose()
{
	Destroy();
}

bool CBaseWnd::OnCreate()
{
	OnLanguageChanged();

	return true;
}

void CBaseWnd::OnDestroy()
{
	if (m_hWnd)
	{
		SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)0);
		DefWindowProc(m_hWnd, WM_DESTROY, 0, 0);
		m_hWnd = NULL;
	}
}

void CBaseWnd::OnPaint(CMLGraphics *pGraph)
{
}

void CBaseWnd::OnLanguageChanged()
{
}

void CBaseWnd::ActivateWindow()
{
#ifdef _WINCE_
	if (::GetForegroundWindow() != m_hWnd)
		SetForegroundWindow(m_hWnd);
	if (IsWindowEnabled(m_hWnd))
		SetActiveWindow(m_hWnd);
#else
	DWORD       dwTimeOutPrev = 0;

	SendMessage(m_hWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
	::ShowWindow(m_hWnd, SW_SHOW);

	if (!IsTopmost())
	{
		::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}
	// save old
	SystemParametersInfo(/*SPI_GETFOREGROUNDLOCKTIMEOUT*/0x2000, 0, (LPVOID)&dwTimeOutPrev, 0/* SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE*/);
	// set new
	SystemParametersInfo(/*SPI_SETFOREGROUNDLOCKTIMEOUT*/0x2001, 0, (LPVOID)0, SPIF_SENDWININICHANGE/* | SPIF_UPDATEINIFILE*/);

	if (::GetForegroundWindow() != m_hWnd)
		SetForegroundWindow(m_hWnd);
	if (IsWindowEnabled(m_hWnd))
		SetActiveWindow(m_hWnd);

	// restore old
	SystemParametersInfo(/*SPI_SETFOREGROUNDLOCKTIMEOUT*/0x2001, dwTimeOutPrev, (LPVOID)0, SPIF_SENDWININICHANGE/* | SPIF_UPDATEINIFILE*/);
#endif
}

BOOL CBaseWnd::ShowWindow(int nCmdShow)
{
	return ::ShowWindow(m_hWnd, nCmdShow);
}

UINT CBaseWnd::GetDlgItemText(int nIDItem, tstring &str)
{
	int		nLen;

	nLen = SendDlgItemMessage(m_hWnd, nIDItem, WM_GETTEXTLENGTH, 0, 0);
	if (nLen > 0)
	{
		str.resize(nLen + 1);
		nLen = ::GetDlgItemText(m_hWnd, nIDItem, (LPTSTR)str.c_str(), str.capacity());

		// Under certain conditions, the DefWindowProc function returns 
		// a value that is larger than the actual length of the text
		// So, must call str.resize again.
		str.resize(nLen);
	}
	else
		str.resize(0);

	return nLen;
}

bool CBaseWnd::GetDlgItemRect(int nIDItem, RECT* lpRect)
{
	HWND	hWnd = ::GetDlgItem(m_hWnd, nIDItem);
	if (!hWnd)
		return false;

	return Tobool(::GetWindowRect(hWnd, lpRect));
}

void CBaseWnd::ScreenToClient(RECT &rc)
{
	::ScreenToClient(m_hWnd, (LPPOINT)&rc.left);
	::ScreenToClient(m_hWnd, (LPPOINT)&rc.right);
}

void CBaseWnd::ClientToScreen(RECT &rc)
{
	::ClientToScreen(m_hWnd, (LPPOINT)&rc.left);
	::ClientToScreen(m_hWnd, (LPPOINT)&rc.right);
}

void CBaseWnd::ScreenToClient(POINT &pt)
{
	::ScreenToClient(m_hWnd, &pt);
}

void CBaseWnd::ClientToScreen(POINT &pt)
{
	::ClientToScreen(m_hWnd, &pt);
}

bool CBaseWnd::GetWindowRect(RECT* lpRect)
{
	return Tobool(::GetWindowRect(m_hWnd, lpRect));
}

bool CBaseWnd::GetClientRect(RECT* lpRect)
{
	return Tobool(::GetClientRect(m_hWnd, lpRect));
}

void CBaseWnd::SetParent(CBaseWnd *pWndParent)
{
	if (pWndParent)
	{
		DWORD	dwStyle;
		dwStyle = GetWindowLong(m_hWnd, GWL_STYLE);
		SetWindowLong(m_hWnd, GWL_STYLE, dwStyle | WS_CHILD);
		::SetParent(m_hWnd, pWndParent->GetHandle());
	}
	else
	{
		DWORD	dwStyle;
		dwStyle = GetWindowLong(m_hWnd, GWL_STYLE);
		SetWindowLong(m_hWnd, GWL_STYLE, dwStyle & ~WS_CHILD);
		::SetParent(m_hWnd, NULL);
	}
}

CBaseWnd *CBaseWnd::GetParent()
{
	return FromHandle(::GetParent(m_hWnd));
}

bool CBaseWnd::SetTimer(UINT nTimerId, UINT nElapse)
{
	return Tobool(::SetTimer(m_hWnd, nTimerId, nElapse, NULL));
}

void CBaseWnd::KillTimer(UINT nTimerId)
{
	::KillTimer(m_hWnd, nTimerId);
}

int CBaseWnd::GetWindowText(tstring &str)
{
	int		nLen;

	nLen = SendMessage(m_hWnd, WM_GETTEXTLENGTH, 0, 0);
	if (nLen > 0)
	{
		str.resize(nLen + 1);
		nLen = ::GetWindowText(m_hWnd, (LPTSTR)str.c_str(), str.capacity());

		// Under certain conditions, the DefWindowProc function returns 
		// a value that is larger than the actual length of the text
		// So, must call str.resize again.
		str.resize(nLen);
	}
	else
		str.resize(0);

	return nLen;
}

bool CBaseWnd::SetWndCursor(CCursor *pCursor)
{
	SetClassLong(m_hWnd, GCL_HCURSOR, (LONG)pCursor->GetHandle());
	return true;
}

bool CBaseWnd::SetFocus()
{
	return ::SetFocus(m_hWnd) != NULL;
}

bool CBaseWnd::SetWindowRgn(CRegion *pRgn)
{
	if (pRgn == NULL)
		return Tobool(::SetWindowRgn(m_hWnd, NULL, true));
	else
		return Tobool(::SetWindowRgn(m_hWnd, pRgn->GetHandle(), true));
}

bool CBaseWnd::SetCapture()
{
	::SetCapture(m_hWnd);

	return true;
}

void CBaseWnd::ReleaseCapture()
{
	::ReleaseCapture();
}

CMLGraphics *CBaseWnd::GetGraphics()
{
	HDC				hdc;
	CMLGraphics		*pGraph;

	hdc = GetDC(m_hWnd);

	pGraph = new CMLGraphics;
	pGraph->Attach(hdc);

	return pGraph;
}

void CBaseWnd::ReleaseGraphics(CMLGraphics *pGraph)
{
	ReleaseDC(m_hWnd, pGraph->GetHandle());

	delete pGraph;
}

BOOL CBaseWnd::InvalidateRect(const RECT *lpRect, BOOL bErase)
{
	return ::InvalidateRect(m_hWnd, lpRect, bErase);
}

bool CBaseWnd::IsChild()
{
	return Tobool(::IsChildWnd(m_hWnd));
}

bool CBaseWnd::IsMouseCaptured()
{
	return GetCapture() == m_hWnd;
}

bool CBaseWnd::IsIconic()
{
#ifdef _WINCE_
	return false;
#else
	return Tobool(::IsIconic(m_hWnd));
#endif
}

bool CBaseWnd::IsWindow()
{
	return Tobool(::IsWindow(m_hWnd));
}

bool CBaseWnd::IsVisible()
{
	return Tobool(::IsWindowVisible(m_hWnd));
}

bool CBaseWnd::IsTopmost()
{
	DWORD	dwStyleEx;

	dwStyleEx = (DWORD)::GetWindowLong(m_hWnd, GWL_EXSTYLE);

	return ((dwStyleEx & WS_EX_TOPMOST) == WS_EX_TOPMOST);
}

void CBaseWnd::SetTopmost(bool bTopmost)
{
	DWORD	dwStyleEx;
	if (bTopmost)
	{
		dwStyleEx = (DWORD)::GetWindowLong(m_hWnd, GWL_EXSTYLE);
		dwStyleEx |= WS_EX_TOPMOST;
		::SetWindowLong(m_hWnd, GWL_EXSTYLE, dwStyleEx);
		::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |SWP_NOACTIVATE);
	}
	else
	{
		dwStyleEx = (DWORD)::GetWindowLong(m_hWnd, GWL_EXSTYLE);
		dwStyleEx &= ~WS_EX_TOPMOST;
		::SetWindowLong(m_hWnd, GWL_EXSTYLE, dwStyleEx);
		::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |SWP_NOACTIVATE);
	}
}

bool CBaseWnd::IsToolWindow()
{
	return Tobool(::IsToolWindow(m_hWnd));
}

void CBaseWnd::SetToolWindow(bool bToolWindow)
{
	if (bToolWindow)
		ShowAsToolWindow(m_hWnd);
	else
		ShowAsAppWindow(m_hWnd);
}

bool CBaseWnd::SetForeground()
{
	return Tobool(::SetForegroundWindow(m_hWnd));
}

void CBaseWnd::SetWindowPos(int x, int y)
{
	::SetWindowPos(m_hWnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREPOSITION);
}

void CBaseWnd::SetWindowPosSafely(int x, int y)
{
	if (IsChild())
	{
		HWND	hWndParent;
		hWndParent = ::GetParent(m_hWnd);
		if (hWndParent)
		{
			POINT	pt;
			pt.x = x;
			pt.y = y;
			::ScreenToClient(hWndParent, &pt);
			x = pt.x;
			y = pt.y;
		}
	}
	::SetWindowPos(m_hWnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREPOSITION);
}

bool CBaseWnd::MoveWindow(int X, int Y, int nWidth, int nHeight, bool bRepaint)
{
	return Tobool(::MoveWindow(m_hWnd, X, Y, nWidth, nHeight, bRepaint));
}

bool CBaseWnd::MoveWindowSafely(int X, int Y, int nWidth, int nHeight, bool bRepaint)
{
	return Tobool(::MoveWindowSafely(m_hWnd, X, Y, nWidth, nHeight, bRepaint));
}

int CBaseWnd::MessageOut(LPCTSTR lpText, UINT uType, LPCTSTR lpCaption)
{
	TCHAR	szCap[512];
	if (lpCaption == NULL)
	{
		EmptyStr(szCap);
		GetWindowText(szCap, CountOf(szCap));
		lpCaption = szCap;
	}

	return MessageBox(m_hWnd, lpText, lpCaption, uType);
}

bool CBaseWnd::ReplaceChildPos(int nIDChildSrcPos, CBaseWnd *pChildNew)
{
	CRect       rcPos;

	GetDlgItemRect(nIDChildSrcPos, &rcPos);
    ScreenToClient(rcPos);

	pChildNew->MoveWindow(rcPos.left, rcPos.top, rcPos.Width(), rcPos.Height(), TRUE);

	return true;
}

void CBaseWnd::PostUserMessage(int nMessageID, LPARAM param)
{
	PostMessage(m_hWnd, ML_WM_USER, nMessageID, param);
}

void CBaseWnd::SendUserMessage(int nMessageID, LPARAM param)
{
	SendMessage(m_hWnd, ML_WM_USER, nMessageID, param);
}

LRESULT CBaseWnd::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	ASSERT(m_hWnd);
	switch (message)
	{
	case ML_WM_LANGUAGE_CHANGED:
		OnLanguageChanged();
		break;
	case ML_WM_USER:
		OnUserMessage(wParam, lParam);
		break;
	case WM_ACTIVATE:
		{
			WORD		wActived;
			// BOOL		bMinimized;
			// HWND		hWndOld;

			wActived = LOWORD(wParam);
			// bMinimized = HIWORD(wParam);
			// hWndOld = (HWND)lParam;

			OnActivate(Tobool(wActived));
		}
		break;
	case WM_CREATE:
		if (OnCreate())
			return 0;
		else
			return -1;
	case WM_CLOSE:
		OnClose();
		return 0;
	case WM_COMMAND:
		{
			UINT uId    = LOWORD(wParam);
			UINT uEvent = HIWORD(wParam);
			OnCommand(uId, uEvent);
		}
		break;
	case WM_CONTEXTMENU:
		{
			//			int xPos = LOWORD(lParam);
			//			int yPos = HIWORD(lParam);
			if (lParam == -1)
			{
				POINT	pt;
				GetCursorPos(&pt);
				OnContexMenu(pt.x, pt.y);
			}
			else
				OnContexMenu(LOWORD(lParam), HIWORD(lParam));
		}
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT		ps;
			HDC				hdc;
			hdc = BeginPaint(m_hWnd, &ps);
			CMLGraphics		*pGraph;

			pGraph = new CMLGraphics;
			pGraph->Attach(hdc);

			OnPaint(pGraph);

			delete pGraph;

			EndPaint(m_hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		{
			OnDestroy();
			if (m_hWnd)
			{
				SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)0);
				DefWindowProc(m_hWnd, message, wParam, lParam);
				m_hWnd = NULL;
			}
			// g_mapWnd.remove(m_hWnd);
		}
		return 0;
	case WM_DROPFILES:
		OnDropFiles((HDROP)wParam);
		break;
	case WM_HSCROLL:
		OnHScroll(LOWORD(wParam), HIWORD(wParam), NULL);
		break;
	case WM_KEYDOWN: // pass keyboard messages to main winamp window (for processing)
		OnKeyDown(wParam, lParam);
		break;
	case WM_KEYUP:
		OnKeyUp(wParam, lParam);
		break;
	case WM_CHAR:
		//	case WM_UNICHAR:
		OnChar(wParam);
		break;
	case WM_LBUTTONDOWN:
		OnLButtonDown((UINT)wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
		break;
	case WM_LBUTTONDBLCLK:
		OnLButtonDblClk((UINT)wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
		break;
	case WM_LBUTTONUP:
		OnLButtonUp((UINT)wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
		break;
	case WM_RBUTTONDOWN:
		OnRButtonDown((UINT)wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
		break;
	case WM_RBUTTONUP:
		OnRButtonUp((UINT)wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
		break;
	case WM_MOUSEMOVE:
		OnMouseMove((UINT)wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
		break;
	case WM_MOUSEWHEEL:
		{
			OnMouseWheel(short(HIWORD(wParam)), LOWORD(wParam), CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)));
		}
		break;
	case WM_MOVE:
		OnMove(LOWORD(lParam), HIWORD(lParam));
		return 0;
	case WM_KILLFOCUS:
		OnKillFocus();
		break;
	case WM_SETFOCUS:
		OnSetFocus();
		break;
	case WM_SIZE:
		if (wParam == SIZE_RESTORED)
		{
			OnSize(LOWORD(lParam), HIWORD(lParam));
			if (m_WndSizeMode != WndSizeMode_Normal)
			{
				m_WndSizeMode = WndSizeMode_Normal;
				OnSizeModeChanged(m_WndSizeMode);
			}
		}
		else if (wParam == SIZE_MINIMIZED)
		{
			m_WndSizeMode = WndSizeMode_Minimized;
			OnSizeModeChanged(m_WndSizeMode);
		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			OnSize(LOWORD(lParam), HIWORD(lParam));
			m_WndSizeMode = WndSizeMode_Maximized;
			OnSizeModeChanged(m_WndSizeMode);
		}
		break;
	case WM_VSCROLL:
		{
			// CSkinVScrollBarOSStyle
			if (lParam)
			{
				IScrollNotify	*pNotify;
				pNotify = (IScrollNotify *)GetWindowLong((HWND)lParam, GWL_USERDATA);
				if (pNotify)
				{
					pNotify->OnVScroll(LOWORD(wParam), HIWORD(wParam), NULL);
					return 0;
				}
			}
			OnVScroll(LOWORD(wParam), HIWORD(wParam), NULL);
		}
		break;
	case WM_TIMER:
		OnTimer((UINT)wParam);
		break;
	default:
		return DefWindowProc(m_hWnd, message, wParam, lParam);
	}
	return 0;
}

void CBaseWnd::SetTransparent(BYTE nAlpha, bool bClickThrough)
{
	// If it's child old, ignore this.
	if (::GetParent(m_hWnd))
		return;

	m_nAlpha = nAlpha;
	m_bClickThrough = bClickThrough;

	if (m_bTranslucencyLayered)
	{
		InvalidateRect(NULL, true);
		return;
	}

	COLORREF		clrTrans = RGB(0, 0, 0);
	DWORD			dwFlags = 0;

	if (nAlpha < 255)
		dwFlags = LWA_ALPHA;

	if (IsClickThrough())
	{
		SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
	}
	else
	{
		SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
	}

	if (dwFlags != 0 || IsClickThrough())
		::SetLayeredWindow(m_hWnd, clrTrans, nAlpha, dwFlags);
	else
		::UnSetLayeredWindow(m_hWnd);
}

BOOL CBaseWnd::UpdateLayeredWindowUsingMemGraph(CRawGraph *pGraph)
{
// 	int		w, h;
// 	HBITMAP	hBmp;
// 
// 	hBmp = Load32Bitbmp(_T("c:\\main??.png"), w, h);
// 	TestUpdateLayeredWindow(m_hWnd, hBmp, w, h);
// 	return true;
/*	CMemAggGraphics		*pAgg;

	pAgg = (CMemAggGraphics *)pGraph;

	TestUpdateLayeredWindow(m_hWnd, pAgg->GetMemBITMAP(), pAgg->Width(), pAgg->Height());

	return true;*/

	HDC		hdc, hdcSrc;
	CRect	rcWnd;
	POINT	ptSrc, ptDest;
	SIZE	sizeWnd;
	DWORD	dwStyle, dwStyleNew;
	BLENDFUNCTION	blend;

	hdc = GetDC(m_hWnd);

	hdcSrc = pGraph->GetHandle();

	// ????WS_EX_LAYERED??
	dwStyle = GetWindowLong(m_hWnd, GWL_EXSTYLE);
	dwStyleNew = dwStyle | WS_EX_LAYERED;
	if (IsClickThrough())
		dwStyleNew |= WS_EX_TRANSPARENT;
	else
		dwStyleNew &= ~WS_EX_TRANSPARENT;
	if (dwStyle != dwStyleNew)
		SetWindowLong(m_hWnd, GWL_EXSTYLE, dwStyleNew);

	GetWindowRect(&rcWnd);

	blend.AlphaFormat = AC_SRC_ALPHA;
	blend.BlendOp = AC_SRC_OVER;
	blend.BlendFlags = 0;
	blend.SourceConstantAlpha = m_nAlpha;

	ptSrc.x = ptSrc.y = 0;
	ptDest.x = rcWnd.left;
	ptDest.y = rcWnd.top;
	sizeWnd.cx = rcWnd.Width();
	sizeWnd.cy = rcWnd.Height();

	BOOL bRet = UpdateLayeredWindow(m_hWnd, hdc, &ptDest, &sizeWnd, hdcSrc, &ptSrc, 0, &blend, ULW_ALPHA);
	//BOOL bRet = UpdateLayeredWindow(m_hWnd, NULL, NULL, NULL, hdcSrc, &ptSrc, 0, &blend, ULW_ALPHA);

	ReleaseDC(m_hWnd, hdc);

	return bRet;
}

CBaseWnd *CBaseWnd::FromHandle(HWND hWnd)
{
	ASSERT(hWnd != NULL);
	// if (hWnd == NULL)
	// 	return NULL;
	return g_mapWnd.fromhandle(hWnd);
}

void CBaseWnd::Attach(HWND hWnd)
{
	m_hWnd = hWnd;
}

void CBaseWnd::Detach()
{
	m_hWnd = NULL;
}
