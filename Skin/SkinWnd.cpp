/********************************************************************
	Created  :	2001-12-15 2:36:55
	FileName :	SkinWnd.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "StdAfx.h"
#include "MLSkin.h"
#include "SkinWnd.h"
#include "SkinButton.h"
#include "SkinVScrollBar.h"
#include "SkinCaption.h"

#define _SZ_SKINWND			_T("skinwnd")

/*
const TCHAR SZ_WINAMP2_DEFSKIN_DIR[] = _T("winamp");
const TCHAR SZ_WINAMP2_DEFSKIN_DIR[] = _T("");
*/
// 

#define TIMER_ID_BEG_ALLOC		120
#define TIMER_ID_ANIMATION		117
#define TIMER_ID_DYNAMIC_TRANS	118
#define TIMER_ID_MOUSE_INACTIVE	119

#define TIMER_SPAN_MOUSE_INACTIVE	(3 * 1000)

#define TIMER_SPAN_DYNAMIC_TRANS	30
#define TIME_OUT_TRANS_FADEIN		500
#define TIME_OUT_ANIMATION		30

BOOL IsWndOutOfScreen(const CRect &rc)
{
	CRect		rcRestrict;

	GetMonitorRestrictRect(rc, rcRestrict);

	return ((rc.right < rcRestrict.left + 20) || (rc.left > rcRestrict.right - 20) ||
		(rc.bottom < rcRestrict.top + 10) || (rc.top > rcRestrict.bottom - 40));
}


template<class _TCHAR>
UINT GetColorValue_t(_TCHAR *szColor, UINT nDefault)
{
	UINT	uRet = 0;

	if (*szColor == '#')
		szColor++;

	if (*szColor == '\0')
		return nDefault;

	int		n;

	while (*szColor)
	{
		if (*szColor >= '0' && *szColor <= '9')
			n = *szColor - '0';
		else if (*szColor >= 'a' && *szColor <= 'f')
			n = 10 + *szColor - 'a';
		else if (*szColor >= 'A' && *szColor <= 'F')
			n = 10 + *szColor - 'A';
		else
			break;

		uRet = uRet * 16 + n;
		szColor++;
	}

	if (*szColor != '\0')
		return nDefault;

	uRet = ((uRet & 0xFF) << 16) + (uRet & 0xFF00) + ((uRet & 0xFF0000) >> 16);

	return uRet;	
}

bool GetRectValue(LPCTSTR szValue, CSFImage &image)
{
	return Scan4IntX(szValue, image.m_x, image.m_y, image.m_cx, image.m_cy);
}

bool GetRectValue(LPCTSTR szValue, long &x, long &y, long &cx, long &cy)
{
	return Scan4IntX(szValue, x, y, cx, cy);
}


void ColorToStr(const MLColor &clr, tstring &strColor)
{
	TCHAR				szValue[128];

	ColorToStr(clr.get(), szValue);

	strColor = szValue;
}


// FgColor="#FFFFFF"
UINT GetColorValue(LPCWSTR szColor, UINT nDefault)
{
	UINT		clr;
	LPCWSTR p = ReadColorValue_t(szColor, clr);
	if (*p != '\0')
		return nDefault;
	else
		return clr;
}


// FgColor="#FFFFFF"
UINT GetColorValue(LPCSTR szColor, UINT nDefault)
{
	UINT		clr;
	LPCSTR p = ReadColorValue_t(szColor, clr);
	if (*p != '\0')
		return nDefault;
	else
		return clr;
}

// FgColor="#FFFFFF"
void GetColorValue(MLColor &clr, LPCTSTR szColor)
{
	UINT		uRet;
	LPCTSTR p = ReadColorValue_t(szColor, uRet);
	if (*p == '\0')
		clr.set(uRet);
}


int GetMenuKey(LPCTSTR szText)
{
	while (*szText && *szText != '&')
		szText++;

	if (*szText == '&')
	{
		return szText[1];
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////

CSkinWnd::CSkinWnd() : m_itFocusUIObject(&m_rootConainter)
{
	m_bFreeOnDestory = false;
	m_bManageBySkinFactory = true;

	m_bSkinOpened = false;
	m_nInRedrawUpdate = 0;
	m_bInEditorMode = false;

	m_bWindowsAppearance = false;

	m_bIndividualProperties = false;

	m_bPaintBg = false;
	m_clrBgOrg.set(0, 0, 0);
	m_clrBgOrg.setAlpha(255);
	m_clrFg.set(255, 255, 255);

	m_bActived = FALSE;
	m_bMainAppWnd = false;

	m_pmemGraph = NULL;

	m_rootConainter.m_pSkin = this;
	m_pUIObjCapMouse = NULL;

	m_bUseRgn = FALSE;

	m_rcBoundBox.SetRect(0, 0, 400, 350);
	m_rcReal.SetRect(0, 0, 400, 350);

	m_nTimerIDMax = TIMER_ID_BEG_ALLOC;

	m_dbScale = 1.0;

	m_nWidth = 0;
	m_nHeight = 0;

	m_nNextUIObjIDAlloc = UIOBJ_ID_BEGIN_ALLOC;

	m_nNextAuoUniID = 1;

	m_pSkinFactory = NULL;

	m_bIgnoreNextOnCharMsg = false;

	m_nTimeLatestMouseMsg = 0;
	m_bMouseActive = false;

	m_bTranslucencyLayered = false;
	m_nAlpha = 255;
	m_nTranslucencyAlphaOnActive = -1;
	m_nTranslucencyAlphaOnHover = -1;
	m_nTranslucencyAlpha = -1;
	m_nTranslucencyAlphaDynamic = 255;
	m_dwTransFadeinBegTime = 0;
	m_nTransAlphaBeg = 0;
	m_bClickThrough = false;
	m_bEnableClickThrough = true;

	m_bOnMouseHover = false;
	m_translucencyStatus = TS_NORMAL;

	m_pSjvmNotifyEvent = NULL;
}

CSkinWnd::~CSkinWnd()
{
	CloseSkin();
}

int CSkinWnd::Create(LPCTSTR szClassName, LPCTSTR szCaption, CSkinFactory *pSkinFactory, LPCTSTR szSkinWndName, CBaseWnd *pWndParent, bool bToolWindow, bool bTopmost)
{
	m_pSkinFactory = pSkinFactory;
	m_strSkinWndName = szSkinWndName;

	DWORD	dwExStyle = 0;
	if (bTopmost)
		dwExStyle |= WS_EX_TOPMOST;
	if (bToolWindow)
		dwExStyle |= WS_EX_TOOLWINDOW;

	// WS_THICKFRAME will cause window flashing on vista
	if (!CreateEx(szClassName, szCaption,
		m_rcReal.left, m_rcReal.top, m_rcReal.Width(), m_rcReal.Height(),
		pWndParent,
		DS_NOIDLEMSG | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_MAXIMIZEBOX | WS_VISIBLE, dwExStyle))
		return ERR_FALSE;

	return ERR_OK;
}

int CSkinWnd::OpenDefaultSkin()
{
	if (m_strSkinWndName.empty())
		return ERR_NOT_FOUND;

	tstring			strSkinWndName = m_strSkinWndName;

	return OpenSkin(strSkinWndName.c_str());
}

int CSkinWnd::OpenSkin(LPCTSTR szSkinWndName)
{
	SXNode			*pNode;
	CSimpleXML		xml;

	// before load it we close old first.
	CloseSkin();

	pNode = m_pSkinFactory->GetSkinWndNode(szSkinWndName);
	if (!pNode)
	{
		int nRet = m_pSkinFactory->LoadAppResSkinWndXml(szSkinWndName, xml);
		if (nRet != ERR_OK)
			return nRet;
		pNode = xml.m_pRoot;
	}

	m_strSkinWndName = szSkinWndName;

	CAutoRedrawLock	redrawLock(this);

	return FromXML(pNode);
}

void CSkinWnd::CloseSkin()
{
	if (!m_bSkinOpened)
		return;

	KillTimer(TIMER_ID_ANIMATION);
	m_listAnimations.clear();

	if (m_pSjvmNotifyEvent)
		m_pSjvmNotifyEvent->onDestroy();

	if (m_bManageBySkinFactory)
		CSjvmSkinSystem::OnSkinWndDestory(this);

#ifdef _WIN32_DESKTOP
	if (m_bWindowsAppearance)
	{
		SetWindowLong(m_hWnd, GWL_STYLE, GetWindowLong(m_hWnd, GWL_STYLE) & ~(WS_CAPTION | WS_THICKFRAME));
		::SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_NOMOVE);
		m_bWindowsAppearance = false;
	}
#endif
	m_nAlpha = 255;
	m_nTranslucencyAlphaOnActive = -1;
	m_nTranslucencyAlphaOnHover = -1;
	m_nTranslucencyAlpha = -1;
	m_nTranslucencyAlphaDynamic = 255;
	m_dwTransFadeinBegTime = 0;
	m_nTransAlphaBeg = 0;
	m_bClickThrough = false;
	m_bEnableClickThrough = true;

	m_wndResizer.FixedHeight(false);
	m_wndResizer.FixedWidth(false);
	m_wndResizer.ClearResizeArea();

	//
	// kill timers
	MAP_TIMER_OBJS::iterator	it;

	for (it = m_mapTimerObjs.begin(); it != m_mapTimerObjs.end(); it++)
	{
		KillTimer((*it).first);
	}
	m_mapTimerObjs.clear();

	m_pUIObjCapMouse = NULL;
	m_itFocusUIObject.End();

	m_rootConainter.Destroy();

#ifdef _WIN32_DESKTOP
	KillTimer(TIMER_ID_DYNAMIC_TRANS);
	m_translucencyStatus = TS_NORMAL;
	::UnSetLayeredWindow(m_hWnd);
#endif

	ASSERT(m_nInRedrawUpdate == 0);
	m_nInRedrawUpdate = 0;
	m_bSkinOpened = false;
	m_bPaintBg = false;
	m_clrBgOrg.set(0, 0, 0);
	m_clrBgOrg.setAlpha(255);
	m_clrFg.set(255, 255, 255);

	m_listUnprocessedProperties.clear();

	m_cursor.Destroy();

	m_strCusor.resize(0);
	m_strRgn.resize(0);
	m_strSkinWndName.resize(0);

	m_imageBg.Destroy();

	m_nWidth = 0;
	m_nHeight = 0;

	m_fontProperty.Create(_T("Tohama"), _T(""), 14, FW_NORMAL, false, false);

	m_bUseRgn = FALSE;

	if (m_rgnSkin.IsValid())
		m_rgnSkin.Destroy();

	if (m_pmemGraph)
	{
		delete m_pmemGraph;
		m_pmemGraph = NULL;
	}

	m_bIndividualProperties = false;

	m_nTimeLatestMouseMsg = 0;
	m_bMouseActive = false;
	KillTimer(TIMER_ID_MOUSE_INACTIVE);

	m_pSjvmNotifyEvent = NULL;
	m_vUIObjNotifyHandlers.clear();
}

CUIObject *CSkinWnd::GetUIObjectByClassName(LPCTSTR szClassName)
{
	return m_rootConainter.GetUIObjectByClassName(szClassName);
}

CUIObject * CSkinWnd::GetUIObjectById(int nId, LPCTSTR szClassName)
{
	return m_rootConainter.GetUIObjectById(nId, szClassName);
}

void CSkinWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	OnMouseActiveMsg();

	if (m_cursor.IsValid())
		MLWidget::SetCursor(m_cursor);

	point.x = (long)(point.x / m_dbScale);
	point.y = (long)(point.y / m_dbScale);

	if (m_wndResizer.IsSizing())
	{
		m_wndResizer.OnMouseMessage(nFlags, point);
		return;
	}

	if (m_WndDrag.IsDragging())
	{
		MLWidget::GetCursorPos(&point);
		m_WndDrag.OnDrag(nFlags, point);
		return;
	}

	// first send message to ui objects that captured the mouse
	if (m_pUIObjCapMouse)
	{
		if (IsMouseCaptured())
		{
			if (m_pUIObjCapMouse->OnMouseMove(nFlags, point))
				return;
		}
		else
		{
			// capture lost!
		}
	}

	// then let root container to process it
	if (!m_rootConainter.OnMouseMove(nFlags, point))
	{
		// else, should set mouse resizing cursor
		m_wndResizer.SetSizeCursor(nFlags, point);
	}

#ifdef _WIN32_DESKTOP
	//
	// Change translucency on hover?
	//
	if (m_bTranslucencyLayered && m_bPaintBg
		&& m_translucencyStatus == TS_NORMAL
		&& !m_bActived)
	{
		m_translucencyStatus = TS_ON_HOVER;

		m_bOnMouseHover = Tobool(m_rcBoundBox.PtInRect(point));

		StartTranslucencyFade();
	}
#endif	// _WIN32_DESKTOP
}

void CSkinWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	OnMouseActiveMsg();

	if (m_cursor.IsValid())
		MLWidget::SetCursor(m_cursor);

	point.x = (long)(point.x / m_dbScale);
	point.y = (long)(point.y / m_dbScale);

	SetFocus();

	// first send message to ui objects that captured the mouse!
	if (m_pUIObjCapMouse)
	{
		if (IsMouseCaptured())
		{
			if (m_pUIObjCapMouse->OnLButtonDown(nFlags, point))
				return;
		}
		else
		{
			// capture lost!
		}
	}

	// then let root container to process it
	if (!m_rootConainter.OnLButtonDown(nFlags, point))
	{
		m_wndResizer.OnMouseMessage(nFlags, point);

		if (!m_wndResizer.IsSizing())
		{
			MLWidget::GetCursorPos(&point);
			m_WndDrag.OnDrag(nFlags, point);
		}
	}
}

void CSkinWnd::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	OnMouseActiveMsg();

	if (m_cursor.IsValid())
		MLWidget::SetCursor(m_cursor);

	point.x = (long)(point.x / m_dbScale);
	point.y = (long)(point.y / m_dbScale);

	SetFocus();

	// first send message to ui objects that captured the mouse!
	if (m_pUIObjCapMouse)
	{
		if (IsMouseCaptured())
		{
			if (m_pUIObjCapMouse->OnLButtonDblClk(nFlags, point))
				return;
		}
		else
		{
			// capture lost!
		}
	}

	// then let root container to process it
	m_rootConainter.OnLButtonDblClk(nFlags, point);
}

void CSkinWnd::OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt)
{
	CUIObject	*pObjFocus = GetFocusUIObj();
	if (pObjFocus && pObjFocus->WantMsgMouseWheel())
	{
		pObjFocus->OnMouseWheel(nWheelDistance, nMkeys, pt);
		return;
	}

	// then let root container to process it
	m_rootConainter.OnMouseWheel(nWheelDistance, nMkeys, pt);
}

void CSkinWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_cursor.IsValid())
		MLWidget::SetCursor(m_cursor);

	point.x = (long)(point.x / m_dbScale);
	point.y = (long)(point.y / m_dbScale);

	if (m_wndResizer.IsSizing())
	{
		m_wndResizer.OnMouseMessage(nFlags, point);
//		bMsgProceed = true;
		return;
	}

	if (m_WndDrag.IsDragging())
	{
		POINT ptTemp;
		MLWidget::GetCursorPos(&ptTemp);
		m_WndDrag.OnDrag(nFlags, ptTemp);
//		return;
	}

	// first send message to ui objects that captured the mouse!
	if (m_pUIObjCapMouse)
	{
		if (IsMouseCaptured())
		{
			if (m_pUIObjCapMouse->OnLButtonUp(nFlags, point))
				return;
		}
		else
		{
			// capture lost!
		}
	}

	// then let root container to process it
	m_rootConainter.OnLButtonUp(nFlags, point);
}

void CSkinWnd::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (m_cursor.IsValid())
		MLWidget::SetCursor(m_cursor);

	point.x = (long)(point.x / m_dbScale);
	point.y = (long)(point.y / m_dbScale);

	// first send message to ui objects that captured the mouse!
	if (m_pUIObjCapMouse)
	{
		if (IsMouseCaptured() && m_pUIObjCapMouse->WantMsgRButton())
		{
			if (m_pUIObjCapMouse->OnRButtonDown(nFlags, point))
				return;
		}
		else
		{
			// capture lost!
		}
	}

	// then let root container to process it
	m_rootConainter.OnRButtonDown(nFlags, point);
}

void CSkinWnd::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_cursor.IsValid())
		MLWidget::SetCursor(m_cursor);

	point.x = (long)(point.x / m_dbScale);
	point.y = (long)(point.y / m_dbScale);

	// first send message to ui objects that captured the mouse!
	if (m_pUIObjCapMouse)
	{
		if (IsMouseCaptured() && m_pUIObjCapMouse->WantMsgRButton())
		{
			if (m_pUIObjCapMouse->OnRButtonUp(nFlags, point))
				return;
		}
		else
		{
			// capture lost!
		}
	}

	// then let root container to process it
	m_rootConainter.OnRButtonUp(nFlags, point);
}

// COMMENT
//		if ui objects in skin processed this message
//			return TRUE;
//		else
//			return FALSE;
void CSkinWnd::OnKeyDown(UINT nChar, UINT nFlags)
{
	CUIObject	*pObjFocus = GetFocusUIObj();

	// let focus uiobject to process key message
	if (pObjFocus && pObjFocus->WantMsgAllKeys())
	{
		pObjFocus->OnKeyDown(nChar, nFlags);
		return;
	}

	// tab key to switch between uiobject
	if (nChar == VK_TAB)	//TAB
	{
#ifdef WIN32
		short		nShift;
		bool		bShitDown;
		nShift = GetKeyState(VK_SHIFT);
		bShitDown = ((nShift & 0x8000) == 0x8000);
#else
		bool		bShitDown = IsFlagSet(nFlags, MK_SHIFT);
#endif
		// shift + VK_TAB
		if (bShitDown)
		{
			FocusPrevUIObject();
		}
		else
		{
			FocusNextUIObject();
		}

		// Ignore '\t' WM_CHAR message.
		m_bIgnoreNextOnCharMsg = true;

		return;
	}

	if (nChar == VK_RETURN)
	{
		// let focus uiobject to process key message
		if (pObjFocus && pObjFocus->WantMsgEnterKey())
			pObjFocus->OnKeyDown(nChar, nFlags);
	}

	// let focus uiobject to process key message
	if (pObjFocus && pObjFocus->WantMsgKey())
		pObjFocus->OnKeyDown(nChar, nFlags);
}

void CSkinWnd::OnKeyUp(UINT nChar, UINT nFlags)
{
	// let focus uiobject to process key message
	CUIObject	*pObjFocus = GetFocusUIObj();
	if (pObjFocus && pObjFocus->WantMsgKey())
		pObjFocus->OnKeyUp(nChar, nFlags);
}

void CSkinWnd::OnChar(UINT nChar)
{
	if (m_bIgnoreNextOnCharMsg)
	{
		m_bIgnoreNextOnCharMsg = false;
		return;
	}

	// let focus uiobject to process key message
	CUIObject	*pObjFocus = GetFocusUIObj();
	if (pObjFocus && pObjFocus->WantMsgKey())
		pObjFocus->OnChar(nChar);
}

bool CSkinWnd::UpdateSkinProperty()
{
	if (m_bUseRgn)
	{
		m_rgnSkin.StretchRgn(m_dbScale, m_rcReal.Width(), m_rcReal.Height());

		if (!SetWindowRgn(&m_rgnSkin))
		{
			return FALSE;
		}
	}
	else
	{
		if (m_bWindowsAppearance)
		{
			// Use windows appearance, set region to NULL, windows xp will auto apply the system region.
			SetWindowRgn(NULL);
		}
		else
		{
			// Skin mode, for windows xp or later, if the region is used, set rect region instead.
			CRegion		rgnOrg;
			int			n;

			// don't use any region
			SetWindowRgn(NULL);

#ifdef _WIN32
			// Is region used?
			rgnOrg.CreateRectRgn(0, 0, 1, 1);
			n = GetWindowRgn(m_hWnd, rgnOrg.m_rgn);
			if (n != ERROR)
			{
				// yes, use rect region instead.
				CRegion		rgnSkin;
				if (!rgnSkin.CreateRectRgn(0, 0, m_rcReal.Width(), m_rcReal.Height()))
					return FALSE;
				SetWindowRgn(&rgnSkin);
			}
#endif
		}
	}

	if (!m_cursor.IsValid())
	{
		m_cursor.LoadStdCursor(CCursor::C_ARROW);
	}
	if (m_cursor.IsValid())
		MLWidget::SetCursor(m_cursor);

	if (m_bTranslucencyLayered)
	{
		if (m_bActived)
			m_nTranslucencyAlphaDynamic = m_nTranslucencyAlphaOnActive;
		else
			m_nTranslucencyAlphaDynamic = m_nTranslucencyAlpha;
		RecreateBrush();
	}

	// init 
	OnWindowSizeChanged();

	return TRUE;
}

void CSkinWnd::OnPaint(IMLGraphicsBase *pGraph)
{
	if (IsIconic())
		return;

	CRawGraph		*pMemGraph;

	pMemGraph = GetMemGraphics();

	if (m_bPaintBg)
	{
		// pMemGraph->FillRect(0, 0, m_rcReal.Width(), m_rcReal.Height(), m_clrBg);
		RedrawBackground(pMemGraph, NULL);
	}

	// draw every ui objects on back buffer one by one
	m_rootConainter.Draw(pMemGraph);

	// from buffer to screen dc
	if (m_dbScale == 1.0)
		pMemGraph->DrawToGraphics(pGraph, 0, 0, m_rcBoundBox.Width(), m_rcBoundBox.Height(), 0, 0);
	else
		pMemGraph->DrawToGraphicsStretch(pGraph, 0, 0, m_rcReal.Width(), m_rcReal.Height(), 0, 0, m_rcBoundBox.Width(), m_rcBoundBox.Height());
}


CRawGraph *CSkinWnd::GetMemGraphics()
{
	//	assert(m_pmemGraph);
	if (!m_pmemGraph)
		RecreateMemGraphics();

	return m_pmemGraph;
}

bool CSkinWnd::RecreateMemGraphics()
{
	IMLGraphicsBase		*pGraph;

	pGraph = GetGraphics();

	if (m_pmemGraph)
		delete m_pmemGraph;

#ifdef WIN32
#ifdef _WINCE_
	CMemGraphics		*pMemGraph;
	pMemGraph = new CMemGraphics();
	pMemGraph->Create(m_rcBoundBox.Width(), m_rcBoundBox.Height(), pGraph);
	m_pmemGraph = pMemGraph;
#else
// 	if (m_bTranslucencyLayered)
// 	{
		CRawGraph				*pMemGraph;
		pMemGraph = new CRawGraph();
		pMemGraph->Create(m_rcBoundBox.Width(), m_rcBoundBox.Height(), pGraph);
		m_pmemGraph = pMemGraph;
// 	}
// 	else
// 	{
// 		if (CMLGraphicsFactory::GetGraphicsType() == MGT_GDIPLUS)
// 		{
// 			CMemGdiplusGraphics		*pMemGraph;
// 			pMemGraph = new CMemGdiplusGraphics();
// 			pMemGraph->Create(m_rcBoundBox.Width(), m_rcBoundBox.Height(), pGraph);
// 			m_pmemGraph = pMemGraph;
// 		}
// 		else
// 		{
// 			CMemGraphics		*pMemGraph;
// 			pMemGraph = new CMemGraphics();
// 			pMemGraph->Create(m_rcBoundBox.Width(), m_rcBoundBox.Height(), pGraph);
// 			m_pmemGraph = pMemGraph;
// 		}
// 	}
#endif
#else
	CRawGraph		*pGraphMem = new CRawGraph;

	if (!pGraphMem->Create(m_rcBoundBox.Width(), m_rcBoundBox.Height(), pGraph, 32))
	{
		ERRLOG0(_T("Create Mem DC FAILED!"));
	}
	m_pmemGraph = pGraphMem;
#endif

	if (m_bPaintBg)
	{
		// m_pmemGraph->FillRect(0, 0, m_rcReal.Width(), m_rcReal.Height(), m_clrBg);
		RedrawBackground(m_pmemGraph, NULL);
	}

	ReleaseGraphics(pGraph);

	return true;
}

void CSkinWnd::RecreateBrush()
{
	m_clrBg = m_clrBgOrg;
#ifdef _WIN32_DESKTOP
	if (m_bTranslucencyLayered)
	{
		m_clrBg.setAlpha((byte)m_nTranslucencyAlphaDynamic);
		PreMultiplyColor(m_clrBg, (byte)m_nTranslucencyAlphaDynamic);
	}
#endif
}

void CSkinWnd::OnWindowSizeChanged()
{
	if (IsIconic())
		return;

	RecreateMemGraphics();

	// recalculate all ui objects' position
	RecalculateSizePos();

	InvalidateRect();
}

void CSkinWnd::RecalculateSizePos()
{
	m_rootConainter.m_rcObj.SetRect(0, 0, m_rcBoundBox.Width(), m_rcBoundBox.Height());
	m_rootConainter.RecalculateSizePos();
}

BOOL CSkinWnd::AddTool(LPCTSTR szText, LPCRECT lpRectTool, UINT nIDTool)
{
	ASSERT(!IsEmptyString(szText));
	return m_wndToolTip.AddTool(szText, 
			lpRectTool, nIDTool);
}

void CSkinWnd::DelTool(UINT nIDTool)
{
	if (m_wndToolTip.IsWindow())
		m_wndToolTip.DelTool(nIDTool);
}

void CSkinWnd::InvalidateUIObject(CUIObject *pObj)
{
	assert(pObj);

	if (m_nInRedrawUpdate > 0 || !IsVisible() || IsIconic())
		return;

	if (!pObj->IsVisible() || !pObj->IsParentVisible())
		return;

	if (!m_pmemGraph)
		return;	// SkinWnd isn't created yet.

	if (pObj->IsUseSkinBg() && m_bPaintBg)
	{
		// redraw background ?
		RedrawBackground(m_pmemGraph, pObj->m_rcObj);
	}

	CRawGraph::CClipBoxAutoRecovery	autoCBR(m_pmemGraph);

	m_pmemGraph->SetClipBoundBox(pObj->m_rcObj);

	CRawGraph::COpacityBlendAutoRecovery opacityAR(m_pmemGraph, pObj->GetOpacity());

	pObj->Draw(m_pmemGraph);

	opacityAR.Recover();

	autoCBR.Recover();

#ifdef _WIN32_DESKTOP
	if (m_bTranslucencyLayered)
	{
		UpdateLayeredWindowUsingMemGraph(m_pmemGraph);
		return;
	}
#endif

	IMLGraphicsBase		*pGraphics;

	pGraphics = GetGraphics();

	if (m_dbScale == 1.0)
	{
		m_pmemGraph->DrawToGraphics(pGraphics,
			pObj->m_rcObj.left, pObj->m_rcObj.top, 
			pObj->m_rcObj.Width(), pObj->m_rcObj.Height(),
			pObj->m_rcObj.left, pObj->m_rcObj.top);
	}
	else
	{
		m_pmemGraph->DrawToGraphicsStretch(pGraphics,
			(int)(pObj->m_rcObj.left * m_dbScale), (int)(pObj->m_rcObj.top * m_dbScale), 
			(int)((pObj->m_rcObj.Width() + 1) * m_dbScale), (int)((pObj->m_rcObj.Height() + 1) * m_dbScale),
			pObj->m_rcObj.left, pObj->m_rcObj.top, pObj->m_rcObj.Width(), pObj->m_rcObj.Height());
	}

	ReleaseGraphics(pGraphics);
}

void CSkinWnd::RedrawBackground(CRawGraph *pGraph, RECT *prc)
{
	if (!pGraph)
		return;

	RECT	rc;

	if (prc)
	{
		rc = *prc;
		if (rc.left < 0)
			rc.left = 0;
		if (rc.top < 0)
			rc.top = 0;
		if (rc.bottom > m_rcBoundBox.Height())
			rc.bottom = m_rcBoundBox.Height();
		if (rc.right > m_rcBoundBox.Width())
			rc.right = m_rcBoundBox.Width();
	}
	else
	{
		rc.left = rc.top = 0;
		rc.right = m_rcBoundBox.Width();
		rc.bottom = m_rcBoundBox.Height();
	}

	CRect	rcClip, rcNewClip;
	pGraph->GetClipBoundBox(rcClip);

	rcNewClip.IntersectRect(&rcClip, &rc);

	CRawGraph::CClipBoxAutoRecovery	autoCBR(pGraph);
	pGraph->SetClipBoundBox(rcNewClip);

	if (m_imageBg.IsValid())
	{
		int				nAlphaOld = 255;
		CDrawImageFun	func;

		func.m_bpm = BPM_COPY;
		func.m_pGraph = pGraph;
		func.m_pImage = &m_imageBg;

		nAlphaOld = pGraph->GetOpacityPainting();
		pGraph->SetOpacityPainting((int)m_nTranslucencyAlphaDynamic);

		if (m_nTranslucencyAlphaDynamic != 255)
		{
			MLColor	clr(RGB(0, 0, 0));
			clr.setAlpha(0);
			pGraph->FillRect(&rc, clr);
		}

		if (m_nTranslucencyAlphaDynamic != 0)
			m_bgImagePainter.Blt(0, 0, m_rcBoundBox.Width(), m_rcBoundBox.Height(), func);

		pGraph->SetOpacityPainting(nAlphaOld);
	}
	else
		pGraph->FillRect(&rc, m_clrBg);
}

void CSkinWnd::EnterInDrawUpdate()
{
// 	if (m_nInRedrawUpdate > 0)
// 		ASSERT(m_nInRedrawUpdate == 0);
	m_nInRedrawUpdate++;
}

void CSkinWnd::LeaveInDrawUpdate()
{
	m_nInRedrawUpdate--;
	ASSERT(m_nInRedrawUpdate >= 0);
	if (m_nInRedrawUpdate == 0)
		InvalidateRect();
}

void CSkinWnd::SetMainAppWnd(bool bMainAppWnd)
{
	m_bMainAppWnd = bMainAppWnd;
}

void CSkinWnd::GetWndDragAutoCloseTo(vector<CBaseWnd *> &vWnd)
{
	m_pSkinFactory->GetWndDragAutoCloseTo(vWnd);
}

void CSkinWnd::GetWndDragTrackMove(vector<CBaseWnd *> &vWnd)
{
	if (m_bMainAppWnd)
		m_pSkinFactory->GetWndDragTrackMove(vWnd);
}

void CSkinWnd::SetCaptureMouse(CUIObject *pUIObj)
{
	assert(pUIObj);

	SetCapture();

	m_pUIObjCapMouse = pUIObj;
}

CUIObject * CSkinWnd::GetCaptureMouse()
{
	return m_pUIObjCapMouse;
}

void CSkinWnd::ReleaseCaptureMouse(CUIObject *pUIObj)
{
	ReleaseCapture();
	
	m_pUIObjCapMouse = NULL;
}
/*
void CSkinWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS *lpncsp)
{
	return;
	RECT *lpRect = (LPRECT) (lpncsp);//lParam;				 // new window coordinates 
	lpRect->left = m_rcReal.left;
	lpRect->right = m_rcReal.right;
	lpRect->top = m_rcReal.top;
	lpRect->bottom = m_rcReal.bottom;
}*/

void CSkinWnd::OnMove(int x, int y)
{
	if (!IsWindow())
		return;
	
	if (IsIconic())
		return;

	CRect		rc;

	GetWindowRect(&rc);
	if (!IsWndOutOfScreen(&rc))
	{
		rc.right = rc.left + m_rcReal.Width();
		rc.bottom = rc.top + m_rcReal.Height();
		m_rcReal = rc;
		m_rcBoundBox.left = rc.left;
		m_rcBoundBox.top = rc.top;
		m_rcBoundBox.right = (long)(rc.left + rc.Width() / m_dbScale);
		m_rcBoundBox.bottom = (long)(rc.top + rc.Height() / m_dbScale);

		if (m_pSjvmNotifyEvent)
			m_pSjvmNotifyEvent->onMove(x, y);
	}
}

void CSkinWnd::OnSize(int cx, int cy)
{
	// DWORD		dwStyle = GetWindowLong(m_hWnd, GWL_STYLE);
	// bool		bCaption = IsFlagSet(dwStyle, WS_CAPTION);
	if (!IsIconic())
	{
		CAutoRedrawLock	redrawLock(this);

		CRect		rc;
		GetWindowRect(&rc);
		rc.right = rc.left + cx;
		rc.bottom = rc.top + cy;
		if (IsWndOutOfScreen(&rc))
		{
			MoveWindow(m_rcReal);
		}
		else
		{
//			if (m_rcReal.EqualRect(&rc))
//				return;

			m_rcReal = rc;
			m_rcBoundBox.SetRect(rc.left, rc.top, rc.left + (int)(rc.Width() / m_dbScale), rc.top + (int)(rc.Height() / m_dbScale));

			if (m_bUseRgn)
			{
				m_rgnSkin.StretchRgn(m_dbScale, m_rcReal.Width(), m_rcReal.Height());
				SetWindowRgn(&m_rgnSkin);
			}
			else
			{
				if (m_bWindowsAppearance)
				{
					// Use windows appearance, set region to NULL, windows xp will auto apply the system region.
					SetWindowRgn(NULL);
				}
				else
				{
					// Skin mode, for windows xp or later, if the region is used, set rect region instead.
					CRegion		rgnOrg;
					int			n;

					// don't use any region
					SetWindowRgn(NULL);

#ifdef _WIN32
					// Is region used?
					rgnOrg.CreateRectRgn(0, 0, 1, 1);
					n = GetWindowRgn(m_hWnd, rgnOrg.m_rgn);
					if (n != ERROR)
					{
						// yes, use rect region instead.
						CRegion		rgnSkin;
						if (rgnSkin.CreateRectRgn(0, 0, m_rcReal.Width(), m_rcReal.Height()))
							SetWindowRgn(&rgnSkin);
					}
#endif
				}
			}

			OnWindowSizeChanged();

			if (m_pSjvmNotifyEvent)
				m_pSjvmNotifyEvent->onSize(cx, cy);
		}
	}
}

void CSkinWnd::OnSetFocus()
{
	CUIObject *pObj = GetFocusUIObj();
	if (pObj)
		pObj->OnSetFocus();
}

void CSkinWnd::OnKillFocus()
{
	CUIObject *pObj = GetFocusUIObj();
	if (pObj)
		pObj->OnKillFocus();	
}

BOOL CSkinWnd::MoveWindow(int X, int Y, int nWidth, int nHeight, BOOL bRepaint/* = TRUE*/)
{
	if (nHeight < m_wndResizer.GetMinCy())
		nHeight = m_wndResizer.GetMinCy();
	if (nWidth < m_wndResizer.GetMinCx())
		nWidth = m_wndResizer.GetMinCx();

#ifdef _WIN32
	if (::GetParent(m_hWnd))
		return true;

	if (m_bWindowsAppearance)
		return ::MoveWindow(m_hWnd, X, Y, nWidth, nHeight, bRepaint);
	else
#endif
		return MoveWindowSafely(X, Y, nWidth, nHeight);
}

BOOL CSkinWnd::MoveWindow(RECT &rc, BOOL bRepaint/* = TRUE*/)
{
	return MoveWindow(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, bRepaint);
}

void CSkinWnd::CreateUIObjects(SXNode *pSkinWndNode, CSkinContainer *pParentContainer)
{
	SXNode::iterator	it, itEnd;

	itEnd = pSkinWndNode->listChildren.end();
	for (it = pSkinWndNode->listChildren.begin(); it != itEnd; ++it)
	{
		SXNode			*pNode = *it;
		CUIObject		*pObj;

		if (IsPropertyName(pNode->strName.c_str(), SZ_PN_PROPERTY))
		{
			if (!SetProperty(pNode))
				DBGLOG2(_T("Unknow Property: %s, name: %s"), pNode->strName.c_str(), pNode->GetPropertySafe(SZ_PN_NAME));
			continue;
		}

		pObj = m_pSkinFactory->CreateUIObject(this, pNode->strName.c_str());
		if (pObj)
		{
			pObj->FromXML(pParentContainer, pNode);

			pParentContainer->AddUIObject(pObj);
		}
	}
}

void CSkinWnd::SetProperies(SXNode::LIST_PROPERTIES &listProperties)
{
	SXNode::iterProperties	it;
	for (it = listProperties.begin(); it != listProperties.end(); ++it)
	{
		SXNode::Property	&prop = *it;
		if (!SetProperty(prop.strName.c_str(), prop.strValue.c_str()))
		{
			m_listUnprocessedProperties.push_back(prop);
			DBGLOG2(_T("Unknow Property: %s, %s"), prop.strName.c_str(), prop.strValue.c_str());
		}
	}
}

BOOL CSkinWnd::SetProperty(SXNode *pXmlNode)
{
	LPCTSTR		szName = pXmlNode->GetPropertySafe(SZ_PN_NAME);
	LPCTSTR		szValue;
	ASSERT(szName);
	if (!szName)
		return FALSE;

	if (IsPropertyName(szName, _T("BgImage")))
	{
		// Set background image
		m_imageBg.LoadFromSRM(m_pSkinFactory, pXmlNode->GetPropertySafe(SZ_PN_IMAGE));

		GetRectValue(pXmlNode->GetPropertySafe(SZ_PN_IMAGERECT), m_imageBg);

		szValue = pXmlNode->GetPropertySafe(_T("HorzExtendPos"));
		Scan2IntX(szValue, m_bgImagePainter.nSrcHorzExtendStart, m_bgImagePainter.nSrcHorzExtendEnd);

		szValue = pXmlNode->GetPropertySafe(_T("VertExtendPos"));
		Scan2IntX(szValue, m_bgImagePainter.nSrcVertExtendStart, m_bgImagePainter.nSrcVertExtendEnd);

		m_bgImagePainter.nSrcX = m_imageBg.m_x;
		m_bgImagePainter.nSrcY = m_imageBg.m_y;
		m_bgImagePainter.nSrcWidth = m_imageBg.m_cx;
		m_bgImagePainter.nSrcHeight = m_imageBg.m_cy;

		return TRUE;
	}

	return FALSE;
}

BOOL CSkinWnd::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	assert(IsWindow());

	if (_tcsicmp(szProperty, _T("Region")) == 0)
	{
		TCHAR		szRgn[MAX_PATH];

		m_bUseRgn = false;
		m_strRgn = szValue;

		if (m_rgnSkin.IsValid())
			m_rgnSkin.Destroy();

		if (m_strRgn.empty())
			return TRUE;

		if (!m_pSkinFactory->GetResourceMgr()->GetResourcePathName(szRgn, CountOf(szRgn), szValue))
		{
			ERRLOG1(_T("Can't find region file: %s"), szValue);
			return TRUE;
		}

		if (!m_rgnSkin.LoadRgn(szRgn))
			ERRLOG1(_T("Load region file: %s FAILED!"), szRgn);
		else
		{
			m_bUseRgn = TRUE;
		}
	}
	else if (_tcsicmp(szProperty, _T("regionScaleX1")) == 0)
		m_rgnSkin.SetScaleX1(_ttoi(szValue));
	else if (_tcsicmp(szProperty, _T("regionScaleX2")) == 0)
		m_rgnSkin.SetScaleX2(_ttoi(szValue));
	else if (_tcsicmp(szProperty, _T("regionScaleY1")) == 0)
		m_rgnSkin.SetScaleY1(_ttoi(szValue));
	else if (_tcsicmp(szProperty, _T("regionScaleY2")) == 0)
		m_rgnSkin.SetScaleY2(_ttoi(szValue));
	else if (_tcsicmp(szProperty, _T("regionScaleHeightOrg")) == 0)
		m_rgnSkin.SetScaleHeightOrg(_ttoi(szValue));
	else if (_tcsicmp(szProperty, _T("regionScaleWidthOrg")) == 0)
		m_rgnSkin.SetScaleWidthOrg(_ttoi(szValue));
	else if (_tcsicmp(szProperty, _T("MinWidth")) == 0)
		m_wndResizer.SetMinCx(_ttoi(szValue));
	else if (_tcsicmp(szProperty, _T("MinHeight")) == 0)
		m_wndResizer.SetMinCy(_ttoi(szValue));
	else if (_tcsicmp(szProperty, SZ_PN_WIDTH) == 0)
	{
		m_nWidth = _ttoi(szValue);
// 		if (m_nWidth != 0)
// 		{
// 			m_rcBoundBox.right = m_rcBoundBox.left + m_nWidth;
// 			m_rcReal.right = m_rcReal.left + (long)(m_rcBoundBox.Width() * m_dbScale);
// 		}
	}
	else if (_tcsicmp(szProperty, SZ_PN_HEIGHT) == 0)
	{
		m_nHeight = _ttoi(szValue);
// 		if (m_nHeight != 0)
// 		{
// 			m_rcBoundBox.bottom = m_rcBoundBox.top + m_nHeight;
// 			m_rcReal.bottom = m_rcReal.top + (long)(m_rcBoundBox.Height() * m_dbScale);
// 		}
	}
	else if (_tcsicmp(szProperty, _T("Cursor")) == 0)
	{
		m_cursor.Destroy();
		m_strCusor = szValue;
		if (!m_strCusor.empty())
		{
			TCHAR			szFile[MAX_PATH];
			if (m_pSkinFactory->GetResourceMgr()->GetResourcePathName(szFile, CountOf(szFile), szValue))
				m_cursor.LoadCursorFromFile(szFile);
			else
				ERRLOG1(_T("Can't Load cursor file: %s"), szValue);
		}
	}
	else if (_tcsicmp(szProperty, _T("FixedWidth")) == 0)
		m_wndResizer.FixedWidth(IsTRUE(szValue));
	else if (_tcsicmp(szProperty, _T("FixedHeight")) == 0)
		m_wndResizer.FixedHeight(IsTRUE(szValue));
	else if (_tcsicmp(szProperty, _T("PaintBg")) == 0)
	{
		m_bPaintBg = IsTRUE(szValue);
		if (m_bPaintBg)
			RecreateBrush();
	}
	else if (_tcsicmp(szProperty, _T("BgColor")) == 0 || _tcsicmp(szProperty, _T("BkColor")) == 0
		|| _tcsicmp(szProperty, _T("BackgroundColor")) == 0)
	{
		GetColorValue(m_clrBgOrg, szValue);
		RecreateBrush();
	}
	else if (_tcsicmp(szProperty, _T("FgColor")) == 0)
	{
		GetColorValue(m_clrFg, szValue);
	}
	else if (_tcsicmp(szProperty, SZ_PN_NAME) == 0)
	{
		m_strSkinWndName = szValue;
	}
	else if (IsPropertyName(szProperty, _T("Caption")))
	{
		m_strCaption = szValue;
		SetWindowText(TOLOCAL(szValue));
	}
	else if (_tcsicmp(szProperty, _T("ContextMenu")) == 0)
	{
		m_rootConainter.SetProperty(szProperty, szValue);
	}
	else if (_tcsicmp(szProperty, _T("IndividualProperties")) == 0)
	{
		m_bIndividualProperties = IsTRUE(szValue);
	}
	else if (m_fontProperty.SetProperty(szProperty, szValue))
	{
		if (m_bSkinOpened)
		{
			// Notify every child to change its font if needed.
			m_rootConainter.OnSkinFontChanged();

			// Redraw
			InvalidateRect();
		}
		return TRUE;
	}
#ifdef _WIN32_DESKTOP
	else if (_tcsicmp(szProperty, _T("WindowsAppearance")) == 0)
	{
		m_bWindowsAppearance = IsTRUE(szValue);
		if (m_bWindowsAppearance)
		{
			SetWindowLong(m_hWnd, GWL_STYLE, GetWindowLong(m_hWnd, GWL_STYLE) | WS_CAPTION | WS_THICKFRAME);
			::SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_DEFERERASE);
		}
	}
	else if (IsPropertyName(szProperty, _T("EnableClickThrough")))
	{
		m_bEnableClickThrough = IsTRUE(szValue);
	}
	else if (IsPropertyName(szProperty, _T("EnableTranslucency")))
	{
		m_bTranslucencyLayered = IsTRUE(szValue);
		if (m_pmemGraph)
		{
			delete m_pmemGraph;
			m_pmemGraph = NULL;
		}
	}
	else if (IsPropertyName(szProperty, _T("TranslucencyAlpha")))
	{
		m_nTranslucencyAlpha = _ttoi(szValue);
		if (m_nTranslucencyAlphaOnActive == -1)
			m_nTranslucencyAlphaOnActive = m_nTranslucencyAlpha;
		if (m_nTranslucencyAlphaOnHover == -1)
			m_nTranslucencyAlphaOnHover = m_nTranslucencyAlpha;
		if (m_bSkinOpened)
			StartTranslucencyFade();
		RecreateBrush();
	}
	else if (IsPropertyName(szProperty, _T("TranslucencyAlphaOnActive")))
	{
		m_nTranslucencyAlphaOnActive = _ttoi(szValue);
		if (m_nTranslucencyAlpha == -1)
			m_nTranslucencyAlpha = m_nTranslucencyAlphaOnActive;
		if (m_bSkinOpened)
			StartTranslucencyFade();
		RecreateBrush();
	}
	else if (IsPropertyName(szProperty, _T("TranslucencyAlphaOnHover")))
	{
		m_nTranslucencyAlphaOnHover = _ttoi(szValue);
	}
	else if (IsPropertyName(szProperty, _T("BgImage.Image")))
	{
		m_imageBg.LoadFromSRM(m_pSkinFactory, szValue);
	}
#endif
	else
		return FALSE;

	return TRUE;
}

#ifdef _SKIN_EDITOR_
void CSkinWnd::EnumProperties(CUIObjProperties &listProperties)
{
	listProperties.AddPropStr(SZ_PN_NAME, m_strSkinWndName.c_str());
	listProperties.AddPropStr(_T("Caption"), m_strCaption.c_str(), !m_strCaption.empty());

	bool		bToXmlAttrib;

	bToXmlAttrib = !m_strRgn.empty();
	listProperties.AddPropFile(_T("Region"), m_strRgn.c_str(), _T("*.tr2"), bToXmlAttrib);
// 	listProperties.AddPropInt(_T("RgnStretchStartX"), m_rgnSkin.GetScaleX1(), bToXmlAttrib);
// 	listProperties.AddPropInt(_T("RgnStretchEndX"), m_rgnSkin.GetScaleX2(), bToXmlAttrib);
// 	listProperties.AddPropInt(_T("RgnStretchStartY"), m_rgnSkin.GetScaleY1(), bToXmlAttrib);
// 	listProperties.AddPropInt(_T("RgnStretchEndY"), m_rgnSkin.GetScaleY2(), bToXmlAttrib);
// 	listProperties.AddPropInt(_T("RgnHeight"), m_rgnSkin.GetScaleHeightOrg(), bToXmlAttrib);
// 	listProperties.AddPropInt(_T("RgnWidth"), m_rgnSkin.GetScaleWidthOrg(), bToXmlAttrib);

	listProperties.AddPropInt(_T("MinWidth"), m_wndResizer.GetMinCx(), m_wndResizer.GetMinCx() != 0);
	listProperties.AddPropInt(_T("MinHeight"), m_wndResizer.GetMinCy(), m_wndResizer.GetMinCy() != 0);
	listProperties.AddPropInt(SZ_PN_WIDTH, m_nWidth, m_nWidth != 0);
	listProperties.AddPropInt(SZ_PN_HEIGHT, m_nHeight, m_nHeight != 0);
	listProperties.AddPropFile(_T("Cursor"), m_strCusor.c_str(), _T("*.cur"), !m_strCusor.empty());
	listProperties.AddPropBoolStr(_T("FixedWidth"), m_wndResizer.IsFixedWidth(), m_wndResizer.IsFixedWidth());
	listProperties.AddPropBoolStr(_T("FixedHeight"), m_wndResizer.IsFixedHeight(), m_wndResizer.IsFixedHeight());

	listProperties.AddPropBoolStr(_T("PaintBg"), m_bPaintBg);
	listProperties.AddPropColor(_T("BgColor"), m_clrBgOrg);

	listProperties.AddPropColor(_T("FgColor"), m_clrFg);

	m_fontProperty.EnumProperties(listProperties);

	listProperties.AddPropStr(_T("ContextMenu"), m_rootConainter.m_strContextMenu.c_str(), !m_rootConainter.m_strContextMenu.empty());

	listProperties.AddPropBoolStr(_T("IndividualProperties"), m_bIndividualProperties, m_bIndividualProperties);

#ifdef _WIN32_DESKTOP
	listProperties.AddPropBoolStr(_T("WindowsAppearance"), m_bWindowsAppearance, m_bWindowsAppearance);
	listProperties.AddPropBoolStr(_T("EnableTransparentFeature"), m_bEnableClickThrough, !m_bEnableClickThrough);
	listProperties.AddPropBoolStr(_T("EnableTranslucency"), m_bTranslucencyLayered, m_bTranslucencyLayered);
	listProperties.AddPropInt(_T("TranslucencyAlpha"), m_nTranslucencyAlpha, m_nTranslucencyAlpha != 255);
	listProperties.AddPropInt(_T("TranslucencyAlphaOnActive"), m_nTranslucencyAlphaOnActive, m_nTranslucencyAlphaOnActive != 255);
	listProperties.AddPropInt(_T("TranslucencyAlphaOnHover"), m_nTranslucencyAlphaOnHover, m_nTranslucencyAlphaOnHover != 255);
#endif
}
#endif // _SKIN_EDITOR_

bool CSkinWnd::GetUnprocessedProperty(LPCTSTR szProperty, tstring &strValue)
{
	for (SXNode::iterProperties it = m_listUnprocessedProperties.begin(); it != m_listUnprocessedProperties.end(); ++it)
	{
		SXNode::Property	&prop = *it;
		if (_tcsicmp(szProperty, prop.strName.c_str()) == 0)
		{
			strValue = prop.strValue;
			return true;
		}
	}

	return false;
}

// 
// BOOL CSkinWnd::AddUIObject(CUIObject *pObj)
// {
// 	return m_rootConainter.AddUIObject(pObj);
// }
// 
// BOOL CSkinWnd::AddUIObjectToFront(CUIObject *pObj)
// {
// 	return m_rootConainter.AddUIObjectToFront(pObj);
// }

void CSkinWnd::FocusPrevUIObject()
{
	CUIObject			*pObjFocusOld;

	pObjFocusOld = m_itFocusUIObject.Get();
	if (!pObjFocusOld)
	{
		m_itFocusUIObject.End();
		m_itFocusUIObject--;
	}
	else
		m_itFocusUIObject--;

	while (1)
	{
		CUIObject			*pObj = m_itFocusUIObject.Get();

		if (pObj && pObj->WantMsgKey() && pObj->IsVisible() && pObj->IsParentVisible())
		{
			OnSwitchChildFocus(pObjFocusOld, pObj);
			return;
		}

		// Does loop backup to start?
		if (pObj == pObjFocusOld)
			break;

		if (m_itFocusUIObject.IsBegin())
			m_itFocusUIObject.End();

		m_itFocusUIObject--;
	}

	m_itFocusUIObject.End();
}


void CSkinWnd::FocusNextUIObject()
{
	CUIObject			*pObjFocusOld;

	pObjFocusOld = m_itFocusUIObject.Get();
	if (!pObjFocusOld)
		m_itFocusUIObject.Begin();
	else
		m_itFocusUIObject++;

	while (1)
	{
		CUIObject			*pObj = m_itFocusUIObject.Get();

		if (pObj && pObj->WantMsgKey() && pObj->IsVisible() && pObj->IsParentVisible())
		{
			OnSwitchChildFocus(pObjFocusOld, pObj);
			return;
		}

		// Does loop backup to start?
		if (pObj == pObjFocusOld)
			break;

		if (m_itFocusUIObject.IsEnd())
			m_itFocusUIObject.Begin();
		else
			m_itFocusUIObject++;
	}

	m_itFocusUIObject.End();

	OnSwitchChildFocus(pObjFocusOld, NULL);
}


void CSkinWnd::VerifyFocusUIObject()
{
	CUIObject		*pObj = m_itFocusUIObject.Get();

	if (pObj && pObj->WantMsgKey())
		return;

	FocusNextUIObject();
}


void CSkinWnd::SetFocusUIObject(CUIObject *pObj)
{
	ASSERT(pObj);

	CUIObject		*pObjFocusOld = m_itFocusUIObject.Get();

	if (pObjFocusOld == pObj)
		return;

	if (pObjFocusOld)
		pObjFocusOld->OnKillFocus();
	m_itFocusUIObject.From(pObj);
	pObj->OnSetFocus();
}

void CSkinWnd::SetFocusUIObject(LPCTSTR szId)
{
	CUIObject *pObj = GetUIObjectById(m_pSkinFactory->GetIDByName(szId), NULL);
	if (!pObj)
	{
		DBGLOG1(_T("UIObject: %s doesn't exist."), szId);
		return;
	}

	SetFocusUIObject(pObj);
}

bool CSkinWnd::EnableUIObject(int nId, bool bEnable, bool bRedraw)
{
	CUIObject *pObj = GetUIObjectById(nId, NULL);
	if (!pObj)
	{
		DBGLOG1(_T("UIObject: %s doesn't exist."), GetSkinFactory()->GetStringOfID(nId).c_str());
		return false;
	}

	if (Tobool(pObj->m_bEnable) != bEnable)
	{
		pObj->m_bEnable = bEnable;

		if (bRedraw)
			pObj->Invalidate();
	}

	return true;
}

bool CSkinWnd::SetUIObjectProperty(int nId, LPCTSTR szProperty, LPCTSTR szValue)
{
	CUIObject *pObj = GetUIObjectById(nId, NULL);
	if (!pObj)
	{
		DBGLOG1(_T("UIObject: %s doesn't exist."), GetSkinFactory()->GetStringOfID(nId).c_str());
		return false;
	}

	return Tobool(pObj->SetProperty(szProperty, szValue));
}

void CSkinWnd::SetUIObjectVisible(int nId, bool bVisible, bool bRedraw)
{
	CUIObject *pObj = GetUIObjectById(nId, NULL);
	if (!pObj)
	{
		DBGLOG1(_T("UIObject: %s doesn't exist."), GetSkinFactory()->GetStringOfID(nId).c_str());
		return;
	}

	pObj->SetVisible(bVisible);

	if (bRedraw)
	{
		if (bVisible)
			pObj->Invalidate();
		else
			pObj->m_pContainer->Invalidate();
	}
}

void CSkinWnd::InvalidateUIObject(int nId)
{
	CUIObject *pObj = GetUIObjectById(nId, NULL);
	if (!pObj)
	{
		DBGLOG1(_T("UIObject: %s doesn't exist."), GetSkinFactory()->GetStringOfID(nId).c_str());
		return;
	}

	InvalidateUIObject(pObj);
}

tstring CSkinWnd::GetUIObjectText(int nId)
{
	CUIObject *pObj = GetUIObjectById(nId, NULL);
	if (!pObj)
	{
		DBGLOG1(_T("UIObject: %s doesn't exist."), GetSkinFactory()->GetStringOfID(nId).c_str());
		return _T("");
	}

	return pObj->GetText();
}

void CSkinWnd::SetUIObjectText(int nId, LPCTSTR szText, bool bRedraw)
{
	CUIObject *pObj = GetUIObjectById(nId, NULL);
	if (!pObj)
	{
		DBGLOG1(_T("UIObject: %s doesn't exist."), GetSkinFactory()->GetStringOfID(nId).c_str());
		return;
	}

	pObj->SetProperty(SZ_PN_TEXT, szText);
	if (bRedraw)
		pObj->Invalidate();
}

CUIObject *CSkinWnd::RemoveUIObjectById(int nId)
{
	return m_rootConainter.RemoveUIObjectById(nId);
}

bool CSkinWnd::RemoveUIObject(CUIObject *pObj, bool bFree)
{
	return m_rootConainter.RemoveUIObject(pObj, bFree);
}

void CSkinWnd::OnDestroy()
{
	CloseSkin();

	m_wndToolTip.Destroy();

	CBaseWnd::OnDestroy();

	if (m_bMainAppWnd)
		m_pSkinFactory->OnMainSkinWndDestory(this);

	if (m_bManageBySkinFactory)
		m_pSkinFactory->OnSkinWndDestory(this);
}


// COMMENT:
//		OnCreate must return true to continue the creation of the CWnd object. If the application returns false, the window will be destroyed.
bool CSkinWnd::OnCreate()
{
	if (!CBaseWnd::OnCreate())
		return false;

	//ASSERT(!m_wndToolTip.IsValid());
	if (!m_wndToolTip.Create(this))
		return false;

	if (m_bManageBySkinFactory)
		m_pSkinFactory->OnSkinWndCreate(this);

	m_WndDrag.Init(this, this);//, arrszClassCloseTo);
	m_wndResizer.Init(this, this);//, arrszClassCloseTo, this);

	// Load skin
	OpenDefaultSkin();

	// If translucency is used, call this to update screen explicitly
	if (m_bTranslucencyLayered)
		InvalidateRectOfLayeredWindow(NULL);

	return true;
}

bool CSkinWnd::IsWndActive()
{
	return m_bActived;
}

void CSkinWnd::OnActivate(bool bActived)
{
	CAutoRedrawLock	redrawLock(this);

	m_bActived = bActived;

#ifdef _WIN32_DESKTOP
	StartTranslucencyFade();

	if (bActived)
		m_translucencyStatus = TS_ON_ACTIVE;
	else
		m_translucencyStatus = TS_NORMAL;

	if (m_bEnableClickThrough && m_bClickThrough)
		SetTransparent(m_nAlpha, false, m_clrBg.get(), m_bClickThrough);
#endif
	if (bActived != m_bMouseActive)
	{
		m_bMouseActive = bActived;
		OnMouseActive(m_bMouseActive);
	}

	if (m_pSjvmNotifyEvent)
		m_pSjvmNotifyEvent->onActivate(bActived);
}

void CSkinWnd::AddWndCloseto(CBaseWnd *pWnd, LPCTSTR szWndClass, LPCTSTR szWndName)
{
	m_WndDrag.AddWndCloseto(pWnd, szWndClass, szWndName);
}

void CSkinWnd::TrackMove(CBaseWnd *pWnd, int x, int y)
{
	m_WndDrag.TrackMoveWith(pWnd, x, y);
//	m_WndDrag.TrackCheck();
}

void CSkinWnd::UpdateMemGraphicsToScreen(const RECT* lpRect)
{
	if (m_nInRedrawUpdate > 0 || !IsVisible() || IsIconic())
		return;

#ifdef _WIN32
	if (GetEnableTranslucencyLayered())
		UpdateLayeredWindowUsingMemGraph(m_pmemGraph);
	else
#endif
	{
		IMLGraphicsBase		*pGraphics;

		pGraphics = GetGraphics();

		m_pmemGraph->DrawToGraphics(pGraphics, 
			lpRect->left, lpRect->top, 
			lpRect->right - lpRect->left, lpRect->bottom - lpRect->top,
			lpRect->left, lpRect->top);

		ReleaseGraphics(pGraphics);
	}
}

#ifdef _WIN32

int CSkinWnd::ms_msgIDCustomCommand = RegisterWindowMessage(_T("SkinWndCustomMsgID"));

LRESULT CSkinWnd::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN32_DESKTOP
	if (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST && m_wndToolTip.IsValid())
	{
		MSG		msgRelay;

		msgRelay.hwnd = m_hWnd;
		msgRelay.message = message;
		msgRelay.lParam = lParam;
		msgRelay.wParam = wParam;

		m_wndToolTip.RelayEvent(&msgRelay);
	}
	else if (message == WM_NOTIFY)
	{
		NMHDR	*pnmh = (NMHDR *)lParam;

		if (pnmh->code == TTN_GETDISPINFO)
		{
			NMTTDISPINFO	*lpnmtdi = (NMTTDISPINFO *)lParam;

			if ((lpnmtdi->uFlags & TTF_IDISHWND) != TTF_IDISHWND)
			{
				int		nId;

				nId = lpnmtdi->hdr.idFrom;

				CUIObject	*pObj;

				pObj = GetUIObjectById(nId);
				if (pObj)
				{
					_tcscpy_safe(lpnmtdi->szText, CountOf(lpnmtdi->szText), TOLOCAL(pObj->m_strTooltip.c_str()));
					lpnmtdi->lpszText = lpnmtdi->szText;
					lpnmtdi->hinst = NULL;
				}
			}
		}
	}
	else if (message == WM_GETDLGCODE)
	{
		return DLGC_WANTMESSAGE;
	}
	else if (message == WM_CAPTURECHANGED)
	{
		m_pUIObjCapMouse = NULL;
	}
	else if (message == WM_SYSCHAR)
	{
		if (m_rootConainter.OnMenuKey(wParam, lParam))
			return 0;
	}
// 	else if (message == WM_SYSCOMMAND)
// 	{
// 		
// 		return 0;
// 	}
#endif

	if (message == ms_msgIDCustomCommand)
	{
		OnCustomCommand(wParam);
		return 0;
	}

	switch (message)
	{
#ifndef _WINCE_
	case WM_NCCALCSIZE:
		{
			// reset its client area.
//			fCalcValidRects = (BOOL) wParam;		// valid area flag 
//			lpncsp = (LPNCCALCSIZE_PARAMS) lParam;	// size calculation data	or	 
//			OnNcCalcSize((BOOL)wParam, (LPNCCALCSIZE_PARAMS)lParam);
			if (m_bWindowsAppearance)
				break;
			return 0;
		}
// 	case WM_CAPTURECHANGED:
// 		if (m_pUIObjCapMouse)
// 			ReleaseCaptureMouse(m_pUIObjCapMouse);
// 		break;
#endif
	case WM_DESTROY:
		{
			// after process WM_DESTORY, delete this.
			OnDestroy();
			if (m_hWnd)
			{
				SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)0);
				DefWindowProc(m_hWnd, message, wParam, lParam);
				m_hWnd = NULL;
			}
			if (m_bFreeOnDestory)
				delete this;
			// g_mapWnd.remove(m_hWnd);
		}
		return 0;
	case WM_ERASEBKGND:
		return 0;
	case WM_PAINT:
		{
#ifdef _WIN32_DESKTOP
			if (m_bTranslucencyLayered)
			{
				// updates the position, size, shape, content, and translucency of a layered window
				return DefWindowProc(m_hWnd, message, wParam, lParam);
			}
			else
#endif
			{
				PAINTSTRUCT		ps;
				HDC				hdc;
				hdc = BeginPaint(m_hWnd, &ps);
				IMLGraphicsBase	*pGraph;

				pGraph = CMLGraphicsFactory::CreateMLGraphics(hdc, m_graphType);
				assert(pGraph);

				OnPaint(pGraph);

				CMLGraphicsFactory::ReleaseMLGraphics(pGraph);

				EndPaint(m_hWnd, &ps);
			}
		}
		return 0;
	}

	return CBaseWnd::WndProc(message, wParam, lParam);
}
#endif

void CSkinWnd::EnableTranslucencyLayered(bool bTranslucencyLayered)
{
	if (m_pmemGraph)
	{
		delete m_pmemGraph;
		m_pmemGraph = NULL;
	}

	m_bTranslucencyLayered = bTranslucencyLayered;
}

#ifdef _WIN32_DESKTOP

void CSkinWnd::SetTransparent(BYTE nAlpha, bool bTransparentClr, COLORREF clrTransparent, bool bClickThrough)
{
	// If it's child old, ignore this.
	if (::GetParent(m_hWnd))
		return;

	m_nAlpha = nAlpha;
	m_bClickThrough = bClickThrough;

	if (!m_bEnableClickThrough)
		return;
	if (m_bTranslucencyLayered)
	{
		InvalidateRect(NULL, true);
		return;
	}

	COLORREF		clrTrans = RGB(0, 0, 0);
	DWORD			dwFlags = 0;

	if (nAlpha < 255)
		dwFlags = LWA_ALPHA;

	if (bTransparentClr)
	{
		clrTrans = clrTransparent;
		dwFlags |= LWA_COLORKEY;
	}

	if (bClickThrough && !m_bActived)
	{
		SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
	}
	else
	{
		SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT);
	}

	if (dwFlags != 0 || (bClickThrough && !m_bActived))
		::SetLayeredWindow(m_hWnd, clrTrans, nAlpha, dwFlags);
	else
		::UnSetLayeredWindow(m_hWnd);
}

BOOL CSkinWnd::InvalidateRect(const RECT *lpRect, BOOL bErase)
{
	if (m_nInRedrawUpdate > 0 || !IsVisible() || IsIconic())
		return FALSE;

	if (!m_bTranslucencyLayered)
		return CBaseWnd::InvalidateRect(lpRect, bErase);

	InvalidateRectOfLayeredWindow(lpRect);

	return TRUE;
}

void CSkinWnd::InvalidateRectOfLayeredWindow(const RECT* lpRect)
{
	ASSERT(m_bTranslucencyLayered);
	CRawGraph	*pMemGraph;

	pMemGraph = GetMemGraphics();
	CRawGraph::CClipBoxAutoRecovery	autoCBR(pMemGraph);
	if (lpRect)
		pMemGraph->SetClipBoundBox(*lpRect);

	if (m_bPaintBg)
	{
		// pMemGraph->FillRect(0, 0, m_rcReal.Width(), m_rcReal.Height(), m_clrBg);
		RedrawBackground(pMemGraph, NULL);
	}

	// draw every ui objects on back buffer one by one
	m_rootConainter.Draw(pMemGraph);

	UpdateLayeredWindowUsingMemGraph(pMemGraph);
}

/*
void TestUpdateLayeredWindow(HWND hWnd, HBITMAP hbmp, int nBmpWidth, int nBmpHeight)
{
	HDC		hdc, hdcSrc;
	HBITMAP	hbmpOld;
	BLENDFUNCTION	blend;
	POINT	ptSrc, ptDest;
	SIZE	sizeWnd;
	RECT	rcWnd;
	DWORD	dwStyle;

	hdc = GetDC(hWnd);

	// ????WS_EX_LAYERED??
	dwStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	dwStyle |= WS_EX_LAYERED;
	// SetWindowLong(hWnd, GWL_EXSTYLE, dwStyle & ~ (WS_EX_LAYERED));
	SetWindowLong(hWnd, GWL_EXSTYLE, dwStyle);

	hdcSrc = CreateCompatibleDC(hdc);

	hbmpOld = (HBITMAP)SelectObject(hdcSrc, hbmp);

#if !defined(AC_SRC_ALPHA)
#define AC_SRC_ALPHA                0x01
#endif

	GetWindowRect(hWnd, &rcWnd);

	blend.AlphaFormat = AC_SRC_ALPHA;
	blend.BlendOp = AC_SRC_OVER;
	blend.BlendFlags = 0;
	blend.SourceConstantAlpha = 255;

	ptSrc.x = ptSrc.y = 0;
	ptDest.x = rcWnd.left;
	ptDest.y = rcWnd.top;
	sizeWnd.cx = nBmpWidth;
	sizeWnd.cy = nBmpHeight;

	BOOL bRet = UpdateLayeredWindow(hWnd, hdc, &ptDest, &sizeWnd, hdcSrc, &ptSrc, 0, &blend, ULW_ALPHA);

	SelectObject(hdcSrc, hbmpOld);

	ReleaseDC(hWnd, hdc);
	DeleteDC(hdcSrc);
}

HBITMAP Load32Bitbmp(LPCTSTR szImage, int &nWidth, int &nHeight)
{
	CImgPngFile		png;
	CImageData		imgData;

	png.SetImgData(&imgData);
	if (!png.Open(szImage))
		return NULL;

	nWidth = imgData.GetWidth();
	nHeight = imgData.GetHeight();

	return imgData.ToHBitmap(NULL);
}*/

BOOL CSkinWnd::UpdateLayeredWindowUsingMemGraph(CRawGraph *pGraph)
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
	if (m_bEnableClickThrough && m_bClickThrough && !m_bActived)
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

void CSkinWnd::StartTranslucencyFade()
{
	if (m_bTranslucencyLayered && m_bPaintBg)
	{
		m_dwTransFadeinBegTime = GetTickCount();
		m_nTransAlphaBeg = m_nTranslucencyAlphaDynamic;

		KillTimer(TIMER_ID_DYNAMIC_TRANS);
		SetTimer(TIMER_ID_DYNAMIC_TRANS, TIMER_SPAN_DYNAMIC_TRANS);
	}
}

void CSkinWnd::OnTimerDynamicAlphaChange()
{
	int			nTargetAlpha;

	if (m_translucencyStatus == TS_ON_HOVER)
	{
		//
		// Adjust translucency alpha on mouse hover in/out.
		//

		POINT		point;
		bool		bAlphaAdjusted = false;
		bool		bOnMouseHover;

		GetCursorPos(&point);
		bOnMouseHover = Tobool(m_rcBoundBox.PtInRect(point));
		if (m_bOnMouseHover != bOnMouseHover)
		{
			m_bOnMouseHover = bOnMouseHover;
			m_dwTransFadeinBegTime = GetTickCount();
			m_nTransAlphaBeg = m_nTranslucencyAlphaDynamic;
		}

		if (m_bOnMouseHover)
			nTargetAlpha = m_nTranslucencyAlphaOnHover;
		else
			nTargetAlpha = m_nTranslucencyAlpha;
	}
	else if (IsWndActive())
		nTargetAlpha = m_nTranslucencyAlphaOnActive;
	else
		nTargetAlpha = m_nTranslucencyAlpha;

	DWORD		dwTimeCur = GetTickCount();
	int			nAlphaNew;
	if (dwTimeCur - m_dwTransFadeinBegTime > TIME_OUT_TRANS_FADEIN)
	{
		KillTimer(TIMER_ID_DYNAMIC_TRANS);
		if (m_translucencyStatus == TS_ON_HOVER)
			m_translucencyStatus = TS_NORMAL;
		nAlphaNew = nTargetAlpha;
	}
	else
	{
		float		fRadio = ((float)(dwTimeCur - m_dwTransFadeinBegTime)) / TIME_OUT_TRANS_FADEIN;
		nAlphaNew = m_nTransAlphaBeg + int((nTargetAlpha - m_nTransAlphaBeg) * fRadio);
	}
	if (m_nTranslucencyAlphaDynamic != nAlphaNew)
	{
		m_nTranslucencyAlphaDynamic = nAlphaNew;
		ASSERT(m_nTranslucencyAlphaDynamic >= 0 && m_nTranslucencyAlphaDynamic <= 255);

		RecreateBrush();
		InvalidateRect();
	}
}

#else

void CSkinWnd::SetTransparent(BYTE nAlpha, bool bTransparentClr, COLORREF clrTransparent, bool bClickThrough)
{
}

#endif

void CSkinWnd::OnSkinLoaded()
{
	if (GetFocusUIObj() == NULL)
		FocusNextUIObject();

	if (m_bManageBySkinFactory)
		CSjvmSkinSystem::OnSkinWndCreate(this);

	if (m_pSjvmNotifyEvent)
		m_pSjvmNotifyEvent->onCreate(this);
}

void CSkinWnd::OnLoadWndSizePos()
{
	CRect		rcNew;

	rcNew = m_rcReal;
	if (m_wndResizer.IsFixedWidth())
		rcNew.right = rcNew.left + (long)(m_nWidth * m_dbScale);
	if (m_wndResizer.IsFixedHeight())
		rcNew.bottom = rcNew.top + (long)(m_nHeight * m_dbScale);

	if (rcNew.Width() < m_wndResizer.GetMinCx())
		rcNew.right = rcNew.left + m_wndResizer.GetMinCx();
	if (rcNew.Height() < m_wndResizer.GetMinCy())
		rcNew.bottom = rcNew.top + m_wndResizer.GetMinCy();

	if (rcNew.Width() != m_rcReal.Width() || rcNew.Height() != m_rcReal.Height())
		MoveWindow(rcNew);
}

bool CSkinWnd::OnCustomCommand(int nId)
{
	if (m_pSjvmNotifyEvent && m_pSjvmNotifyEvent->onCommand(nId))
		return true;

	int		nRet = m_pSkinFactory->OnDynamicCmd(nId, this);
	if (nRet == ERR_NOT_FOUND)
		return false;

	if (nRet != ERR_OK)
		ERRLOG2(_T("Execute Dynamic Command failed: %s, %s"), m_pSkinFactory->GetStringOfID(nId).c_str(), (LPCTSTR)Error2Str(nRet));
	return true;
}

void CSkinWnd::PostCustomCommandMsg(int nId)
{
#ifdef _WIN32
	::PostMessage(m_hWnd, ms_msgIDCustomCommand, nId, 0);
#else
	OnCustomCommand(nId);
#endif
}

void CSkinWnd::OnRemoveUIObj(CUIObject *pObj)
{
}


void CSkinWnd::OnAddUIObj(CUIObject *pObj)
{
	// Only skin is opened, OnCreate will be called.
	if (m_bSkinOpened)
	{
		if (!pObj->m_bCreated)
		{
			pObj->m_bCreated = true;
			pObj->OnCreate();
		}

		if (m_itFocusUIObject.Get() == NULL && pObj->WantMsgKey()
			&& pObj->IsVisible() && pObj->IsParentVisible())
			m_itFocusUIObject.From(pObj);
	}
}

void CSkinWnd::OnLanguageChanged()
{
	m_rootConainter.OnLanguageChanged();
}

void CSkinWnd::OnAdjustHue(float hue, float saturation, float luminance)
{
	m_rootConainter.OnAdjustHue(hue, saturation, luminance);
}

// COMMENTS:
//		??TimerObject
// RETURN:
//		???????TIMER_ID
int CSkinWnd::RegisterTimerObject(CUIObject *pObj, int nTimeDuration)
{
	TIMER_OBJECT		TimerObj;

	m_nTimerIDMax++;
	TimerObj.nTimeDuration = nTimeDuration;
	TimerObj.pObj = pObj;
	TimerObj.nTimerId = m_nTimerIDMax;

	// ???TIMER_OBJ???
/*	MAP_TIMER_OBJS::value_type	value;

	value.first = TimerObj.nTimerId;
	value.second = TimerObj;

	m_mapTimerObjs.insert(value);*/
	m_mapTimerObjs[TimerObj.nTimerId] = TimerObj;

	// ??TIMER
	// ????,??nTimeDuration???Objs?????TIMER,???????
	SetTimer(TimerObj.nTimerId, TimerObj.nTimeDuration);

	return TimerObj.nTimerId;
}

// COMMENTS:
//		?????TIMER_ID
void CSkinWnd::UnregisterTimerObject(CUIObject *pObj, int nTimerId)
{
	MAP_TIMER_OBJS::iterator	it;

	it = m_mapTimerObjs.find(nTimerId);
	// assert(it != m_mapTimerObjs.end());

	if (it != m_mapTimerObjs.end())
	{
		KillTimer(nTimerId);
		m_mapTimerObjs.erase(it);
	}
}

// COMMENTS:
//		?????TIMER_ID
void CSkinWnd::UnregisterTimerObject(CUIObject *pObj)
{
	// ???????Timer,?????timer
	MAP_TIMER_OBJS::iterator	it;

	for (it = m_mapTimerObjs.begin(); it != m_mapTimerObjs.end(); )
	{
		TIMER_OBJECT	&timerObj = (*it).second;

		if (timerObj.pObj == pObj)
		{
			KillTimer(timerObj.nTimerId);
			m_mapTimerObjs.erase(it);
			it = m_mapTimerObjs.begin();
		}
		else
			it++;
	}
}

void CSkinWnd::OnTimer (UINT nIDEvent)
{
	MAP_TIMER_OBJS::iterator	it;

#ifdef _WIN32_DESKTOP
	if (nIDEvent == TIMER_ID_DYNAMIC_TRANS)
	{
		OnTimerDynamicAlphaChange();
		return;
	}
#endif // #ifdef _WIN32_DESKTOP

	if (nIDEvent == TIMER_ID_MOUSE_INACTIVE)
	{
		OnTimerMouseInactive();
		return;
	}
	else if (nIDEvent == TIMER_ID_ANIMATION)
	{
		OnTimerAnimation();
		return;
	}

	ASSERT(nIDEvent >= TIMER_ID_BEG_ALLOC);

	it = m_mapTimerObjs.find(nIDEvent);
	assert(it != m_mapTimerObjs.end());

	if (it != m_mapTimerObjs.end())
	{
		(*it).second.pObj->OnTimer(nIDEvent);
		return;
	}
}

void CSkinWnd::GetCursorClientPos(CPoint &pt)
{
	MLWidget::GetCursorPos(&pt);

	ScreenToClient(pt);
}


bool CSkinWnd::RegisterUIObjNotifyHandler(int nIDUIObj, IUIObjNotifyHandler *pHandler)
{
	VecUIObjNotifyHandlers::iterator it;

	for (it = m_vUIObjNotifyHandlers.begin(); it != m_vUIObjNotifyHandlers.end(); ++it)
	{
		UIObjNotifyHandlerInfo	&h = *it;
		if (h.nIDUIObj == nIDUIObj && h.pHandler == pHandler)
			return false;
	}

	UIObjNotifyHandlerInfo		h;
	h.nIDUIObj = nIDUIObj;
	h.pHandler = pHandler;
	m_vUIObjNotifyHandlers.push_back(h);

	return true;
}


bool CSkinWnd::UnregisterUIObjNotifyHandler(IUIObjNotifyHandler *pHandler)
{
	bool		bRet = false;

	for (int i = m_vUIObjNotifyHandlers.size() - 1; i >= 0; i--)
	{
		UIObjNotifyHandlerInfo	&h = m_vUIObjNotifyHandlers[i];
		if (h.pHandler == pHandler)
		{
			m_vUIObjNotifyHandlers.erase(m_vUIObjNotifyHandlers.begin() + i);
			bRet = true;
		}
	}

	return bRet;
}


void CSkinWnd::DispatchUIObjNotify(IUIObjNotify *pNotify)
{
	for (int i = 0; i < m_vUIObjNotifyHandlers.size(); i++)
	{
		UIObjNotifyHandlerInfo	&h = m_vUIObjNotifyHandlers[i];
		if (h.nIDUIObj == pNotify->nID)
		{
			h.pHandler->OnUIObjNotify(pNotify);
		}
	}

	OnUIObjNotify(pNotify);
}


int CSkinWnd::FromXML(SXNode *pXmlNode)
{
	m_bTranslucencyLayered = false;

	GetWindowRect(&m_rcBoundBox);

	m_rootConainter.m_rcObj.SetRect(0, 0, m_rcBoundBox.Width(), m_rcBoundBox.Height());

	m_fontProperty.Create(_T("Tohama"), _T(""), 14, FW_NORMAL, false, false);

	SXNode	*pNodeExtends = NULL;
	LPCTSTR szExtends = pXmlNode->GetProperty(SZ_PN_EXTENDS);
	if (szExtends && !IsEmptyString(szExtends))
	{
		pNodeExtends = m_pSkinFactory->GetExtendSkinWndStyle(szExtends);
	}

	if (pNodeExtends)
		SetProperies(pNodeExtends->listProperties);

	SetProperies(pXmlNode->listProperties);

	// load window size of skin
	OnLoadWndSizePos();

	// Create UIObjects defined in Extends window
	CSkinContainer	*pContainterClientArea = NULL;
	if (pNodeExtends)
	{
		CreateUIObjects(pNodeExtends, &m_rootConainter);
		pContainterClientArea = (CSkinContainer*)m_rootConainter.GetUIObjectByClassName(CSkinClientArea::ClassName());
	}

	if (pContainterClientArea == NULL)
		pContainterClientArea = &m_rootConainter;

	// Create UIObjects
	CreateUIObjects(pXmlNode, pContainterClientArea);

	m_bSkinOpened = true;

	//
	// Call OnCreate of Every Child.
	//
	m_rootConainter.DispatchOnCreateMsg();

	// 
	UpdateSkinProperty();

	OnSkinLoaded();

	tstring			strOnCreateCmd;
	strOnCreateCmd = pXmlNode->GetPropertySafe(_T("OnCreateCommand"));

	if (!strOnCreateCmd.empty())
	{
		V_Str		vStrParam;

		StrSplit(strOnCreateCmd.c_str(), ',', vStrParam);
		for (int i = 0; i < (int)vStrParam.size(); i++)
		{
			TrimStr(vStrParam[i]);
			OnCustomCommand(m_pSkinFactory->GetIDByName(vStrParam[i].c_str()));
		}
	}

	float hue, saturation, luminance;
	m_pSkinFactory->GetResourceMgr()->GetAdjustHueParam(hue, saturation, luminance);
	if (hue != 0.0f)
		OnAdjustHue(hue, saturation, luminance);

	return ERR_OK;
}

#ifdef _SKIN_EDITOR_
void CSkinWnd::ToXML(CXMLWriter &xmlStream)
{
	xmlStream.WriteStartElement(_SZ_SKINWND);

	// m_rootConainter.ToXML(xmlStream);

	CUIObjProperties	properties;

	EnumProperties(properties);
	properties.ToXMLCategoryAttrib(xmlStream);

	// ?????
	m_rootConainter.OnToXMLChild(xmlStream);

	xmlStream.WriteEndElement();
}
#endif // _SKIN_EDITOR_

void CSkinWnd::OnSwitchChildFocus(CUIObject *pUIObjFocusOld, CUIObject *pUIObjFocusNew)
{
	if (pUIObjFocusOld != pUIObjFocusNew)
	{
		if (pUIObjFocusOld)
			pUIObjFocusOld->OnKillFocus();

		if (pUIObjFocusNew)
			pUIObjFocusNew->OnSetFocus();
	}
}


void CSkinWnd::OnMouseActiveMsg()
{
	m_nTimeLatestMouseMsg = GetTickCount();

	KillTimer(TIMER_ID_MOUSE_INACTIVE);
	SetTimer(TIMER_ID_MOUSE_INACTIVE, TIMER_SPAN_MOUSE_INACTIVE);

	if (!m_bMouseActive)
	{
		m_bMouseActive = true;
		OnMouseActive(m_bMouseActive);
		InvalidateRect();
	}
}

void CSkinWnd::OnTimerMouseInactive()
{
	m_bMouseActive = false;
	OnMouseActive(m_bMouseActive);

	KillTimer(TIMER_ID_MOUSE_INACTIVE);

	InvalidateRect();
}

void CSkinWnd::OnMouseActive(boolean bMouseActive)
{
	if (m_pSjvmNotifyEvent)
		m_pSjvmNotifyEvent->onMouseActive(bMouseActive);
}

void CSkinWnd::StartAnimation(int nIDAnimation)
{
	for (ListAnimationObj::iterator it = m_listAnimations.begin(); it != m_listAnimations.end(); ++it)
	{
		CSkinAnimationObj *p = *it;
		if (p->m_nId == nIDAnimation)
			return;
	}

	CSkinAnimationObj *pObj = (CSkinAnimationObj*)GetUIObjectById(nIDAnimation, CSkinAnimationObj::ClassName());
	if (pObj)
	{
		if (m_listAnimations.empty())
			SetTimer(TIMER_ID_ANIMATION, TIME_OUT_ANIMATION);

		m_listAnimations.push_back(pObj);
		pObj->StartAnimation();
	}
	else
		ERRLOG1(_T("Invalid animation id: %s"), m_pSkinFactory->GetStringOfID(nIDAnimation).c_str());
}

void CSkinWnd::StopAnimation(int nIDAnimation)
{
	for (ListAnimationObj::iterator it = m_listAnimations.begin(); it != m_listAnimations.end(); ++it)
	{
		CSkinAnimationObj *p = *it;
		if (p->m_nId == nIDAnimation)
		{
			m_listAnimations.erase(it);
			if (m_listAnimations.empty())
				KillTimer(TIMER_ID_ANIMATION);

			return;
		}
	}
}

bool CSkinWnd::IsInAnimation()
{
	return !m_listAnimations.empty();
}

CObjAnimation *CSkinWnd::GetObjAnimation(int nId, AnimateState &as, int &nMotionPercent)
{
	CObjAnimation	*pAnimation = NULL;
	for (ListAnimationObj::iterator it = m_listAnimations.begin(); it != m_listAnimations.end(); ++it)
	{
		CSkinAnimationObj *p = *it;
		pAnimation = p->GetObjAnimation(nId);
		if (pAnimation)
		{
			as = p->GetState();
			nMotionPercent = p->GetMotionPercent();
			return pAnimation;
		}
	}

	return NULL;
}

void CSkinWnd::OnTimerAnimation()
{
	if (m_listAnimations.empty())
	{
		KillTimer(TIMER_ID_ANIMATION);
		return;
	}

	CAutoRedrawLock	redrawLock(this);

	for (ListAnimationObj::iterator it = m_listAnimations.begin(); it != m_listAnimations.end();)
	{
		CSkinAnimationObj *p = *it;
		if (!p->OneMotion())
		{
			it = m_listAnimations.erase(it);
			OnAnimationDone(p->m_nId);
		}
		else
			++it;
	}

	if (m_listAnimations.empty())
		KillTimer(TIMER_ID_ANIMATION);
}

/*
int RunModalWindow(
HWND hwndDialog,
HWND hwndParent)
{
if(hwndParent != NULL)
EnableWindow(hwndParent,FALSE);

MSG msg;

for(fDone=FALSE;fDone;WaitMessage())
{
while(PeekMessage(&msg,0,0,0,PM_REMOVE))
{
if(msg.message == WM_QUIT)
{
fDone = TRUE;
PostMessage(NULL,WM_QUIT,0,0);
break;
}

if(!IsDialogMessage(hwndDlg,&msg))
{
TranslateMessage(&msg);
DispatchMessage(&msg);
}
}
}

if(hwndParent != NULL)
EnableWindow(hwndParent,TRUE);

DestroyWindow(hwndDialog);

return nResult;
}
*/
