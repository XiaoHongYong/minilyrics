/********************************************************************
	Created  :	2001-12-15 2:36:55
	FileName :	SkinWnd.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "StdAfx.h"
#include "Skin.h"
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

#ifdef _WIN32_DESKTOP

#include <Dwmapi.h>

typedef HRESULT (WINAPI *FUNC_DwmExtendFrameIntoClientArea)(HWND hWnd, const MARGINS* pMarInset);

bool ExtendIntoWholeClient(HWND hwnd, bool bIntoWholeClient)
{
	// Set margins, extending to whole client
	static FUNC_DwmExtendFrameIntoClientArea	pDwmExtendFrameIntoClientArea = NULL;
	if (!pDwmExtendFrameIntoClientArea)
	{
		HMODULE	hModule = LoadLibrary(_T("Dwmapi.dll"));
		if (hModule)
		{
			pDwmExtendFrameIntoClientArea = (FUNC_DwmExtendFrameIntoClientArea)GetProcAddress(hModule, "DwmExtendFrameIntoClientArea");
		}
	}

	if (!pDwmExtendFrameIntoClientArea)
		return false;

	MARGINS margins = {-1};
	HRESULT hr = S_OK;

	if (!bIntoWholeClient)
		memset(&margins, 0, sizeof(margins));

	hr = pDwmExtendFrameIntoClientArea(hwnd,&margins);
	if (!SUCCEEDED(hr))
	{
		DBGLOG1(_T("Failed to apply Aero glass: %s"), (LPCTSTR)Error2Str(hr));
		return false;
	}

	return true;
}

#endif // #ifdef _WIN32_DESKTOP

BOOL IsWndOutOfScreen(const CRect &rc)
{
	CRect		rcRestrict;

	if (GetMonitorRestrictRect(rc, rcRestrict))
	{
		return ((rc.right < rcRestrict.left + 20) || (rc.left > rcRestrict.right - 20) ||
			(rc.bottom < rcRestrict.top + 10) || (rc.top > rcRestrict.bottom - 40));
	}
	else
		return false;
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

bool GetRectValue(LPCTSTR szValue, RECT &rc)
{
	if (!Scan4IntX(szValue, rc.left, rc.top, rc.right, rc.bottom))
		return false;
	rc.right += rc.left;
	rc.bottom += rc.top;
	return true;
}


void ColorToStr(const MLColor &clr, tstring &strColor)
{
	TCHAR				szValue[128];

	StringFromColor(szValue, clr.get());

	strColor = szValue;
}

// FgColor="#FFFFFF"
void GetColorValue(MLColor &clr, LPCTSTR szColor)
{
	clr.set(StringToColor(szColor, clr.get()));
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

CSkinWnd::CSkinWnd()
{
	m_bFreeOnDestory = false;
	m_bManageBySkinFactory = true;

	m_bSkinOpened = false;
	m_nInRedrawUpdate = 0;
	m_bNeedRedraw = false;
	m_bInEditorMode = false;

	m_bWindowsAppearance = false;
	m_bAeroGlass = false;

	m_bActived = FALSE;
	m_bMainAppWnd = false;
	m_bDialogWnd = false;

	m_pmemGraph = NULL;

	m_rootConainter.m_pSkin = this;
	m_pUIObjCapMouse = NULL;

	m_pUIObjHandleContextMenuCmd = NULL;

	m_bUseRgn = FALSE;

	m_rcBoundBox.SetRect(0, 0, 600, 350);
	m_rcReal.SetRect(0, 0, 600, 350);

	CRect rcRestrict;
	if (GetMonitorRestrictRect(m_rcBoundBox, rcRestrict)) {
		m_rcBoundBox.OffsetRect((rcRestrict.Width() - m_rcBoundBox.Width()) / 2,
			(rcRestrict.Height() - m_rcBoundBox.Height()) / 2);
		m_rcReal = m_rcBoundBox;
	}

	m_nTimerIDMax = TIMER_ID_BEG_ALLOC;

	m_dbScale = 1.0;

	m_nWidth = 0;
	m_nHeight = 0;
	m_bRememberSizePos = true;

	m_nNextUIObjIDAlloc = UIOBJ_ID_BEGIN_ALLOC;

	m_nNextAuoUniID = 1;

	m_pSkinFactory = NULL;

	m_bIgnoreNextOnCharMsg = false;

	m_nTimeLatestMouseMsg = 0;
	m_bMouseActive = false;

	m_animateType = AT_UNKNOWN;
	m_animateDuration = 0;

	m_bTranslucencyLayered = false;
	m_nAlpha = 255;
	m_nTranslucencyAlphaOnActive = -1;
	m_nTranslucencyAlphaOnHover = -1;
	m_nTranslucencyAlphaDefault = -1;
	m_nCurTranslucencyAlpha = 255;
	m_dwTransFadeinBegTime = 0;
	m_nTransAlphaBeg = 0;
	m_bClickThrough = false;
	m_bEnableClickThrough = true;

	m_bOnMouseHover = false;
	m_translucencyStatus = TS_NORMAL;

	m_pSjvmNotifyEvent = NULL;

#ifndef _WIN32
	m_rcMemUpdate.top = m_rcMemUpdate.bottom = 0;
#endif
}

CSkinWnd::~CSkinWnd()
{
	CloseSkin();
}

int CSkinWnd::Create(SkinWndStartupInfo &skinWndStartupInfo, CSkinFactory *pSkinFactory, bool bToolWindow, bool bTopmost, bool bVisible)
{
	m_pSkinFactory = pSkinFactory;
	m_strSkinWndName = skinWndStartupInfo.strSkinWnd;
	m_mapExchangePool = skinWndStartupInfo.mapExchangePool;
	m_bMainAppWnd = skinWndStartupInfo.bMainWnd;

	OnPreCreate(bTopmost, bVisible);

	CAutoRedrawLock drawLock(this);

	CreateForSkin(skinWndStartupInfo.strClassName.c_str(), skinWndStartupInfo.strCaptionText.c_str(),
		m_rcReal.left, m_rcReal.top, m_rcReal.Width(), m_rcReal.Height(), skinWndStartupInfo.pWndParent,
		bToolWindow, bTopmost, bVisible);

	return ERR_OK;
}

bool CSkinWnd::Destroy()
{
	m_bOnDestroy = true;

	return CBaseWnd::Destroy();
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

	// the skin was opened before, so save latest settings.
	if (m_bRememberSizePos)
		SaveWndPos();

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
	if (m_bAeroGlass)
	{
		m_bAeroGlass = false;
		ExtendIntoWholeClient(m_hWnd, false);
	}
#endif
	m_nAlpha = 255;
	m_nTranslucencyAlphaOnActive = -1;
	m_nTranslucencyAlphaOnHover = -1;
	m_nTranslucencyAlphaDefault = -1;
	m_nCurTranslucencyAlpha = 255;
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

	m_pUIObjHandleContextMenuCmd = NULL;

	m_pUIObjCapMouse = NULL;

	m_rootConainter.OnDestroy();
	m_rootConainter.Destroy();

#ifdef _WIN32_DESKTOP
	KillTimer(TIMER_ID_DYNAMIC_TRANS);
	m_translucencyStatus = TS_NORMAL;
	::UnSetLayeredWindow(m_hWnd);
#endif

	ASSERT(m_nInRedrawUpdate == 0);
	m_nInRedrawUpdate = 0;
	m_bNeedRedraw = false;
	m_bSkinOpened = false;

	m_listUnprocessedProperties.clear();

	m_cursor.Destroy();

	m_strCusor.resize(0);
	m_strRgn.resize(0);
	// Remember skin window name, it can be used to reload it.
	// m_strSkinWndName.resize(0);

	m_nWidth = 0;
	m_nHeight = 0;
	m_bRememberSizePos = true;

	m_fontProperty.Create(_T("Tohama"), _T(""), 14, FW_NORMAL, false, false);

	m_bUseRgn = FALSE;

	if (m_rgnSkin.IsValid())
		m_rgnSkin.Destroy();

	if (m_pmemGraph)
	{
		delete m_pmemGraph;
		m_pmemGraph = NULL;
	}

	m_nTimeLatestMouseMsg = 0;
	m_bMouseActive = false;
	KillTimer(TIMER_ID_MOUSE_INACTIVE);

	m_pSjvmNotifyEvent = NULL;
	m_vUIObjNotifyHandlers.clear();

	m_animateType = AT_UNKNOWN;
	m_animateDuration = 0;
}

CUIObject *CSkinWnd::GetUIObjectByClassName(LPCTSTR szClassName)
{
	return m_rootConainter.GetUIObjectByClassName(szClassName);
}

CUIObject * CSkinWnd::GetUIObjectById(int nId, LPCTSTR szClassName)
{
	return m_rootConainter.GetUIObjectById(nId, szClassName);
}

void CSkinWnd::ProcessMouseMove(CPoint point)
{
	OnMouseActiveMsg();

#ifdef _WIN32_DESKTOP
	//
	// Change translucency on hover?
	//
	if (m_bTranslucencyLayered
		&& m_translucencyStatus == TS_NORMAL
		&& !m_bActived)
	{
		m_translucencyStatus = TS_ON_HOVER;

		m_bOnMouseHover = Tobool(m_rcBoundBox.PtInRect(point));

		StartTranslucencyFade();
	}
#endif	// _WIN32_DESKTOP

	if (m_cursor.IsValid())
		MLWidget::SetCursor(m_cursor);
}

void CSkinWnd::OnMouseDrag(UINT nFlags, CPoint point)
{
	ProcessMouseMove(point);

	point.x = (int)(point.x / m_dbScale);
	point.y = (int)(point.y / m_dbScale);

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
			if (m_pUIObjCapMouse->OnMouseDrag(point))
				return;
		}
		else
		{
			// capture lost!
		}
	}

	// then let root container to process it
	if (!m_rootConainter.OnMouseDrag(point))
	{
		// else, should set mouse resizing cursor
		m_wndResizer.OnMouseMessage(nFlags, point);
	}
}

void CSkinWnd::OnMouseMove(CPoint point)
{
	ProcessMouseMove(point);

	// first send message to ui objects that captured the mouse
	if (m_pUIObjCapMouse)
	{
		if (IsMouseCaptured())
		{
			if (m_pUIObjCapMouse->OnMouseMove(point))
				return;
		}
		else
		{
			// capture lost!
		}
	}

	// then let root container to process it
	if (!m_rootConainter.OnMouseMove(point))
	{
		// else, should set mouse resizing cursor
		m_wndResizer.OnMouseMessage(0, point);
	}
}

void CSkinWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	OnMouseActiveMsg();

	if (m_cursor.IsValid())
		MLWidget::SetCursor(m_cursor);

	point.x = (int)(point.x / m_dbScale);
	point.y = (int)(point.y / m_dbScale);

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

	point.x = (int)(point.x / m_dbScale);
	point.y = (int)(point.y / m_dbScale);

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
/*	InvalidateRect();
	
	OnSize(m_rcBoundBox.Width(), m_rcBoundBox.Height());
	
	MoveWindow(m_rcReal);*/
	
	if (m_cursor.IsValid())
		MLWidget::SetCursor(m_cursor);

	point.x = (int)(point.x / m_dbScale);
	point.y = (int)(point.y / m_dbScale);

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

	point.x = (int)(point.x / m_dbScale);
	point.y = (int)(point.y / m_dbScale);

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

	point.x = (int)(point.x / m_dbScale);
	point.y = (int)(point.y / m_dbScale);

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
#ifdef _DEBUG
	{
		// Dump UIObject
		if (IsModifierKeyPressed(MK_CONTROL, nFlags))
		{
			if (nChar == VK_F1)
			{
				int nDeep = 0;
				m_rootConainter.DumpUIObject(nDeep);
			}
			else if (nChar == VK_F2)
			{
				POINT pt;
				MLWidget::GetCursorPos(&pt);
				ScreenToClient(pt);
				DBGLOG2(_T("Current Pos: %d, %d"), pt.x, pt.y);
			}
			else if (nChar == VK_F3)
			{
				m_pSkinFactory->DumpUID();
			}
		}
	}
#endif

	CUIObject	*pObjFocus = GetFocusUIObj();
	if (pObjFocus)
	{
		if (pObjFocus->WantMsgAllKeys())
		{
			pObjFocus->OnKeyDown(nChar, nFlags);
			return;
		}

		if (IsDialogWnd())
		{
			if (nChar == VK_ESCAPE)
			{
				OnCancel();
				return;
			}
			else if (nChar == VK_RETURN && !pObjFocus->WantMsgEnterKey())
			{
				OnOK();
				return;
			}
		}
	}

	// tab key to switch between uiobject
	if (nChar == VK_TAB)	//TAB
	{
		bool bShitDown = IsModifierKeyPressed(MK_SHIFT, nFlags);
		// shift + VK_TAB
		if (bShitDown)
			m_rootConainter.FocusToPrev();
		else
			m_rootConainter.FocusToNext();

		// Ignore '\t' WM_CHAR message.
		m_bIgnoreNextOnCharMsg = true;

		return;
	}

	m_rootConainter.OnKeyDown(nChar, nFlags);
}

void CSkinWnd::OnKeyUp(UINT nChar, UINT nFlags)
{
	// Let focus UIObject to process key message
	CUIObject	*pObjFocus = GetFocusUIObj();
	if (pObjFocus && pObjFocus->WantMsgKey())
		pObjFocus->OnKeyUp(nChar, nFlags);
	else
		m_rootConainter.OnKeyUp(nChar, nFlags);
}

void CSkinWnd::OnContexMenu(int xPos, int yPos)
{
	if (m_rootConainter.GetMenu())
		m_rootConainter.GetMenu()->TrackPopupMenu(xPos, yPos, this);
}

void CSkinWnd::OnCommand(UINT uID, UINT nNotifyCode)
{
	if (m_pUIObjHandleContextMenuCmd)
	{
		// Only handle 1 command.
		if (m_pUIObjHandleContextMenuCmd->OnCommand(uID))
		{
			m_pUIObjHandleContextMenuCmd = NULL;
			return;
		}
		m_pUIObjHandleContextMenuCmd = NULL;
	}

	if (m_rootConainter.OnCommand(uID))
		return;

	int	nUID = m_pSkinFactory->GetUIDByMenuID(uID);
	if (nUID != UID_INVALID)
		OnCustomCommand(nUID);
	else
		CBaseWnd::OnCommand(uID, nNotifyCode);
}

void CSkinWnd::OnOK()
{
	if (m_rootConainter.OnOK())
		PostDestroy();
}

void CSkinWnd::OnCancel()
{
	if (m_rootConainter.OnCancel())
		PostDestroy();
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

			// don't use any region
			SetWindowRgn(NULL);

#ifdef _WIN32
			// Is region used?
			rgnOrg.CreateRectRgn(0, 0, 1, 1);
			int n = GetWindowRgn(m_hWnd, rgnOrg.m_rgn);
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
			m_nCurTranslucencyAlpha = m_nTranslucencyAlphaOnActive;
		else
			m_nCurTranslucencyAlpha = m_nTranslucencyAlphaDefault;
	}

#ifdef _WIN32_DESKTOP
	if (m_bWindowsAppearance)
	{
		SetWindowLong(m_hWnd, GWL_STYLE, GetWindowLong(m_hWnd, GWL_STYLE) | WS_CAPTION | WS_THICKFRAME);
		::SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_DEFERERASE);
	}

	if (m_bAeroGlass)
	{
		if (!ExtendIntoWholeClient(m_hWnd, true))
		{
			// Failed to set to Aero mode
			m_bAeroGlass = false;
		}
		if (m_bAeroGlass)
			m_rootConainter.m_clrBg.setAlpha(0);
	}
#endif // #ifdef _WIN32_DESKTOP

	// init 
	OnWindowSizeChanged();

	return TRUE;
}

void CSkinWnd::OnPaint(CMLGraphics *pGraph, CRect &rcClip)
{
	if (IsIconic() || m_bOnDestroy)
		return;

	bool		bRedraw = true;
	CRawGraph	*pMemGraph;

	pMemGraph = GetMemGraphics();

#ifndef _WIN32
	if (m_rcMemUpdate.top != m_rcMemUpdate.bottom)
	{
		m_rcMemUpdate.top = m_rcMemUpdate.bottom = 0;
		if (m_rcMemUpdate == rcClip)
			bRedraw = false;
	}
#endif // #ifdef _WIN32

	if (bRedraw)
	{
		// pMemGraph->SetClipBoundBox(rcClip);

		// draw every UI objects on back buffer one by one
		m_rootConainter.Draw(pMemGraph);
	}
	
/*	CRect rc(20, 20, 50, 30);
	MLColor clr(RGB(10, 10, 10));
	clr.setAlpha(50);
	pMemGraph->FillRect(&rc, clr);*/

	// from buffer to screen
	// if (m_dbScale == 1.0)
		pMemGraph->DrawToGraphics(pGraph, rcClip.left, rcClip.top, rcClip.Width(), rcClip.Height(), rcClip.left, rcClip.top);
	//pMemGraph->DrawToGraphics(pGraph, 0, 0, m_rcBoundBox.Width(), m_rcBoundBox.Height(), 0, 0);
	// else
		// pMemGraph->DrawToGraphicsStretch(pGraph, 0, 0, m_rcReal.Width(), m_rcReal.Height(), 0, 0, m_rcBoundBox.Width(), m_rcBoundBox.Height());
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
	CMLGraphics		*pGraph;

	pGraph = GetGraphics();

	if (m_pmemGraph)
		delete m_pmemGraph;

	CRawGraph				*pMemGraph;
	pMemGraph = new CRawGraph();
	pMemGraph->Create(m_rcBoundBox.Width(), m_rcBoundBox.Height(), pGraph);
	m_pmemGraph = pMemGraph;

	ReleaseGraphics(pGraph);

	return true;
}

MLColor CSkinWnd::GetTranslucencyColor(const MLColor &clr) const
{
	if (m_bTranslucencyLayered)
	{
		MLColor clrNew = clr;
		clrNew.setAlpha((byte)m_nCurTranslucencyAlpha);
		PreMultiplyColor(clrNew, (byte)m_nCurTranslucencyAlpha);
		return clrNew;
	}
	else
		return clr;
}

void CSkinWnd::OnWindowSizeChanged()
{
	if (IsIconic())
		return;

	RecreateMemGraphics();

	// recalculate all ui objects' position
	m_rootConainter.m_rcObj.SetRect(0, 0, m_rcBoundBox.Width(), m_rcBoundBox.Height());
	m_rootConainter.OnSize();

	InvalidateRect();
}

BOOL CSkinWnd::AddTool(LPCTSTR szText, LPCRECT lpRectTool, UINT nIDTool)
{
	ASSERT(!IsEmptyString(szText));
	return m_wndToolTip.AddTool(szText, 
			lpRectTool, nIDTool);
}

void CSkinWnd::DelTool(UINT nIDTool)
{
	if (m_wndToolTip.IsValid())
		m_wndToolTip.DelTool(nIDTool);
}

void CSkinWnd::InvalidateUIObject(CUIObject *pObj)
{
	assert(pObj);

	if (!IsVisible() || IsIconic())
		return;

	if (m_nInRedrawUpdate > 0)
	{
		m_bNeedRedraw = true;
		return;
	}

	if (!pObj->IsVisible() || !pObj->IsParentVisible())
		return;

	if (!m_pmemGraph)
		return;	// SkinWnd isn't created yet.

#ifdef _WIN32
	CRawGraph::CClipBoxAutoRecovery	autoCBR(m_pmemGraph);

	m_pmemGraph->SetClipBoundBox(pObj->m_rcObj);

	if (pObj->IsUseParentBg())
	{
		// Redraw background
		ASSERT(pObj->GetParent() != NULL);
		pObj->GetParent()->RedrawBackground(m_pmemGraph, pObj->m_rcObj);
	}

	CRawGraph::COpacityBlendAutoRecovery opacityAR(m_pmemGraph, pObj->GetOpacity());

	pObj->TryToCallInitialUpdate();
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

	CMLGraphics		*pGraphics;

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
#else // _WIN32
	InvalidateRect(&pObj->m_rcObj);
#endif
}

void CSkinWnd::EnterInDrawUpdate()
{
	m_nInRedrawUpdate++;
}

void CSkinWnd::LeaveInDrawUpdate()
{
	m_nInRedrawUpdate--;
	ASSERT(m_nInRedrawUpdate >= 0);
	if (m_nInRedrawUpdate == 0)
	{
		if (m_bNeedRedraw)
			InvalidateRect();
	}
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
		m_rcBoundBox.right = (int)(rc.left + rc.Width() / m_dbScale);
		m_rcBoundBox.bottom = (int)(rc.top + rc.Height() / m_dbScale);

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

					// don't use any region
					SetWindowRgn(NULL);

#ifdef _WIN32
					// Is region used?
					rgnOrg.CreateRectRgn(0, 0, 1, 1);
					int n = GetWindowRgn(m_hWnd, rgnOrg.m_rgn);
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

void CSkinWnd::SetProperies(SXNode::LIST_PROPERTIES &listProperties)
{
	SXNode::iterProperties	it;
	for (it = listProperties.begin(); it != listProperties.end(); ++it)
	{
		SXNode::Property	&prop = *it;
		if (!SetProperty(prop.strName.c_str(), prop.strValue.c_str()))
		{
			if (!IsPropertyName(prop.strName.c_str(), SZ_PN_EXTENDS)) // Do NOT log extends
			{
				m_listUnprocessedProperties.push_back(prop);
				DBGLOG2(_T("Unknow Property: %s, %s"), prop.strName.c_str(), prop.strValue.c_str());
			}
		}
	}
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
	}
	else if (_tcsicmp(szProperty, SZ_PN_HEIGHT) == 0)
	{
		m_nHeight = _ttoi(szValue);
	}
	else if (IsPropertyName(szProperty, _T("RememberSizePos")))
		m_bRememberSizePos = IsTRUE(szValue);
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
	else if (_tcsicmp(szProperty, SZ_PN_NAME) == 0)
	{
		m_strSkinWndName = szValue;
	}
	else if (IsPropertyName(szProperty, _T("Caption")))
	{
		m_strCaption = _TL(szValue);
		SetWindowText(m_strCaption.c_str());
	}
	else if (_tcsicmp(szProperty, _T("ContextMenu")) == 0)
	{
		m_rootConainter.SetProperty(szProperty, szValue);
	}
	else if (_tcsicmp(szProperty, _T("Menu")) == 0)
	{
		m_strMenuName = szValue;
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
	else if (IsPropertyName(szProperty, _T("WindowsAppearance")))
	{
		m_bWindowsAppearance = IsTRUE(szValue);
	}
	else if (IsPropertyName(szProperty, _T("AeroGlass")))
	{
		m_bAeroGlass = IsTRUE(szValue);
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
		m_nTranslucencyAlphaDefault = _ttoi(szValue);
		if (m_nTranslucencyAlphaOnActive == -1)
			m_nTranslucencyAlphaOnActive = m_nTranslucencyAlphaDefault;
		if (m_nTranslucencyAlphaOnHover == -1)
			m_nTranslucencyAlphaOnHover = m_nTranslucencyAlphaDefault;
		if (m_bSkinOpened)
			StartTranslucencyFade();
	}
	else if (IsPropertyName(szProperty, _T("TranslucencyAlphaOnActive")))
	{
		m_nTranslucencyAlphaOnActive = _ttoi(szValue);
		if (m_nTranslucencyAlphaDefault == -1)
			m_nTranslucencyAlphaDefault = m_nTranslucencyAlphaOnActive;
		if (m_bSkinOpened)
			StartTranslucencyFade();
	}
	else if (IsPropertyName(szProperty, _T("TranslucencyAlphaOnHover")))
	{
		m_nTranslucencyAlphaOnHover = _ttoi(szValue);
	}
#endif
	else if (IsPropertyName(szProperty, _T("IsDialog")))
		m_bDialogWnd = IsTRUE(szValue);
	else if (IsPropertyName(szProperty, _T("Animate")))
		m_animateType = AnimateTypeFromString(szValue);
	else if (IsPropertyName(szProperty, _T("AnimateDuration")))
		m_animateDuration = _ttoi(szValue);
	else if (m_rootConainter.SetProperty(szProperty, szValue))
	{
		DBGLOG2(_T("Property is set to Root Container: %s, %s"), szProperty, szValue);
		return true;
	}
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

	m_fontProperty.EnumProperties(listProperties);

	listProperties.AddPropStr(_T("ContextMenu"), m_rootConainter.m_strContextMenu.c_str(), !m_rootConainter.m_strContextMenu.empty());

#ifdef _WIN32_DESKTOP
	listProperties.AddPropBoolStr(_T("WindowsAppearance"), m_bWindowsAppearance, m_bWindowsAppearance);
	listProperties.AddPropBoolStr(_T("EnableTransparentFeature"), m_bEnableClickThrough, !m_bEnableClickThrough);
	listProperties.AddPropBoolStr(_T("EnableTranslucency"), m_bTranslucencyLayered, m_bTranslucencyLayered);
	listProperties.AddPropInt(_T("TranslucencyAlpha"), m_nTranslucencyAlphaDefault, m_nTranslucencyAlphaDefault != 255);
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

void CSkinWnd::SetFocusUIObject(int nId)
{
	m_rootConainter.SetFocusUIObject(nId);
}

CUIObject *CSkinWnd::GetFocusUIObj()
{
	return m_rootConainter.GetFocusUIObject();
}

bool CSkinWnd::EnableUIObject(int nId, bool bEnable, bool bRedraw)
{
	return m_rootConainter.EnableUIObject(nId, bEnable, bRedraw);
}

bool CSkinWnd::SetUIObjectProperty(int nId, LPCTSTR szProperty, LPCTSTR szValue)
{
	return m_rootConainter.SetUIObjectProperty(nId, szProperty, szValue);
}

void CSkinWnd::SetUIObjectVisible(int nId, bool bVisible, bool bRedraw)
{
	m_rootConainter.SetUIObjectVisible(nId, bVisible, bRedraw);
}

void CSkinWnd::InvalidateUIObject(int nId)
{
	m_rootConainter.InvalidateUIObject(nId);
}

tstring CSkinWnd::GetUIObjectText(int nId)
{
	return m_rootConainter.GetUIObjectText(nId);
}

void CSkinWnd::SetUIObjectText(int nId, LPCTSTR szText, bool bRedraw)
{
	m_rootConainter.SetUIObjectText(nId, szText, bRedraw);
}

void CSkinWnd::CheckButton(int nId, bool bCheck)
{
	m_rootConainter.CheckButton(nId, bCheck);
}

bool CSkinWnd::IsButtonChecked(int nId)
{
	return m_rootConainter.IsButtonChecked(nId);
}

void CSkinWnd::CheckToolbarButton(LPCTSTR szToolbarId, LPCTSTR szButtonId, bool bCheck)
{
	m_rootConainter.CheckToolbarButton(szToolbarId, szButtonId, bCheck);
}

void CSkinWnd::CheckToolbarButton(int nToolbarId, int nButtonId, bool bCheck)
{
	m_rootConainter.CheckToolbarButton(nToolbarId, nButtonId, bCheck);
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
	m_bOnDestroy = false;

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
		InvalidateRect(NULL);

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

	if (!m_bOnDestroy)
	{
#ifdef _WIN32_DESKTOP
		StartTranslucencyFade();

		if (bActived)
			m_translucencyStatus = TS_ON_ACTIVE;
		else
			m_translucencyStatus = TS_NORMAL;

		if (m_bEnableClickThrough && m_bClickThrough)
			SetTransparent(m_nAlpha, m_bClickThrough);
#endif
		if (bActived != m_bMouseActive)
		{
			m_bMouseActive = bActived;
			OnMouseActive(m_bMouseActive);
		}

		if (m_pSjvmNotifyEvent)
			m_pSjvmNotifyEvent->onActivate(bActived);
	}

	if (bActived && !IsIconic())
		m_pSkinFactory->OnSkinWndActivate(this);
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
	if (!IsVisible() || IsIconic())
		return;

	if (m_nInRedrawUpdate > 0)
	{
		m_bNeedRedraw = true;
		return;
	}

#ifdef _WIN32
	if (GetEnableTranslucencyLayered())
		UpdateLayeredWindowUsingMemGraph(m_pmemGraph);
	else
	{
		CMLGraphics		*pGraphics;

		pGraphics = GetGraphics();

		m_pmemGraph->DrawToGraphics(pGraphics, 
			lpRect->left, lpRect->top, 
			lpRect->right - lpRect->left, lpRect->bottom - lpRect->top,
			lpRect->left, lpRect->top);

		ReleaseGraphics(pGraphics);
	}
#else // #ifdef _WIN32
	if (m_rcMemUpdate.top == m_rcMemUpdate.bottom)
		m_rcMemUpdate = *lpRect;

	InvalidateRect(lpRect);
#endif // #ifdef _WIN32
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
					_tcscpy_safe(lpnmtdi->szText, CountOf(lpnmtdi->szText), _TL(pObj->m_strTooltip.c_str()));
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
				CRect rcClip;
				if (!::GetUpdateRect(m_hWnd, &rcClip, false))
					return 0;

				PAINTSTRUCT		ps;
				HDC				hdc;
				hdc = BeginPaint(m_hWnd, &ps);
				CMLGraphics	*pGraph;

				pGraph = new CMLGraphics;
				pGraph->Attach(hdc);

				OnPaint(pGraph, rcClip);

				delete pGraph;

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
	if (!bTranslucencyLayered)
		m_nCurTranslucencyAlpha = 255;
}

#ifdef _WIN32_DESKTOP

BOOL CSkinWnd::InvalidateRect(const RECT *lpRect, BOOL bErase)
{
	if (!IsVisible() || IsIconic())
		return FALSE;

	if (m_nInRedrawUpdate > 0)
	{
		m_bNeedRedraw = true;
		return false;
	}

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

void CSkinWnd::StartTranslucencyFade()
{
	if (m_bTranslucencyLayered)
	{
		m_dwTransFadeinBegTime = GetTickCount();
		m_nTransAlphaBeg = m_nCurTranslucencyAlpha;

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
			m_nTransAlphaBeg = m_nCurTranslucencyAlpha;
		}

		if (m_bOnMouseHover)
			nTargetAlpha = m_nTranslucencyAlphaOnHover;
		else
			nTargetAlpha = m_nTranslucencyAlphaDefault;
	}
	else if (IsWndActive())
		nTargetAlpha = m_nTranslucencyAlphaOnActive;
	else
		nTargetAlpha = m_nTranslucencyAlphaDefault;

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
	if (m_nCurTranslucencyAlpha != nAlphaNew && m_bTranslucencyLayered)
	{
		m_nCurTranslucencyAlpha = nAlphaNew;
		ASSERT(m_nCurTranslucencyAlpha >= 0 && m_nCurTranslucencyAlpha <= 255);

		InvalidateRect();
	}
}

#endif

void CSkinWnd::OnSkinLoaded()
{
	if (GetFocusUIObj() == NULL)
		m_rootConainter.FocusToNext();

	if (m_bManageBySkinFactory)
		CSjvmSkinSystem::OnSkinWndCreate(this);

	if (m_pSjvmNotifyEvent)
		m_pSjvmNotifyEvent->onCreate(this);
}

#ifdef _WIN32_DESKTOP
BOOL ShowAsAppWindowNoRefresh(HWND hWnd)
{
	DWORD dwStyleEx;
	dwStyleEx = (DWORD)GetWindowLong(hWnd, GWL_EXSTYLE);
	if (IsFlagSet(dwStyleEx, WS_EX_TOOLWINDOW) || IsFlagSet(dwStyleEx, WS_EX_PALETTEWINDOW))
	{
		dwStyleEx &= ~(WS_EX_TOOLWINDOW | WS_EX_PALETTEWINDOW);
		dwStyleEx |= WS_EX_APPWINDOW;
		SetWindowLong(hWnd, GWL_EXSTYLE, dwStyleEx);
		return TRUE;
	}
	return FALSE;
}

BOOL ShowAsToolWindowNoRefresh(HWND hWnd)
{
	DWORD dwStyleEx;
	dwStyleEx = (DWORD)GetWindowLong(hWnd, GWL_EXSTYLE);
	if (!IsFlagSet(dwStyleEx, WS_EX_TOOLWINDOW) && !IsFlagSet(dwStyleEx, WS_EX_PALETTEWINDOW))
	{
		dwStyleEx |= WS_EX_TOOLWINDOW;// | WS_EX_PALETTEWINDOW;
		dwStyleEx &= ~WS_EX_APPWINDOW;
		SetWindowLong(hWnd, GWL_EXSTYLE, dwStyleEx);
		return TRUE;
	}
	return FALSE;
}
#endif // #ifdef _WIN32_DESKTOP

bool CSkinWnd::OnCustomCommand(int nId)
{
	if (m_pSjvmNotifyEvent && m_pSjvmNotifyEvent->onCommand(nId))
		return true;

	if (m_rootConainter.OnCustomCommand(nId))
		return true;

	int		nRet = m_pSkinFactory->OnDynamicCmd(nId, this);
	if (nRet != ERR_NOT_FOUND && nRet != ERR_OK)
		ERRLOG2(_T("Execute Dynamic Command failed: %s, %s"), m_pSkinFactory->GetStringOfID(nId).c_str(), (LPCTSTR)Error2Str(nRet));

	switch (nId)
	{
	case CMD_OK:
		OnOK();
		break;
	case CMD_CANCEL:
		OnCancel();
		break;
	case CMD_CLOSE:
		if (m_rootConainter.OnClose())
			PostDestroy();
		break;
	case CMD_QUIT:
		CSkinApp::GetInstance()->PostQuitMessage();
		break;
	case CMD_MINIMIZE:
		if (IsToolWindow())
			ShowWindow(SW_HIDE);
		else
			ShowWindow(SW_SHOWMINIMIZED);
		break;
	case CMD_MAXIMIZE:
		if (IsZoomed())
			ShowWindow(SW_RESTORE);
		else
		{
#ifdef _WIN32
			bool	bToolWindow = Tobool(IsToolWindow());
			if (bToolWindow)
				ShowAsAppWindowNoRefresh(GetHandle());

			ShowWindow(SW_SHOWMAXIMIZED);

			if (bToolWindow)
				ShowAsToolWindowNoRefresh(GetHandle());
#else
			ShowWindow(SW_SHOWMAXIMIZED);
#endif
		}
		break;
	case CMD_MENU:
		{
			CPoint	pt;

			MLWidget::GetCursorPos(&pt);
			OnContexMenu(pt.x, pt.y);
		}
		break;
	default:
		return false;
	}

	return true;
}

void CSkinWnd::PostCustomCommandMsg(int nId)
{
#ifdef _WIN32
	::PostMessage(m_hWnd, ms_msgIDCustomCommand, nId, 0);
#endif
	
#ifdef _MAC_OSX
	::PostCustomCommandMsgMac(this, nId);
#else
	OnCustomCommand(nId);
#endif
}

void CSkinWnd::PostShortcutKeyCmd(int nId)
{
	PostCustomCommandMsg(nId);
}

void CSkinWnd::OnUserMessage(int nMessageID, LPARAM param)
{
	m_rootConainter.OnUserMessage(nMessageID, param);
}

void CSkinWnd::OnRemoveUIObj(CUIObject *pObj)
{
	if (pObj == m_pUIObjCapMouse)
		ReleaseCaptureMouse(m_pUIObjCapMouse);
}

void CSkinWnd::OnAddUIObj(CUIObject *pObj)
{
}

void CSkinWnd::OnLanguageChanged()
{
	m_rootConainter.OnLanguageChanged();
	InvalidateRect(NULL, true);
}

void CSkinWnd::OnAdjustHue(float hue, float saturation, float luminance)
{
	if (!m_bSkinOpened)
		return;

	m_fontProperty.OnAdjustHue(this, hue, saturation, luminance);

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
	int nTimerId = m_nTimerIDMax;

	m_mapTimerObjs[nTimerId] = TimerObj;

	SetTimer(nTimerId, TimerObj.nTimeDuration);

	return nTimerId;
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
			KillTimer((*it).first);
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
	for (int i = 0; i < (int)m_vUIObjNotifyHandlers.size(); i++)
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
		m_rootConainter.CreateChild(pNodeExtends);
		pContainterClientArea = (CSkinContainer*)m_rootConainter.GetUIObjectByClassName(CSkinClientArea::ClassName());

		// Set properties to client area.
		LPCTSTR szDefaultPage = pXmlNode->GetProperty(_T("DefaultPage"));
		if (szDefaultPage)
			pContainterClientArea->SetProperty(_T("DefaultPage"), szDefaultPage);
	}

	if (pContainterClientArea == NULL)
		pContainterClientArea = &m_rootConainter;

	pContainterClientArea->CreateChild(pXmlNode);

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

void CSkinWnd::OnMouseActive(bool bMouseActive)
{
	if (m_pSjvmNotifyEvent)
		m_pSjvmNotifyEvent->onMouseActive(bMouseActive);
}

void CSkinWnd::StartAnimation(int nUIDAnimation)
{
	for (ListAnimation::iterator it = m_listAnimations.begin(); it != m_listAnimations.end(); ++it)
	{
		CSkinAnimation *am = *it;
		if (am->GetUIObjectID() == nUIDAnimation)
			return;
	}

	CSkinAnimationUIObj *pObj = (CSkinAnimationUIObj*)GetUIObjectById(nUIDAnimation, CSkinAnimationUIObj::ClassName());
	if (pObj)
	{
		if (m_listAnimations.empty())
			SetTimer(TIMER_ID_ANIMATION, TIME_OUT_ANIMATION);

		m_listAnimations.push_back(new CSkinAnimation(pObj));
	}
	else
		ERRLOG1(_T("Invalid animation id: %s"), m_pSkinFactory->GetStringOfID(nUIDAnimation).c_str());
}

void CSkinWnd::StartAnimation(CUIObject *pObjTarget, CAnimationMotion *pAnimation, int nDurationTime)
{
	if (m_listAnimations.empty())
		SetTimer(TIMER_ID_ANIMATION, TIME_OUT_ANIMATION);

	m_listAnimations.push_back(new CSkinAnimation(pObjTarget, pAnimation, nDurationTime));
}

void CSkinWnd::StopAnimation(int nIDAnimation)
{
	for (ListAnimation::iterator it = m_listAnimations.begin(); it != m_listAnimations.end(); ++it)
	{
		CSkinAnimation *am = *it;
		if (am->GetUIObjectID() == nIDAnimation)
		{
			m_listAnimations.erase(it);
			if (m_listAnimations.empty())
				KillTimer(TIMER_ID_ANIMATION);
			return;
		}
	}
}

void CSkinWnd::StopAnimation(CUIObject *pObjTarget)
{
	for (ListAnimation::iterator it = m_listAnimations.begin(); it != m_listAnimations.end(); ++it)
	{
		CSkinAnimation *am = *it;
		if (am->GetUIObject() == pObjTarget)
		{
			m_listAnimations.erase(it);
			if (m_listAnimations.empty())
				KillTimer(TIMER_ID_ANIMATION);
			return;
		}
	}
}

AnimateType CSkinWnd::GetAnimationType() const
{
	if (m_animateType == AT_UNKNOWN)
		return AT_MOVE;
	return m_animateType;
}

int CSkinWnd::GetAnimationDuration() const
{
	if (m_animateDuration > 0)
		return m_animateDuration;

	return DEFAULT_ANIMATION_DURATION;
}

void CSkinWnd::OnTimerAnimation()
{
	CAutoRedrawLock	redrawLock(this);

	for (ListAnimation::iterator it = m_listAnimations.begin(); it != m_listAnimations.end();)
	{
		CSkinAnimation *am = *it;
		if (!am->OnAnimate())
		{
			it = m_listAnimations.erase(it);
		}
		else
			++it;
	}

	if (m_listAnimations.empty())
		KillTimer(TIMER_ID_ANIMATION);
}

/*void CSkinWnd::OnLoadWndSizePos()
{
	CRect		rcNew;

	rcNew = m_rcReal;
	if (m_wndResizer.IsFixedWidth())
		rcNew.right = rcNew.left + (int)(m_nWidth * m_dbScale);
	if (m_wndResizer.IsFixedHeight())
		rcNew.bottom = rcNew.top + (int)(m_nHeight * m_dbScale);

	if (rcNew.Width() < m_wndResizer.GetMinCx())
		rcNew.right = rcNew.left + m_wndResizer.GetMinCx();
	if (rcNew.Height() < m_wndResizer.GetMinCy())
		rcNew.bottom = rcNew.top + m_wndResizer.GetMinCy();

	if (rcNew.Width() != m_rcReal.Width() || rcNew.Height() != m_rcReal.Height())
		MoveWindow(rcNew);
}*/

void CSkinWnd::OnLoadWndSizePos()
{
#ifdef _WIN32_WCE
// 	rcWnd.left = CW_USEDEFAULT;
// 	rcWnd.right =  CW_USEDEFAULT;
// 	rcWnd.top = 26;
// 	rcWnd.bottom = 294;
#else
#ifdef _WIN32
	if (::GetParent(m_hWnd))
		return;
#endif

	// WndSizeMode		sizeMode;
	CRect			rcNew;
	bool			bCenterWnd = true;

	if (IsZoomed())
		ShowWindow(SW_RESTORE);
		// dwShowFlag = SW_RESTORE;

	GetWindowRect(&m_rcReal);

	rcNew = m_rcReal;

	if (m_bRememberSizePos)
	{
		rcNew.left = g_Profile.GetProfileInt(GetSkinWndName(), _T("wnd_left"), rcNew.left);
		rcNew.top = g_Profile.GetProfileInt(GetSkinWndName(), _T("wnd_top"), rcNew.top);
		rcNew.right = rcNew.left + g_Profile.GetProfileInt(GetSkinWndName(), _T("wnd_width"), m_nWidth);
		rcNew.bottom = rcNew.top + g_Profile.GetProfileInt(GetSkinWndName(), _T("wnd_height"), m_nHeight);
		// if (!rcNew.EqualRect(&m_rcReal))
		//	bCenterWnd = false;
		bCenterWnd = false;

		if (m_wndResizer.IsFixedWidth())
			rcNew.right = long(rcNew.left + m_nWidth * m_dbScale);
		if (m_wndResizer.IsFixedHeight())
			rcNew.bottom = long(rcNew.top + m_nHeight * m_dbScale);
	}
	else
	{
		rcNew.right = long(rcNew.left + m_nWidth * m_dbScale);
		rcNew.bottom = long(rcNew.top + m_nHeight * m_dbScale);
	}

	if (rcNew.Width() < m_wndResizer.GetMinCx())
		rcNew.right = rcNew.left + m_wndResizer.GetMinCx();
	if (rcNew.Height() < m_wndResizer.GetMinCy())
		rcNew.bottom = rcNew.top + m_wndResizer.GetMinCy();

	if (bCenterWnd)
	{
		CRect	rcScreen;
		int		w = rcNew.Width(), h = rcNew.Height();

		GetMonitorRestrictRect(rcNew, rcScreen);
		rcNew.left = (rcScreen.left + rcScreen.right - w) / 2;
		rcNew.top = (rcScreen.top + rcScreen.bottom - h) / 2;
		rcNew.right = rcNew.left + w;
		rcNew.bottom = rcNew.top + h;
	}

	if (!rcNew.EqualRect(&m_rcReal))
		MoveWindowSafely(rcNew.left, rcNew.top, rcNew.Width(), rcNew.Height(), true);

/*	if (!m_wndResizer.IsFixedWidth() && !m_wndResizer.IsFixedHeight())
	{
// 		sizeMode = (WndSizeMode)g_Profile.GetProfileInt(GetSkinWndName(), _T("SizeMode"), WndSizeMode_Normal);
// 		if (sizeMode == WndSizeMode_Maximized)
// 			dwShowFlag = SW_MAXIMIZE;
// 			// ShowWindow(SW_MAXIMIZE);
// 		else if (sizeMode == WndSizeMode_Minimized)
// 			dwShowFlag = SW_MINIMIZE;
			// ShowWindow(SW_MINIMIZE);
	}*/

/*	WINDOWPLACEMENT		wp;
	memset(&wp, 0, sizeof(wp));
	wp.length = sizeof(wp);
	wp.flags = 0;
	wp.ptMaxPosition.y = wp.ptMaxPosition.x = -1;
	wp.ptMinPosition.y = wp.ptMinPosition.x = -1;
	wp.rcNormalPosition = rcNew;
	wp.showCmd = dwShowFlag;
	SetWindowPlacement(GetHandle(), &wp);*/
#endif
}

// COMMENT:
//		save current skin window size pos.
void CSkinWnd::SaveWndPos()
{
	CRect			rcToSave;

#ifdef _WIN32_DESKTOP
	if (::GetParent(m_hWnd))
		return;

	WINDOWPLACEMENT	wp;

	wp.length = sizeof(wp);
	if (GetWindowPlacement(m_hWnd, &wp))
	{
		rcToSave = wp.rcNormalPosition;
	}
	else
		rcToSave = m_rcReal;

	WndSizeMode		sizeMode;
	if (IsFlagSet(wp.showCmd, SW_SHOWMAXIMIZED))
		sizeMode = WndSizeMode_Maximized;
	else if (IsFlagSet(wp.showCmd, SW_SHOWMINIMIZED))
		sizeMode = WndSizeMode_Minimized;
	else
		sizeMode = WndSizeMode_Normal;

	g_Profile.WriteProfileInt(GetSkinWndName(), _T("SizeMode"), sizeMode);
#else
	if (!GetWindowRect(&rcToSave))
		return;
#endif

	g_Profile.WriteProfileInt(GetSkinWndName(), _T("wnd_left"), rcToSave.left);
	g_Profile.WriteProfileInt(GetSkinWndName(), _T("wnd_top"), rcToSave.top);
	g_Profile.WriteProfileInt(GetSkinWndName(), _T("wnd_width"), rcToSave.Width());
	g_Profile.WriteProfileInt(GetSkinWndName(), _T("wnd_height"), rcToSave.Height());
}
