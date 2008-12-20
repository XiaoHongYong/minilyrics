// ListBox.h: interface for the CListBox class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "WidgetListCtrl.h"

#ifndef LBN_SELCHANGE
#define LBN_SELCHANGE       1
#define LBN_DBLCLK          2
#define LBN_SELCANCEL       3

#define LB_ERR				(-1)
#endif

class CWidgetListBox : public CWidgetListCtrl
{
public:
	CWidgetListBox()
	{
	}
	virtual ~CWidgetListBox()
	{
	}

	virtual bool Create(CBaseWnd *pWndParent, int nID = ID_UNDEFINE);
	// void Attach(CBaseDialogEx *pWndParent, int nID);

public:
	int GetCount() const;
	int GetCurSel();
	int SetCurSel(int nSelect);
	int GetHorizontalExtent() const;
	void SetHorizontalExtent(int cxExtent);
	int GetSelCount() const;
	void GetText(int nIndex, LPTSTR lpszBuffer) const;
	void GetText(int nIndex, tstring &str) const;
	int GetTextLen(int nIndex) const;
	int FindStringExact(int nIndexStart, LPCTSTR lpszFind) const;

	int AddString(LPCTSTR lpszItem);
	int DeleteString(UINT nIndex);
	int InsertString(int nIndex, LPCTSTR lpszItem);

	void ResetContent();
	int FindString(int nStartAfter, LPCTSTR lpszItem) const;
	int SelectString(int nStartAfter, LPCTSTR lpszItem);

protected:
	virtual void OnRowActivate(GtkTreePath *path);
	virtual gboolean OnSelect(GtkTreePath *path, gboolean path_currently_selected);

};
