// BaseWnd.h: interface for the CBaseWnd class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _ML_WINDOW_H_INC_
#define _ML_WINDOW_H_INC_

#include "../widgetif.h"
#include "Cursor.h"

#ifdef _WINCE_
#ifndef DS_NOIDLEMSG
#define DS_NOIDLEMSG	0
#endif
#endif

const int ML_WM_LANGUAGE_CHANGED		= (WM_USER + 1212);
const int ML_LANGUAGE_CHANGED_PARAM		= 172172;
#define ML_WM_USER				(WM_USER + 1213)

LRESULT CALLBACK BaseWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

class CBaseWnd  
{
public:
	CBaseWnd();
	virtual ~CBaseWnd();

public:
	virtual BOOL Create(LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent, DWORD dwStyle = DS_NOIDLEMSG | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX, int nID = ID_UNDEFINE);
	virtual BOOL CreateEx(LPCTSTR szClassName, LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent, DWORD dwStyle = DS_NOIDLEMSG | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX, DWORD dwExStyle = 0, int nID = ID_UNDEFINE);

	virtual bool Destroy();

	void PostDestroy();

	void Attach(CBaseWnd *pWnd) { m_hWnd = pWnd->m_hWnd; }

public:
	//
	// Messages
	//
	virtual void OnActivate(bool bActived) { }
	virtual void OnClose();
	virtual void OnCommand(UINT uID, UINT nNotifyCode) { }
	virtual void OnContexMenu(int xPos, int yPos) { }
	virtual bool OnCreate();
	virtual void OnDestroy();

	virtual void OnDropFiles(HDROP hDrop) { }

	virtual void OnKeyDown(UINT nChar, UINT nFlags) { }
	virtual void OnKeyUp(UINT nChar, UINT nFlags) { }
	virtual void OnChar(UINT nChar) { }
	virtual void OnLButtonUp(UINT nFlags, CPoint point) { }
	virtual void OnLButtonDown(UINT nFlags, CPoint point) { }
	virtual void OnLButtonDblClk(UINT nFlags, CPoint point) { }
	virtual void OnRButtonUp( UINT nFlags, CPoint point ) { }
	virtual void OnRButtonDown( UINT nFlags, CPoint point ) { }
	virtual void OnMouseMove(UINT nFlags, CPoint point) { }
	virtual void OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt) { }

	virtual void OnMove(int x, int y) { }

	virtual void OnPaint(CMLGraphics *pGraph);

	virtual void OnKillFocus() { }
	virtual void OnSetFocus() { }

	virtual void OnSize(int cx, int cy) { }

	virtual void OnSizeModeChanged(WndSizeMode sizeMode) { }

	virtual void OnTimer (UINT nIDEvent) { }

	virtual void OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar) { }
	virtual void OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar) { }

	// Language Changed Notification message
	virtual void OnLanguageChanged();

public:
	//
	// Operations
	//
	void ActivateWindow();

	BOOL ShowWindow(int nCmdShow);

	UINT GetDlgItemText(int nIDItem, LPTSTR szString, int nMaxCount) { return ::GetDlgItemText(m_hWnd, nIDItem, szString, nMaxCount); }
	UINT GetDlgItemText(int nIDItem, tstring &str);
	BOOL SetDlgItemText(int nIDItem, LPCTSTR szString) { return ::SetDlgItemText(m_hWnd, nIDItem, szString); }
	UINT GetDlgItemInt(int nIDItem, BOOL *pTranslated, BOOL bSigned) { return ::GetDlgItemInt(m_hWnd, nIDItem, pTranslated, bSigned); }
	BOOL SetDlgItemInt(int nIDItem, UINT uValue, BOOL bSigned) { return ::SetDlgItemInt(m_hWnd, nIDItem, uValue, bSigned); }
	bool EnableDlgItem(int nIDItem, bool bEnable) { return Tobool(::EnableWindow(::GetDlgItem(m_hWnd, nIDItem), bEnable)); }
	bool EnableWindow(bool bEnable) { return Tobool(::EnableWindow(m_hWnd, bEnable)); }
	bool ShowDlgItem(int nIDItem, int nCmdShow) { return Tobool(::ShowWindow(::GetDlgItem(m_hWnd, nIDItem), nCmdShow)); }
	CBaseWnd *GetDlgItem(int nIDItem) { return FromHandle(::GetDlgItem(m_hWnd, nIDItem)); }
	bool GetDlgItemRect(int nIDItem, RECT* lpRect);

	bool SetTimer(UINT nTimerId, UINT nElapse);
	void KillTimer(UINT nTimerId);

	int GetWindowText(LPTSTR szString, int nMaxCount) { return ::GetWindowText(m_hWnd, szString, nMaxCount); }
	int GetWindowText(tstring &str);
	bool SetWindowText(LPCTSTR szText) { return Tobool(::SetWindowText(m_hWnd, szText)); }

	bool SetWndCursor(CCursor *pCursor);

	bool SetFocus();

	bool SetWindowRgn(CRegion *pRgn);

	bool SetCapture();
	void ReleaseCapture();

	void ScreenToClient(RECT &rc);
	void ClientToScreen(RECT &rc);

	void ScreenToClient(POINT &pt);
	void ClientToScreen(POINT &pt);

	bool GetWindowRect(RECT* lpRect);
	bool GetClientRect(RECT* lpRect);

	void SetParent(CBaseWnd *pWndParent);
	CBaseWnd *GetParent();

	CMLGraphics *GetGraphics();
	void ReleaseGraphics(CMLGraphics *pGraph);

	virtual BOOL InvalidateRect(const RECT* lpRect = NULL, BOOL bErase = FALSE);

	void CheckButton(int nIDButton, bool bCheck) { CheckDlgButton(m_hWnd, nIDButton, bCheck ? BST_CHECKED : BST_UNCHECKED); }
	bool IsButtonChecked(int nIDButton) { return IsDlgButtonChecked(m_hWnd, nIDButton) == BST_CHECKED; }

	bool IsSameWnd(CBaseWnd *pWnd) { return m_hWnd == pWnd->m_hWnd; }

	bool IsChild();

	bool IsMouseCaptured();

	bool IsIconic();

#ifdef _WINCE_
	bool IsZoomed() { return true; }
#else
	bool IsZoomed() { return Tobool(::IsZoomed(m_hWnd)); }
#endif

	bool IsWindow();

	bool IsValid() { return m_hWnd != NULL; }

	bool IsVisible();

	bool IsTopmost();
	void SetTopmost(bool bTopmost);

	bool IsToolWindow();
	void SetToolWindow(bool bToolWindow);

	bool SetForeground();

	void SetWindowPos(int x, int y);
	void SetWindowPosSafely(int x, int y);

	bool MoveWindow(int X, int Y, int nWidth, int nHeight, bool bRepaint = true);

	bool MoveWindowSafely(int X, int Y, int nWidth, int nHeight, bool bRepaint = true);

	int MessageOut(LPCTSTR lpText, UINT uType = MB_ICONINFORMATION | MB_OK, LPCTSTR lpCaption = NULL);

	bool ReplaceChildPos(int nIDChildSrcPos, CBaseWnd *pChildNew);

	void PostUserMessage(int nMessageID, LPARAM param);
	void SendUserMessage(int nMessageID, LPARAM param);

	virtual void OnUserMessage(int nMessageID, LPARAM param) { }

public:
	static CBaseWnd *FromHandle(HWND hWnd);
	void Attach(HWND hWnd);
	void Detach();

	HWND GetHandle()
	{
		return m_hWnd;
	}
	// void SetWndHandle(HWND hWnd);
	// BOOL SendMessage(UINT Msg, WPARAM wParam, LPARAM lParam);
	// BOOL PostMessage(UINT Msg, WPARAM wParam, LPARAM lParam);

	virtual LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);

protected:
    HWND				m_hWnd;
	WndSizeMode			m_WndSizeMode;

};

#endif // !defined(AFX_BASEWND_H__E1E1D147_5970_4781_97AB_BE14D02F0FF4__INCLUDED_)
