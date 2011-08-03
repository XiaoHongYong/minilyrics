#pragma once

#include "PreferencePageBase.h"

/*
class CPreferPageAssociation : public CPreferencePageBase
{
public:
	CPreferPageAssociation();
	virtual ~CPreferPageAssociation();

	BOOL OnInitDialog();
	void OnDestroy();
	void OnCommand(UINT uID, UINT nNotifyCode);

	CWidgetListCtrl		m_listCtrl;

};*/

#ifdef _WIN32_WCE
class CPreferPageScreen: public CPreferencePageBase
{
public:
	CPreferPageScreen();
	virtual ~CPreferPageScreen();

	BOOL OnInitDialog();
	void OnCommand(UINT uID, UINT nNotifyCode);
	void OnDestroy();

	CWidgetListBox	m_cbTurnScreenOffTimer;

	bool			m_bEnableTurnScreenOff;
	int				m_nTurnOffAfter;

};
#endif
