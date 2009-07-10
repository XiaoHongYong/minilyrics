#include "StdAfx.h"
#include "WidgetListBox.h"

// void CWidgetListBox::Attach(CBaseDialogEx *pWndParent, int nID)
// {
// 	
// }
// 

bool CWidgetListBox::Create(CBaseWnd *pWndParent, int nID)
{
	if (!CWidgetListCtrl::Create(pWndParent, nID))
		return false;

	SetHeaderVisible(false);

	SetSingleSelection(true);

	InsertColumn(0, _T(""));

	CreateModel();

	return true;
}

int CWidgetListBox::GetCount() const
{
	return GetItemCount();
}

int CWidgetListBox::GetCurSel() 
{
	GtkTreeSelection	*pSelection;

	pSelection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_window));
	if (!pSelection)
		return -1;

	GtkTreeIter		iter;

	if (gtk_tree_selection_get_selected(pSelection, &m_model, &iter))
		return IndexFromGtkTreeIter(iter);

	return -1;
}

int CWidgetListBox::SetCurSel(int nSelect)
{
	GtkTreeSelection	*pSelection;

	pSelection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_window));
	if (!pSelection)
		return -1;

	if (nSelect == -1)
	{
		gtk_tree_selection_unselect_all(pSelection);
		return -1;
	}

	GtkTreePath		*path;

	path = GtkTreePathFromIndex(nSelect);

	gtk_tree_selection_select_path(pSelection, path);

	gtk_tree_path_free(path);

	return nSelect;
}

// retrieve from a list box the width, in pixels, by which the list box can 
// be scrolled horizontally (the scrollable width) if the list box has a 
// horizontal scroll bar. 
int CWidgetListBox::GetHorizontalExtent() const
{
	return 0;
}

void CWidgetListBox::SetHorizontalExtent(int cxExtent)
{

}

int CWidgetListBox::GetSelCount() const
{
	GtkTreeSelection	*pSelection;

	pSelection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m_window));
	if (!pSelection)
		return 0;

	return gtk_tree_selection_count_selected_rows(pSelection);
}

void CWidgetListBox::GetText(int nIndex, LPTSTR lpszBuffer) const
{
	GetItemText(nIndex, 0, lpszBuffer, 512);
}

void CWidgetListBox::GetText(int nIndex, tstring &str) const
{
	GetItemText(nIndex, 0, str);
}

int CWidgetListBox::GetTextLen(int nIndex) const
{
	tstring		str;

	GetText(nIndex, str);

	return str.size();
}

int CWidgetListBox::FindStringExact(int nIndexStart, LPCTSTR lpszFind) const
{
	GtkTreeIter		iter;
	int				n = 0;

	nIndexStart++;

	if (!GtkTreeIterFromIndex(nIndexStart, iter))
		return -1;

	do
	{
		GValue		val = { 0, };

		gtk_tree_model_get_value(m_model, &iter, 0, &val);
		ASSERT(G_VALUE_HOLDS_STRING(&val));

		if (G_VALUE_HOLDS_STRING(&val))
		{
			if (_tcsicmp((gchar*)g_value_get_string(&val), lpszFind) == 0)
				return n;
		}
		n++;
	}
	while (gtk_tree_model_iter_next(m_model, &iter));

	return -1;
}

int CWidgetListBox::AddString(LPCTSTR lpszItem)
{
	return AppendItem(lpszItem);
}

int CWidgetListBox::DeleteString(UINT nIndex)
{
	GtkTreeIter		iter;
	GtkTreePath		*path;
	int				n;

	path = GtkTreePathFromIndex(nIndex);

	if (gtk_tree_model_get_iter(m_model, &iter, path))
	{
		gtk_list_store_remove(GTK_LIST_STORE(m_model), &iter);

		n = GetItemCount();
	}
	else
		n = -1;

	gtk_tree_path_free(path);

	return n;
}

int CWidgetListBox::InsertString(int nIndex, LPCTSTR lpszItem)
{
	return InsertItem(nIndex, lpszItem);
}

void CWidgetListBox::ResetContent()
{
	DeleteAllItems();
}

int CWidgetListBox::FindString(int nStartAfter, LPCTSTR lpszItem) const
{
	GtkTreeIter		iter;
	int				n = 0;
	int				nLenComp = _tcslen(lpszItem);

	nStartAfter++;

	if (!GtkTreeIterFromIndex(nStartAfter, iter))
		return -1;

	do
	{
		GValue		val = { 0, };

		gtk_tree_model_get_value(m_model, &iter, 0, &val);
		ASSERT(G_VALUE_HOLDS_STRING(&val));

		if (G_VALUE_HOLDS_STRING(&val))
		{
			if (_tcsnicmp((gchar*)g_value_get_string(&val), lpszItem, nLenComp) == 0)
				return n;
		}
		n++;
	}
	while (gtk_tree_model_iter_next(m_model, &iter));

	return -1;
}

int CWidgetListBox::SelectString(int nStartAfter, LPCTSTR lpszItem)
{
	int			n;

	n = FindStringExact(nStartAfter, lpszItem);
	if (n == -1)
		return -1;

	return SetCurSel(n);
}

void CWidgetListBox::OnRowActivate(GtkTreePath *path)
{
	m_pWndParent->OnCommand(m_nID, LBN_DBLCLK);
}

gboolean CWidgetListBox::OnSelect(GtkTreePath *path, gboolean path_currently_selected)
{
	// DBGLOG0(_T("CWidgetListBox::LBN_SELCHANGE"));
	if (path_currently_selected)
		m_pWndParent->OnCommand(m_nID, LBN_SELCHANGE);
}
