
#pragma once

#include "SkinWnd.h"

class IPopupSkinWndNotify
{
public:
	virtual void PopupSkinWndOnSelected() { }
	// virtual void PopupSkinWndOnDestroy() { }

};

class CPopupSkinWnd : public CSkinWnd
{
public:
	CPopupSkinWnd();
	~CPopupSkinWnd();

	virtual int Create(CSkinFactory *pSkinFactory, LPCTSTR szSkinWndName,
		CBaseWnd *pWndParent, IPopupSkinWndNotify *pNotify, const CRect &rc);

	virtual void OnKillFocus();

protected:
	IPopupSkinWndNotify			*m_pPopupSkinWndNotify;

};
