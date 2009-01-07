// SkinMenuBar.cpp: implementation of the CSkinMenuBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MLSkin.h"
#include "SkinMenuBar.h"

#if (WINVER < 0x0500)

#define MNS_NOCHECK         0x80000000
#define MNS_MODELESS        0x40000000
#define MNS_DRAGDROP        0x20000000
#define MNS_AUTODISMISS     0x10000000
#define MNS_NOTIFYBYPOS     0x08000000
#define MNS_CHECKORBMP      0x04000000

#define MIM_MAXHEIGHT               0x00000001
#define MIM_BACKGROUND              0x00000002
#define MIM_HELPID                  0x00000004
#define MIM_MENUDATA                0x00000008
#define MIM_STYLE                   0x00000010
#define MIM_APPLYTOSUBMENUS         0x80000000

typedef struct tagMENUINFO
{
	DWORD   cbSize;
	DWORD   fMask;
	DWORD   dwStyle;
	UINT    cyMax;
	HBRUSH  hbrBack;
	DWORD   dwContextHelpID;
	ULONG_PTR dwMenuData;
}   MENUINFO, FAR *LPMENUINFO;
typedef MENUINFO CONST FAR *LPCMENUINFO;

WINUSERAPI
BOOL
WINAPI
GetMenuInfo(
			__in HMENU,
			__inout LPMENUINFO);

WINUSERAPI
BOOL
WINAPI
SetMenuInfo(
			__in HMENU,
			__in LPCMENUINFO);

WINUSERAPI
BOOL
WINAPI
EndMenu(
		VOID);

#endif // WINVER < 0x0500

void SetMenuModeless(HMENU hMenu)
{
	MENUINFO	info;

	memset(&info, 0, sizeof(info));
	info.cbSize = sizeof(info);
	info.fMask = MIM_STYLE | MIM_APPLYTOSUBMENUS;
	info.dwStyle = MNS_MODELESS;

	typedef BOOL (WINAPI *FuncSetMenuInfo)(
		__in HMENU,
		__in LPCMENUINFO);

	HMODULE			hDll = LoadLibrary(_T("user32.dll"));
	FuncSetMenuInfo	f;

	f = (FuncSetMenuInfo)GetProcAddress(hDll, "SetMenuInfo");
	if (f)
		f(hMenu, &info);

	FreeLibrary(hDll);
}

void _EndMenu()
{
	typedef BOOL
		(WINAPI *
		FuncEndMenu)(
		VOID);

	HMODULE			hDll = LoadLibrary(_T("user32.dll"));
	FuncEndMenu	f;

	f = (FuncEndMenu)GetProcAddress(hDll, "EndMenu");
	if (f)
		f();

	FreeLibrary(hDll);

	MSG stMsg = { 0 };
	while (PeekMessage(&stMsg, NULL, 0, 0, PM_REMOVE ))
	{
		TranslateMessage(&stMsg);
		DispatchMessage(&stMsg);
	}
}

#define SPACE_SUBMENUS		(m_rcObj.Height() / 2)

UIOBJECT_CLASS_NAME_IMP(CSkinMenuBar, _T("MenuBar"))

CSkinMenuBar::CSkinMenuBar()
{
	m_nMsgWant = UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_LBUTTON | UO_MSG_WANT_ALL_KEYS;

	m_bLBtDown = false;
	m_bHover = false;

	m_nSelSubMenu = -1;

	m_bOutlinedHover = false;
	m_bOutlinedPressed = false;
	m_bMenuPopup = false;

	m_clrHover.set(0);
	m_clrPressed.set(0);

	m_pMenu = NULL;
}


CSkinMenuBar::~CSkinMenuBar()
{
	if (m_pMenu)
		delete m_pMenu;
}


void CSkinMenuBar::OnCreate()
{
	CUIObject::OnCreate();

	m_font.OnCreate(m_pSkin);

	ASSERT(m_pMenu);
	if (m_pMenu)
	{
		m_pMenu->SetModeless(true);

		for (int i = 0; i < 255; i++)
		{
			SubMenu		item;

			if (m_pMenu->GetMenuItemInfo(i, item.strText, item.nID, item.bHasSubMenu, true))
			{
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

	for (int i = 0; i < m_vSubMenus.size(); i++)
	{
		SubMenu		&item = m_vSubMenus[i];
		MLColor		clrText, clrOutlined;
		SIZE		size;
		bool		bOutlined;

		pGraph->GetTextExtentPoint32(item.strText.c_str(), item.strText.size(), &size);
		item.nWidth = size.cx + nSpaceSubMenus * 2;

		if (x + item.nWidth > m_rcObj.right)
			break;

		rcItem.left = x;
		rcItem.right = x + item.nWidth;

// 		if (i % 2 == 0)
// 		{
// 			CMLBrush	br;
// 			br.CreateSolidBrush(MLColor(0));
// 			pGraph->FillRect(rcItem.left, rcItem.top, rcItem.Width(), rcItem.Height(), &br);
// 		}

		if (i == m_nSelSubMenu && m_bLBtDown)
		{
			clrText = m_clrPressed;
			clrOutlined = m_clrOutlinedPressed;
			bOutlined = m_bOutlinedPressed;
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
				&rcItem, clrText, clrOutlined, DT_VCENTER | DT_CENTER);
		else
		{
			pGraph->SetTextColor(clrText);
			pGraph->DrawText(item.strText.c_str(), item.strText.size(), 
			&rcItem, DT_VCENTER | DT_CENTER);
		}

		x += m_vSubMenus[i].nWidth;
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
		m_bMenuPopup = true;
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


bool CSkinMenuBar::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (HitTestSubMenu(point) == -1)
		return false;

	return true;
}


bool CSkinMenuBar::OnMouseMove(UINT nFlags, CPoint point)
{
	int			nSelSubMenuNew;

	DBGLOG2(_T("Mouse Move: %d, %d"), point.x, point.y);

	nSelSubMenuNew = HitTestSubMenu(point);
	if (nSelSubMenuNew == -1 && !m_bLBtDown && !m_bHover && !m_bMenuPopup)
		return false;

	if (IsPtIn(point))
	{
		if (m_bMenuPopup)
		{
			if (nSelSubMenuNew != m_nSelSubMenu && nSelSubMenuNew != -1)
			{
				m_nSelSubMenu = nSelSubMenuNew;
				if (m_nSelSubMenu != -1 && m_vSubMenus[m_nSelSubMenu].bHasSubMenu)
					TrackPopupMenu();
			}
			return true;
		}

		if (!m_bLBtDown && (nFlags & MK_LBUTTON))
		{
			m_bLBtDown = TRUE;

			if (m_bHover)
				m_bHover = false;
			else
				m_pSkin->SetCaptureMouse(this);

			m_nSelSubMenu = nSelSubMenuNew;

			Invalidate();

			// Popup sel menu item
			if (m_nSelSubMenu != -1 && m_vSubMenus[m_nSelSubMenu].bHasSubMenu)
				TrackPopupMenu();
		}
		else if (!m_bLBtDown && !m_bHover)
		{
			m_bHover = true;

			m_pSkin->SetCaptureMouse(this);

			m_nSelSubMenu = nSelSubMenuNew;

			Invalidate();
		}
		else if (m_bHover)
		{
			if (m_nSelSubMenu != nSelSubMenuNew)
			{
				m_nSelSubMenu = nSelSubMenuNew;
				Invalidate();
			}
		}
		else if (m_bLBtDown)
		{
			if (m_nSelSubMenu != nSelSubMenuNew)
			{
				m_nSelSubMenu = nSelSubMenuNew;
				Invalidate();

				// Popup new menu
				if (m_nSelSubMenu != -1 && m_vSubMenus[m_nSelSubMenu].bHasSubMenu)
					TrackPopupMenu();
			}
		}
	}
	else
	{
		if (m_bLBtDown)
		{
			m_bLBtDown = FALSE;

			m_pSkin->ReleaseCaptureMouse(this);

			m_nSelSubMenu = nSelSubMenuNew;

			Invalidate();
		}
		else if (m_bHover)
		{
			m_bHover = false;

			m_pSkin->ReleaseCaptureMouse(this);

			m_nSelSubMenu = nSelSubMenuNew;

			Invalidate();
		}
	}

	return true;
}


void CSkinMenuBar::OnKeyDown(UINT nChar, UINT nFlags)
{
	switch (nChar)
	{
	case VK_LEFT:
		if (m_nSelSubMenu > 0)
		{
			m_nSelSubMenu--;
			if (m_nSelSubMenu >= 0 && m_nSelSubMenu < m_vSubMenus.size())
			{
				ExitTrackingMenuMode();

				TrackPopupMenu();
			}
		}
		break;
	case VK_RIGHT:
		if (m_nSelSubMenu > 0)
		{
			m_nSelSubMenu++;
			if (m_nSelSubMenu >= 0 && m_nSelSubMenu < m_vSubMenus.size())
			{
				ExitTrackingMenuMode();

				TrackPopupMenu();
			}
		}
		break;
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
	else
		return FALSE;

	return TRUE;
}


void CSkinMenuBar::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);
}


int CSkinMenuBar::HitTestSubMenu(const POINT point)
{
	int			x = m_rcObj.left;

	if (point.x < m_rcObj.left)
		return -1;

	for (int i = 0; i < m_vSubMenus.size(); i++)
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

	if (n >= m_vSubMenus.size())
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
	if (m_nSelSubMenu == -1)
		return;

	CPoint	pt;

	pt.x = GetItemX(m_nSelSubMenu);
	pt.y = m_rcObj.bottom;

	m_pSkin->ClientToScreen(pt);

	if (m_pMenu)
		m_pMenu->TrackPopupSubMenu(pt.x, pt.y, m_nSelSubMenu, m_pSkin);
}

void CSkinMenuBar::ExitTrackingMenuMode()
{
	// if (::GetFocus() == m_pSkin->GetHandle())
	// ::SetFocus(m_pSkin->GetHandle());

	_EndMenu();
/*
	// ::ReleaseCapture();
	SendMessage(m_pSkin->GetHandle(), WM_CANCELMODE, 0 , 0);
	SendMessage(m_pSkin->GetHandle(), WM_CANCELMODE, 0 , 0);

	m_pSkin->SetCaptureMouse(this);

	MSG stMsg = { 0 };
	while (PeekMessage(&stMsg, NULL, 0, 0, PM_REMOVE ))
	{
		TranslateMessage(&stMsg);
		DispatchMessage(&stMsg);
	}*/
}
