#include "../StdAfx.h"
#include "WidgetTreeCtrl.h"

enum
{
	COL_NAME			= 0,
	COL_USER_DATA		= 1,
	COL_COUNT			= 2
};

CWidgetTreeCtrl::CWidgetTreeCtrl()
{
	m_model = NULL;
}

CWidgetTreeCtrl::~CWidgetTreeCtrl()
{

}

bool CWidgetTreeCtrl::Create(CBaseWnd *pWndParent, int nID)
{
	m_pWndParent = pWndParent;
	m_nID = nID;

	m_window = gtk_tree_view_new();

	CreateModel();

	SetHeaderVisible(false);

	SetSingleSelection(true);

	GtkCellRenderer     *renderer;
	GtkTreeSelection	*selection;

	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(m_window),
										COL_NAME,
										"",
										renderer,
										"text",
										COL_NAME,
										NULL);

	gtk_signal_connect(GTK_OBJECT(m_window), "row-activated",
		GTK_SIGNAL_FUNC(CBRowActivated), this);

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_window));

	gtk_tree_selection_set_select_function(selection, CBSelect, this, NULL);

// 	TreeItemIter	iter;
// 	iter = AppendItem(NULL, "xxxx2", 0);
// 	InsertItem(&iter, NULL, "child1");

	return true;
}


BOOL CWidgetTreeCtrl::Attach(CBaseWnd *pWndParent, int nCtrlID)
{
	CWidgetCtrlBase::Attach(pWndParent, nCtrlID);

	m_model = gtk_tree_view_get_model(GTK_TREE_VIEW(m_window));
	ASSERT(m_window);
	ASSERT(m_model);

	return m_window != NULL;
}

bool CWidgetTreeCtrl::GetSelected(TreeItemIter &iter)
{
	GtkTreeSelection	*pSelection;

	pSelection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_window));
	if (!pSelection)
		return false;

	if (gtk_tree_selection_get_selected(pSelection, &m_model, &iter))
		return true;

	return false;
}

bool CWidgetTreeCtrl::SetSelected(TreeItemIter &iter)
{
	GtkTreeSelection	*pSelection;

	pSelection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_window));
	if (pSelection)
	{
		GtkTreePath		*path;

		path = gtk_tree_model_get_path(m_model, &iter);
		if (path)
		{
			gtk_tree_selection_select_path(pSelection, path);
			gtk_tree_path_free(path);

			return true;
		}
	}

	return false;
}

void CWidgetTreeCtrl::GetItemText(TreeItemIter &nItem, tstring &strText) const
{
	ASSERT(m_model);
	if (!m_model)
		return;

	GValue		val = { 0, };

	gtk_tree_model_get_value(m_model, &nItem, COL_NAME, &val);
	ASSERT(G_VALUE_HOLDS_STRING(&val));

	if (G_VALUE_HOLDS_STRING(&val))
		strText = g_value_get_string(&val);
	else
		strText.resize(0);
}

BOOL CWidgetTreeCtrl::SetItemText(TreeItemIter &nItem, LPCTSTR lpszText)
{
	gtk_tree_store_set(GTK_TREE_STORE(m_model), &nItem, COL_NAME, lpszText, -1);
	return true;
}

UINT CWidgetTreeCtrl::GetItemData(TreeItemIter &nItem) const
{
	ASSERT(m_model);
	if (!m_model)
		return 0;

	GValue		val = { 0, };

	gtk_tree_model_get_value(m_model, &nItem, COL_USER_DATA, &val);
	ASSERT(G_VALUE_HOLDS_UINT(&val));

	if (G_VALUE_HOLDS_UINT(&val))
		return g_value_get_uint(&val);
	else
		return 0;
}

BOOL CWidgetTreeCtrl::SetItemData(TreeItemIter &nItem, UINT dwData)
{
	gtk_tree_store_set(GTK_TREE_STORE(m_model), &nItem, COL_USER_DATA, dwData, -1);
	return true;
}

// if parent == NULL, the item will be appended at the top level.
TreeItemIter CWidgetTreeCtrl::AppendItem(TreeItemIter *parent, LPCTSTR lpszItem, UINT nUserData)
{
	ASSERT(m_model);

	TreeItemIter	iter;

	gtk_tree_store_append(GTK_TREE_STORE(m_model), &iter, parent);

	gtk_tree_store_set(GTK_TREE_STORE(m_model), &iter, COL_NAME, lpszItem, -1);
	gtk_tree_store_set(GTK_TREE_STORE(m_model), &iter, COL_USER_DATA, nUserData, -1);

	return iter;
}

TreeItemIter CWidgetTreeCtrl::InsertItem(TreeItemIter *parent, TreeItemIter *iterInsertBefore, LPCTSTR lpszItem, UINT nUserData)
{
	ASSERT(m_model);

	TreeItemIter	iter;

	gtk_tree_store_insert_before(GTK_TREE_STORE(m_model), &iter, parent, iterInsertBefore);

	gtk_tree_store_set(GTK_TREE_STORE(m_model), &iter, COL_NAME, lpszItem, -1);
	gtk_tree_store_set(GTK_TREE_STORE(m_model), &iter, COL_USER_DATA, nUserData, -1);

	return iter;
}

BOOL CWidgetTreeCtrl::DeleteItem(TreeItemIter &nItem)
{
	return gtk_tree_store_remove(GTK_TREE_STORE(m_model), &nItem);
}

BOOL CWidgetTreeCtrl::DeleteAllItems()
{
	gtk_tree_store_clear(GTK_TREE_STORE(m_model));
	return true;
}

bool CWidgetTreeCtrl::GetFirstChild(TreeItemIter *parent, TreeItemIter &child)
{
	if (parent)
		return gtk_tree_model_get_iter_first(m_model, &child);
	else
		return gtk_tree_model_iter_children(m_model, &child, parent);
}

bool CWidgetTreeCtrl::NextSibling(TreeItemIter &item)
{
	return gtk_tree_model_iter_next(m_model, &item);
}

void CWidgetTreeCtrl::Expand(TreeItemIter &item, bool bExpand)
{
	GtkTreePath		*path;
	path = gtk_tree_model_get_path(m_model, &item);
	if (bExpand)
		gtk_tree_view_expand_row(GTK_TREE_VIEW(m_window), path, false);
	else
		gtk_tree_view_collapse_row(GTK_TREE_VIEW(m_window), path);
	gtk_tree_path_free(path);
}

void CWidgetTreeCtrl::ExpandAll(bool bExpand)
{
	if (bExpand)
		gtk_tree_view_expand_all(GTK_TREE_VIEW(m_window));
	else
		gtk_tree_view_collapse_all(GTK_TREE_VIEW(m_window));
}

void CWidgetTreeCtrl::SetSingleSelection(bool bSigSel)
{
	GtkTreeSelection	*pSelection;

	pSelection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_window));
	if (pSelection)
		gtk_tree_selection_set_mode(pSelection, bSigSel ? GTK_SELECTION_SINGLE : GTK_SELECTION_MULTIPLE);
}

void CWidgetTreeCtrl::SetHeaderVisible(bool bVisible)
{
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(m_window), bVisible);
}

void CWidgetTreeCtrl::CreateModel()
{
	ASSERT(m_model == NULL);

	GtkTreeStore	*store;
	GType			type[COL_COUNT];

	type[COL_NAME] = G_TYPE_STRING;
	type[COL_USER_DATA] = G_TYPE_UINT;

	store = gtk_tree_store_newv(COL_COUNT, type);
	ASSERT(store);

	m_model = GTK_TREE_MODEL(store);
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_window), m_model);
}

gboolean CWidgetTreeCtrl::OnSelect(GtkTreePath *path, gboolean path_currently_selected)
{
	WIDGET_TREEVIEW_NOTIFY_INFO	info;

	info.nCtrlID = m_nID;
	info.widgetType = WT_TREE_VIEW;
	gtk_tree_model_get_iter(m_model, &info.iter, path);

	if (path_currently_selected)
		info.nCode = WN_TV_UNSELECTED;
	else
		info.nCode = WN_TV_SELECTED;

	m_pWndParent->OnNotify(&info);
}

void CWidgetTreeCtrl::CBRowActivated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer userdata)
{
	CWidgetTreeCtrl		*pThis = (CWidgetTreeCtrl *)userdata;
	WIDGET_TREEVIEW_NOTIFY_INFO	info;

	info.nCtrlID = pThis->m_nID;
	info.widgetType = WT_TREE_VIEW;

	info.nCode = WN_GEN_DBL_CLICK;
	gtk_tree_model_get_iter(pThis->m_model, &info.iter, path);
	pThis->m_pWndParent->OnNotify(&info);
}

gboolean CWidgetTreeCtrl::CBSelect(GtkTreeSelection *selection,
		GtkTreeModel     *model,
		GtkTreePath      *path,
		gboolean          path_currently_selected,
		gpointer          userdata)
{
	CWidgetTreeCtrl		*pThis = (CWidgetTreeCtrl *)userdata;

	pThis->OnSelect(path, path_currently_selected);

	return TRUE; /* allow selection state to change */
}
