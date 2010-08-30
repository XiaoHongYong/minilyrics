#ifndef _PREFER_PAGE_ADVANCED_H_
#define _PREFER_PAGE_ADVANCED_H_

#pragma once

#include "PreferencePageBase.h"
#include "../widget/win32/WidgetTooltipListCtrl.h"
#include "SkinListCtrl.h"
#include "SkinEditCtrl.h"

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


class CPagePfAdvanced : public CPagePfBase, public IEditNotification
{
public:
	CPagePfAdvanced();
	~CPagePfAdvanced();

	static LPCTSTR GetPageID();

	void OnInitialUpdate();

	void OnDestroy();

	virtual void OnUIObjNotify(IUIObjNotify *pNotify);

	bool OnCommand(int nId);
	bool OnCustomCommand(int nId);

	bool OnTimer(UINT nTimerId);

	void OnTextChanged();

	void UpdateValues();

protected:
	enum {
		IMG_UNMODIFIED = 0,
		IMG_MODIFIED,
	};

	CPreferItem *GetCurItem();
	void ShowCustomizeMenu(int x, int y);

	void ListAllItems();

protected:
	VecPreferItems				m_vPreferItems;
	VecPreferItems				m_vFilteredItems;
	int							m_nMenuIdStart, m_nMenuIdEnd;
	CSkinListCtrl				*m_pListItems;
	tstring						m_strLastSearch;

	int				CID_LIST_SETTINGS, CID_E_ADV_SEARCH;
	bool			m_bIgnoreSettingListNotify;

};


#endif // _PREFER_PAGE_ADVANCED_H_
