// Menu.cpp: implementation of the CMLMenu class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MLMenu.h"

static void PopupGtkMenuPositionFunc(GtkMenu *menu,
			gint *x,
			gint *y,
			gboolean *push_in,
			gpointer user_data)
{
	*x = LOWORD((long)user_data);
	*y = HIWORD((long)user_data);

	*push_in = true;
}

#define MAKELONG(a, b) ((long)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16)) 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMLMenu::CMLMenu()
{
	m_menu = NULL;
	m_pWnd = NULL;
}

CMLMenu::~CMLMenu()
{
	Destroy();
}

void PrintMenuList(CMenuResource::LIST_MENU_ITEMS &listMenu, int &nDeep);

bool CMLMenu::LoadMenu(int nID)
{
	Destroy();

	CMenuResource		*pMenuRes;
	pMenuRes = CResourceMgr::GetMenu(nID);
	if (!pMenuRes)
	{
		DBGLOG1(_T("Can't get menu resource, ID: %d"), nID);
		return false;
	}

	CMenuResource::CopyMenuItems(pMenuRes->GetMenu(), m_menuRes.listItems);
	// int		nDeep  = 0;
	// PrintMenuList(m_menuRes.listItems, nDeep);
	m_menu = CreateSubMenu(m_menuRes.listItems);

	return m_menu != NULL;
}

void CMLMenu::Destroy()
{
	if (m_menu)
	{
		gtk_widget_destroy(m_menu);
		m_menu = NULL;
	}
	for (int i = 0; i < m_vParamlist.size(); i++)
	{
		delete m_vParamlist[i];
	}
	m_vParamlist.clear();
}

void CMLMenu::TrackPopupMenu(int x, int y, CBaseWnd *pWnd)
{
	// printf("TrackPopupMenu this: 0x%X, pWnd: 0x%X", this, pWnd);
	m_pWnd = pWnd;
	gtk_menu_popup(GTK_MENU(m_menu), NULL, NULL, PopupGtkMenuPositionFunc, (gpointer)MAKELONG(x, y), 0, gtk_get_current_event_time());
}

void CMLMenu::EnableItem(int nID, bool bEnable)
{
	CMenuResource::MenuItem		*pItem;
	pItem = m_menuRes.GetItemByID(nID);
	if (pItem)
	{
		gtk_widget_set_sensitive(pItem->widget, bEnable);
	}
}

void CMLMenu::CheckItem(int nID, bool bCheck)
{
	
}

void CMLMenu::CheckRadioItem(int nID, int nStartID, int nEndID)
{

}

bool CMLMenu::CreatePopupMenu()
{
	Destroy();

	m_menu = gtk_menu_new();

	return m_menu != NULL;
}

void CMLMenu::AppendItem(UINT nID, LPCTSTR szText)
{
	GtkWidget			*menuItem;
	MenuClickParam		*param = new MenuClickParam;

	m_vParamlist.push_back(param);
	param->pThis = this;
	param->nID = nID;

	menuItem = gtk_menu_item_new_with_mnemonic(szText);
	g_signal_connect_swapped(G_OBJECT(menuItem), "activate",
		G_CALLBACK(GtkMenuItemClick), (gpointer)param);
	gtk_menu_shell_append(GTK_MENU_SHELL(m_menu), menuItem);
    gtk_widget_show(menuItem);
}

void CMLMenu::InsertItem(int nPos, UINT nID, LPCTSTR szText)
{
	GtkWidget			*menuItem;
	MenuClickParam		*param = new MenuClickParam;

	m_vParamlist.push_back(param);
	param->pThis = this;
	param->nID = nID;

	menuItem = gtk_menu_item_new_with_mnemonic(szText);
	g_signal_connect_swapped(G_OBJECT(menuItem), "activate",
		G_CALLBACK(GtkMenuItemClick), (gpointer)param);
	gtk_menu_shell_insert(GTK_MENU_SHELL(m_menu), menuItem, nPos);
    gtk_widget_show(menuItem);
}

bool CMLMenu::LoadPopupMenu(int nID, int nSubMenu)
{
	DBGLOG2(_T("LoadPopupMenu: %d, %d"), nID, nSubMenu);

	Destroy();

	CMenuResource		*pMenuRes;
	CMenuResource::MenuItem		*pSubMenu;
	pMenuRes = CResourceMgr::GetMenu(nID);
	if (!pMenuRes)
	{
		DBGLOG1(_T("Can't find menu resource: %d"), nID);
		return false;
	}

	pSubMenu = pMenuRes->GetSubMenu(nSubMenu);
	if (!pSubMenu || pSubMenu->type != CMenuResource::MIT_POPUP)
	{
		DBGLOG2(_T("Get submenu: %d of menu resource: %d FAILED"), nSubMenu, nID);
		return false;
	}

	CMenuResource::CopyMenuItems(pSubMenu->listItems, m_menuRes.listItems);
	// int		nDeep  = 0;
	// PrintMenuList(m_menuRes.listItems, nDeep);
	// PrintMenuList(pSubMenu->listItems, nDeep);
	m_menu = CreateSubMenu(m_menuRes.listItems);

	return m_menu != NULL;
}

GtkWidget *CMLMenu::CreateSubMenu(CMenuResource::LIST_MENU_ITEMS &listMenu)
{
	CMenuResource::iterator		it, itEnd;
	itEnd = listMenu.end();
	GtkWidget		*menu;

	menu =  gtk_menu_new();
	for (it = listMenu.begin(); it != itEnd; ++it)
	{
		GtkWidget		*menuItem;
		CMenuResource::MenuItem		*item = *it;
		if (item->type == CMenuResource::MIT_POPUP)
		{
			// DBGLOG1(_T("Menu Popup: %s"), item->strName.c_str());
			GtkWidget			*subMenu;
			menuItem = gtk_menu_item_new_with_mnemonic(item->strName.c_str());
			subMenu = CreateSubMenu(item->listItems);
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuItem), subMenu);
		    gtk_widget_show(menuItem);
		}
		else if (item->type == CMenuResource::MIT_SEPEARTOR)
		{
			menuItem = gtk_separator_menu_item_new();
		    gtk_widget_show(menuItem);
		}
		else
		{
			// DBGLOG1(_T("Menu Item: %s"), item->strName.c_str());
			MenuClickParam		*param = new MenuClickParam;
			m_vParamlist.push_back(param);
			param->pThis = this;
			param->nID = item->nID;
			menuItem = gtk_menu_item_new_with_mnemonic(item->strName.c_str());
			g_signal_connect_swapped(G_OBJECT(menuItem), "activate",
				G_CALLBACK(GtkMenuItemClick), (gpointer)param);
		    gtk_widget_show(menuItem);
		}
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuItem);
		item->widget = menuItem;
	}

	return menu;
}

GtkWidget *CMLMenu::CreateMenuItem(CMenuResource::MenuItem *pItem)
{
	MenuClickParam		*param = new MenuClickParam;
	GtkWidget			*menuItem;

	m_vParamlist.push_back(param);
	param->pThis = this;
	param->nID = pItem->nID;

	menuItem = gtk_menu_item_new_with_mnemonic(pItem->strName.c_str());
	g_signal_connect_swapped(G_OBJECT(menuItem), "activate",
		G_CALLBACK(GtkMenuItemClick), (gpointer)param);
	gtk_widget_show(menuItem);
	pItem->widget = menuItem;

	return menuItem;
}

void CMLMenu::GtkMenuItemClick(gpointer user_data)
{
	ASSERT(user_data);
	MenuClickParam		*param = (MenuClickParam *)user_data;
	param->pThis->m_pWnd->OnCommand(param->nID, 0);
}
