// SkinMenuBar.cpp: implementation of the CSkinMenuBar class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MLSkin.h"
#include "SkinMenuBar.h"

#ifdef _WIN32

HHOOK	CMenuBarMsgHandler::ms_hMsgHook = NULL;
bool	CMenuBarMsgHandler::ms_bPrimaryMenu = true;
bool	CMenuBarMsgHandler::ms_bSubMenuItem = false;
HWND	CMenuBarMsgHandler::ms_hWndMsgReceiver = 0;
HMENU	CMenuBarMsgHandler::ms_hMenuPopup = NULL;

void CMenuBarMsgHandler::PostTrackPopupMenu()
{
	// Exit popup menu first
	SendMessage(m_pMenuBar->m_pSkin->GetHandle(), WM_CANCELMODE, 0 , 0);

	PostMessage(m_hWnd, WM_MB_TRACKMENU, 0, 0);
}

bool CMenuBarMsgHandler::TrackPopupSubMenu(int x, int y, CMLMenu *pMenu, int nSubMenu, CBaseWnd *pWnd, RECT *prcNotOverlap)
{
	HMENU	hMenu = pMenu->GetSubMenu(nSubMenu);
	ASSERT(pWnd);
	ASSERT(hMenu);
	if (!hMenu)
		return false;

	ASSERT(ms_hMsgHook == NULL);
	if (ms_hMsgHook == NULL)
	{
		// Install message hook
		ms_bPrimaryMenu = true;
		ms_bSubMenuItem = false;
		ms_hWndMsgReceiver = m_hWnd;
		ms_hMenuPopup = hMenu;
		ms_hMsgHook = ::SetWindowsHookEx(WH_MSGFILTER, MessageHookProc, 0, ::GetCurrentThreadId());
	}

	m_bMenuPopup = true;
	m_pMenuBar->Invalidate();

	pMenu->TrackPopupSubMenu(x, y, nSubMenu, pWnd, prcNotOverlap);
	// ::TrackPopupMenuEx(hMenu, TPM_LEFTALIGN | TPM_VERTICAL, x, y, pWnd->GetHandle(), prcNotOverlap ? &tpmParam : NULL);

	m_bMenuPopup = false;
	m_pMenuBar->Invalidate();

	if (ms_hMsgHook)
	{
		// Uninstall message hook
		::UnhookWindowsHookEx(ms_hMsgHook);
		ms_hMsgHook = 0;
		ms_hMenuPopup = NULL;
		ms_hWndMsgReceiver = NULL;
	}	

	return true;
}


void CMenuBarMsgHandler::OnKeyDown(UINT nChar, UINT nFlags)
{
	m_pMenuBar->OnKeyDown(nChar, nFlags);
}


void CMenuBarMsgHandler::OnMouseMove(UINT nFlags, CPoint point)
{
	if (WindowFromPoint(point) == m_pMenuBar->m_pSkin->GetHandle())
	{
		m_pMenuBar->m_pSkin->ScreenToClient(point);
		m_pMenuBar->OnMouseMove(nFlags, point);
	}
}


LRESULT CMenuBarMsgHandler::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_MB_TRACKMENU && m_pMenuBar)
	{
		m_pMenuBar->TrackPopupMenu();

		return 0;
	}
	else
		return CBaseWnd::WndProc(message, wParam, lParam);
}


LRESULT CALLBACK CMenuBarMsgHandler::MessageHookProc(int code, WPARAM wParam, LPARAM lParam )
{
	if (code == MSGF_MENU)
	{
		MSG* pMsg = ( MSG* )lParam;

		ProcessHookMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
	}

	return ::CallNextHookEx(ms_hMsgHook, code, wParam, lParam );
}

void CMenuBarMsgHandler::ProcessHookMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch ( message )
	{
	case WM_MENUSELECT:
		{
			HMENU	hMenu = (HMENU)lParam;
			UINT	wFlags = HIWORD(wParam);
			ms_bPrimaryMenu = (ms_hMenuPopup == hMenu);
			ms_bSubMenuItem = (wFlags & MF_POPUP ) != 0;
		}
		break;
	case WM_INITMENUPOPUP:
		{
			HMENU	hMenu = (HMENU)wParam;
			int		nIndex = LOWORD(lParam);
			ms_bPrimaryMenu = (ms_hMenuPopup == hMenu);
			ms_bSubMenuItem = (::GetSubMenu(hMenu, nIndex) != 0);
		}
		break;
	case WM_MOUSEMOVE:
		{
			::SendMessage(ms_hWndMsgReceiver, message, wParam, lParam);
			break;
		}
	case WM_KEYDOWN:
		{
			UINT nChar = ( UINT )wParam;
			bool bForward = false;  // by default

			switch (nChar)
			{
			case VK_LEFT:
				bForward = ms_bPrimaryMenu;
				break;
			case VK_RIGHT:
				bForward = !ms_bSubMenuItem;
				break;
				// Do not process escape
				// case VK_ESCAPE:
				//	m_bEscape = m_bPrimaryMenu;
				//	break;
			}

			// Should we forward this message to the menu bar?
			if (bForward)
				::SendMessage(ms_hWndMsgReceiver, message, wParam, lParam);
			break;
		}
	default:
		break;
	}
}

#endif // _WIN32

//////////////////////////////////////////////////////////////////////////

#define SPACE_SUBMENUS		(m_font.GetHeight() / 2)

UIOBJECT_CLASS_NAME_IMP(CSkinMenuBar, _T("MenuBar"))

CSkinMenuBar::CSkinMenuBar()
{
	m_wndMsgReceiver.m_pMenuBar = this;

	m_nMsgWant = UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_LBUTTON | UO_MSG_WANT_MENU_KEY;

	m_bLBtDown = false;
	m_bHover = false;

	m_nSelSubMenu = -1;

	m_bOutlinedHover = false;
	m_bOutlinedPressed = false;

	m_clrHover.set(0);
	m_clrPressed.set(0);
	m_clrBgPressed.set(0);
	m_clrBgPressed.setAlpha(128);

	m_pMenu = NULL;
	m_ptLast.x = m_ptLast.y = 0;
}


CSkinMenuBar::~CSkinMenuBar()
{
	m_wndMsgReceiver.Destroy();

	if (m_pMenu)
		delete m_pMenu;
}


void CSkinMenuBar::OnCreate()
{
	CUIObject::OnCreate();

	m_wndMsgReceiver.Create(NULL, 0, 0,  1, 1, NULL, WS_POPUP);

	m_font.OnCreate(m_pSkin);

	if (m_strMenu.empty() && !IsEmptyString(m_pSkin->GetMenuName()))
	{
		m_pSkin->GetSkinFactory()->LoadMenu(m_pSkin, &m_pMenu, 
			m_pSkin->GetMenuName());
	}

	if (m_pMenu)
	{
		for (int i = 0; i < 255; i++)
		{
			SubMenu		item;

			if (m_pMenu->GetMenuItemInfo(i, item.strText, item.nID, item.bHasSubMenu, true))
			{
				item.chMenuKey = GetMenuKey(item.strText.c_str());
				m_vSubMenus.push_back(item);
			}
		}
	}
}


void CSkinMenuBar::Draw(CRawGraph *pGraph)
{
	int			x = 0;
	int			y = 0;
	int			nSpaceSubMenus = SPACE_SUBMENUS;
	CRect		rcItem;

	if (m_font.IsGood())
		pGraph->SetFont(m_font.GetFont());

	rcItem.top = m_rcObj.top;
	rcItem.bottom = m_rcObj.bottom;
	x = m_rcObj.left;

	for (int i = 0; i < (int)m_vSubMenus.size(); i++)
	{
		SubMenu		&item = m_vSubMenus[i];
		MLColor		clrText, clrOutlined;
		SIZE		size;
		bool		bOutlined;

		pGraph->GetTextExtentPoint32(item.strText.c_str(), item.strText.size(), &size);
		item.nWidth = size.cx + nSpaceSubMenus;

		if (x + item.nWidth > m_rcObj.right)
			break;

		rcItem.left = x;
		rcItem.right = x + item.nWidth;

		if (i == m_nSelSubMenu && (m_bLBtDown || m_wndMsgReceiver.IsMenuPopuped()))
		{
			clrText = m_clrPressed;
			clrOutlined = m_clrOutlinedPressed;
			bOutlined = m_bOutlinedPressed;
			pGraph->FillRect(&rcItem, m_clrBgPressed, BPM_OP_BLEND | BPM_CHANNEL_RGB);
		}
		else if (i == m_nSelSubMenu && m_bHover)
		{
			clrText = m_clrHover;
			clrOutlined = m_clrOutlinedHover;
			bOutlined = m_bOutlinedHover;
		}
		else
		{
			clrText = m_font.GetTextColor();
			clrOutlined = m_font.GetColorOutlined();
			bOutlined = m_font.IsOutlined();
		}

		if (bOutlined)
			pGraph->DrawTextOutlined(item.strText.c_str(), item.strText.size(), 
				&rcItem, clrText, clrOutlined, DT_VCENTER | DT_CENTER | DT_PREFIX_TEXT | DT_SINGLELINE);
		else
		{
			pGraph->SetTextColor(clrText);
			pGraph->DrawText(item.strText.c_str(), item.strText.size(), 
			&rcItem, DT_VCENTER | DT_CENTER | DT_PREFIX_TEXT | DT_SINGLELINE);
		}

		x += item.nWidth;
	}
}


bool CSkinMenuBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!m_rcObj.PtInRect(point))
		return false;

	m_nSelSubMenu = HitTestSubMenu(point);
	if (m_nSelSubMenu == -1)
		return false;

	m_bLBtDown = TRUE;

	if (m_bHover)
		m_bHover = false;
	else
		m_pSkin->SetCaptureMouse(this);

	Invalidate();

	// Popup sel menu item
	if (m_vSubMenus[m_nSelSubMenu].bHasSubMenu)
	{
		TrackPopupMenu();
	}
	else
		m_pSkin->OnCommand(m_vSubMenus[m_nSelSubMenu].nID, 0);

	return true;
}


bool CSkinMenuBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	// m_pSkin->ReleaseCaptureMouse(this);

	if (!m_bLBtDown)
		return true;

	m_bLBtDown = FALSE;

	Invalidate();

	return true;
}


bool CSkinMenuBar::OnMouseMove(UINT nFlags, CPoint point)
{
	int			nSelSubMenuNew;

	// Filter repeated mouse message when set message hook.
	if (point == m_ptLast)
		return true;
	m_ptLast = point;

	nSelSubMenuNew = HitTestSubMenu(point);
	if (nSelSubMenuNew == -1 && !m_bLBtDown && !m_bHover && !m_wndMsgReceiver.IsMenuPopuped())
		return false;

	if (IsPtIn(point))
	{
		if (m_wndMsgReceiver.IsMenuPopuped())
		{
			if (nSelSubMenuNew != m_nSelSubMenu && nSelSubMenuNew != -1)
			{
				m_nSelSubMenu = nSelSubMenuNew;
				Invalidate();
				if (m_nSelSubMenu != -1 && m_vSubMenus[m_nSelSubMenu].bHasSubMenu)
					m_wndMsgReceiver.PostTrackPopupMenu();
			}
			return true;
		}

		if (!m_bLBtDown && !m_bHover)
		{
			m_bHover = true;

			m_pSkin->SetCaptureMouse(this);

			m_nSelSubMenu = nSelSubMenuNew;

			Invalidate();
		}
		else if (m_bHover || m_bLBtDown)
		{
			if (m_nSelSubMenu != nSelSubMenuNew)
			{
				m_nSelSubMenu = nSelSubMenuNew;
				Invalidate();
			}
		}
	}
	else
	{
		if (m_bHover)
		{
			m_bHover = false;

			m_nSelSubMenu = nSelSubMenuNew;

			Invalidate();
		}
	}

	return true;
}


void CSkinMenuBar::OnKeyDown(UINT nChar, UINT nFlags)
{
	if (m_nSelSubMenu == -1)
		return;

	switch (nChar)
	{
	case VK_LEFT:
		{
			if (m_nSelSubMenu > 0)
				m_nSelSubMenu--;
			else
				m_nSelSubMenu = m_vSubMenus.size() - 1;

			if (m_nSelSubMenu >= 0 && m_nSelSubMenu < (int)m_vSubMenus.size())
			{
				Invalidate();
				if (m_wndMsgReceiver.IsMenuPopuped())
					m_wndMsgReceiver.PostTrackPopupMenu();
			}
		}
		break;
	case VK_RIGHT:
		{
			if (m_nSelSubMenu >= (int)m_vSubMenus.size() - 1)
				m_nSelSubMenu = 0;
			else
				m_nSelSubMenu++;

			if (m_nSelSubMenu >= 0 && m_nSelSubMenu < (int)m_vSubMenus.size())
			{
				Invalidate();
				if (m_wndMsgReceiver.IsMenuPopuped())
					m_wndMsgReceiver.PostTrackPopupMenu();
			}
		}
		break;
	}
}


bool CSkinMenuBar::OnMenuKey(UINT nChar, UINT nFlags)
{
	for (int i = 0; i < (int)m_vSubMenus.size(); i++)
	{
		if (CharUpr(m_vSubMenus[i].chMenuKey) == CharUpr(nChar))
		{
			m_nSelSubMenu = i;
			if (m_vSubMenus[i].bHasSubMenu)
				m_wndMsgReceiver.PostTrackPopupMenu();
			else
				m_pSkin->OnCommand(m_vSubMenus[m_nSelSubMenu].nID, 0);
			return true;
		}
	}

	return false;
}


void CSkinMenuBar::OnSetFocus()
{
	// DBGLOG0(_T("OnSetFocus"));
}


void CSkinMenuBar::OnKillFocus()
{
	// DBGLOG0(_T("OnKillFocus"));

	if (m_bLBtDown)
	{
		m_bLBtDown = false;
		Invalidate();
	}
}


BOOL CSkinMenuBar::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (IsPropertyName(szProperty, _T("Menu")))
	{
		m_strMenu = szValue;
		m_pSkin->GetSkinFactory()->LoadMenu(m_pSkin, &m_pMenu, szValue);
		ASSERT(m_pMenu);
	}
	else if (m_font.SetProperty(szProperty, szValue))
		return TRUE;
	else if (IsPropertyName(szProperty, _T("TextColorHover")))
		GetColorValue(m_clrHover, szValue);
	else if (IsPropertyName(szProperty, _T("TextOutlinedColorHover")))
	{
		GetColorValue(m_clrOutlinedHover, szValue);
		m_bOutlinedHover = true;
	}
	else if (IsPropertyName(szProperty, _T("TextColorPressed")))
		GetColorValue(m_clrPressed, szValue);
	else if (IsPropertyName(szProperty, _T("TextOutlinedColorPressed")))
	{
		GetColorValue(m_clrOutlinedPressed, szValue);
		m_bOutlinedPressed = true;
	}
	else if (IsPropertyName(szProperty, _T("BgColorPressed")))
		GetColorValue(m_clrBgPressed, szValue);
	else if (IsPropertyName(szProperty, _T("BgPressedAlpha")))
		m_clrBgPressed.setAlpha(_ttoi(szValue));
	else
		return FALSE;

	return TRUE;
}


#ifdef _SKIN_EDITOR_
void CSkinMenuBar::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);
}
#endif // _SKIN_EDITOR_


int CSkinMenuBar::HitTestSubMenu(const POINT point)
{
	int			x = m_rcObj.left;

	if (point.x < m_rcObj.left)
		return -1;

	for (int i = 0; i < (int)m_vSubMenus.size(); i++)
	{
		x += m_vSubMenus[i].nWidth;
		if (x > m_rcObj.right)
			return -1;

		if (point.x < x)
			return i;
	}

	return -1;
}


int CSkinMenuBar::GetItemX(int n)
{
	int			x = m_rcObj.left;

	if (n >= (int)m_vSubMenus.size())
		n = m_vSubMenus.size() - 1;
	for (int i = 0; i < n; i++)
	{
		x += m_vSubMenus[i].nWidth;
		if (x > m_rcObj.right)
			return x;
	}

	return x;
}


void CSkinMenuBar::TrackPopupMenu()
{
	if (m_nSelSubMenu == -1 || m_pMenu == NULL)
		return;

	if (m_pSkin->GetCaptureMouse() != this)
		m_pSkin->SetCaptureMouse(this);

	CPoint	pt;
	CRect	rc, rcMonitor;

	pt.x = GetItemX(m_nSelSubMenu);
	pt.y = m_rcObj.bottom;

	rc = m_rcObj;
	rc.left = pt.x;
	rc.right = rc.left + m_vSubMenus[m_nSelSubMenu].nWidth;
	m_pSkin->ClientToScreen(rc);

	m_pSkin->ClientToScreen(pt);

	GetMonitorRestrictRect(rc, rcMonitor);
	if (rc.left < rcMonitor.left)
	{
		rc.right = rcMonitor.left + rc.Width();
		pt.x = rc.left = rcMonitor.left;
	}

	m_bLBtDown = false;
#ifdef _WIN32
	m_wndMsgReceiver.TrackPopupSubMenu(pt.x, pt.y, m_pMenu, m_nSelSubMenu, m_pSkin, &rc);
#endif
}
