// WidgetTabCtrl.h: interface for the CWidgetTabCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WIDGETTABCTRL_H__CC1D9E54_5039_4CC8_8330_9C3F504CE4BE__INCLUDED_)
#define AFX_WIDGETTABCTRL_H__CC1D9E54_5039_4CC8_8330_9C3F504CE4BE__INCLUDED_

#pragma once

#include "WidgetCtrlBase.h"

class CWidgetTabCtrl : public CWidgetCtrlBase
{
public:
	CWidgetTabCtrl();
	virtual ~CWidgetTabCtrl();

public:
	int GetItemCount() const;

	int GetCurSel() const;
	int SetCurSel(int nItem);
	int GetCurFocus() const;
	void SetCurFocus(int nItem);

	BOOL InsertItem(int nItem, LPCTSTR lpszItem);
	BOOL InsertItem(int nItem, LPCTSTR lpszItem, int nImage);

	BOOL InsertItem(UINT nMask, int nItem, LPCTSTR lpszItem, int nImage, LPARAM lParam);

	BOOL DeleteItem(int nItem);
	BOOL DeleteAllItems();

};

#endif // !defined(AFX_WIDGETTABCTRL_H__CC1D9E54_5039_4CC8_8330_9C3F504CE4BE__INCLUDED_)
