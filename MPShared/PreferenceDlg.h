/********************************************************************
	Created  :	2002/01/04	21:41
	FileName :	PreferenceDlg.h
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#if !defined(AFX_PREFERENCEDLG_H__C764EAD4_A895_4986_9757_463182BE40A4__INCLUDED_)
#define AFX_PREFERENCEDLG_H__C764EAD4_A895_4986_9757_463182BE40A4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MPSkinDialog.h"
#include "PreferencePageBase.h"

class CPreferenceDlg2 : public CMPSkinDialog
{
public:
	CPreferenceDlg2(bool bFloatingLyrMode = false, PreferPageID defaultPage = PAGE_UI);

	virtual void OnSkinLoaded();

	virtual bool OnCustomCommand(int nId);

protected:
	int				m_nDefaultRootPage;

};



class CPreferenceDlg : public CBaseDialog
{
public:
	CPreferenceDlg();
	virtual ~CPreferenceDlg();

	BOOL Create(CBaseWnd *pWndParent, PreferPageID idToactivate = PAGE_UI, bool bFloatingLyrMode = false);

public:
	BOOL OnInitDialog();
	void OnDestroy();
	BOOL OnNotify(WIDGET_NOTIFY_INFO *info);
	void OnLanguageChanged();

protected:
	void SetCurPage(PreferPageID id);

	void ConstructPreferencePages();

	CPreferencePageBase *GetChild(CPreferencePageBase *pPageParent);

	CLangTreeCtrlHelper		m_LangTreeCtrlHelper;
	CWidgetTreeCtrl			m_treePf;
	bool					m_bFloatingLyrMode;

	typedef vector<CPreferencePageBase *>		V_PFPAGE;

	V_PFPAGE				m_vPreferencePages;

};

#endif // !defined(AFX_PREFERENCEDLG_H__C764EAD4_A895_4986_9757_463182BE40A4__INCLUDED_)
