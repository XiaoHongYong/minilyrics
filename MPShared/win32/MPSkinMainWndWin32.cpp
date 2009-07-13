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

#ifndef _WIN32_WCE
const UINT g_msgTaskBarCreated = RegisterWindowMessage(_T("TaskBarCreated"));
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
#if defined(_MPLAYER) && defined(_WIN32)
	LPCTSTR			szCmdLine;

	// get command line
	szCmdLine = GetCommandLine();
#ifndef _WIN32_WCE
	szCmdLine = CmdLineNext(szCmdLine);
#endif
	if (IsEmptyString(szCmdLine))
	{
		// Load default playlist
		tstring		strDefPlaylist;
		tstring		strCurrentPlaylist;

		NewPlaylistName(strDefPlaylist);
		strCurrentPlaylist = g_Profile.GetProfileString(_T("Latest Playlist"), strDefPlaylist.c_str());
		if (g_Player.LoadPlaylist(strCurrentPlaylist.c_str()))
			g_Player.m_strCurrentPlaylist = strCurrentPlaylist;
		else
			g_Player.m_strCurrentPlaylist = strDefPlaylist;

		int	nNowPlaying = g_Profile.GetProfileInt(SZ_SECT_PLAYER, _T("NowPlayingIdx"), 0);
		g_Player.m_spPlayer->SetCurrentMediaInPlaylist(nNowPlaying);
	}
	else
	{
		// excute cmdline
		SendCommandLine(m_hWnd, szCmdLine);
	}

	if (g_Profile.GetProfileBool(_T("StartPlayAtStartup"), false))
		g_Player.Play();
#endif

	if (!CMPSkinMainWndBase::OnCreate())
		return false;

	g_eventDispather.RegisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
	g_eventDispather.RegisterHanlder(ET_PLAYER_CUR_MEDIA_INFO_CHANGED, this);

	m_mlTrayIcon.Init(this);

	return true;
}

void CMPSkinMainWnd::OnDestroy()
{
	g_eventDispather.UnregisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
	g_eventDispather.UnregisterHanlder(ET_PLAYER_CUR_MEDIA_INFO_CHANGED, this);
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
			_tcscpy_safe(szCaption, CountOf(szCaption), SZ_ML_VERSION);

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
		CMPSkinWnd::OnEvent(pEvent);
}

void CMPSkinMainWnd::OnCopyData(HWND hWnd, PCOPYDATASTRUCT pCopyData)
{
#ifdef _MPLAYER
	vector<tstring> vCmdLine;
	LPCTSTR			szCmdLine;
	int				i;

	if (pCopyData->dwData == MC_OPEN_FILES)
	{
		szCmdLine = (LPCTSTR)pCopyData->lpData;
		CmdLineAnalysis(szCmdLine, vCmdLine);

		// deal with option cmd line
		for (i = 0; i < (int)vCmdLine.size(); i++)
		{
			tstring		&str = vCmdLine[i];
			if (str[0] == '/' || str[0] == '-')
			{
				vCmdLine.erase(vCmdLine.begin() + i);
				i--;
			}
		}

		if (vCmdLine.empty())
			return;

		if (vCmdLine.size() == 1 && FileIsExtSame(vCmdLine[0].c_str(), _T(".m3u")))
		{
			// Open playlist
			if (g_Player.m_bPlModified)
				SaveCurrentPlaylist();

			g_Player.m_strCurrentPlaylist = vCmdLine[0];
			g_Player.SetPlaylistModified(false);
		}
		else
		{
			GetDefaultPlaylistName(g_Player.m_strCurrentPlaylist);
			g_Player.SetPlaylistModified(true);
		}

		g_Player.ClearPlaylist();

		for (i = 0; i < (int)vCmdLine.size(); i++)
		{
			g_Player.AddToPlaylist(vCmdLine[i].c_str());
		}

		g_Player.Play();
	}
#endif
}

LRESULT CMPSkinMainWnd::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COPYDATA:
		OnCopyData(NULL, (PCOPYDATASTRUCT)lParam);
		break;
	case MPWM_TRAYICN:
		m_mlTrayIcon.OnMyNotifyIcon(wParam, lParam);
		return 0;
	case WM_HOTKEY:
		// OnHotKey(int nId, UINT fuModifiers, UINT uVirtKey)
		m_hotKey.OnHotKey((int)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
		return 0;
#ifndef _MPLAYER
	case WM_SYSCOMMAND:
		if (wParam == SC_RESTORE)
		{
			if (IsIconic())
				g_mpApp.GetSkinFactory()->RestoreAll();
		}
		break;
	case WM_MM_NOTIFY:
		{
			if (wParam == MM_WAKEUP)
				Activate();
		}
		return 0;
#endif
	default:
#ifndef _WIN32_WCE
		if (message == g_msgTaskBarCreated)
		{
			m_mlTrayIcon.UpdateShowIconPos();
			m_mlTrayIcon.UpdatePlayerSysTrayIcon();
		}
#endif
		break;
	}

	return CMPSkinWnd::WndProc(message, wParam, lParam);
}

#ifndef _MPLAYER
bool CMPSkinMainWnd::IsTopmost()
{
	HWND	hWndParent = GetRootParentWnd();
	if (hWndParent)
	{
		return Tobool(::IsTopmostWindow(hWndParent));
	}

	return CBaseWnd::IsTopmost();
}

bool CMPSkinMainWnd::IsIconic()
{
	HWND	hWndParent = GetRootParentWnd();
	if (hWndParent)
	{
		return Tobool(::IsIconic(hWndParent));
	}

	return CBaseWnd::IsIconic();
}

void CMPSkinMainWnd::Topmost(bool bTopmost)
{
	HWND	hWndParent = GetRootParentWnd();
	if (hWndParent)
	{
		::TopmostWindow(hWndParent, bTopmost);
		return;
	}

	CBaseWnd::SetTopmost(bTopmost);
}

extern bool				g_bAutoMinimized;

void CMPSkinMainWnd::Minimize()
{
	if (GetRootParentWnd())
		return;

	ShowWindow(SW_MINIMIZE);

	g_bAutoMinimized = FALSE;

	if (IsToolWindow())
		ShowWindow(SW_HIDE);
}

void CMPSkinMainWnd::Activate()
{
	HWND	hWndParent = GetRootParentWnd();
	if (hWndParent)
	{
		::ActivateWindow(hWndParent);
		return;
	}

	::ActivateWindow(m_hWnd);
}

HWND CMPSkinMainWnd::GetRootParentWnd()
{
	HWND		hParent;
	HWND		hTemp;

	hTemp = hParent = ::GetParent(m_hWnd);
	for (int i = 1; hTemp && i <= 3; i++)
	{
		hParent = hTemp;
		hTemp = ::GetParent(hTemp);
	}

	return hParent;
}
#endif
