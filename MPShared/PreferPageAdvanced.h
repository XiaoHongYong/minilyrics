#ifndef _PREFER_PAGE_ADVANCED_H_
#define _PREFER_PAGE_ADVANCED_H_

#pragma once

#include "PreferencePageBase.h"

class CPreferItem
{
public:
	CPreferItem(LPCTSTR szName) : m_strName(szName) { }
	virtual tstring GetValue() = 0;
	virtual void GetOptions(V_Str &vString, int &nRadio) = 0;
	virtual void SetOption(int nIndex) = 0;
	virtual bool IsDefault() = 0;
	virtual void Reset() = 0;

	tstring			m_strName;

};

typedef vector<CPreferItem *>		VecPreferItems;


//
// Internet settings
//
class CPreferPageAdvanced : public CPreferencePageBase
{
public:
	CPreferPageAdvanced();
	~CPreferPageAdvanced();

	BOOL OnInitDialog();

	void OnCommand(UINT uID, UINT nNotifyCode);
	void OnDestroy();

	BOOL OnNotify(WIDGET_NOTIFY_INFO *info);

protected:
	CPreferItem *GetCurItem();
	void ShowCustomizeMenu(int x, int y);

protected:
	VecPreferItems				m_vPreferItems;
	int							m_nMenuIdStart, m_nMenuIdEnd;
	CWidgetListCtrl				m_listItems;


};


#endif // _PREFER_PAGE_ADVANCED_H_
