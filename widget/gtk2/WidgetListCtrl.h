// ListBox.h: interface for the CListBox class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "CommonWidget.h"

#ifndef LVS_EX_CHECKBOXES
#define LVS_EX_CHECKBOXES       0x00000004
#define LVS_EX_FULLROWSELECT    0x00000020 // applies to report mode only

#define LVCFMT_LEFT             0x0000
#define LVCFMT_RIGHT            0x0001
#define LVCFMT_CENTER           0x0002
#endif

class CWidgetListCtrl : public CWidgetCtrlBase
{
public:
	CWidgetListCtrl();
	~CWidgetListCtrl();

public:
	BOOL LoadColumnWidth(LPCTSTR szKeyName);
	BOOL SaveColumnWidth(LPCTSTR szKeyName);

	virtual bool Create(CBaseWnd *pWndParent, int nID = ID_UNDEFINE);

	BOOL Attach(CBaseWnd *pWndParent, int nCtrlID);

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

//	HIMAGELIST SetImageList(HIMAGELIST hImgList, DWORD dwImgListType = LVSIL_SMALL);

public:
	void SetHeaderVisible(bool bVisible);

	void SetSingleSelection(bool bSigSel);


protected:
	// gtk+ related functions
	void CreateModel();

	GtkTreePath *GtkTreePathFromIndex(int nIndex) const;
	bool GtkTreeIterFromIndex(int nIndex, GtkTreeIter &iter) const;
	int IndexFromGtkTreeIter(GtkTreeIter &iter) const;

	virtual gboolean OnSelect(GtkTreePath *path, gboolean path_currently_selected);
	virtual void OnRowActivate(GtkTreePath *path);

	static void CBRowActivated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *col, gpointer userdata);

	static gboolean CBSelect(GtkTreeSelection *selection,
		GtkTreeModel     *model,
		GtkTreePath      *path,
		gboolean          path_currently_selected,
		gpointer          userdata);

protected:
	GtkTreeModel		*m_model;
	int					m_nColCount;	

};
