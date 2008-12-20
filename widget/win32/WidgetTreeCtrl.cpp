#include "StdAfx.h"
#include "WidgetTreeCtrl.h"


CWidgetTreeCtrl::CWidgetTreeCtrl()
{
}

CWidgetTreeCtrl::~CWidgetTreeCtrl()
{
}

BOOL CWidgetTreeCtrl::Create(DWORD dwStyle, const RECT& rect, HWND hWndParent, UINT nID)
{
	m_hWnd = ::CreateWindowEx(0, WC_TREEVIEW, _T(""), dwStyle, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, hWndParent, (HMENU)nID, GetAppInstance(), NULL);

	return m_hWnd != NULL;
}

bool CWidgetTreeCtrl::GetSelected(TreeItemIter &iter)
{
	ASSERT(::IsWindow(m_hWnd));
	iter = (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_CARET, 0);
	return iter != NULL;
}

bool CWidgetTreeCtrl::SetSelected(TreeItemIter &iter)
{
	ASSERT(::IsWindow(m_hWnd));
	return Tobool(::SendMessage(m_hWnd, TVM_SELECTITEM, TVGN_CARET, (LPARAM)iter));
}

void CWidgetTreeCtrl::GetItemText(TreeItemIter &nItem, tstring &strText) const
{
	ASSERT(::IsWindow(m_hWnd));
	TVITEM		item;
	TCHAR		szBuff[512] = _T("");
	item.hItem = nItem;
	item.mask = TVIF_TEXT | TVIF_HANDLE;
	item.pszText = szBuff;
	item.cchTextMax = CountOf(szBuff);
	::SendMessage(m_hWnd, TVM_GETITEM, 0, (LPARAM)&item);
	strText = szBuff;
}

BOOL CWidgetTreeCtrl::SetItemText(TreeItemIter &nItem, LPCTSTR lpszText)
{
	ASSERT(::IsWindow(m_hWnd));
	TVITEM item;
	item.hItem = nItem;
	item.mask = TVIF_TEXT;
	item.pszText = (LPTSTR)lpszText;
	::SendMessage(m_hWnd, TVM_SETITEM, 0, (LPARAM)&item);
	return true;
}

UINT CWidgetTreeCtrl::GetItemData(TreeItemIter &nItem) const
{
	ASSERT(::IsWindow(m_hWnd));
	TVITEM item;
	item.hItem = nItem;
	item.mask = TVIF_PARAM;
	item.lParam = 0;
	::SendMessage(m_hWnd, TVM_GETITEM, 0, (LPARAM)&item);
	return (UINT)item.lParam;
}

BOOL CWidgetTreeCtrl::SetItemData(TreeItemIter &nItem, UINT dwData)
{
	ASSERT(::IsWindow(m_hWnd));
	TVITEM item;
	item.hItem = nItem;
	item.mask = TVIF_PARAM;
	item.lParam = dwData;
	return Tobool(::SendMessage(m_hWnd, TVM_SETITEM, 0, (LPARAM)&item));
}

// if parent == NULL, the item will be appended at the top level.
TreeItemIter CWidgetTreeCtrl::AppendItem(TreeItemIter *parent, LPCTSTR lpszItem, UINT nUserData)
{
	TVINSERTSTRUCT tvis;
	if (parent)
		tvis.hParent = *parent;
	else
		tvis.hParent = TVI_ROOT;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT;
	tvis.item.pszText = (LPTSTR) lpszItem;
	tvis.item.iImage = 0;
	tvis.item.iSelectedImage = 0;
	tvis.item.state = 0;
	tvis.item.stateMask = 0;
	tvis.item.lParam = nUserData;
	return (HTREEITEM)::SendMessage(m_hWnd, TVM_INSERTITEM, 0, (LPARAM)&tvis);
}

TreeItemIter CWidgetTreeCtrl::InsertItem(TreeItemIter *parent, TreeItemIter *iterInsertBefore, LPCTSTR lpszItem, UINT nUserData)
{
	TVINSERTSTRUCT tvis;
	if (parent)
		tvis.hParent = *parent;
	else
		tvis.hParent = TVI_ROOT;
	if (iterInsertBefore)
		tvis.hInsertAfter = *iterInsertBefore;
	else
		tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_PARAM | TVIF_TEXT;
	tvis.item.pszText = (LPTSTR) lpszItem;
	tvis.item.iImage = 0;
	tvis.item.iSelectedImage = 0;
	tvis.item.state = 0;
	tvis.item.stateMask = 0;
	tvis.item.lParam = nUserData;
	return (HTREEITEM)::SendMessage(m_hWnd, TVM_INSERTITEM, 0, (LPARAM)&tvis);
}

BOOL CWidgetTreeCtrl::DeleteItem(TreeItemIter &nItem)
{
	ASSERT(::IsWindow(m_hWnd));
	return (BOOL)::SendMessage(m_hWnd, TVM_DELETEITEM, 0, (LPARAM)nItem);
}

BOOL CWidgetTreeCtrl::DeleteAllItems()
{
	ASSERT(::IsWindow(m_hWnd));
	return (BOOL)::SendMessage(m_hWnd, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT);
}

bool CWidgetTreeCtrl::GetFirstChild(TreeItemIter *parent, TreeItemIter &child)
{
	ASSERT(::IsWindow(m_hWnd));
	if (parent)
		child = (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)*parent);
	else
		child = (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_ROOT, 0);
	return child != NULL;
}

bool CWidgetTreeCtrl::NextSibling(TreeItemIter &item)
{
	ASSERT(::IsWindow(m_hWnd));
	TreeItemIter	next;
	next = (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)item);
	return next != NULL;
}

void CWidgetTreeCtrl::Expand(TreeItemIter &item, bool bExpand)
{
	ASSERT(::IsWindow(m_hWnd));
	::SendMessage(m_hWnd, TVM_EXPAND, 
		bExpand ? TVE_EXPAND : TVE_COLLAPSE, (LPARAM)item);
}

void CWidgetTreeCtrl::ExpandAll(bool bExpand)
{
	ExpandWithChildren(TVI_ROOT, bExpand ? TVE_EXPAND : TVE_COLLAPSE);
}

void CWidgetTreeCtrl::ExpandWithChildren(TreeItemIter item, UINT nCode)
{
	TreeItemIter	child;

	::SendMessage(m_hWnd, TVM_EXPAND, nCode, (LPARAM)item);

	child = (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)item);
	while (child)
	{
		if (::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)child))
			ExpandWithChildren(child, nCode);

		child = (HTREEITEM)::SendMessage(m_hWnd, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)child);
	}
}
