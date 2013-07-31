// MPlayerApp.cpp: implementation of the CMPlayerApp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerAppBase.h"
#include "DownloadMgr.h"
#include "VersionUpdate.h"
#include "OnlineSearch.h"
#include "AutoProcessEmbeddedLyrics.h"
#include "MPFloatingLyrWnd.h"
#include "LyricShowObj.h"


CMPlayerApp::CMPlayerApp()
{
	m_bRunning = false;

	m_appMode = SA_LYRICS_PLAYER;
	m_appModeQuitToStart = SA_UNKNOWN;
}

CMPlayerApp::~CMPlayerApp()
{
}

bool CMPlayerApp::IsAnotherInstanceRunning()
{
	return false;
}

bool CMPlayerApp::SetRunningFlag()
{
	// Is Product already running?
	m_bRunning = true;

	return true;
}

bool CMPlayerApp::Init()
{
	g_Log.SetSrcRootDir(_T(__FILE__), 2);

	g_LangTool.SetMacro(SZ_MACRO_PRODUCT_NAME, SZ_APP_NAME);
	g_LangTool.SetMacro(SZ_MACRO_COMPANY_NAME, SZ_COMPANY_NAME);

	return CMPlayerAppBase::Init();
}

void RestartToLyricsDownloader();

void CMPlayerApp::Quit()
{
	if (!m_bRunning)
		return;
	
	m_bRunning = false;

	CMPlayerAppBase::Quit();
	
	g_playerMgr.Quit();
	
#ifndef _MPLAYER
	if (m_appModeQuitToStart == SA_IPOD_LYRICS_DOWNLOADER)
		RestartToLyricsDownloader();
#endif
}

void CMPlayerApp::RestartToAppMode(AppMode appMode)
{
	m_appModeQuitToStart = appMode;

	PostQuitMessageMac();
}

