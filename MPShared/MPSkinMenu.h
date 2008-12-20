// MPSkinMenu.h: interface for the CMPSkinMenu class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPSKINMENU_H__EEA43535_74D6_4767_9640_38376F9FEE6E__INCLUDED_)
#define AFX_MPSKINMENU_H__EEA43535_74D6_4767_9640_38376F9FEE6E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "SkinMenu.h"

struct MenuItemCheck
{
	UINT		nID;
	LPCTSTR		szSection;
	LPCTSTR		szName;
	LPCTSTR		szCheckValue;
	HMENU		hMenu;
};

struct MenuRadioGroupIDName
{
	UINT		nID;
	LPCTSTR		szCheckValue;
};

struct MenuRadioGroup
{
	LPCTSTR		szSection;
	LPCTSTR		szName;
	MenuRadioGroupIDName	*idNames;
	HMENU		hMenu;
	UINT		nRadioStartID, nRadioEndID;
};

class CMenuAutoCheck
{
public:
	void InitProcSubMenu(HMENU hMenu);

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

};

class CMPSkinMenu : public CSkinMenu  
{
public:
	CMPSkinMenu();
	virtual ~CMPSkinMenu();

	virtual void TrackPopupMenu(int x, int y, CBaseWnd *pWnd);

	virtual void OnLoadMenu();

	void AddUICheckStatusIf(IUICheckStatus *pIfUICheckStatus) { m_autoCheckMenu.AddUICheckStatusIf(pIfUICheckStatus); }

protected:
	CMenuAutoCheck		m_autoCheckMenu;

};

#endif // !defined(AFX_MPSKINMENU_H__EEA43535_74D6_4767_9640_38376F9FEE6E__INCLUDED_)
