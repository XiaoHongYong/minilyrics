// MPSkinInfoCtrl.cpp: implementation of the CMPSkinInfoTextCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerAppBase.h"
#include "MPSkinInfoTextCtrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CMPSkinInfoTextCtrl, _T("InfoText"))

CMPSkinInfoTextCtrl::CMPSkinInfoTextCtrl()
{
	m_nMsgWant = UO_MSG_WANT_LBUTTON;
	m_nTimerIDHideInfo = 0;
}

CMPSkinInfoTextCtrl::~CMPSkinInfoTextCtrl()
{
	if (m_nTimerIDHideInfo != 0)
		m_pSkin->UnregisterTimerObject(this, m_nTimerIDHideInfo);

	CMPlayerAppBase::GetEventsDispatcher()->UnregisterHanlder(ET_UI_INFO_TEXT, this);
	CMPlayerAppBase::GetEventsDispatcher()->UnregisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
	CMPlayerAppBase::GetEventsDispatcher()->UnregisterHanlder(ET_PLAYER_CUR_MEDIA_INFO_CHANGED, this);
}

void CMPSkinInfoTextCtrl::OnCreate()
{
	CSkinScrollText::OnCreate();

	CMPlayerAppBase::GetEventsDispatcher()->RegisterHanlder(ET_UI_INFO_TEXT, this);
	CMPlayerAppBase::GetEventsDispatcher()->RegisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
	CMPlayerAppBase::GetEventsDispatcher()->RegisterHanlder(ET_PLAYER_CUR_MEDIA_INFO_CHANGED, this);

	UpdateShowTrackInfo();
}

void CMPSkinInfoTextCtrl::OnEvent(const IEvent *pEvent)
{
	if (pEvent->eventType == ET_UI_INFO_TEXT)
	{
		SetText(pEvent->strValue.c_str());
		Invalidate();
		m_pSkin->UnregisterTimerObject(this);
		m_nTimerIDHideInfo = m_pSkin->RegisterTimerObject(this, 3000);
	}
	else if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED || pEvent->eventType == ET_PLAYER_CUR_MEDIA_INFO_CHANGED)
	{
		UpdateShowTrackInfo();
		Invalidate();
	}
}

bool CMPSkinInfoTextCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	return true;
}

bool CMPSkinInfoTextCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	return true;
}

void CMPSkinInfoTextCtrl::OnTimer(int nId)
{
	if (nId == m_nTimerIDHideInfo)
	{
		m_pSkin->UnregisterTimerObject(this, m_nTimerIDHideInfo);
		m_nTimerIDHideInfo = 0;
		UpdateShowTrackInfo();
		Invalidate();
	}
	else
		CSkinScrollText::OnTimer(nId);
}

void CMPSkinInfoTextCtrl::Draw(CRawGraph *pGraph)
{
	if (IsFlagSet(m_dwAlignText, AT_CENTER))
		CSkinStaticText::Draw(pGraph);
	else
		CSkinScrollText::Draw(pGraph);
}

void CMPSkinInfoTextCtrl::UpdateShowTrackInfo()
{
	SetText(g_Player.GetFullTitle());
}
