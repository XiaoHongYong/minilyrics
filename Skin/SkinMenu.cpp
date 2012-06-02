// SkinMenu.cpp: implementation of the CSkinMenu class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Skin.h"
#include "SkinMenu.h"

//////////////////////////////////////////////////////////////////////////

int CSkinMenu::Item::FromXML(CSkinFactory *pSkinFactory, SXNode *pNode)
{
	// <Item Action="Append" Type="item" Name="Playlist Editor..." IDCmd="dyncmd_openclose_playlist_wnd" CanCheck="true" IsCheckedCmd="dyncmd_is_mediainfo_bar_visible"/>
	LPCTSTR		szValue;

	szValue = pNode->GetPropertySafe(_T("Type"));
	if (_tcsicmp(szValue, _T("item")) == 0)
		type = IT_ITEM;
	else if (_tcsicmp(szValue, _T("popup")) == 0)
		type = IT_POPUP;
	else if (_tcsicmp(szValue, _T("seperator")) == 0)
		type = IT_SEPERATOR;
	else
		type = IT_ITEM;

	szValue = pNode->GetPropertySafe(_T("Action"));
	if (_tcsicmp(szValue, _T("Append")) == 0 || _tcsicmp(szValue, _T("InsertByID")) == 0)
	{
		if (_tcsicmp(szValue, _T("Append")) == 0)
			action = A_APPEND;
		else
		{
			action = A_INSERT_BY_ID;
			nPos = pSkinFactory->GetIDByName(pNode->GetPropertySafe(_T("Position")));
			nPos = pSkinFactory->GetMenuIDByUID(nPos, false);
		}

		strName = pNode->GetPropertySafe(_T("Name"));
		bCanCheck = IsTRUE(pNode->GetPropertySafe(_T("CanCheck")));
		nIDCmd = pSkinFactory->GetIDByName(pNode->GetPropertySafe(_T("IDCmd")));
		nIDCmdIsChecked = pSkinFactory->GetIDByName(pNode->GetPropertySafe(_T("IsCheckedCmd")));
		nMenuID = pSkinFactory->GetMenuIDByUID(nIDCmd, true);
	}
	else if (_tcsicmp(szValue, _T("Remove")) == 0)
	{
		action = A_REMOVE;
		nPos = pNode->GetPropertyInt(_T("Position"));
	}
	else
		action = A_UNKNOWN;

	return ERR_OK;
}

CSkinMenu::CSkinMenu()
{
	m_nOrgAppenPos = 0;
	m_nAppendPosition = 0;
}

CSkinMenu::~CSkinMenu()
{

}

int CSkinMenu::LoadMenu(int nID)
{
	tstring strFile = CSkinApp::GetInstance()->GetSkinFactory()->GetResourceMgr()->GetResourcePathName(_T("menu.txt"));
	if (strFile.empty())
		return ERR_NOT_FOUND;

	CTextFile file;

	int nRet = file.Open(strFile.c_str(), CTextFile::OM_READ, ED_LATIN9_ISO);
	if (nRet != ERR_OK)
		return nRet;

	tstring line;
	file.ReadLine(line);
	if (_tcscmp(line.c_str(), _T("menufile_v1")) != 0)
	{
		ERRLOG1(_T("Invalid menu file format: %s"), strFile.c_str());
		return ERR_BAD_FILE_FORMAT;
	}

	CreatePopupMenu();

	CStrPrintf strMenu(_T("menu:%d"), nID);

	while (file.ReadLine(line))
	{
		if (_tcscmp(line.c_str(), strMenu.c_str()) != 0)
			continue;

		LoadMenu(file, *this);
		
		OnLoadMenu();
		return ERR_OK;
	}
	
	ERRLOG1(_T("Failed to load menu, NOT exists: %d"), nID);

	return ERR_NOT_FOUND;
}

void CSkinMenu::LoadMenu(CTextFile &file, CMLMenu &menu)
{
	CColonSeparatedValues values;
	tstring line;
	while (file.ReadLine(line))
	{
		VecString vValues;
		values.Split(line.c_str(), vValues);
		if (vValues.size() == 0)
			continue;

		if (_tcscmp(vValues[0].c_str(), _T("popup")) == 0)
		{
			CMLMenu subMenu = menu.AppendSubmenu(_TL(vValues[1].c_str()));
			LoadMenu(file, subMenu);
		}
		else if (_tcscmp(vValues[0].c_str(), _T("item")) == 0)
		{
			if (vValues.size() != 4)
			{
				ERRLOG1(_T("Invalid menu item line: %s"), line.c_str());
				continue;
			}
			int nID = StringToInt(vValues[1].c_str(), -1);
			GetShortcutKey(nID, vValues[3]);
			menu.AppendItem(nID, 
				_TL(vValues[2].c_str()), vValues[3].c_str());
		}
		else if (_tcscmp(vValues[0].c_str(), _T("separator")) == 0)
		{
			menu.AppendSeperator();
		}
		else if (_tcscmp(vValues[0].c_str(), _T("end")) == 0)
		{
			return;
		}
		else
			ERRLOG1(_T("Invalid menu item line: %s"), line.c_str());
	}
}

void CSkinMenu::TrackPopupMenu(int x, int y, CBaseWnd *pWnd, RECT *prcNotOverlap)
{
	CSkinFactory *pSkinFactory = CSkinApp::GetInstance()->GetSkinFactory();
	LIST_ITEMS::iterator	it, itEnd;
	int						nRet;

	itEnd = m_listItems.end();
	for (it = m_listItems.begin(); it != itEnd; ++it)
	{
		Item	&item = *it;
		if (item.nIDCmdIsChecked != UID_INVALID && item.bCanCheck)
		{
			nRet = pSkinFactory->OnDynamicCmd(item.nIDCmdIsChecked, (CSkinWnd *)pWnd);
			CheckItem(item.nMenuID, nRet == ERR_OK);
		}
	}

	CMLMenu::TrackPopupMenu(x, y, pWnd, prcNotOverlap);
}

void CSkinMenu::TrackPopupSubMenu(int x, int y, int nSubMenu, CBaseWnd *pWnd, RECT *prcNotOverlap)
{
	CSkinFactory *pSkinFactory = CSkinApp::GetInstance()->GetSkinFactory();
	LIST_ITEMS::iterator	it, itEnd;
	int						nRet;

	itEnd = m_listItems.end();
	for (it = m_listItems.begin(); it != itEnd; ++it)
	{
		Item	&item = *it;
		if (item.nIDCmdIsChecked != UID_INVALID && item.bCanCheck)
		{
			nRet = pSkinFactory->OnDynamicCmd(item.nIDCmdIsChecked, (CSkinWnd *)pWnd);
			CheckItem(item.nMenuID, nRet == ERR_OK);
		}
	}

	CMLMenu::TrackPopupSubMenu(x, y, nSubMenu, pWnd, prcNotOverlap);
}

int CSkinMenu::FromXML(SXNode *pNodeMenu, int nAppendPos)
{
	if (!pNodeMenu)
		return ERR_NOT_FOUND;

	ASSERT(_tcsicmp(pNodeMenu->strName.c_str(), _T("Menu")) == 0);

	if (!IsValid())
		CreatePopupMenu();

	m_nAppendPosition = nAppendPos;

	SXNode::iterator it, itEnd;
	itEnd = pNodeMenu->listChildren.end();
	for (it = pNodeMenu->listChildren.begin(); it != itEnd; ++it)
	{
		SXNode		*pNode = *it;

		// OK, found the control
		if (_tcsicmp(pNode->strName.c_str(), _T("Item")) == 0)
		{
			Item	item;
			item.FromXML(CSkinApp::GetInstance()->GetSkinFactory(), pNode);

			m_listItems.push_back(item);
			if (item.action == A_APPEND)
			{
				InsertItem(m_nAppendPosition, item.nMenuID,
					_TL(item.strName.c_str()));
				m_nAppendPosition++;
			}
			else if (item.action == A_REMOVE)
			{
				RemoveItem(item.nPos);
				if (item.nPos < m_nAppendPosition)
					m_nAppendPosition--;
			}
			else  if (item.action == A_INSERT_BY_ID)
			{
				InsertItemByID(item.nPos, item.nMenuID,
					_TL(item.strName.c_str()));
			}
		}
	}

	return ERR_OK;
}

int CSkinMenu::GetSubMenuPos(int nSubMenu) const
{
	if (nSubMenu > m_nOrgAppenPos)
		return nSubMenu + m_nAppendPosition - m_nOrgAppenPos;
	else
		return nSubMenu;
}
