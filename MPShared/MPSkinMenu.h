// MPSkinMenu.h: interface for the CMPSkinMenu class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPSKINMENU_H__EEA43535_74D6_4767_9640_38376F9FEE6E__INCLUDED_)
#define AFX_MPSKINMENU_H__EEA43535_74D6_4767_9640_38376F9FEE6E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "../Skin/SkinMenu.h"

int CmdIdToEncoding(int nCmdId);

int EncodingIdToCmd(CHAR_ENCODING nEncodingId);

bool OnCommandSkin(int nCmdId);

struct MenuItemCheck
{
	UINT		nID;
	LPCTSTR		szSection;
	LPCTSTR		szName;
	LPCTSTR		szCheckValue;
	CMLMenu		menu;
};

struct MenuRadioGroupIDName
{
	UINT		nID;
	LPCTSTR		szCheckValue;
};

struct MenuRadioGroup
{
	UINT		nRadioStartID, nRadioEndID;
	MenuRadioGroupIDName	*idNames;
	CMLMenu		menu;
};

class CMenuAutoCheck
{
public:
	CMenuAutoCheck() {
		m_nInsertSkinsPos = 0;
		m_nInsertNewItemPos = 0;
	}

	void InitProcSubMenu(CMLMenu &menu);

	void UpdateMenuCheckStatus();
	void AddUICheckStatusIf(IUICheckStatus *pIfUICheckStatus);

protected:
	bool GetChecked(UINT nID);
	bool GetRadioChecked(MenuRadioGroupIDName *idNames, UINT &nIDChecked);

protected:
	typedef list<MenuItemCheck	>	LIST_MENU_ITEMS_CHECK;
	typedef list<MenuRadioGroup	>	LIST_MENU_ITEMS_RADIO;
	typedef list<IUICheckStatus *>	LIST_UI_CHECK_STATUS;

	LIST_MENU_ITEMS_CHECK			m_vMenuItemsCheck;
	LIST_MENU_ITEMS_RADIO			m_vMenuItemsRadio;
	LIST_UI_CHECK_STATUS			m_listUICheckStatusIf;

public:
	CMLMenu							m_hInsertSkinsMenu;
	int								m_nInsertSkinsPos;

	CMLMenu							m_hInsertNewItemMenu;
	int								m_nInsertNewItemPos;

};

class CMPSkinMenu : public CSkinMenu  
{
public:
	CMPSkinMenu();
	virtual ~CMPSkinMenu();

	virtual void TrackPopupMenu(int x, int y, CBaseWnd *pWnd, RECT *prcNotOverlap = NULL);
	virtual void TrackPopupSubMenu(int x, int y, int nSubMenu, CBaseWnd *pWnd, RECT *prcNotOverlap = NULL);

	virtual void OnLoadMenu();

	void AddUICheckStatusIf(IUICheckStatus *pIfUICheckStatus) { m_autoCheckMenu.AddUICheckStatusIf(pIfUICheckStatus); }
	
	void UpdateMenuStatus();

protected:	
	virtual bool GetShortcutKey(int nMenuID, tstring &strShortcut);

	void InsertSkinMenu(CMLMenu &menu, int nPosStart);

protected:
	CMenuAutoCheck		m_autoCheckMenu;

};

class CLyrEditorMenu : public CMPSkinMenu
{
public:
	virtual void TrackPopupSubMenu(int x, int y, int nSubMenu, CBaseWnd *pWnd, RECT *prcNotOverlap = NULL);

};

#endif // !defined(AFX_MPSKINMENU_H__EEA43535_74D6_4767_9640_38376F9FEE6E__INCLUDED_)
