#include "StdAfx.h"
#include "MLSkin.h"
#include "PopupSkinWnd.h"

CPopupSkinWnd::CPopupSkinWnd()
{
	m_pPopupSkinWndNotify = NULL;
}

CPopupSkinWnd::~CPopupSkinWnd()
{

}

int CPopupSkinWnd::Create(CSkinFactory *pSkinFactory, LPCTSTR szSkinWndName, 
			CBaseWnd *pWndParent, IPopupSkinWndNotify *pNotify, const CRect &rc)
{
	m_rcReal = rc;
	int nRet = CSkinWnd::Create(_T("ZPMenu"), _T(""), pSkinFactory, szSkinWndName, pWndParent, true);
	if (nRet != ERR_OK)
		return nRet;

	m_pPopupSkinWndNotify = pNotify;
	return ERR_OK;
}

void CPopupSkinWnd::OnKillFocus()
{
	if (m_pPopupSkinWndNotify)
		m_pPopupSkinWndNotify->PopupSkinWndOnDestroy();

	PostDestroy();
}
