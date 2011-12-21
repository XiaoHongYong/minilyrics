//
//  BaseWnd.h
//  MiniLyricsMac
//
//  Created by Hongyong Xiao on 11/15/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#pragma once

struct _BaseWndInfo;

typedef int LPARAM;

class CCursor;
class CRegion;

class CBaseWnd
{
public:
	CBaseWnd();
	virtual ~CBaseWnd();

	virtual bool CreateForSkin(LPCTSTR szClassName, LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent, bool bToolWindow = true, bool bTopmost = false, bool bVisible = true);

	virtual bool Destroy();

	void PostDestroy();

public:
	//
	// Messages
	//
	virtual void OnActivate(bool bActived) { }
	virtual bool OnClose() { return true; }
	virtual void OnCommand(UINT uID, UINT nNotifyCode) { }
	virtual void OnContexMenu(int xPos, int yPos) { }
	virtual bool OnCreate();
	virtual void OnDestroy();
	
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

	UINT GetDlgItemText(int nIDItem, LPTSTR szString, int nMaxCount);
	UINT GetDlgItemText(int nIDItem, tstring &str);
	BOOL SetDlgItemText(int nIDItem, LPCTSTR szString);
	UINT GetDlgItemInt(int nIDItem, BOOL *pTranslated, BOOL bSigned);
	BOOL SetDlgItemInt(int nIDItem, UINT uValue, BOOL bSigned);
	bool EnableDlgItem(int nIDItem, bool bEnable);
	bool EnableWindow(bool bEnable);
	bool ShowDlgItem(int nIDItem, int nCmdShow);
	CBaseWnd *GetDlgItem(int nIDItem);
	bool GetDlgItemRect(int nIDItem, RECT* lpRect);

	bool SetTimer(UINT nTimerId, UINT nElapse);
	void KillTimer(UINT nTimerId);
	
	size_t GetWindowText(LPTSTR szString, int nMaxCount);
	size_t GetWindowText(tstring &str);
	bool SetWindowText(LPCTSTR szText);

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

	void CheckButton(int nIDButton, bool bCheck);
	bool IsButtonChecked(int nIDButton);
	
	bool IsSameWnd(CBaseWnd *pWnd);

	bool IsChild();

	bool IsMouseCaptured();

	bool IsIconic();

	bool IsZoomed();

	bool IsWindow();

	bool IsValid();

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

	virtual void OnUserMessage(int nMessageID, LPARAM param) { }
	
public:
	// Translucency related APIs
	bool				m_bTranslucencyLayered;				// Is following alpha setting enabled?
	
	// Alpha
	int					m_nAlpha;
	bool				m_bClickThrough;
	
	virtual void SetTransparent(BYTE nAlpha, bool bClickThrough);
	virtual bool IsClickThrough() { return m_bClickThrough; }
	
	BOOL UpdateLayeredWindowUsingMemGraph(CRawGraph *pGraph);

protected:
	_BaseWndInfo		*m_info;
	WndSizeMode			m_WndSizeMode;
	CBaseWnd			*m_parent;

};