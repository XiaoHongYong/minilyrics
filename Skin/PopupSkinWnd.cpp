#include "StdAfx.h"
#include "Skin.h"
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
	int nRet = CSkinWnd::Create(_T("ZPMenu"), _T(""), pSkinFactory, szSkinWndName, pWndParent, true, false);
	if (nRet != ERR_OK)
		return nRet;

	// SetTopmost(true);

	m_pPopupSkinWndNotify = pNotify;
	return ERR_OK;
}

void CPopupSkinWnd::OnKillFocus()
{
//	if (m_pPopupSkinWndNotify)
//		m_pPopupSkinWndNotify->PopupSkinWndOnDestroy();

	PostDestroy();
}

void CPopupSkinWnd::OnKeyDown(UINT nChar, UINT nFlags)
{
	CUIObject	*pObjFocus = GetFocusUIObj();

	// let focus uiobject to process key message
	if (pObjFocus && pObjFocus->WantMsgAllKeys())
	{
		pObjFocus->OnKeyDown(nChar, nFlags);
		return;
	}

	if (nChar == VK_ESCAPE)
	{
		PostDestroy();
		return;
	}

	CSkinWnd::OnKeyDown(nChar, nFlags);
}
