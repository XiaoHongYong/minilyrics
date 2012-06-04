// Menu.cpp: implementation of the CMLMenu class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BaseWnd.h"
#include "MLMenu.h"


#ifdef _WINCE_

#ifndef DEFAULT_GUI_FONT
#define DEFAULT_GUI_FONT  SYSTEM_FONT
#endif

int GetMenuItemCount(HMENU hMenu)
{
	const int MAX_NUM_ITEMS = 256;
	int  iPos, iCount;

	MENUITEMINFO mii;
	memset((char *)&mii, 0, sizeof(MENUITEMINFO));
	mii.cbSize = sizeof(MENUITEMINFO);

	iCount = 0;
	for (iPos = 0; iPos < MAX_NUM_ITEMS; iPos++)
	{
		if(!GetMenuItemInfo(hMenu, (UINT)iPos, TRUE, &mii))
			break;
		iCount++;
	}

	return iCount;
}
#endif

BOOL ToLocalMenu(CMLMenu *pMenu)
{
	return ToLocalMenu(pMenu->GetHandle());
}

BOOL ToLocalMenu(HMENU hMenu)
{
	if (g_LangTool.IsEnglish() && !g_LangTool.HasMacro())
		return TRUE;

	// int				nCount;
	MENUITEMINFO	MenuItemInfo;
	TCHAR			szString[256];
	TCHAR			szAccKey[256];
	LPCTSTR			szPos;

	// don't use get menu item count, under win ce, it will cost a lot time
	for (int i = 0; i < 256; i++)
	{
		memset(&MenuItemInfo, 0, sizeof(MenuItemInfo));

		MenuItemInfo.cbSize = sizeof(MenuItemInfo);
		MenuItemInfo.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_DATA;
		MenuItemInfo.dwTypeData = szString;
		MenuItemInfo.cch = CountOf(szString);
		szString[0] = '\0';

		// get menu item by pos.
		if (!GetMenuItemInfo(hMenu, i, TRUE, &MenuItemInfo))
			break;

		if (MenuItemInfo.hSubMenu != NULL)
		{
			ToLocalMenu(MenuItemInfo.hSubMenu);
		}

		if (IsEmptyString(szString))
			continue;

		//
		// translate menu string
		//
		szPos = _tcschr(szString, '\t');
		if (!szPos)
			_tcscpy_safe(szString, CountOf(szString), _TL(szString));
		else
		{
			_tcscpy_safe(szAccKey, CountOf(szAccKey), szPos);
			szString[int(szPos - szString)] = '\0';
			_tcscpy_safe(szString, CountOf(szString), _TL(szString));
			_tcscat_safe(szString, CountOf(szString), szAccKey);
		}

		MenuItemInfo.cch = _tcslen(szString) + 1;
		MenuItemInfo.fMask = MIIM_DATA | MIIM_TYPE;
		if (!SetMenuItemInfo(hMenu, i, TRUE, &MenuItemInfo))
		{
			OSError		Err;
			ERRLOG1(_T("SetMenuItemInfo: %s"), Err.Description());
			continue;
		}
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////

CMLMenu::CMLMenu()
{
	m_hMenu = NULL;
	m_nSubMenu = -1;
	m_bFree = true;
}

CMLMenu::~CMLMenu()
{
	Destroy();
}

bool CMLMenu::LoadMenu(int nID)
{
	Destroy();

	m_hMenu = ::LoadMenu(GetAppInstance(), MAKEINTRESOURCE(nID));
	ASSERT(m_hMenu);
	m_nSubMenu = -1;

	ToLocalMenu(this);

	if (m_hMenu != NULL)
		OnLoadMenu();

	return m_hMenu != NULL;
}

bool CMLMenu::CreatePopupMenu()
{
	Destroy();

	m_hMenu = ::CreatePopupMenu();

	return m_hMenu != NULL;
}

void CMLMenu::Destroy()
{
	if (m_hMenu && m_bFree)
	{
		DestroyMenu(m_hMenu);
		m_hMenu = NULL;
	}
}

void CMLMenu::TrackPopupMenu(int x, int y, CBaseWnd *pWnd, RECT *prcNotOverlap)
{
	ASSERT(pWnd);

	TPMPARAMS		tpmParam;

	if (prcNotOverlap)
	{
		tpmParam.cbSize = sizeof(tpmParam);
		tpmParam.rcExclude = *prcNotOverlap;
	}

	TrackPopupMenuEx(GetHandle(), TPM_LEFTALIGN, x, y, pWnd->GetHandle(), prcNotOverlap ? &tpmParam : NULL);
}

void CMLMenu::TrackPopupSubMenu(int x, int y, int nSubMenu, CBaseWnd *pWnd, RECT *prcNotOverlap)
{
	ASSERT(pWnd);

	TPMPARAMS		tpmParam;

	if (prcNotOverlap)
	{
		tpmParam.cbSize = sizeof(tpmParam);
		tpmParam.rcExclude = *prcNotOverlap;
	}

	TrackPopupMenuEx(GetSubMenuHandle(nSubMenu), TPM_LEFTALIGN | TPM_VERTICAL, x, y, pWnd->GetHandle(), prcNotOverlap ? &tpmParam : NULL);
}

void CMLMenu::EnableItem(int nID, bool bEnable)
{
	HMENU	hMenu = GetHandle();

	EnableMenuItem(hMenu, nID, bEnable ? MF_ENABLED : MF_GRAYED);
}

void CMLMenu::CheckItem(int nID, bool bCheck)
{
	HMENU	hMenu = GetHandle();

	CheckMenuItem(hMenu, nID, bCheck ? MF_CHECKED : MF_UNCHECKED);
}

void CMLMenu::CheckRadioItem(int nID, int nStartID, int nEndID)
{
	HMENU	hMenu = GetHandle();

	CheckMenuRadioItem(hMenu, nStartID, nEndID, nID, MF_BYCOMMAND);
}

int CMLMenu::GetItemCount()
{
	return ::GetMenuItemCount(GetHandle());
}

void CMLMenu::AppendItem(UINT nID, LPCTSTR szText, LPCTSTR szShortcutKey)
{
	if (IsEmptyString(szShortcutKey))
		::AppendMenu(m_hMenu, MF_STRING, nID, szText);
	else
		::AppendMenu(m_hMenu, MF_STRING, nID, (tstring(szText) + _T("\t") + szShortcutKey).c_str());
}

void CMLMenu::AppendSeperator()
{
	AppendMenu(m_hMenu, MF_SEPARATOR, 0, NULL);
}

void CMLMenu::InsertItem(int nPos, UINT nID, LPCTSTR szText, LPCTSTR szShortcutKey)
{
	if (IsEmptyString(szShortcutKey))
		::InsertMenu(GetHandle(), nPos, MF_BYPOSITION, nID, szText);
	else
		::InsertMenu(GetHandle(), nPos, MF_BYPOSITION, nID, (tstring(szText) + _T("\t") + szShortcutKey).c_str());
}

void CMLMenu::InsertItemByID(UINT nPosID, UINT nID, LPCTSTR szText, LPCTSTR szShortcutKey)
{
	HMENU	hMenu = FindMenuPos(GetHandle(), nPosID);
	ASSERT(hMenu);
	if (hMenu == NULL)
		return;

	if (IsEmptyString(szShortcutKey))
		::InsertMenu(hMenu, nPosID, MF_BYCOMMAND, nID, szText);
	else
		::InsertMenu(hMenu, nPosID, MF_BYCOMMAND, nID, (tstring(szText) + _T("\t") + szShortcutKey).c_str());
}

void CMLMenu::InsertSeperator(int nPos)
{
	::InsertMenu(GetHandle(), nPos, MF_SEPARATOR, 0, NULL);
}

CMLMenu CMLMenu::AppendSubmenu(LPCTSTR szText)
{
	HMENU hSubMenu = ::CreatePopupMenu();
	::AppendMenu(GetHandle(), MF_POPUP, (UINT_PTR)hSubMenu, szText);

	CMLMenu menu;
	menu.Attach(hSubMenu);
	menu.m_bFree = false;

	return menu;
}

CMLMenu CMLMenu::InsertSubmenu(int nPos, LPCTSTR szText)
{
	HMENU hSubMenu = ::CreatePopupMenu();
	::InsertMenu(GetHandle(), nPos, MF_BYPOSITION | MF_POPUP, (UINT_PTR)hSubMenu, szText);

	CMLMenu menu;
	menu.Attach(hSubMenu);
	menu.m_bFree = false;

	return menu;
}


void CMLMenu::RemoveItem(int nPos)
{
	::DeleteMenu(GetHandle(), nPos, MF_BYPOSITION);
}

bool CMLMenu::GetMenuItemText(UINT nItem, tstring &strText, bool bByPosition)
{
	UINT		nID;
	bool		bSubMenu;

	return GetMenuItemInfo(nItem, strText, nID, bSubMenu, bByPosition);
}

bool CMLMenu::GetMenuItemInfo(UINT nItem, tstring &strText, UINT &nID, bool &bSubMenu, bool bByPosition)
{
	MENUITEMINFO	MenuItemInfo;
	TCHAR			szString[256];

	MenuItemInfo.cbSize = sizeof(MenuItemInfo);
	MenuItemInfo.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_DATA | MIIM_ID;
	MenuItemInfo.dwTypeData = szString;
	MenuItemInfo.cch = CountOf(szString);
	szString[0] = '\0';

	// get menu item by pos.
	if (!::GetMenuItemInfo(GetHandle(), nItem, bByPosition, &MenuItemInfo))
		return false;

	strText = szString;
	nID = MenuItemInfo.wID;
	bSubMenu = MenuItemInfo.hSubMenu != NULL;

	return true;
}

bool CMLMenu::LoadPopupMenu(int nID, int nSubMenu)
{
	Destroy();

	m_hMenu = ::LoadMenu(GetAppInstance(), MAKEINTRESOURCE(nID));
	ASSERT(m_hMenu);
	m_nSubMenu = nSubMenu;

	ToLocalMenu(::GetSubMenu(m_hMenu, m_nSubMenu));

	if (m_hMenu != NULL)
		OnLoadMenu();

	return m_hMenu != NULL;
}

bool CMLMenu::HasItem(int nPos)
{
	MENUITEMINFO	mii;

	memset(&mii, 0, sizeof(mii));

	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_TYPE;
	mii.dwTypeData = NULL;
	mii.cch = 0;

	// get menu item by pos.
	if (!::GetMenuItemInfo(GetHandle(), nPos, TRUE, &mii))
		return false;

	return true;
}

bool CMLMenu::HasSubmenu(int nPos)
{
	return GetSubMenuHandle(nPos) != NULL;
}

CMLMenu CMLMenu::GetSubmenu(int nPos)
{
	CMLMenu menu;

	menu.Attach(GetSubMenuHandle(nPos));
	menu.m_bFree = false;

	return menu;
}

CMLMenu & CMLMenu::operator = (CMLMenu &menu)
{
	m_hMenu = menu.m_hMenu;
	m_nSubMenu = menu.m_nSubMenu;
	m_bFree = false;

	return *this;
}

HMENU CMLMenu::GetSubMenuHandle(int nPos)
{
	if (m_nSubMenu == -1)
		return ::GetSubMenu(m_hMenu, nPos);
	else
	{
		HMENU		hSubMenu;
		hSubMenu = ::GetSubMenu(m_hMenu, m_nSubMenu);
		return ::GetSubMenu(hSubMenu, nPos);
	}
}

HMENU CMLMenu::GetHandle()
{
	if (m_nSubMenu == -1)
		return m_hMenu;
	else
	{
		return ::GetSubMenu(m_hMenu, m_nSubMenu);
	}
}

HMENU CMLMenu::FindMenuPos(HMENU hMenu, UINT nIDMenu)
{
	MENUITEMINFO	mii;

	// don't use get menu item count, under win ce, it will cost a lot time
	for (int i = 0; i < 256; i++)
	{
		memset(&mii, 0, sizeof(mii));

		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_ID;
		mii.dwTypeData = NULL;
		mii.cch = 0;

		// get menu item by pos.
		if (!::GetMenuItemInfo(hMenu, i, TRUE, &mii))
			break;

		if (mii.hSubMenu != NULL)
		{
			HMENU h = FindMenuPos(mii.hSubMenu, nIDMenu);
			if (h != NULL)
				return h;
		}
		else
		{
			if (mii.wID == nIDMenu)
				return hMenu;
		}
	}

	return NULL;
}
