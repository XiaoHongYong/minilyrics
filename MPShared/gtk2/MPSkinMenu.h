// MPSkinMenu.h: interface for the CMPSkinMenu class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MPSKINMENU_H__EEA43535_74D6_4767_9640_38376F9FEE6E__INCLUDED_)
#define AFX_MPSKINMENU_H__EEA43535_74D6_4767_9640_38376F9FEE6E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "SkinMenu.h"

class CMPSkinMenu : public CSkinMenu  
{
public:
	CMPSkinMenu();
	virtual ~CMPSkinMenu();

	virtual void TrackPopupMenu(int x, int y, CBaseWnd *pWnd);

	virtual void OnLoadMenu();

	void AddUICheckStatusIf(IUICheckStatus *pIfUICheckStatus) {  }

protected:

};

#endif // !defined(AFX_MPSKINMENU_H__EEA43535_74D6_4767_9640_38376F9FEE6E__INCLUDED_)
