
#pragma once

enum WIDGET_TYPE
{
	WT_UNKNOWN,
	WT_LIST_CTRL,
	WT_TREE_VIEW,
	WT_TAB_CTRL,
};

enum WIDGET_NOTIFY_CODE
{
	WN_GEN_DBL_CLICK	= 0x0001,

	// list view notify code
	WN_LV_UNSELECTED	= 0x0101,
	WN_LV_SELECTED,

	// tree view notify code
	WN_TV_UNSELECTED	= 0x0201,
	WN_TV_SELECTED,
	WN_TV_EXPANDING,
	WN_TV_EXPANDED,

	WN_TC_SELCHANGE		= 0x0301,
};

struct WIDGET_NOTIFY_INFO
{
	int				nCtrlID;
	uint16			widgetType;		// WIDGET_TYPE
	uint16			nCode;			// WIDGET_NOTIFY_CODE
};

struct WIDGET_LISTVIEW_NOTIFY_INFO : public WIDGET_NOTIFY_INFO
{
	int				nItem;
	int				nSubItem;
};

// tree view item handle
#ifdef _WIN32
typedef		HTREEITEM		TreeItemIter;
#endif

#ifdef _LINUX_GTK2
typedef		GtkTreeIter		TreeItemIter;
#endif

struct WIDGET_TREEVIEW_NOTIFY_INFO : public WIDGET_NOTIFY_INFO
{
	TreeItemIter		iter;
};
