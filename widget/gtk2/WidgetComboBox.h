// ListBox.h: interface for the CListBox class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "WidgetListCtrl.h"

#ifndef CBN_SELCHANGE
#define CB_ERR				(-1) 
#define CBN_SELCHANGE       1
#define CBN_DBLCLK          2
#endif

class CWidgetComboBox : public CWidgetCtrlBase
{
public:
	CWidgetComboBox();
	~CWidgetComboBox();

	virtual bool Create(CBaseWnd *pWndParent, int nID = ID_UNDEFINE);

	BOOL Attach(CBaseWnd *pWndParent, int nCtrlID);

	int GetCount() const;

	virtual int GetCurSel();
	void SetCurSel(int nIndex);

	int AddString(LPCTSTR szStr);
	void SetItemData(int nIndex, UINT nItemData);
	UINT GetItemData(int nIndex);
	UINT GetItemText(int nIndex, tstring &str);

	bool DeleteString(UINT nIndex);

	void ResetContent();

	int FindStringExact(int nIndexStart, LPCTSTR lpszFind) const;

	int SelectString(int nStartAfter, LPCTSTR lpszString);

	void GetSelItemText(tstring &str);

protected:
	static void cb_changed(GtkWidget *widget, gpointer data);

	GtkTreePath *GtkTreePathFromIndex(int nIndex) const;
	bool GtkTreeIterFromIndex(int nIndex, GtkTreeIter &iter) const;
	int IndexFromGtkTreeIter(GtkTreeIter &iter) const;

protected:
	GtkTreeModel		*m_model;

};
