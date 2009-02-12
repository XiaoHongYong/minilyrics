#include "StdAfx.h"
#include "WidgetListCtrl.h"

CWidgetListCtrl::CWidgetListCtrl()
{
}

CWidgetListCtrl::~CWidgetListCtrl()
{

}

BOOL CWidgetListCtrl::LoadColumnWidth(LPCTSTR szKeyName)
{
	for (int i = 0; ; i++)
	{
		TCHAR		szKey[256];
		TCHAR		szValue[256];
		int			nCx;

		_stprintf_s(szKey, CountOf(szKey), _T("%s_%d"), szKeyName, i);
		g_Profile.GetProfileString(szKey, _T(""), szValue, 256);
		if (IsEmptyString(szValue))
			break;

		nCx = _ttoi(szValue);
		if (nCx <= 0)
			nCx = 10;
		if (nCx >= 500)
			nCx = 300;
		::SendMessage(m_hWnd, LVM_SETCOLUMNWIDTH, i, nCx);
	}
	return TRUE;
}

BOOL CWidgetListCtrl::SaveColumnWidth(LPCTSTR szKeyName)
{
	for (int i = 0; ; i++)
	{
		TCHAR		szKey[256];
		LVCOLUMN	Column;

		Column.mask = LVCF_WIDTH;
		_stprintf_s(szKey, CountOf(szKey), _T("%s_%d"), szKeyName, i);
		if (!::SendMessage(m_hWnd, LVM_GETCOLUMN, i, (LPARAM)&Column))
			break;

		g_Profile.WriteProfileInt(szKey, Column.cx);
	}
	return TRUE;
}

bool CWidgetListCtrl::Create(CBaseWnd *pWndParent, int nID)
{
	return false;
}

int CWidgetListCtrl::GetItemCount() const
{
	ASSERT(::IsWindow(m_hWnd));
	return (int) ::SendMessage(m_hWnd, LVM_GETITEMCOUNT, 0, 0L);
}

int CWidgetListCtrl::GetColumnWidth(int nCol) const
{
	ASSERT(::IsWindow(m_hWnd));
	return (int) ::SendMessage(m_hWnd, LVM_GETCOLUMNWIDTH, nCol, 0);
}

BOOL CWidgetListCtrl::SetColumnWidth(int nCol, int cx)
{
	ASSERT(::IsWindow(m_hWnd));
	return (BOOL) ::SendMessage(m_hWnd, LVM_SETCOLUMNWIDTH, nCol, MAKELPARAM(cx, 0));
}

UINT CWidgetListCtrl::GetSelectedCount() const
{
	ASSERT(::IsWindow(m_hWnd));
	return (UINT) ::SendMessage(m_hWnd, LVM_GETSELECTEDCOUNT, 0, 0L);
}

int  CWidgetListCtrl::GetNextSelectedItem(int nPos)
{
	ASSERT(::IsWindow(m_hWnd));
	return (int)::SendMessage(m_hWnd, LVM_GETNEXTITEM, nPos, MAKELPARAM(LVNI_SELECTED, 0));
}

BOOL CWidgetListCtrl::GetCheck(int nItem) const
{
	ASSERT(::IsWindow(m_hWnd));
	int nState = (int)::SendMessage(m_hWnd, LVM_GETITEMSTATE, (WPARAM)nItem,
		(LPARAM)LVIS_STATEIMAGEMASK);
	// Return zero if it's not checked, or nonzero otherwise.
	return ((BOOL)(nState >> 12) -1);
};

BOOL CWidgetListCtrl::SetCheck(int nItem, BOOL fCheck)
{
	ASSERT(::IsWindow(m_hWnd));
	LVITEM lvi;
	lvi.stateMask = LVIS_STATEIMAGEMASK;

	/*
	Since state images are one-based, 1 in this macro turns the check off, and
	2 turns it on.
	*/
	lvi.state = INDEXTOSTATEIMAGEMASK((fCheck ? 2 : 1));
	return (BOOL) ::SendMessage(m_hWnd, LVM_SETITEMSTATE, nItem, (LPARAM)&lvi);
}

BOOL CWidgetListCtrl::SetItemSelectionState(int nItem, BOOL bSelected)
{
	ASSERT(::IsWindow(m_hWnd));
	LV_ITEM _ms_lvi;

	_ms_lvi.stateMask = 0xFF;
	_ms_lvi.state = LVIS_SELECTED;

	return SendMessage(m_hWnd, LVM_SETITEMSTATE, (WPARAM)nItem, (LPARAM)(LV_ITEM FAR *)&_ms_lvi);
}

COLORREF CWidgetListCtrl::GetBkColor() const
{
	ASSERT(::IsWindow(m_hWnd));
	return (COLORREF) ::SendMessage(m_hWnd, LVM_GETBKCOLOR, 0, 0L);
}

BOOL CWidgetListCtrl::SetBkColor(COLORREF cr)
{
	ASSERT(::IsWindow(m_hWnd));
	return (BOOL) ::SendMessage(m_hWnd, LVM_SETBKCOLOR, 0, cr);;
}

void CWidgetListCtrl::SetExtendedStyle(DWORD dwNewStyle)
{
	ASSERT(::IsWindow(m_hWnd));
	ListView_SetExtendedListViewStyle(m_hWnd, dwNewStyle);
}

void CWidgetListCtrl::GetItemText(int nItem, int nSubItem, LPTSTR lpszText, int nLen) const
{
	ASSERT(::IsWindow(m_hWnd));
	LVITEM lvi;
	memset(&lvi, 0, sizeof(LVITEM));
	lvi.iSubItem = nSubItem;
	lvi.cchTextMax = nLen;
	lvi.pszText = lpszText;
	::SendMessage(m_hWnd, LVM_GETITEMTEXT, (WPARAM)nItem,
		(LPARAM)&lvi);
}

void CWidgetListCtrl::GetItemText(int nItem, int nSubItem, tstring &strText) const
{
	TCHAR		szText[512];

	GetItemText(nItem, nSubItem, szText, CountOf(szText));
	strText = szText;
}

BOOL CWidgetListCtrl::SetItemText(int nItem, int nSubItem, LPCTSTR lpszText)
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT((GetWindowLong(m_hWnd, GWL_STYLE) & LVS_OWNERDATA) == 0);
	LVITEM lvi;
	lvi.iSubItem = nSubItem;
	lvi.pszText = (LPTSTR)lpszText;
	return (BOOL) ::SendMessage(m_hWnd, LVM_SETITEMTEXT, nItem, (LPARAM)&lvi);
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
	ASSERT(::IsWindow(m_hWnd));

	LVCOLUMN column;
	column.mask = LVCF_TEXT|LVCF_FMT;
	column.pszText = (LPTSTR)lpszColumnHeading;
	column.fmt = nFormat;
	if (nWidth != -1)
	{
		column.mask |= LVCF_WIDTH;
		column.cx = nWidth;
	}
	if (nSubItem != -1)
	{
		column.mask |= LVCF_SUBITEM;
		column.iSubItem = nSubItem;
	}
	return (int) ::SendMessage(m_hWnd, LVM_INSERTCOLUMN, nCol, (LPARAM)&column);
}

BOOL CWidgetListCtrl::DeleteColumn(int nCol)
{
	ASSERT(::IsWindow(m_hWnd));
	return (BOOL) ::SendMessage(m_hWnd, LVM_DELETECOLUMN, nCol, 0);
}

int CWidgetListCtrl::AppendItem(LPCTSTR lpszItem)
{
	ASSERT(::IsWindow(m_hWnd));
	LVITEM item;
	item.mask = LVIF_TEXT;
	item.iItem = -1;
	item.iSubItem = 0;
	item.pszText = (LPTSTR)lpszItem;
	item.state = 0;
	item.stateMask = 0;
	item.iImage = 0;
	item.lParam = 0;

	return ::SendMessage(m_hWnd, LVM_INSERTITEM, 0, (LPARAM)&item);
}

int  CWidgetListCtrl::InsertItem(int nItem, LPCTSTR lpszItem)
{
	ASSERT(::IsWindow(m_hWnd));
	LVITEM item;
	item.mask = LVIF_TEXT;
	item.iItem = nItem;
	item.iSubItem = 0;
	item.pszText = (LPTSTR)lpszItem;
	item.state = 0;
	item.stateMask = 0;
	item.iImage = 0;
	item.lParam = 0;

	return ::SendMessage(m_hWnd, LVM_INSERTITEM, 0, (LPARAM)&item);
}

int  CWidgetListCtrl::InsertItem(int nItem, LPCTSTR lpszItem, int nImage)
{
	ASSERT(::IsWindow(m_hWnd));
	LVITEM item;
	item.mask = LVIF_TEXT | TVIF_IMAGE;
	item.iItem = nItem;
	item.iSubItem = 0;
	item.pszText = (LPTSTR)lpszItem;
	item.state = 0;
	item.stateMask = 0;
	item.iImage = nImage;
	item.lParam = 0;

	return ::SendMessage(m_hWnd, LVM_INSERTITEM, 0, (LPARAM)&item);
}

BOOL CWidgetListCtrl::DeleteItem(int nItem)
{
	ASSERT(::IsWindow(m_hWnd));
	return (BOOL) ::SendMessage(m_hWnd, LVM_DELETEITEM, nItem, 0L);
}

BOOL CWidgetListCtrl::DeleteAllItems()
{
	ASSERT(::IsWindow(m_hWnd));
	return (BOOL) ::SendMessage(m_hWnd, LVM_DELETEALLITEMS, 0, 0L);
}

HIMAGELIST CWidgetListCtrl::SetImageList(HIMAGELIST hImgList, DWORD dwImgListType)
{
	ASSERT(::IsWindow(m_hWnd)); 
	return ListView_SetImageList(m_hWnd, hImgList, dwImgListType);
}

DWORD CWidgetListCtrl::GetItemData(int nItem) const
{
	ASSERT(::IsWindow(m_hWnd));
	LVITEM lvi;
	memset(&lvi, 0, sizeof(LVITEM));
	lvi.iItem = nItem;
	lvi.mask = LVIF_PARAM;
	::SendMessage(m_hWnd, LVM_GETITEM, 0, (LPARAM)&lvi);
	return (DWORD)lvi.lParam;
}

BOOL CWidgetListCtrl::SetItem(int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem,
	int nImage, UINT nState, UINT nStateMask, LPARAM lParam)
{
	ASSERT(::IsWindow(m_hWnd));
	ASSERT((GetWindowLong(m_hWnd, GWL_STYLE) & LVS_OWNERDATA)==0);
	LVITEM lvi;
	lvi.mask = nMask;
	lvi.iItem = nItem;
	lvi.iSubItem = nSubItem;
	lvi.stateMask = nStateMask;
	lvi.state = nState;
	lvi.pszText = (LPTSTR) lpszItem;
	lvi.iImage = nImage;
	lvi.lParam = lParam;
	return (BOOL) ::SendMessage(m_hWnd, LVM_SETITEM, 0, (LPARAM)&lvi);
}
