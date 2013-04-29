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

	UpdateCaptionText();

	return true;
}

void CMPSkinMainWndBase::OnDestroy()
{
	IEventHandler::UnregisterHandler();

#ifndef _WIN32_WCE
	// Save iconic status
	g_Profile.WriteInt(_T("Minimized"), IsIconic());
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

void CMPSkinMainWndBase::UpdateCaptionText()
{
	LPCTSTR szAppName;
	if (CMPlayerApp::GetInstance()->GetCurrentAppMode() == SA_IPOD_LYRICS_DOWNLOADER)
		szAppName = _TLM("Lyrics Downloader for iPod");
	else
		szAppName = SZ_APP_NAME;

	SetCaptionText(_TL(szAppName));
}
