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
	// virtual bool Create(CBaseWnd *pWndParent, int nID = ID_UNDEFINE);

	BOOL Create(DWORD dwStyle, const RECT& rect, HWND hWndParent, UINT nID);

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

protected:
	void ExpandWithChildren(TreeItemIter item, UINT nCode);

};
