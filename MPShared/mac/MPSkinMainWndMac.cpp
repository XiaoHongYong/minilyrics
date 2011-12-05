// MPSkinMainWnd.cpp: implementation of the CMPSkinMainWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "MPSkinMainWnd.h"

#ifdef _WIN32
#include "MPMsg.h"
#endif

#ifdef _MPLAYER
#include "MPHelper.h"
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPSkinMainWnd::CMPSkinMainWnd()
{
}

CMPSkinMainWnd::~CMPSkinMainWnd()
{
}

bool CMPSkinMainWnd::OnCreate()
{
	if (!CMPSkinMainWndBase::OnCreate())
		return false;

	RegisterHandler(CMPlayerAppBase::GetEventsDispatcher(), ET_PLAYER_CUR_MEDIA_CHANGED, ET_PLAYER_CUR_MEDIA_INFO_CHANGED);

	m_mlTrayIcon.Init(this);

	return true;
}

void CMPSkinMainWnd::OnDestroy()
{
	m_mlTrayIcon.Quit();

	CMPSkinMainWndBase::OnDestroy();
}

void CMPSkinMainWnd::OnEvent(const IEvent *pEvent)
{
	if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED 
		|| pEvent->eventType == ET_PLAYER_CUR_MEDIA_INFO_CHANGED)
	{
		// Update main window caption
		TCHAR		szCaption[512];

		if (g_Player.IsMediaOpened())
			_stprintf_s(szCaption, CountOf(szCaption), _T("%s - %s"), g_Player.GetFullTitle(), SZ_APP_NAME);
		else
			_tcscpy_safe(szCaption, CountOf(szCaption), GetAppNameLong().c_str());

		SetWindowText(szCaption);

		m_mlTrayIcon.UpdateTrayIconText(szCaption);
	}
	else if (pEvent->eventType == ET_UI_SETTINGS_CHANGED)
	{
		if (IsPropertyName(pEvent->strName.c_str(), _T("ShowIconOn")))
			m_mlTrayIcon.UpdateShowIconPos();
		else
			CMPSkinMainWndBase::OnEvent(pEvent);
	}
	else
		CMPSkinMainWndBase::OnEvent(pEvent);
}

#ifndef _MPLAYER

void CMPSkinMainWnd::Topmost(bool bTopmost)
{
	CBaseWnd::SetTopmost(bTopmost);
}

extern bool				g_bAutoMinimized;

void CMPSkinMainWnd::Minimize()
{
	ShowWindow(SW_MINIMIZE);
}

void CMPSkinMainWnd::Activate()
{
}
#endif
