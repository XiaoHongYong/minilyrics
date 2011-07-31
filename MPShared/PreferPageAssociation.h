// PreferPageAssociation.h: interface for the CPreferPageAssociation class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PREFERPAGEASSOCIATION_H__E28F2CD8_AABF_44E9_8983_4522DE2A5FEE__INCLUDED_)
#define AFX_PREFERPAGEASSOCIATION_H__E28F2CD8_AABF_44E9_8983_4522DE2A5FEE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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

#endif // !defined(AFX_PREFERPAGEASSOCIATION_H__E28F2CD8_AABF_44E9_8983_4522DE2A5FEE__INCLUDED_)
