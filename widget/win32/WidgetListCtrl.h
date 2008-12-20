// WidgetListCtrl.h: interface for the CWidgetListCtrl class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "WidgetCtrlBase.h"

class CWidgetListCtrl : public CWidgetCtrlBase
{
public:
	CWidgetListCtrl();
	~CWidgetListCtrl();

public:
	BOOL LoadColumnWidth(LPCTSTR szKeyName);
	BOOL SaveColumnWidth(LPCTSTR szKeyName);

	virtual bool Create(CBaseWnd *pWndParent, int nID = ID_UNDEFINE);

public:
	int GetItemCount() const;

	int GetColumnWidth(int nCol) const;

	BOOL SetColumnWidth(int nCol, int cx);

	UINT GetSelectedCount() const;

	int GetNextSelectedItem(int nPos = -1);

	BOOL GetCheck(int nItem) const;

	BOOL SetCheck(int nItem, BOOL fCheck = TRUE);

	BOOL SetItemSelectionState(int nItem, BOOL bSelected = TRUE);

	COLORREF GetBkColor() const;

	BOOL SetBkColor(COLORREF cr);

	void SetExtendedStyle(DWORD dwNewStyle = LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	void GetItemText(int nItem, int nSubItem, LPTSTR lpszText, int nLen) const;
	void GetItemText(int nItem, int nSubItem, tstring &strText) const;

	BOOL SetItemText(int nItem, int nSubItem, LPCTSTR lpszText);

// 	BOOL SetItem(int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem,
// 		int nImage, UINT nState, UINT nStateMask, LPARAM lParam);
// 
// 	BOOL SetItemData(int nItem, DWORD dwData);
// 
// 	DWORD GetItemData(int nItem) const;

	int InsertColumn(int nCol, LPCTSTR lpszColumnHeading,
		int nFormat = LVCFMT_LEFT, int nWidth = -1, int nSubItem = -1);

	BOOL DeleteColumn(int nCol);

	int AppendItem(LPCTSTR lpszItem);

	int InsertItem(int nItem, LPCTSTR lpszItem);

	int InsertItem(int nItem, LPCTSTR lpszItem, int nImage);

	BOOL DeleteItem(int nItem);

	BOOL DeleteAllItems();

// 	int InsertItem(UINT nMask, int nItem, LPCTSTR lpszItem, UINT nState,
// 		UINT nStateMask, int nImage, LPARAM lParam);

	//
	// for win32 only
	//
	HIMAGELIST SetImageList(HIMAGELIST hImgList, DWORD dwImgListType = LVSIL_SMALL);

	BOOL SetItemData(int nItem, DWORD dwData)
		{ ASSERT(::IsWindow(m_hWnd)); return SetItem(nItem, 0, LVIF_PARAM, NULL, 0, 0, 0, (LPARAM)dwData); }

	DWORD GetItemData(int nItem) const;

	BOOL SetItem(int nItem, int nSubItem, UINT nMask, LPCTSTR lpszItem,
		int nImage, UINT nState, UINT nStateMask, LPARAM lParam);

};
