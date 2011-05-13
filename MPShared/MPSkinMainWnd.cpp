// MPSkinMainWnd.cpp: implementation of the CMPSkinMainWndBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "MPSkinMainWnd.h"
#include "DlgAbout.h"
#include "VersionUpdate.h"

//////////////////////////////////////////////////////////////////////

CMPSkinMainWndBase::CMPSkinMainWndBase()
{
}

CMPSkinMainWndBase::~CMPSkinMainWndBase()
{
}

bool CMPSkinMainWndBase::OnCreate()
{
	if (!CMPSkinWnd::OnCreate())
		return false;

	CMPlayerAppBase::GetEventsDispatcher()->RegisterHanlder(ET_UI_SETTINGS_CHANGED, this);

	return true;
}

void CMPSkinMainWndBase::OnDestroy()
{
	CMPlayerAppBase::GetEventsDispatcher()->UnregisterHanlder(ET_UI_SETTINGS_CHANGED, this);

	if (IsMainAppWnd() && m_strSkinWndName.size())
		g_Profile.WriteProfileString(_T("DefaultSkinWnd"), GetSkinWndName());

#ifndef _WIN32_WCE
	// Save iconic status
	g_Profile.WriteProfileInt(_T("Minimized"), IsIconic());
#endif

	CMPSkinWnd::OnDestroy();
}

void CMPSkinMainWndBase::OnEvent(const IEvent *pEvent)
{
	if (pEvent->eventType == ET_UI_SETTINGS_CHANGED)
	{
		if (IsPropertyName(pEvent->strName.c_str(), _T("topmost")))
			CMPlayerAppBase::GetMPSkinFactory()->TopmostAll(IsTRUE(pEvent->strValue.c_str()));
		else
			CMPSkinWnd::OnEvent(pEvent);
	}
	else
		CMPSkinWnd::OnEvent(pEvent);
}
