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
	CMPlayerAppBase::GetEventsDispatcher()->RegisterHanlder(ET_LICENSE_CHANGED, this);

	UpdateCaptionText();

	return true;
}

void CMPSkinMainWndBase::OnDestroy()
{
	CMPlayerAppBase::GetEventsDispatcher()->UnregisterHanlder(ET_UI_SETTINGS_CHANGED, this);
	CMPlayerAppBase::GetEventsDispatcher()->UnregisterHanlder(ET_LICENSE_CHANGED, this);

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
	if (pEvent->eventType == ET_LICENSE_CHANGED)
		UpdateCaptionText();
	else
		CMPSkinWnd::OnEvent(pEvent);
}

void CMPSkinMainWndBase::UpdateCaptionText()
{
	if (g_licenseManager.IsLicenseValid())
		SetCaptionText(SZ_APP_NAME);
	else
	{
		CStrPrintf strCaption(_T("%s (%s)"), SZ_APP_NAME, TOLOCAL_T("Unactivated"));
		SetCaptionText(strCaption.c_str());
	}
}
