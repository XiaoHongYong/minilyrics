// WidgetTreeCtrl.h: interface for the CWidgetTreeCtrl class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "WidgetCtrlBase.h"

class CWidgetTreeCtrl : public CWidgetCtrlBase
{
public:
	CWidgetTreeCtrl();
	~CWidgetTreeCtrl();

public:
	virtual bool Create(CBaseWnd *pWndParent, int nID = ID_UNDEFINE);

	BOOL Attach(CBaseWnd *pWndParent, int nCtrlID);

public:
	bool GetSelected(TreeItemIter &iter);

	bool SetSelected(TreeItemIter &iter);

	void GetItemText(TreeItemIter &nItem, tstring &strText) const;

	BOOL SetItemText(TreeItemIter &nItem, LPCTSTR lpszText);

	UINT GetItemData(TreeItemIter &nItem) const;

	BOOL SetItemData(TreeItemIter &nItem, UINT dwData);

	// if parent == NULL, the item will be appended at the top level.
	TreeItemIter AppendItem(TreeItemIter *parent, LPCTSTR lpszItem, UINT nUserData = 0);

	// if iterInsertBefore == NULL, it will be appended at the tail.
	TreeItemIter InsertItem(TreeItemIter *parent, TreeItemIter *iterInsertBefore, LPCTSTR lpszItem, UINT nUserData = 0);

	BOOL DeleteItem(TreeItemIter &nItem);

	BOOL DeleteAllItems();

	bool GetFirstChild(TreeItemIter *parent, TreeItemIter &child);
	bool NextSibling(TreeItemIter &item);

	void Expand(TreeItemIter &item, bool bExpand);
	void ExpandAll(bool bExpand);

public:
	void SetHeaderVisible(bool bVisible);

	void SetSingleSelection(bool bSigSel);

protected:
	// gtk+ related functions
	void CreateModel();

	virtual gboolean OnSelect(GtkTreePath *path, gboolean path_currently_selected);

	static void CBRowActivated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer userdata);

	static gboolean CBSelect(GtkTreeSelection *selection,
		GtkTreeModel     *model,
		GtkTreePath      *path,
		gboolean          path_currently_selected,
		gpointer          userdata);

protected:
	GtkTreeModel		*m_model;	

};
