#include "StdAfx.h"
#include "WidgetListCtrl.h"

CWidgetListCtrl::CWidgetListCtrl()
{
	m_model = NULL;
	m_nColCount = 0;
}

CWidgetListCtrl::~CWidgetListCtrl()
{

}

BOOL CWidgetListCtrl::LoadColumnWidth(LPCTSTR szKeyName)
{
	return false;
}

BOOL CWidgetListCtrl::SaveColumnWidth(LPCTSTR szKeyName)
{
	return false;
}

bool CWidgetListCtrl::Create(CBaseWnd *pWndParent, int nID)
{
	GtkTreeSelection	*selection;

	m_pWndParent = pWndParent;
	m_nID = nID;

	m_window = gtk_tree_view_new();

//	printf("Create CWidgetListCtrl");

/*	InsertColumn(0, "col1");
	InsertColumn(1, "col2");

	int	iter = AppendItem("xxxx");

	iter = AppendItem("xxxx");
	iter = AppendItem("xxxx");
	iter = AppendItem("xxxx2");

	SetItemText(iter, 1, "colss2");*/
	gtk_signal_connect(GTK_OBJECT(m_window), "row-activated",
		GTK_SIGNAL_FUNC(CBRowActivated), this);

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_window));

	gtk_tree_selection_set_select_function(selection, CBSelect, this, NULL);

	return true;
}

BOOL CWidgetListCtrl::Attach(CBaseWnd *pWndParent, int nCtrlID)
{
	CWidgetCtrlBase::Attach(pWndParent, nCtrlID);

	m_model = gtk_tree_view_get_model(GTK_TREE_VIEW(m_window));
	ASSERT(m_window);
	// ASSERT(m_model);

	return m_window != NULL;
}

int CWidgetListCtrl::GetItemCount() const
{
//	ASSERT(m_model);
	if (!m_model)
		return 0;

	return gtk_tree_model_iter_n_children(m_model, NULL);
}

int CWidgetListCtrl::GetColumnWidth(int nCol) const
{
	GtkTreeViewColumn	*pCol;

	pCol = gtk_tree_view_get_column(GTK_TREE_VIEW(m_window), nCol);
	if (!pCol)
		return 0;

	return gtk_tree_view_column_get_width(pCol);
}

BOOL CWidgetListCtrl::SetColumnWidth(int nCol, int cx)
{
	GtkTreeViewColumn	*pCol;

	pCol = gtk_tree_view_get_column(GTK_TREE_VIEW(m_window), nCol);
	if (!pCol)
		return false;

	gtk_tree_view_column_set_fixed_width(pCol, cx);

	return true;
}

UINT CWidgetListCtrl::GetSelectedCount() const
{
	GtkTreeSelection	*pSelection;

	pSelection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_window));
	if (!pSelection)
		return 0;

	return gtk_tree_selection_count_selected_rows(pSelection);
}

int  CWidgetListCtrl::GetNextSelectedItem(int nPos)
{
//	ASSERT(m_model);
	if (!m_model)
		return -1;

	GtkTreeSelection	*pSelection;
	GList				*pList;
	int					n = -1;

	pSelection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_window));
	if (!pSelection)
		return -1;

	nPos++;
	pList = gtk_tree_selection_get_selected_rows(pSelection, &m_model);
	if (pList)
	{
		while (pList)
		{
			n = gtk_tree_path_get_indices((GtkTreePath *)pList->data)[0];
			if (n >= nPos)
			{
				g_list_free(pList);
				return n;
			}
		}
		g_list_free(pList);
	}

	return -1;
}

BOOL CWidgetListCtrl::GetCheck(int nItem) const
{
	return 0;
};

BOOL CWidgetListCtrl::SetCheck(int nItem, BOOL fCheck)
{
	return 0;
}

BOOL CWidgetListCtrl::SetItemSelectionState(int nItem, BOOL bSelected)
{
	GtkTreeSelection	*pSelection;

	pSelection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_window));
	if (pSelection)
	{
		GtkTreePath		*path;

		path = GtkTreePathFromIndex(nItem);
		if (bSelected)
			gtk_tree_selection_select_path(pSelection, path);
		else
			gtk_tree_selection_unselect_path(pSelection, path);
		gtk_tree_path_free(path);
	}

	return TRUE;
}

COLORREF CWidgetListCtrl::GetBkColor() const
{
	return 0;
}

BOOL CWidgetListCtrl::SetBkColor(COLORREF cr)
{
	return 0;
}

void CWidgetListCtrl::SetExtendedStyle(DWORD dwNewStyle)
{
}

void CWidgetListCtrl::GetItemText(int nItem, int nSubItem, LPTSTR lpszText, int nLen) const
{
	ASSERT(m_model);
	if (!m_model)
		return;

	GValue		val = { 0, };
	GtkTreeIter	iter;

	if (!GtkTreeIterFromIndex(nItem, iter))
		return;

	gtk_tree_model_get_value(m_model, &iter, nSubItem, &val);
	ASSERT(G_VALUE_HOLDS_STRING(&val));

	if (G_VALUE_HOLDS_STRING(&val))
	{
		_tcscpy_safe(lpszText, nLen, (gchar*)g_value_get_string(&val));
	}
	else
		EmptyStr(lpszText);
}

void CWidgetListCtrl::GetItemText(int nItem, int nSubItem, tstring &strText) const
{
	ASSERT(m_model);
	if (!m_model)
		return;


	GValue		val = { 0, };
	GtkTreeIter	iter;

	if (!GtkTreeIterFromIndex(nItem, iter))
		return;

	gtk_tree_model_get_value(m_model, &iter, nSubItem, &val);
	ASSERT(G_VALUE_HOLDS_STRING(&val));

	if (G_VALUE_HOLDS_STRING(&val))
		strText = (gchar*)g_value_get_string(&val);
	else
		strText.resize(0);
}

BOOL CWidgetListCtrl::SetItemText(int nItem, int nSubItem, LPCTSTR lpszText)
{
	ASSERT(m_model);
	if (!m_model)
		return false;

	GtkTreeIter	iter;

	if (!GtkTreeIterFromIndex(nItem, iter))
		return false;

	gtk_list_store_set(GTK_LIST_STORE(m_model), &iter, nSubItem, lpszText, -1);

	return TRUE;
}
// 
// BOOL CWidgetListCtrl::SetItem(int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem,
// 	int nImage, UINT nState, UINT nStateMask, LPARAM lParam)
// {
// 	return 0;
// }
// 
// BOOL CWidgetListCtrl::SetItemData(int nItem, DWORD dwData)
// {
// 	return 0;
// }
// 
// DWORD CWidgetListCtrl::GetItemData(int nItem) const
// {
// 	return 0;
// }
// 
int CWidgetListCtrl::InsertColumn(int nCol, LPCTSTR lpszColumnHeading,
	int nFormat, int nWidth, int nSubItem)
{
	GtkCellRenderer     *renderer;

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(m_window),
										nCol,
										lpszColumnHeading,
										renderer,
										"text",
										nCol,
										NULL);

	m_nColCount++;

	return ERR_OK;
}

BOOL CWidgetListCtrl::DeleteColumn(int nCol)
{
	ASSERT(m_window);
	GtkTreeViewColumn		*pCol;

	pCol = gtk_tree_view_get_column(GTK_TREE_VIEW(m_window), nCol);
	if (!pCol)
		return FALSE;

	gtk_tree_view_remove_column(GTK_TREE_VIEW(m_window), pCol);

	return true;
}

int  CWidgetListCtrl::AppendItem(LPCTSTR lpszItem)
{
	if (!m_model)
		CreateModel();

	ASSERT(m_model);
	if (!m_model)
		return -1;

	GtkTreeIter		iter;

	gtk_list_store_append(GTK_LIST_STORE(m_model), &iter);

	gtk_list_store_set(GTK_LIST_STORE(m_model), &iter, 0, lpszItem, -1);

	return IndexFromGtkTreeIter(iter);
}

int  CWidgetListCtrl::InsertItem(int nItem, LPCTSTR lpszItem)
{
	if (!m_model)
		CreateModel();

	ASSERT(m_model);
	if (!m_model)
		return -1;

	GtkTreeIter		iter;
	GtkTreeIter		iterInsBefore;

	if (!GtkTreeIterFromIndex(nItem, iterInsBefore))
		return AppendItem(lpszItem);

	gtk_list_store_insert_before(GTK_LIST_STORE(m_model), &iter, &iterInsBefore);

	gtk_list_store_set(GTK_LIST_STORE(m_model), &iter, 0, lpszItem, -1);

	return IndexFromGtkTreeIter(iter);
}

int  CWidgetListCtrl::InsertItem(int nItem, LPCTSTR lpszItem, int nImage)
{
	return InsertItem(nItem, lpszItem);
}

BOOL CWidgetListCtrl::DeleteItem(int nItem)
{
	ASSERT(m_model);
	if (!m_model)
		return false;

	GtkTreeIter		iter;

	if (!GtkTreeIterFromIndex(nItem, iter))
		return false;

	return gtk_list_store_remove(GTK_LIST_STORE(m_model), &iter);
}

BOOL CWidgetListCtrl::DeleteAllItems()
{
	// ASSERT(m_model);
	if (!m_model)
		return false;

	gtk_list_store_clear(GTK_LIST_STORE(m_model));

	return true;
}

void CWidgetListCtrl::SetSingleSelection(bool bSigSel)
{
	GtkTreeSelection	*pSelection;

	pSelection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_window));
	if (pSelection)
		gtk_tree_selection_set_mode(pSelection, bSigSel ? GTK_SELECTION_SINGLE : GTK_SELECTION_MULTIPLE);
}

void CWidgetListCtrl::SetHeaderVisible(bool bVisible)
{
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(m_window), bVisible);
}

void CWidgetListCtrl::CreateModel()
{
	ASSERT(m_model == NULL);

	GtkListStore  *store;
	GType			type[64];

	ASSERT(m_nColCount > 0);
	ASSERT(CountOf(type) >= m_nColCount);
	for (int i = 0; i < m_nColCount; i++)
		type[i] = G_TYPE_STRING;

	store = gtk_list_store_newv(m_nColCount, type);
	ASSERT(store);

	m_model = GTK_TREE_MODEL(store);
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_window), m_model);
}

GtkTreePath *CWidgetListCtrl::GtkTreePathFromIndex(int nIndex) const
{
	GtkTreePath		*path;

	path = gtk_tree_path_new();

	gtk_tree_path_append_index(path, nIndex);

	return path;
}

bool CWidgetListCtrl::GtkTreeIterFromIndex(int nIndex, GtkTreeIter &iter) const
{
	GtkTreePath		*path;
	bool			bRet;
	
	path = GtkTreePathFromIndex(nIndex);

	bRet = gtk_tree_model_get_iter(m_model, &iter, path);

	gtk_tree_path_free(path);

	return bRet;
}

int CWidgetListCtrl::IndexFromGtkTreeIter(GtkTreeIter &iter) const
{
	GtkTreePath		*path;
	int				n;

	path = gtk_tree_model_get_path(m_model, &iter);
	if (path)
	{
		n = gtk_tree_path_get_indices(path)[0];
		gtk_tree_path_free(path);
	}
	else
		n = -1;

	return n;
}

gboolean CWidgetListCtrl::OnSelect(GtkTreePath *path, gboolean path_currently_selected)
{
	WIDGET_LISTVIEW_NOTIFY_INFO	info;

	info.nCtrlID = m_nID;
	info.widgetType = WT_LIST_CTRL;
	info.nItem = gtk_tree_path_get_indices(path)[0];
	info.nSubItem = 0;

	if (path_currently_selected)
		info.nCode = WN_LV_SELECTED;
	else
		info.nCode = WN_LV_UNSELECTED;

	m_pWndParent->OnNotify(&info);
}

void CWidgetListCtrl::OnRowActivate(GtkTreePath *path)
{
	WIDGET_LISTVIEW_NOTIFY_INFO	info;

	info.nCtrlID = m_nID;
	info.widgetType = WT_LIST_CTRL;
	info.nItem = gtk_tree_path_get_indices(path)[0];
	info.nSubItem = 0;

	info.nCode = WN_GEN_DBL_CLICK;
	m_pWndParent->OnNotify(&info);
}

void CWidgetListCtrl::CBRowActivated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer userdata)
{
	CWidgetListCtrl		*pThis = (CWidgetListCtrl *)userdata;
	pThis->OnRowActivate(path);
}

gboolean CWidgetListCtrl::CBSelect(GtkTreeSelection *selection,
		GtkTreeModel     *model,
		GtkTreePath      *path,
		gboolean          path_currently_selected,
		gpointer          userdata)
{
	CWidgetListCtrl		*pThis = (CWidgetListCtrl *)userdata;

	pThis->OnSelect(path, path_currently_selected);

	return TRUE; /* allow selection state to change */
}
