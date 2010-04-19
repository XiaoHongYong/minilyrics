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

class CPreferenceDlg : public CMPSkinDialog
{
public:
	CPreferenceDlg(bool bFloatingLyrMode = false, PreferPageID defaultPage = PAGE_UI);

	virtual void OnSkinLoaded();

	virtual bool OnCustomCommand(int nId);

protected:
	int				m_nDefaultRootPage;

};

#endif // !defined(AFX_PREFERENCEDLG_H__C764EAD4_A895_4986_9757_463182BE40A4__INCLUDED_)
