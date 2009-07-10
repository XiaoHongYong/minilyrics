// Menu.h: interface for the CMLMenu class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENU_H__4207B3F1_5148_479E_9F72_167E76626985__INCLUDED_)
#define AFX_MENU_H__4207B3F1_5148_479E_9F72_167E76626985__INCLUDED_

#include "../ResourceMgr.h"
#include "BaseWnd.h"

class CMLMenu  
{
public:
	CMLMenu();
	virtual ~CMLMenu();

	bool IsValid() const { return m_menu != NULL; }

	bool LoadMenu(int nID);

	void Destroy();

	virtual void TrackPopupMenu(int x, int y, CBaseWnd *pWnd);

	virtual bool OnCommand(CBaseWnd *pWnd, int nCmdID) { return false; }

	void EnableItem(int nID, bool bEnable);
	void CheckItem(int nID, bool bCheck);
	void CheckRadioItem(int nID, int nStartID, int nEndID);

	//
	// Create menu...
	//
	bool CreatePopupMenu();

	void AppendItem(UINT nID, LPCTSTR szText);
	void InsertItem(int nPos, UINT nID, LPCTSTR szText);
	void RemoveItem(int nPos);

	bool GetMenuItemText(UINT nItem, tstring &strText, bool bByPosition);

	bool GetMenuItemInfo(UINT nItem, tstring &strText, UINT &nID, bool &bSubMenu, bool bByPosition);

public:
	bool LoadPopupMenu(int nID, int nSubMenu);

	virtual void OnLoadMenu() { }

protected:
	GtkWidget *CreateSubMenu(CMenuResource::LIST_MENU_ITEMS &listMenu);
	GtkWidget *CreateMenuItem(CMenuResource::MenuItem *pItem);

	static void GtkMenuItemClick(gpointer user_data);

	struct MenuClickParam
	{
		CMLMenu		*pThis;
		int			nID;
	};

protected:
	GtkWidget		*m_menu;
	CBaseWnd		*m_pWnd;
	CMenuResource::MenuItem		m_menuRes;
	vector<MenuClickParam*>		m_vParamlist;

};

#endif // !defined(AFX_MENU_H__4207B3F1_5148_479E_9F72_167E76626985__INCLUDED_)
