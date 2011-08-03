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

const UINT g_msgMediaMonkeyEmbeder = RegisterWindowMessage(_T("MediaMonkeyEmbederToMiniLyrics"));

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

	CMPlayerAppBase::GetEventsDispatcher()->RegisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
	CMPlayerAppBase::GetEventsDispatcher()->RegisterHanlder(ET_PLAYER_CUR_MEDIA_INFO_CHANGED, this);

	m_mlTrayIcon.Init(this);

	return true;
}

void CMPSkinMainWnd::OnDestroy()
{
	CMPlayerAppBase::GetEventsDispatcher()->UnregisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
	CMPlayerAppBase::GetEventsDispatcher()->UnregisterHanlder(ET_PLAYER_CUR_MEDIA_INFO_CHANGED, this);
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

void CMPSkinMainWnd::OnCopyData(WPARAM wParam, PCOPYDATASTRUCT pCopyData)
{
#ifdef _MPLAYER
	vector<tstring> vCmdLine;
	LPCTSTR			szCmdLine;
	int				i;

	if (pCopyData->dwData != ML_SEND_CMD_LINE && wParam != ML_SEND_CMD_LINE)
		return;

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
		g_Player.SaveCurrentPlaylist();

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
#endif
}

LRESULT CMPSkinMainWnd::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COPYDATA:
		OnCopyData(wParam, (PCOPYDATASTRUCT)lParam);
		break;
	case MPWM_TRAYICN:
		m_mlTrayIcon.OnMyNotifyIcon(wParam, lParam);
		return 0;
	case WM_HOTKEY:
		// OnHotKey(int nId, UINT fuModifiers, UINT uVirtKey)
		CMPlayerAppBase::GetHotkey().OnHotKey((int)wParam, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
		return 0;
#ifndef _MPLAYER
	case WM_SYSCOMMAND:
		if (wParam == SC_RESTORE)
		{
			if (IsIconic())
				CMPlayerAppBase::GetMPSkinFactory()->RestoreAll();
		}
		break;
#endif
	default:
#ifndef _WIN32_WCE
		if (message == g_msgTaskBarCreated)
		{
			m_mlTrayIcon.UpdateShowIconPos();
			m_mlTrayIcon.UpdatePlayerSysTrayIcon();
		}
#endif
#ifndef _MPLAYER
		if (message == g_msgMediaMonkeyEmbeder)
		{
			CMPSkinMainWnd *pMainWnd = CMPlayerAppBase::GetMainWnd();
			if (pMainWnd && pMainWnd->GetEnableTranslucencyLayered())
				pMainWnd->EnableTranslucencyLayered(false);
			g_Player.m_playerInfo.startupFlags |= ML_EMBEDDED_WND_IMMOVABLE;
			g_Player.m_playerInfo.startupFlags &= ~(ML_SUPPORT_CLASSIC_SKINS | ML_SUPPORT_EMBEDDED);

			pMainWnd->m_WndDrag.EnableDrag(false);
			pMainWnd->m_wndResizer.FixedWidth(true);
			pMainWnd->m_wndResizer.FixedHeight(true);
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
