#ifndef _ML_WINDOW_H_INC_
#define _ML_WINDOW_H_INC_

#include "WidgetLocalization.h"
#include "../widgetif.h"

#ifndef WIN32
/*
 * Key State Masks for Keyboard/Mouse Messages
 */
#define MK_LBUTTON          0x0001 
#define MK_RBUTTON          0x0002
#define MK_SHIFT            0x0004
#define MK_CONTROL          0x0008
#define MK_MBUTTON          0x0010
#define MK_ALT				0x20
#endif

/*
 * ShowWindow() Commands
 */
#define SW_HIDE             0
#define SW_SHOWNORMAL       1
#define SW_NORMAL           1
#define SW_SHOWMINIMIZED    2
#define SW_SHOWMAXIMIZED    3
#define SW_MAXIMIZE         3
#define SW_SHOWNOACTIVATE   4
#define SW_SHOW             5
#define SW_MINIMIZE         6
#define SW_SHOWMINNOACTIVE  7
#define SW_SHOWNA           8
#define SW_RESTORE          9
#define SW_SHOWDEFAULT      10
#define SW_FORCEMINIMIZE    11

// message out dialog style.
#define MB_OK                       0x00000000L
#define MB_OKCANCEL                 0x00000001L
#define MB_YESNOCANCEL              0x00000003L
#define MB_YESNO                    0x00000004L
#define MB_ICONINFORMATION			0x00000040L
#define MB_ICONWARNING				0x00000030L
#define MB_ICONQUESTION				0x00000020L
#define MB_ICONERROR                0x00000010L

// Window style
#define DS_NOIDLEMSG				0x100L
#define WS_OVERLAPPED       0x00000000L
#define WS_POPUP            0x80000000L
#define WS_CHILD            0x40000000L
#define WS_MINIMIZE         0x20000000L
#define WS_VISIBLE          0x10000000L
#define WS_DISABLED         0x08000000L
#define WS_CLIPSIBLINGS     0x04000000L
#define WS_CLIPCHILDREN     0x02000000L
#define WS_MAXIMIZE         0x01000000L
#define WS_CAPTION          0x00C00000L     /* WS_BORDER | WS_DLGFRAME  */
#define WS_BORDER           0x00800000L
#define WS_DLGFRAME         0x00400000L
#define WS_VSCROLL          0x00200000L
#define WS_HSCROLL          0x00100000L
#define WS_SYSMENU          0x00080000L
#define WS_NOOWNERDRAW		0x08000000L


#define WS_MINIMIZEBOX      0x00020000L
#define WS_MAXIMIZEBOX      0x00010000L

#define WHEEL_DELTA			120

typedef UINT WPARAM;
typedef long LPARAM;

//
// platform independ functions about window system.
//

bool MLWidgetInit(int argc, char *argv[]);

//
// CBaseWnd implementation in gtk
//
class CBaseWnd
{
public:
	CBaseWnd();
	virtual ~CBaseWnd();

	struct CmdHandlerInfo
	{
		CBaseWnd		*pWnd;
		UINT			nCmdID;
	};

	// To handle platform independent timer
	struct TimerInfo
	{
		uint32		nTimerId;				// timer id
		uint32		nNativeTimerHandle;		// timer handler in gtk
		CBaseWnd	*pWnd;
	};


public:
	virtual BOOL Create(LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent, DWORD dwStyle = DS_NOIDLEMSG | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX, int nID = ID_UNDEFINE);
	virtual BOOL CreateEx(LPCTSTR szClassName, LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent, DWORD dwStyle = DS_NOIDLEMSG | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX, DWORD dwExStyle = 0, int nID = ID_UNDEFINE);

	virtual bool Destroy();

	virtual void PostDestroy();

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

	virtual void OnKeyDown(UINT nChar, UINT nFlags) { }
	virtual void OnKeyUp(UINT nChar, UINT nFlags) { }


	// nFlag: see "Key State Masks for Mouse Messages" MK_MBUTTON
	// nFlags
	// Indicates whether various virtual keys are down. This parameter can be any combination of the following values: 
	// MK_CONTROL   Set if the CTRL key is down.
	// MK_LBUTTON   Set if the left mouse button is down.
	// MK_MBUTTON   Set if the middle mouse button is down.
	// MK_RBUTTON   Set if the right mouse button is down.
	// MK_SHIFT   Set if the SHIFT key is down. 
//	virtual void OnMButtonDblClick(UINT nFlags, CPoint point);
//	virtual void OnRButtonDblClick(UINT nFlags, CPoint point);
	virtual void OnLButtonUp(UINT nFlags, CPoint point) { }
	virtual void OnLButtonDown(UINT nFlags, CPoint point) { }
	virtual void OnLButtonDblClk(UINT nFlags, CPoint point) { }
//	virtual void OnMButtonUp(UINT nFlags, CPoint point);
//	virtual void OnMButtonDown(UINT nFlags, CPoint point);
	virtual void OnRButtonUp( UINT nFlags, CPoint point ) { }
	virtual void OnRButtonDown( UINT nFlags, CPoint point ) { }
	virtual void OnMouseMove(UINT nFlags, CPoint point) { }
	virtual void OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt) { }

	virtual void OnMove(int x, int y) { }

	virtual BOOL OnNotify(WIDGET_NOTIFY_INFO *info) { return true; }
	virtual void OnPaint(IMLGraphicsBase *pGraph);

	virtual void OnKillFocus() { }
	virtual void OnSetFocus() { }

	virtual void OnSize(int cx, int cy) { }

	virtual void OnSizeModeChanged(WndSizeMode sizeMode) { }

	virtual void OnTimer (UINT nIDEvent) { }

	virtual void OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar) { }
	virtual void OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar) { }

	// Language Changed Notification message
	virtual void OnLanguageChanged();

	void PostUserMessage(int nMessageID, LPARAM param);
	void SendUserMessage(int nMessageID, LPARAM param);

	virtual void OnUserMessage(int nMessageID, LPARAM param) { }

public:
	//
	// Operations
	//
	void ActivateWindow();

	BOOL ShowWindow(int nCmdShow);

	// Gtk dialog supports these functions only.
	virtual UINT GetDlgItemText(int nIDItem, LPTSTR szString, int nMaxCount) { szString[0] = '\0'; return 0; }
	virtual UINT GetDlgItemText(int nIDItem, tstring &str) { str.resize(0); }
	virtual BOOL SetDlgItemText(int nIDItem, LPCTSTR szString) { return false; }
	virtual UINT GetDlgItemInt(int nIDItem, BOOL *pTranslated, BOOL bSigned) { if (pTranslated) *pTranslated = false; return false; }
	virtual BOOL SetDlgItemInt(int nIDItem, UINT uValue, BOOL bSigned) { return false; }
	virtual bool EnableDlgItem(int nIDItem, bool bEnable) { return false; }
	virtual bool ShowDlgItem(int nIDItem, int nCmdShow) { return false; }
	virtual CBaseWnd *GetDlgItem(int nIDItem) { return NULL; }
	virtual bool GetDlgItemRect(int nIDItem, RECT* lpRect) { lpRect->left = lpRect->top = lpRect->right = lpRect->bottom = 0; return false; }

	bool EnableWindow(bool bEnable) { gtk_widget_set_sensitive(m_window, bEnable); }

	bool SetTimer(UINT nTimerId, UINT nElapse);
	void KillTimer(UINT nTimerId);

	virtual int GetWindowText(LPTSTR szString, int nMaxCount);
	virtual int GetWindowText(tstring &str);
	bool SetWindowText(LPCTSTR szText) { 	gtk_window_set_title(GTK_WINDOW(m_window), szText); }

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
	CBaseWnd *GetParent() { return m_pWndParent; }

	IMLGraphicsBase *GetGraphics();
	void ReleaseGraphics(IMLGraphicsBase *pGraph);

	BOOL InvalidateRect(const RECT* lpRect = NULL, BOOL bErase = FALSE);

	virtual void CheckButton(int nIDButton, bool bCheck) { }
	virtual bool IsButtonChecked(int nIDButton) { return false; }

	bool IsSameWnd(CBaseWnd *pWnd) { return m_window == pWnd->m_window; }

	bool IsChild();

	bool IsMouseCaptured();

	bool IsIconic();

	bool IsZoomed() { return m_WndSizeMode == WndSizeMode_Maximized; }

	bool IsWindow();

	bool IsValid() { return m_window != NULL; }

	bool IsVisible();

	bool IsEnabled();

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

	virtual bool ReplaceChildPos(int nIDChildSrcPos, CBaseWnd *pChildNew) { return false; }

public:
	void ConnectGtkSignals();
	void CreateDrawingArea();

	// gtk platform's functions
	void GtkSigConnectClicked(GtkObject *pObj, uint32 nId);

	virtual GtkWidget *GetDlgItemHandle(int nIDItem) { return NULL; }

	CmdHandlerInfo *GetCmdHandlerInfo(UINT nCmdID);

	// static gtk event callback functions
	static int CBDeleteEvent(GtkWidget *widget, GdkEvent *event, gpointer data);
	static void CBDestroyEvent(GtkWidget *widget, gpointer data);
	static void CBGtkCmdMsg(GtkWidget *widget, gpointer data);
	static gboolean CBExposeEvent(GtkWidget *widget, GdkEventExpose *event, gpointer data);
	static gboolean CBConfigureEvent(GtkWidget *widget, GdkEventConfigure *event, gpointer data);
	static gboolean CBmotion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data);
	static gboolean CB_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data);
	static gboolean CB_key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer data);
	static gboolean CBbutton_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
	static gboolean CBbutton_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data);
	static gboolean cb_window_state_event(GtkWidget *widget, GdkEventWindowState *event, gpointer data);
	static gboolean CBTimer(gpointer data);

	GtkWidget *GetHandle() { return m_window; }
	int GetCtrlID() { return m_nID; }
	void GtkContainerAdd(CBaseWnd *pWnd);

protected:
	CBaseWnd		*m_pWndParent;

	bool			m_bTopmost;
	bool			m_bFirstShow;
	int				m_nID;
	GtkWidget		*m_window;

	GtkWidget		*m_drawingArea;

	vector<CmdHandlerInfo *>		m_vHandleInfo;

	vector<TimerInfo *>				m_vTimerInfo;

	WndSizeMode		m_WndSizeMode;

	// 多语言支持辅助对象
	CLangDialogHelper	m_LangDlgHelper;

};

#endif // _ML_WINDOW_H_INC_
