// SkinMenu.h: interface for the CSkinMenu class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINMENU_H__66CE8E34_92F0_41E0_996C_AE1170E3AEE9__INCLUDED_)
#define AFX_SKINMENU_H__66CE8E34_92F0_41E0_996C_AE1170E3AEE9__INCLUDED_

#pragma once

class CSkinMenu : public CMLMenu  
{
public:
	enum ItemType
	{
		IT_ITEM,
		IT_POPUP,
		IT_SEPERATOR,
	};

	enum Action
	{
		A_UNKNOWN,
		A_INSERT_BY_ID,
		A_APPEND,
		A_REMOVE,
	};

	struct Item
	{
		int				nMenuID;
		int				nPos;
		ItemType		type;
		Action			action;
		bool			bCanCheck;
		tstring			strName;
		int				nIDCmd;
		int				nIDCmdIsChecked;

		int FromXML(CSkinFactory *pSkinFactory, SXNode *pNode);
	};

	CSkinMenu();
	virtual ~CSkinMenu();

	int LoadMenu(int nID);

	void TrackPopupMenu(int x, int y, CBaseWnd *pWnd, RECT *prcNotOverlap = NULL);

	void TrackPopupSubMenu(int x, int y, int nSubMenu, CBaseWnd *pWnd, RECT *prcNotOverlap = NULL);

	virtual int FromXML(SXNode *pNodeMenu, int nAppendPos);

	virtual bool GetShortcutKey(int nMenuID, tstring &strShortcut) { return false; }

	virtual int GetOrgAppendPos() { return m_nOrgAppenPos; }

	virtual void SetOrgAppendPos(int nPos) { m_nOrgAppenPos = nPos; }

	int GetSubMenuPos(int nSubMenu) const;

protected:
	void LoadMenu(CTextFile &file, CMLMenu &menu);

protected:
	typedef list<Item>		LIST_ITEMS;

	int				m_nOrgAppenPos;
	int				m_nAppendPosition;
	LIST_ITEMS		m_listItems;

};

#endif // !defined(AFX_SKINMENU_H__66CE8E34_92F0_41E0_996C_AE1170E3AEE9__INCLUDED_)
