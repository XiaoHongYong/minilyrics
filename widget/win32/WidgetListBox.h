// WidgetListBox.h: interface for the CWidgetListBox class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "WidgetCtrlBase.h"

class CWidgetListBox : public CWidgetCtrlBase
{
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
	int SetItemData(int nIndex, DWORD dwItemData);
	DWORD GetItemData(int nIndex) const;

	void ResetContent();
	int FindString(int nStartAfter, LPCTSTR lpszItem) const;
	int SelectString(int nStartAfter, LPCTSTR lpszItem);

};
