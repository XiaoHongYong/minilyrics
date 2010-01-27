// WidgetTooltipListCtrl.h: interface for the CWidgetTooltipListCtrl class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "WidgetCtrlBase.h"
#include "WidgetListCtrl.h"

class CWidgetTooltipListCtrl : public CWidgetListCtrl
{
public:
	CWidgetTooltipListCtrl();
	~CWidgetTooltipListCtrl();

public:
	BOOL Attach(CBaseWnd *pWndParent, int nCtrlID);

	int InsertItem(int nItem, LPCTSTR lpszItem);

	int InsertItem(int nItem, LPCTSTR lpszItem, int nImage);

	BOOL SetItemText(int nItem, int nSubItem, LPCTSTR lpszText);

	void SetToolTip(int nItem, int nSubItem, LPCTSTR lpszText);

};
