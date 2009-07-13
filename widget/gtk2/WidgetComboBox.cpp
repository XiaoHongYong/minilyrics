#include "StdAfx.h"
#include "WidgetComboBox.h"


enum
{
	COL_NAME			= 0,
	COL_USER_DATA		= 1,
	COL_COUNT			= 2
};

//////////////////////////////////////////////////////////////////////////

CWidgetComboBox::CWidgetComboBox()
{
	m_model = NULL;
}

CWidgetComboBox::~CWidgetComboBox()
{

}

bool CWidgetComboBox::Create(CBaseWnd *pWndParent, int nID)
{
	m_nID = nID;
	m_pWndParent = pWndParent;

	GtkListStore	*store;

	store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_UINT);
	ASSERT(store);

	m_model = GTK_TREE_MODEL(store);
	m_window = gtk_combo_box_new_with_model(m_model);
	ASSERT(m_window);

	g_signal_connect(m_window, "changed",
						  G_CALLBACK (cb_changed), this); 

	return true;
}

BOOL CWidgetComboBox::Attach(CBaseWnd *pWndParent, int nCtrlID)
{
	CWidgetCtrlBase::Attach(pWndParent, nCtrlID);

	m_model = gtk_combo_box_get_model(GTK_COMBO_BOX(m_window));
	ASSERT(m_window);
	ASSERT(m_model);

	return m_window != NULL;
}

void CWidgetComboBox::Detach()
{
	m_window = NULL;
}

int CWidgetComboBox::GetCount() const
{
	return gtk_tree_model_iter_n_children(m_model, NULL);
}

int CWidgetComboBox::GetCurSel()
{
	return gtk_combo_box_get_active(GTK_COMBO_BOX(m_window));
}

void CWidgetComboBox::SetCurSel(int nIndex)
{
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_window), nIndex);
}

int CWidgetComboBox::AddString(LPCTSTR szStr)
{
	ASSERT(m_model);
	if (!m_model)
		return -1;

	GtkTreeIter		iter;

	gtk_list_store_append(GTK_LIST_STORE(m_model), &iter);

	gtk_list_store_set(GTK_LIST_STORE(m_model), &iter, COL_NAME, szStr, -1);

	return IndexFromGtkTreeIter(iter);
}

int CWidgetComboBox::InsertString(int nIndex, LPCTSTR lpszString)
{
	ASSERT(m_model);
	if (!m_model)
		return -1;

	GtkTreeIter		iter;

	gtk_list_store_insert(GTK_LIST_STORE(m_model), &iter, nIndex);

	gtk_list_store_set(GTK_LIST_STORE(m_model), &iter, COL_NAME, lpszString, -1);

	return IndexFromGtkTreeIter(iter);
}

void CWidgetComboBox::SetItemData(int nIndex, UINT nItemData)
{
	ASSERT(m_model);
	if (!m_model)
		return;

	GtkTreeIter		iter;

	if (!GtkTreeIterFromIndex(nIndex, iter))
		return;

	gtk_list_store_set(GTK_LIST_STORE(m_model), &iter, COL_USER_DATA, nItemData, -1);
}

UINT CWidgetComboBox::GetItemData(int nIndex)
{
	ASSERT(m_model);
	if (!m_model)
		return 0;

	GtkTreeIter		iter;

	if (!GtkTreeIterFromIndex(nIndex, iter))
		return 0;

	GValue		val = { 0, };

	gtk_tree_model_get_value(m_model, &iter, COL_USER_DATA, &val);
	ASSERT(G_VALUE_HOLDS_UINT(&val));

	if (G_VALUE_HOLDS_UINT(&val))
		return g_value_get_uint(&val);
	else
		return 0;
}

UINT CWidgetComboBox::GetItemText(int nIndex, tstring &str)
{
	str.resize(0);

	ASSERT(m_model);
	if (!m_model)
		return 0;

	GValue		val = { 0, };
	GtkTreeIter	iter;

	if (!GtkTreeIterFromIndex(nIndex, iter))
		return 0;

	gtk_tree_model_get_value(m_model, &iter, COL_NAME, &val);
	ASSERT(G_VALUE_HOLDS_STRING(&val));

	if (G_VALUE_HOLDS_STRING(&val))
		str = (gchar*)g_value_get_string(&val);

	return str.size();
}

bool CWidgetComboBox::DeleteString(UINT nIndex)
{
	ASSERT(m_model);
	if (!m_model)
		return false;

	GtkTreeIter		iter;

	if (!GtkTreeIterFromIndex(nIndex, iter))
		return false;

	return gtk_list_store_remove(GTK_LIST_STORE(m_model), &iter);
}

void CWidgetComboBox::ResetContent()
{
	ASSERT(m_model);
	if (!m_model)
		return;

	gtk_list_store_clear(GTK_LIST_STORE(m_model));
}

int CWidgetComboBox::FindStringExact(int nIndexStart, LPCTSTR lpszFind) const
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

int CWidgetComboBox::SelectString(int nStartAfter, LPCTSTR lpszString)
{
	int			n;

	n = FindStringExact(nStartAfter, lpszString);
	if (n == -1)
		return -1;

	SetCurSel(n);

	return n;
}

void CWidgetComboBox::GetSelItemText(tstring &str)
{
	int			n = GetCurSel();
	if (n == -1)
		return;

	GetItemText(n, str);
}

GtkTreePath *CWidgetComboBox::GtkTreePathFromIndex(int nIndex) const
{
	GtkTreePath		*path;

	path = gtk_tree_path_new();

	gtk_tree_path_append_index(path, nIndex);

	return path;
}

bool CWidgetComboBox::GtkTreeIterFromIndex(int nIndex, GtkTreeIter &iter) const
{
	GtkTreePath		*path;
	bool			bRet;
	
	path = GtkTreePathFromIndex(nIndex);

	bRet = gtk_tree_model_get_iter(m_model, &iter, path);

	gtk_tree_path_free(path);

	return bRet;
}

int CWidgetComboBox::IndexFromGtkTreeIter(GtkTreeIter &iter) const
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

void CWidgetComboBox::cb_changed(GtkWidget *widget, gpointer data)
{
	CWidgetComboBox		*pThis = (CWidgetComboBox *)data;
	ASSERT(pThis);

	pThis->m_pWndParent->OnCommand(pThis->m_nID, CBN_SELCHANGE);
}
