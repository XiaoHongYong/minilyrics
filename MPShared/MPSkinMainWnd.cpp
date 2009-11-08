// MPSkinMainWnd.cpp: implementation of the CMPSkinMainWndBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "MPSkinMainWnd.h"
#include "RegistrationDlg.h"
#include "VersionUpdate.h"

#define TIMER_SHOW_REG				1
#define TIMER_REMOVE_ERROR_INFO		2

#define TIME_OUT_REMOVE_ERROR_INFO		(1000 * 40)

void ShowRegisterNoticeSafely()
{
	g_mpApp.GetMainWnd()->SetTimer(TIMER_SHOW_REG, 500);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
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

	g_eventDispather.RegisterHanlder(ET_UI_SETTINGS_CHANGED, this);
	g_eventDispather.RegisterHanlder(ET_UI_LONG_ERROR_TEXT, this);

	SetTimer(TIMER_SHOW_REG, 500);

	return true;
}

void CMPSkinMainWndBase::OnDestroy()
{
	g_eventDispather.UnregisterHanlder(ET_UI_SETTINGS_CHANGED, this);
	g_eventDispather.UnregisterHanlder(ET_UI_LONG_ERROR_TEXT, this);

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
			g_mpApp.GetSkinFactory()->TopmostAll(IsTRUE(pEvent->strValue.c_str()));
		else
			CMPSkinWnd::OnEvent(pEvent);
	}
	else if (pEvent->eventType == ET_UI_LONG_ERROR_TEXT)
	{
		// Add error info in lyrics.
		vector<tstring>		vStr;

		StrSplit(pEvent->strValue.c_str(), '\n', vStr);
		for (int i = 0; i < vStr.size(); i++)
			TrimStr(vStr[i], '\r');
		g_LyricData.AddTextInLyrics(vStr, LSA_SHOW_SIMPLE | LSA_ERROR_INFO);

		KillTimer(TIMER_REMOVE_ERROR_INFO);
		SetTimer(TIMER_REMOVE_ERROR_INFO, TIME_OUT_REMOVE_ERROR_INFO);
		g_mpApp.DispatchLyricsChangedSyncEvent();
	}
	else
		CMPSkinWnd::OnEvent(pEvent);
}

void CMPSkinMainWndBase::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == TIMER_SHOW_REG)
	{
		KillTimer(nIDEvent);

		if (g_Player.HasVideo())
			return;

		//
		// 检查是否过期
		CVersion		verCur(MAJOR_VERSION, MINOR_VERSION, BUILD_NUMBER);
		CVersion		verObs;
		if (verObs.From(g_Profile.GetProfileString(_T("ObsoleteVersion"), _T("0.0.0"))) &&
			verCur <= verObs)
			ShowVersionExpiredDlg(this);

		ShowRegisterNotice(this, false);
	}
	else if (nIDEvent == TIMER_REMOVE_ERROR_INFO)
	{
		KillTimer(TIMER_REMOVE_ERROR_INFO);
		g_LyricData.RemoveErrrorInfo();
	}
	else
		CMPSkinWnd::OnTimer(nIDEvent);
}
