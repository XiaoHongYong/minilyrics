// BaseWnd.cpp: implementation of the CBaseWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "../stdafx.h"
#include "BaseWnd.h"

#define BASEWNDCLASSNAME		_T("")

//
// platform independ functions about window system.
//

//#define ASSERT_NOTIMP(str)		assert(0 && str);
#define ASSERT_NOTIMP(str)		(0)

bool MLWidgetInit(int argc, char *argv[])
{
	GtkWidget	*widget;

	gtk_init(&argc, &argv);

	widget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_GtkContextObj.SetGtkWindow(widget);
	gtk_widget_realize(widget);

	return true;
}

UINT GdkModifierTypeToKeyState(guint state)
{
	UINT	flags = 0;

	if ((state & GDK_SHIFT_MASK) == GDK_SHIFT_MASK)
		flags |= MK_SHIFT;
	if ((state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK)
		flags |= MK_CONTROL;
	if ((state & GDK_MOD1_MASK) == GDK_MOD1_MASK)
		flags |= MK_ALT;
	if ((state & GDK_BUTTON1_MASK) == GDK_BUTTON1_MASK)
		flags |= MK_LBUTTON;
	if ((state & GDK_BUTTON2_MASK) == GDK_BUTTON2_MASK)
		flags |= MK_MBUTTON;
	if ((state & GDK_BUTTON3_MASK) == GDK_BUTTON3_MASK)
		flags |= MK_RBUTTON;

	return flags;
}

UINT GdkKeyValueToMLKeyCode(guint key)
{
	UINT		nKeyCode;

	if (key == GDK_Shift_R)
		nKeyCode = VK_SHIFT;
	else if (key == GDK_Control_R)
		nKeyCode = VK_CONTROL;

	return nKeyCode;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBaseWnd::CBaseWnd()
{
	m_nID = ID_UNDEFINE;
	m_window = (GtkWidget *)NULL;
	m_drawingArea = (GtkWidget *)NULL;
	m_WndSizeMode = WndSizeMode_Normal;
	m_bFirstShow = false;
	m_pWndParent = NULL;
	m_bTopmost = false;
}

CBaseWnd::~CBaseWnd()
{
	// free command handler.
	for (int i = 0; i < m_vHandleInfo.size(); i++)
	{
		delete m_vHandleInfo[i];
	}
	m_vHandleInfo.clear();


	// free timer handler.
	for (int i = 0; i < m_vTimerInfo.size(); i++)
	{
		delete m_vTimerInfo[i];
	}
	m_vTimerInfo.clear();

	Destroy();
}


BOOL CBaseWnd::Create(LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent, DWORD dwStyle, int nID)
{
	return CreateEx(BASEWNDCLASSNAME, szCaption, x, y, nWidth, nHeight, pWndParent, dwStyle, nID);
}

BOOL CBaseWnd::CreateEx(LPCTSTR szClassName, LPCTSTR szCaption, int x, int y, int nWidth, int nHeight, CBaseWnd *pWndParent, DWORD dwStyle, int nID)
{
	m_nID = nID;

	m_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	assert(m_window);
	g_GtkContextObj.SetGtkWindow(m_window);

	gtk_window_set_default_size(GTK_WINDOW(m_window), nWidth, nHeight);

	if ((dwStyle & WS_CAPTION) == 0)
		gtk_window_set_decorated(GTK_WINDOW(m_window), false);

	if ((dwStyle & WS_NOOWNERDRAW) == 0)
		CreateDrawingArea();

	ConnectGtkSignals();

	if (IsFlagSet(dwStyle, WS_VISIBLE))
		ShowWindow(SW_SHOW);

	OnCreate();

	return TRUE;
}

bool CBaseWnd::Destroy()
{
	if (m_window)
	{
		gtk_widget_destroy(m_window);
		m_window = NULL;
	}
	return true;
}

void CBaseWnd::OnClose()
{
	// Destroy();
}

bool CBaseWnd::OnCreate()
{
	// 在开始的时候初始化多语言支持对象
//	m_LangDlgHelper.InitAddAll(m_hWnd);

	// 切换为本地语言
//	OnLanguageChanged();

	return true;
}

void CBaseWnd::OnDestroy()
{
//	m_LangDlgHelper.Destroy();
}

void CBaseWnd::OnPaint(IMLGraphicsBase *pGraph)
{
	//	RECT	rc;

	//	GetClientRect(m_hWnd, &rc);
	//	Rectangle(hdc, 0, 0, rc.right, rc.bottom);
}

void CBaseWnd::OnLanguageChanged()
{
// 	m_LangDlgHelper.ToLocal();
}

void CBaseWnd::ActivateWindow()
{
	ASSERT(GTK_IS_WINDOW(m_window));
	if (GTK_IS_WINDOW(GTK_WINDOW(m_window)))
		gtk_window_present(GTK_WINDOW(m_window));
// 	DWORD       dwTimeOutPrev = 0;
// 
// 	SendMessage(m_hWnd, WM_SYSCOMMAND, SC_RESTORE, 0);
// 	::ShowWindow(m_hWnd, SW_SHOW);
// 
// 	if (!IsTopmost())
// 	{
// 		::SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
// 		::SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
// 	}
// 	// save old
// 	SystemParametersInfo(/*SPI_GETFOREGROUNDLOCKTIMEOUT*/0x2000, 0, (LPVOID)&dwTimeOutPrev, 0/* SPIF_SENDWININICHANGE | SPIF_UPDATEINIFILE*/);
// 	// set new
// 	SystemParametersInfo(/*SPI_SETFOREGROUNDLOCKTIMEOUT*/0x2001, 0, (LPVOID)0, SPIF_SENDWININICHANGE/* | SPIF_UPDATEINIFILE*/);
// 
// 	if (::GetForegroundWindow() != m_hWnd)
// 		SetForegroundWindow(m_hWnd);
// 	if (IsWindowEnabled(m_hWnd))
// 		SetActiveWindow(m_hWnd);
// 
// 	// restore old
// 	SystemParametersInfo(/*SPI_SETFOREGROUNDLOCKTIMEOUT*/0x2001, dwTimeOutPrev, (LPVOID)0, SPIF_SENDWININICHANGE/* | SPIF_UPDATEINIFILE*/);
}

BOOL CBaseWnd::ShowWindow(int nCmdShow)
{
	if (nCmdShow == SW_SHOW)
		gtk_widget_show(m_window);
	else if (nCmdShow == SW_HIDE)
		gtk_widget_hide(m_window);
	else if (nCmdShow == SW_MAXIMIZE)
	{
		ASSERT(GTK_IS_WINDOW(m_window));
		if (GTK_IS_WINDOW(m_window))
			gtk_window_maximize(GTK_WINDOW(m_window));
	}
	else if (nCmdShow == SW_RESTORE)
	{
		ASSERT(GTK_IS_WINDOW(m_window));
		if (GTK_IS_WINDOW(m_window))
		{
			if (m_WndSizeMode == WndSizeMode_Minimized)
				gtk_window_deiconify(GTK_WINDOW(m_window));
			else if (m_WndSizeMode == WndSizeMode_Maximized)
				gtk_window_unmaximize(GTK_WINDOW(m_window));
		}
	}
	else if (nCmdShow == SW_MINIMIZE)
	{
		ASSERT(GTK_IS_WINDOW(m_window));
		if (GTK_IS_WINDOW(m_window))
			gtk_window_iconify(GTK_WINDOW(m_window));
	}
		


	return true;
}

void CBaseWnd::ScreenToClient(CRect &rc)
{
	RECT	rcScr;

	GetWindowRect(&rcScr);

	rc.left -= rcScr.left;
	rc.top -= rcScr.top;
	rc.right -= rcScr.left;
	rc.bottom -= rcScr.top;
}

void CBaseWnd::ClientToScreen(CRect &rc)
{
	RECT	rcScr;

	GetWindowRect(&rcScr);

	rc.left += rcScr.left;
	rc.top += rcScr.top;
	rc.right += rcScr.left;
	rc.bottom += rcScr.top;
}

void CBaseWnd::ScreenToClient(CPoint &pt)
{
	RECT	rcScr;

	GetWindowRect(&rcScr);

	pt.x -= rcScr.left;
	pt.y -= rcScr.top;
}

void CBaseWnd::ClientToScreen(CPoint &pt)
{
	RECT	rcScr;

	GetWindowRect(&rcScr);

	pt.x += rcScr.left;
	pt.y += rcScr.top;
}

// The dimensions are given in screen coordinates that are relative to the upper-left corner of the screen. 
bool CBaseWnd::GetWindowRect(RECT* lpRect)
{
	ASSERT(m_window);
	int			x = 0, y = 0, cx = 0, cy = 0;

	if (m_window->parent == NULL)
	{
		// a window
		gtk_window_get_position(GTK_WINDOW(m_window), &x, &y);
		gtk_window_get_size(GTK_WINDOW(m_window), &cx, &cy);

		lpRect->left = x;
		lpRect->top = y;
		lpRect->right = lpRect->left + cx;
		lpRect->bottom = lpRect->top + cy;

		return true;
	}

	// for child widget
	GtkWidget		*p = m_window;
	GtkAllocation	rc;

	rc = p->allocation;
	p = p->parent;
	while (p->parent)
	{
		rc.x += p->allocation.x;
		rc.y += p->allocation.y;
		p = p->parent;
	}

	gtk_window_get_position(GTK_WINDOW(p), &x, &y);
	rc.x += x;
	rc.y += y;

	lpRect->left = rc.x;
	lpRect->top = rc.y;
	lpRect->right = lpRect->left + rc.width;
	lpRect->bottom = lpRect->top + rc.height;

	return true;
}

bool CBaseWnd::GetClientRect(RECT* lpRect)
{
	int		cx, cy;

	lpRect->left = 0;
	lpRect->top = 0;
	lpRect->right = m_window->allocation.width;
	lpRect->bottom = m_window->allocation.height;
}

void CBaseWnd::SetParent(CBaseWnd *pWndParent)
{
	ASSERT_NOTIMP("CBaseWnd::SetParent");
// 	if (pWndParent)
// 	{
// 		DWORD	dwStyle;
// 		dwStyle = GetWindowLong(m_hWnd, GWL_STYLE);
// 		SetWindowLong(m_hWnd, GWL_STYLE, dwStyle | WS_CHILD);
// 		::SetParent(m_hWnd, pWndParent->GetHandle());
// 	}
// 	else
// 	{
// 		DWORD	dwStyle;
// 		dwStyle = GetWindowLong(m_hWnd, GWL_STYLE);
// 		SetWindowLong(m_hWnd, GWL_STYLE, dwStyle & ~WS_CHILD);
// 		::SetParent(m_hWnd, NULL);
// 	}
}

bool CBaseWnd::SetTimer(UINT nTimerId, UINT nElapse)
{
	TimerInfo		*pInfo = NULL;

	for (int i = 0; i < m_vTimerInfo.size(); i++)
	{
		if (m_vTimerInfo[i]->nTimerId == nTimerId)
		{
			pInfo = m_vTimerInfo[i];
			gtk_timeout_remove(pInfo->nNativeTimerHandle);
			DBGLOG1("SetTimer: %d, find existed timer, remove it and set again.\n", nTimerId);
			break;
		}
	}

	if (!pInfo)
	{
		DBGLOG1("SetTimer: %d, Create new timer.\n", nTimerId);
		pInfo = new TimerInfo;
		pInfo->pWnd = this;
		pInfo->nTimerId = nTimerId;
		pInfo->nNativeTimerHandle = 0;
		m_vTimerInfo.push_back(pInfo);
	}

	pInfo->nNativeTimerHandle = gtk_timeout_add(nElapse, CBTimer, pInfo);
}

void CBaseWnd::KillTimer(UINT nTimerId)
{
	for (int i = 0; i < m_vTimerInfo.size(); i++)
	{
		if (m_vTimerInfo[i]->nTimerId == nTimerId)
		{
			TimerInfo		*pInfo;
			pInfo = m_vTimerInfo[i];
			gtk_timeout_remove(pInfo->nNativeTimerHandle);
			m_vTimerInfo.erase(m_vTimerInfo.begin() + i);
			delete pInfo;
			return;
		}
	}

	// ERRLOG1("CBaseWnd::RemoveTimer, TimerId not found: %d", nTimerId);
	// assert(0 && "CBaseWnd::RemoveTimer timer id Not found!");
}

int CBaseWnd::GetWindowText(LPTSTR szString, int nMaxCount)
{
	LPCTSTR	szTitle;

	ASSERT(m_window);
	if (GTK_IS_WINDOW(m_window))
		szTitle = gtk_window_get_title(GTK_WINDOW(m_window));
	else
		szTitle = gtk_widget_get_name(m_window);
	if (szTitle)
		_tcscpy_safe(szString, nMaxCount, szTitle);
	else
		EmptyStr(szString);
}

int CBaseWnd::GetWindowText(tstring &str)
{
	LPCTSTR	szTitle;

	ASSERT(m_window);
	if (GTK_IS_WINDOW(m_window))
		szTitle = gtk_window_get_title(GTK_WINDOW(m_window));
	else
		szTitle = gtk_widget_get_name(m_window);
	if (szTitle)
		str = szTitle;
	else
		str.resize(0);

	return str.size();
}

bool CBaseWnd::SetWndCursor(CCursor *pCursor)
{
	ASSERT(m_window);
	if (GTK_IS_WINDOW(m_window))
	{
		if (pCursor)
			gdk_window_set_cursor(m_window->window, pCursor->m_cursor);
		else
			gdk_window_set_cursor(m_window->window, NULL);
	}

	return true;
}

bool CBaseWnd::SetFocus()
{
	gtk_widget_grab_focus(m_window);
	return true;
}

bool CBaseWnd::SetWindowRgn(CRegion *pRgn)
{
	ASSERT(m_window);
	if (GTK_IS_WINDOW(m_window))
	{
		if (pRgn)
			gdk_window_shape_combine_region(m_window->window, pRgn->m_rgn, 0, 0);
			// gtk_widget_region_intersect(m_window, pRgn->m_rgn);
		else
			gdk_window_shape_combine_region(m_window->window, NULL, 0, 0);
	}
}

bool CBaseWnd::SetCapture()
{
	GdkGrabStatus	ret;
	ret = gdk_pointer_grab(m_window->window, true, 
		GdkEventMask(GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK), NULL, NULL, GDK_CURRENT_TIME);
	assert(ret == GDK_GRAB_SUCCESS);
	return ret == GDK_GRAB_SUCCESS;
}

void CBaseWnd::ReleaseCapture()
{
	gdk_pointer_ungrab(GDK_CURRENT_TIME);
}

IMLGraphicsBase *CBaseWnd::GetGraphics()
{
	CMLGraphics *pGraphics;

	assert(m_drawingArea);

	pGraphics = new CMLGraphics;
	if (m_drawingArea->window)
	{
		pGraphics->Init(m_drawingArea->window);
	}
	// pGraphics->m_pGc = m_drawingArea->style->fg_gc[GTK_WIDGET_STATE(m_drawingArea)];
	// MLDebug("gdk_gc_ref\n");
	// gdk_gc_ref(pGraphics->m_pGc);

	return pGraphics;
}

void CBaseWnd::ReleaseGraphics(IMLGraphicsBase *pGraph)
{
	assert(pGraph);
	delete pGraph;
}

BOOL CBaseWnd::InvalidateRect(const RECT *lpRect, BOOL bErase)
{
	if (!m_window)
	{
		DBGLOG0(_T("CBaseWnd::InvalidateRect, ERROR, window hasn't been created yet."));
		return false;
	}

	if (lpRect)
		gtk_widget_queue_draw_area(m_window, lpRect->left, lpRect->top, lpRect->right - lpRect->left, lpRect->bottom - lpRect->top);
	else
		gtk_widget_queue_draw(m_window);

	return true;
}

bool CBaseWnd::IsChild()
{
	return m_window->parent != NULL;
}

bool CBaseWnd::IsMouseCaptured()
{
	return Tobool(gdk_pointer_is_grabbed());
}

bool CBaseWnd::IsIconic()
{
	return m_WndSizeMode == WndSizeMode_Minimized;
}

bool CBaseWnd::IsWindow()
{
	return m_window != NULL;
}

bool CBaseWnd::IsVisible()
{
	return GTK_WIDGET_VISIBLE(m_window);
}

bool CBaseWnd::IsEnabled()
{
	return GTK_WIDGET_IS_SENSITIVE(m_window);
}

bool CBaseWnd::IsTopmost()
{
	ASSERT(m_window);
	ASSERT(m_window->window);
// 	if (m_window && m_window->window)
// 	{
// 		GdkWindowState	state = gdk_window_get_state(m_window->window);
// 
// 		return IsFlagSet(state, GDK_WINDOW_STATE_ABOVE);
// 	}

	return m_bTopmost;

	// ASSERT_NOTIMP("CBaseWnd::IsTopmost");
	// return false;
}

void CBaseWnd::SetTopmost(bool bTopmost)
{
	ASSERT(m_window);
	ASSERT(GTK_IS_WINDOW(m_window));
	gtk_window_set_keep_above(GTK_WINDOW(m_window), bTopmost);
	m_bTopmost = bTopmost;
}

bool CBaseWnd::IsToolWindow()
{
	return false;
}

void CBaseWnd::SetToolWindow(bool bToolWindow)
{
	// ASSERT_NOTIMP("CBaseWnd::SetToolWindow");
}

bool CBaseWnd::SetForeground()
{
	return false;
}

void CBaseWnd::SetWindowPos(int x, int y)
{
	ASSERT(m_window);
	if (GTK_IS_WINDOW(m_window))
		gtk_window_move(GTK_WINDOW(m_window), x, y);
	else
	{
		GtkAllocation	size = m_window->allocation;

		size.x = x;
		size.y = y;
		gtk_widget_size_allocate(m_window, &size);
	}
}

void CBaseWnd::SetWindowPosSafely(int x, int y)
{
	return SetWindowPos(x, y);
// 	if (IsChild())
// 	{
// 		HWND	hWndParent;
// 		hWndParent = ::GetParent(m_hWnd);
// 		if (hWndParent)
// 		{
// 			POINT	pt;
// 			pt.x = x;
// 			pt.y = y;
// 			::ScreenToClient(hWndParent, &pt);
// 			x = pt.x;
// 			y = pt.y;
// 		}
// 	}
// 	::SetWindowPos(m_hWnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREPOSITION);
}

bool CBaseWnd::MoveWindow(int X, int Y, int nWidth, int nHeight, bool bRepaint)
{
	ASSERT(m_window);
	if (GTK_IS_WINDOW(m_window))
	{
		gtk_window_move(GTK_WINDOW(m_window), X, Y);
		gtk_window_resize(GTK_WINDOW(m_window), nWidth, nHeight);
	}
	else
	{
		GtkAllocation	size;

		size.x = X;
		size.y = Y;
		size.width = nWidth;
		size.height = nHeight;
		gtk_widget_size_allocate(m_window, &size);
	}
	return true;
}

bool CBaseWnd::MoveWindowSafely(int X, int Y, int nWidth, int nHeight, bool bRepaint)
{
	return MoveWindow(X, Y, nWidth, nHeight, bRepaint);
	// return Tobool(::MoveWindowSafely(m_hWnd, X, Y, nWidth, nHeight, bRepaint));
}

int CBaseWnd::MessageOut(LPCTSTR lpText, UINT uType, LPCTSTR lpCaption)
{
	ASSERT(m_window);

	TCHAR	szCap[512];
	if (lpCaption == NULL)
	{
		EmptyStr(szCap);
		GetWindowText(szCap, CountOf(szCap));
		lpCaption = szCap;
	}

	GtkWidget		*msgDlg, *parent;
	int				nRet;

	GtkMessageType		msgType;
	GtkButtonsType		btType;
	if (IsFlagSet(uType, MB_OK))
		btType = GTK_BUTTONS_OK;
	else if (IsFlagSet(uType, MB_YESNO))
		btType = GTK_BUTTONS_YES_NO;
	else if (IsFlagSet(uType, MB_OKCANCEL))
		btType = GTK_BUTTONS_OK_CANCEL;
	else
		btType = GTK_BUTTONS_OK;

	if (IsFlagSet(uType, MB_ICONWARNING))
		msgType = GTK_MESSAGE_WARNING;
	else if (IsFlagSet(uType, MB_ICONQUESTION))
		msgType = GTK_MESSAGE_QUESTION;
	else if (IsFlagSet(uType, MB_ICONINFORMATION))
		msgType = GTK_MESSAGE_INFO;
	else if (IsFlagSet(uType, MB_ICONERROR))
		msgType = GTK_MESSAGE_ERROR;
	else
		msgType = GTK_MESSAGE_INFO;

	parent = m_window;
	while (parent->parent)
		parent = parent->parent;

	msgDlg = gtk_message_dialog_new(GTK_WINDOW(parent), GTK_DIALOG_MODAL, msgType, btType,  "%s", lpText);
	nRet = gtk_dialog_run(GTK_DIALOG (msgDlg));
	gtk_widget_destroy (msgDlg);

	return nRet;
}


//////////////////////////////////////////////////////////////////////////
// gtk platform's functions

void CBaseWnd::ConnectGtkSignals()
{
	//	gtk_window_set_decorated(GTK_WINDOW(m_window), false);

	/*当给窗口一个"delete_event"信号时(这个信号是由窗口管
	*理器发出的，通常是在点击窗口标题条右边的"×"按钮，或
	*者在标题条的快捷菜单上选择"close"选项时发出的)，我们
	*要求调用上面定义的delete_event()函数传递给这个回调函数
	*的数据是NULL，回调函数会忽略这个参数*/
	gtk_signal_connect(GTK_OBJECT(m_window), "delete_event",
		GTK_SIGNAL_FUNC(CBDeleteEvent), this);

	/*这里，我们给"destory"事件连接一个信号处理函数，
	*当我们在窗口上调用gtk_widget_destroy()函数
	*或者在"delete_event"事件的回调函数中返回FALSE
	* 时会发生这个事件*/
	gtk_signal_connect(GTK_OBJECT(m_window), "destroy",
		GTK_SIGNAL_FUNC(CBDestroyEvent), this);
	g_print("Register CBDestroyEvent: %X\n", this);

	/* 设置窗口的边框宽度*/
	// gtk_container_set_border_width(GTK_CONTAINER(m_pWnd), 0);
	// gtk_container_set_border_width(GTK_CONTAINER (m_window), 100);

	g_signal_connect(m_window, "motion_notify_event",
		G_CALLBACK(CBmotion_notify_event), this);
	g_signal_connect(m_window, "key_press_event",
		G_CALLBACK(CB_key_press_event), this);
	g_signal_connect(m_window, "key_release_event",
		G_CALLBACK(CB_key_release_event), this);
	g_signal_connect(m_window, "button_press_event",
		G_CALLBACK(CBbutton_press_event), this);
	g_signal_connect(m_window, "button_release_event",
		G_CALLBACK(CBbutton_release_event), this);

	g_signal_connect(m_window, "window_state_event",
		G_CALLBACK(cb_window_state_event), this);
}

void CBaseWnd::CreateDrawingArea()
{
	//
	// Create drawing area
	//
	m_drawingArea = gtk_drawing_area_new();
	assert(m_drawingArea);
	gtk_container_add(GTK_CONTAINER(m_window), m_drawingArea);
	/* Signals used to handle backing pixmap */
	g_signal_connect(m_drawingArea, "expose_event",
		G_CALLBACK(CBExposeEvent), this);
	g_signal_connect(m_drawingArea,"configure_event",
		G_CALLBACK(CBConfigureEvent), this);

	/* Event signals */
	//
	//		g_signal_connect(m_drawingArea, "motion_notify_event",
	//			G_CALLBACK(CBmotion_notify_event), this);
	//		g_signal_connect(m_drawingArea, "button_press_event",
	//			G_CALLBACK(CBbutton_press_event), this);
	//		g_signal_connect(m_drawingArea, "button_release_event",
	//			G_CALLBACK(CBbutton_release_event), this);


	/* Ask to receive events the drawing area doesn't normally
	* subscribe to
	*/
	gtk_widget_set_events(m_drawingArea, gtk_widget_get_events(m_drawingArea)
		| GDK_LEAVE_NOTIFY_MASK
		| GDK_BUTTON_PRESS_MASK
		| GDK_BUTTON_RELEASE_MASK
		| GDK_POINTER_MOTION_MASK
		| GDK_POINTER_MOTION_HINT_MASK);
	// MLDebug("gtk_widget_set_events\n");

	gtk_widget_show(m_drawingArea);
}

void CBaseWnd::GtkSigConnectClicked(GtkObject *pObj, uint32 nId)
{
	gtk_signal_connect(pObj, "clicked",
		GTK_SIGNAL_FUNC(CBGtkCmdMsg), GetCmdHandlerInfo(nId));
}

void CBaseWnd::CBGtkCmdMsg(GtkWidget *widget, gpointer data)
{
	CmdHandlerInfo	*pInfo;
	pInfo = (CmdHandlerInfo *)data;

	if (pInfo)
	{
		pInfo->pWnd->OnCommand(pInfo->nCmdID, 0);
	}
}

int CBaseWnd::CBDeleteEvent(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	CBaseWnd		*pThis;

	pThis = (CBaseWnd *)data;

	/*如果在"delete_event"处理程序中返回FALSE，GTK将引发一个"destroy"
	*信号，返回T R U E意味着你不想关闭窗口。
	* 这些在弹出"你真的要退出? "对话框时很有作用*/
	g_print("delete event occurred\n");
	/* 将T R U E改为F A L S E，主窗口就会用一个" d e l e t e _ e v e n t "信号，然后退出*/

	pThis->OnClose();

	return false;
}

/* 另一个回调函数*/
void CBaseWnd::CBDestroyEvent(GtkWidget *widget, gpointer data)
{
	CBaseWnd		*pThis;

	pThis = (CBaseWnd *)data;

	g_print("CBDestroyEvent occurred: %X\n", data);

	pThis->OnDestroy();

	pThis->m_window = NULL;
	pThis->m_pWndParent = NULL;
	pThis->m_bFirstShow = false;
	pThis->m_drawingArea = NULL;
}

gboolean CBaseWnd::CBExposeEvent(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	CBaseWnd		*pThis;
	CMLGraphics		*pGtkGraph;

	// MLDebug("CBExposeEvent\n");

	pThis = (CBaseWnd *)data;

	pGtkGraph = (CMLGraphics*)pThis->GetGraphics();
	pThis->OnPaint(pGtkGraph);
	pThis->ReleaseGraphics(pGtkGraph);

	return true;
}

gboolean CBaseWnd::CBConfigureEvent(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
	CBaseWnd		*pThis;
	// MLDebug("CBConfigureEvent\n");

	pThis = (CBaseWnd *)data;

	pThis->OnSize(widget->allocation.width, widget->allocation.height);
	pThis->OnMove(widget->allocation.x, widget->allocation.y);

	return true;
}

gboolean CBaseWnd::CBmotion_notify_event(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	CBaseWnd		*pThis;
	UINT			flags = 0;

	pThis = (CBaseWnd *)data;

	// MLDebug("GdkEventMotion-root-x,y: %.02f, %.02f\n", event->x, event->y);

	// When using the GDK_POINTER_MOTION_HINT_MASK the server will only send a single GDK_MOTION_NOTIFY event (which is marked as a hint) until the application asks for more, by calling gdk_window_get_pointer(). 
	if (event->is_hint)
	{
		int x, y;
		GdkModifierType state;
		gdk_window_get_pointer(event->window, &x, &y, &state);
	}

	flags = GdkModifierTypeToKeyState(event->state);

	pThis->OnMouseMove(flags, CPoint((int)event->x, (int)event->y));

	return true;
/*

	int x, y;
	UINT	flags;
	GdkModifierType state;
	gdk_window_get_pointer (event->window, &x, &y, &state);
	MLDebug("\n\ngdk_window_get_pointer: %d, %d\n", x, y);

	GdkDisplay *display = gdk_display_get_default();
	assert(display);
	gdk_display_get_pointer(display, NULL, &x, &y, &state);
	MLDebug("gdk_display_get_pointer: %d, %d\n", x, y);
	MLDebug("GdkEventMotion-x,y: %d, %d\n", (int)event->x, (int)event->y);
	MLDebug("GdkEventMotion-root-x,y: %02f, %02f\n", event->x, event->y);

  // if (state & GDK_BUTTON1_MASK)
	if (event->is_hint)
	{
//		gdk_window_get_pointer(widget->window, &x, &y, &state);
//		x += widget->allocation.x;
//		y += widget->allocation.y;
//		MLDebug("mouse motion Hint: %d, %d, allocation: %d, %d\n, ", x, y, widget->allocation.x, widget->allocation.y);
		pThis->OnMouseMove(flags, CPoint(x, y));
	}
	else
		pThis->OnMouseMove(flags, CPoint((int)event->x, (int)event->y));
*/
}

gboolean CBaseWnd::CB_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	// MLDebug("CB_key_press_event\n");
	CBaseWnd		*pThis;
	UINT			flags = 0;

	pThis = (CBaseWnd *)data;

	flags = GdkModifierTypeToKeyState(event->state);

	pThis->OnKeyDown(GdkKeyValueToMLKeyCode(event->keyval), flags);

	return true;
}

gboolean CBaseWnd::CB_key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	// MLDebug("CB_key_release_event\n");
	CBaseWnd		*pThis;
	UINT			flags = 0;

	pThis = (CBaseWnd *)data;

	flags = GdkModifierTypeToKeyState(event->state);

	pThis->OnKeyUp(GdkKeyValueToMLKeyCode(event->keyval), flags);

	return true;
}

// GdkEventType type; the type of the event (GDK_BUTTON_PRESS, GDK_2BUTTON_PRESS, GDK_3BUTTON_PRESS or GDK_BUTTON_RELEASE). 
// guint button; the button which was pressed or released, numbered from 1 to 5. Normally button 1 is the left mouse button, 2 is the middle button, and 3 is the right button. On 2-button mice, the middle button can often be simulated by pressing both mouse buttons together.  
// guint state; a bit-mask representing the state of the modifier keys (e.g. Control, Shift and Alt) and the pointer buttons. See GdkModifierType.  
gboolean CBaseWnd::CBbutton_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	// MLDebug("CBbutton_press_event\n");
	CBaseWnd		*pThis;
	UINT			flags = 0;

	pThis = (CBaseWnd *)data;

	flags = GdkModifierTypeToKeyState(event->state);

	flags |= MK_LBUTTON;

	if (event->type == GDK_BUTTON_PRESS)
	{
		if (event->button == 1)
			pThis->OnLButtonDown(flags, CPoint((long)event->x, (long)event->y));
// 		else if (event->button == 2)
// 			pThis->OnMButtonDown(flags, CPoint((long)event->x, (long)event->y));
		else if (event->button == 3)
			pThis->OnRButtonDown(flags, CPoint((long)event->x, (long)event->y));
//		else
//			MLDebug("Unknown\n");
	}
	else if (event->type == GDK_2BUTTON_PRESS)
	{
		if (event->button == 1)
			pThis->OnLButtonDblClk(flags, CPoint((long)event->x, (long)event->y));
// 		else if (event->button == 2)
// 			pThis->OnMButtonDblClick(flags, CPoint((long)event->x, (long)event->y));
//		else if (event->button == 3)
//			pThis->OnRButtonDblClick(flags, CPoint((long)event->x, (long)event->y));
//		else
//			MLDebug("Unknown\n");
	}
//	else
//		MLDebug("event->type: %d\n", event->type);

	return true;
}

// GdkEventType type; the type of the event (GDK_BUTTON_PRESS, GDK_2BUTTON_PRESS, GDK_3BUTTON_PRESS or GDK_BUTTON_RELEASE). 
// guint button; the button which was pressed or released, numbered from 1 to 5. Normally button 1 is the left mouse button, 2 is the middle button, and 3 is the right button. On 2-button mice, the middle button can often be simulated by pressing both mouse buttons together.  
// guint state; a bit-mask representing the state of the modifier keys (e.g. Control, Shift and Alt) and the pointer buttons. See GdkModifierType.  
gboolean CBaseWnd::CBbutton_release_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
//	MLDebug("CBbutton_release_event\n");
	CBaseWnd		*pThis;
	UINT			flags = 0;

	pThis = (CBaseWnd *)data;

	flags = GdkModifierTypeToKeyState(event->state);

	assert(event->type == GDK_BUTTON_RELEASE);
	if (event->button == 1)
		pThis->OnLButtonUp(flags, CPoint((long)event->x, (long)event->y));
// 	else if (event->button == 2)
// 		pThis->OnMButtonUp(flags, CPoint((long)event->x, (long)event->y));
	else if (event->button == 3)
		pThis->OnRButtonUp(flags, CPoint((long)event->x, (long)event->y));
//	else
//		MLDebug("Unknown\n");

	return true;
}

gboolean CBaseWnd::cb_window_state_event(GtkWidget *widget, GdkEventWindowState *event, gpointer data)
{
	CBaseWnd		*pThis;
	
	pThis = (CBaseWnd *)data;
	
	// We don't care about anything but changes in the maximized/icon
	// states
// 	if (event->changed_mask &
// 		(GDK_WINDOW_STATE_ICONIFIED|GDK_WINDOW_STATE_MAXIMIZED) == 0)
// 	{
// 		return false;
// 	}
	
	if (event->new_window_state & GDK_WINDOW_STATE_ICONIFIED)
	{
//		MLDebug(("\tIconified\n"));
		pThis->m_WndSizeMode = WndSizeMode_Minimized;
	}
	else if (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED)
	{
//		MLDebug(("\tMaximized\n"));
		pThis->m_WndSizeMode = WndSizeMode_Maximized;
	}
/*	else if (event->new_window_state & GDK_WINDOW_STATE_ABOVE)
	{
//		MLDebug(("\tMaximized\n"));
		DBGLOG0(_T("*******above******"));
		pThis->m_bTopmost = true;
	}
	else if (event->new_window_state & GDK_WINDOW_STATE_BELOW)
	{
		pThis->m_bTopmost = false;
		DBGLOG0(_T("*******below******"));
	}
*/	else
	{
//		MLDebug(("\tNormal\n"));
		pThis->m_WndSizeMode = WndSizeMode_Normal;
	}
	pThis->OnSizeModeChanged(pThis->m_WndSizeMode);

	return true;
}

gboolean CBaseWnd::CBTimer(gpointer data)
{
	TimerInfo	*pInfo = (TimerInfo *)data;
	assert(pInfo);

	pInfo->pWnd->OnTimer(pInfo->nTimerId);

	return true;
}

CBaseWnd::CmdHandlerInfo *CBaseWnd::GetCmdHandlerInfo(UINT nCmdID)
{
	for (int i = 0; i < m_vHandleInfo.size(); i++)
	{
		if (m_vHandleInfo[i]->nCmdID == nCmdID)
			return m_vHandleInfo[i];
	}

	CmdHandlerInfo		*pInfo;

	pInfo = new CmdHandlerInfo;
	pInfo->pWnd = this;
	pInfo->nCmdID = nCmdID;

	return pInfo;
}

void CBaseWnd::GtkContainerAdd(CBaseWnd *pWnd)
{
	ASSERT(pWnd);
    gtk_container_add (GTK_CONTAINER (m_window), pWnd->GetHandle());
}

//////////////////////////////////////////////////////////////////////////
