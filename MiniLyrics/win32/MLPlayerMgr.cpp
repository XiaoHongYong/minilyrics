#include "StdAfx.h"
#include "MPlayerApp.h"
#include "MLPlayerMgr.h"
#include "LyricsTool.h"
#include "MPMsg.h"
#include "PreferenceDlg.h"
#include "DlgAbout.h"
#include "DlgStartMiniLyrics.h"
#include "DlgSetupPlayerPlugins.h"
#include "../MLPlayerPluginAgentLib/MLPlayerPluginAgentLib.h"
#include "PlayerEventDispatcher.h"


CMLPlayerMgr			g_playerMgr;
HWND					g_hWndPlayerServiceOnStartup = NULL;


HWND GetMLSericeWindow()
{
	return g_playerMgr.GetHandle();
}

bool ProcessCmdLine(LPCTSTR szCmdLine, AppMode &appMode)
{
	// Parse command line
	V_Str		vCmdLine;
	CmdLineAnalysis(szCmdLine, vCmdLine);

	DBGLOG1(_T("Enter ProcessCmdLine: %s"), szCmdLine);

	for (size_t i = 0; i < vCmdLine.size(); i++)
	{
		if (_tcsicmp(vCmdLine[i].c_str(), _T("/Wnd")) == 0 && i + 1 < vCmdLine.size())
		{
			g_hWndPlayerServiceOnStartup = (HWND)HexToInt(vCmdLine[i + 1].c_str());
			if (!IsWindow(g_hWndPlayerServiceOnStartup)) {
				g_hWndPlayerServiceOnStartup = NULL;
			}
			i++;
		}
		else if (_tcsicmp(vCmdLine[i].c_str(), _T("/iPodLyricsDownloader")) == 0)
			appMode = SA_IPOD_LYRICS_DOWNLOADER;
		else if (_tcsicmp(vCmdLine[i].c_str(), _T("/Setup")) == 0)
		{
			CDlgSetupPlayerPlugins	dlg;
			return dlg.DoModal(NULL) == IDOK;
		}
		else if (_tcsicmp(vCmdLine[i].c_str(), _T("/Uninstall")) == 0)
		{
			// Uninstall MiniLyrics plugins for players.
			CPlayerPluginInstaller	installer;
			installer.Init();
			tstring strErr;
			installer.Uninstall(strErr);

			PostQuitMessage(0);
			return false;
		}
	}

	if (g_hWndPlayerServiceOnStartup != NULL)
	{
		if (!g_playerMgr.IsValid())
			g_playerMgr.Create();

		CPlayer::QueryInfo(g_hWndPlayerServiceOnStartup);
	}

	return true;
}

bool CMLPlayerMgr::Create()
{
	m_vPlayerPluginAgent.clear();

	RegisteredPlayerPluginAgent(m_vPlayerPluginAgent);

	return CTinyBaseWnd::Create(GetAppInstance(), SZ_MINILYRICS_SERVICE_CLASS_NAME);
}

bool CMLPlayerMgr::ConnectToPlayer()
{
	if (!IsValid())
	{
		if (!Create())
			return false;
	}

	if (m_listPlayers.size() == 1)
	{
		// Player info is specified in the command line, 
		// Check whether MiniLyrics shall start with it?
		PlayerInfo &playerInfo = m_listPlayers.front();

		if (!IsEmptyString(playerInfo.playerName))
		{
			if (!g_Profile.GetProfileBool(playerInfo.playerName, true))
				return false;
		}
	}

	if (m_listPlayers.empty())
		QueryRunningPlayerInfo();

	if (m_listPlayers.size() == 1)
		return ConnectToPlayer(m_listPlayers.front().hWndPluginService);
	else
	{
		// Give a second chance to check g_hWndPlayerServiceOnStartup
		if (IsWindow(g_hWndPlayerServiceOnStartup)
			&& ConnectToPlayer(g_hWndPlayerServiceOnStartup))
			return true;

		// Popup and let user to choose the player.
		CDlgStartMiniLyrics		dlg;
		return dlg.DoModal(NULL) == IDOK;
	}
}

void CMLPlayerMgr::OnPlayerInfoReturned(PlayerInfo *playerInfo)
{
	// Add it in the list of players.
	ListPlayerInfo::iterator it = m_listPlayers.begin();
	for (; it != m_listPlayers.end(); ++it)
	{
		PlayerInfo &item = *it;
		if (item.hWndPluginService == playerInfo->hWndPluginService)
			return;
	}
	if (it == m_listPlayers.end())
		m_listPlayers.push_back(*playerInfo);
}

void CMLPlayerMgr::OnPlayerDisconnected()
{
	g_Player.Disconnect();

	memset(&g_Player.m_playerInfo, 0, sizeof(g_Player.m_playerInfo));
	CMPlayerAppBase::GetInstance()->PostQuitMessage();
}

bool CMLPlayerMgr::ConnectToPlayer(HWND hWndPluginService)
{
	ListPlayerInfo::iterator it = m_listPlayers.begin();
	for (; it != m_listPlayers.end(); ++it)
	{
		if ((*it).hWndPluginService == hWndPluginService)
			break;
	}

	if (it == m_listPlayers.end())
		return false;

	PlayerInfo &playerInfo = *it;
	m_hWndCurPlugin = playerInfo.hWndPluginService;
	g_Player.Connect(playerInfo);

	// Turn off not connected player agent.
	for (int i = 0; i < (int)m_vPlayerPluginAgent.size(); i++)
	{
		IPlayerPluginAgent *pAgent = m_vPlayerPluginAgent[i];
		if (!pAgent->IsConnected())
			pAgent->TurnOff();
	}

	return true;
}

void CMLPlayerMgr::QueryRunningPlayerInfo()
{
	HWND	hWndPluginService = NULL;

	m_listPlayers.clear();

	// Turn on player agent, if it's running.
	for (int i = 0; i < (int)m_vPlayerPluginAgent.size(); i++)
	{
		IPlayerPluginAgent *pAgent = m_vPlayerPluginAgent[i];
		if (pAgent->IsPlayerRunning())
			pAgent->TurnOn();
		else
			pAgent->TurnOff();
	}

	while (1)
	{
		hWndPluginService = FindWindowEx(NULL, hWndPluginService, SZ_PLUGIN_SERVICE_CLASS_NAME, NULL);
		if (hWndPluginService == NULL)
			break;

		CPlayer::QueryInfo(hWndPluginService);
	}
}

bool CMLPlayerMgr::OnCopyData(WPARAM wParam, COPYDATASTRUCT *pcds)
{
	if (wParam == ML_MSG_VERSION && pcds->dwData == M2M_PLAYER_INFO)
	{
		ASSERT(sizeof(PlayerInfo) == pcds->cbData);
		OnPlayerInfoReturned((PlayerInfo *)pcds->lpData);
		return true;
	}
	else if (wParam != (WPARAM)GetCurPluginServiceWnd())
	{
		DBGLOG1(_T("Unknown WM_COPYDATA verify code: %X"), wParam);
		return true;
	}

	// MessageToPlayer
	switch (pcds->dwData)
	{
	case M2M_DISCONNECTED:
		OnPlayerDisconnected();
		break;
	case M2M_MEDIA_CHANGED:
		g_Player.OnMediaInfoChangedOfMiniLyrics((MLMediaInfo *)pcds->lpData);
		break;
	case M2M_PLAYER_STAT_CHANGED:
		{
			ASSERT(sizeof(DWORD) == pcds->cbData);
			CEventPlayerStatusChanged	*pEvent = new CEventPlayerStatusChanged();
			pEvent->eventType = ET_PLAYER_STATUS_CHANGED;
			g_Player.m_playerState = pEvent->status = (PLAYER_STATE) *(DWORD*)pcds->lpData;
			CMPlayerAppBase::GetEventsDispatcher()->DispatchUnsyncEvent(pEvent);
		}
		break;
	case M2M_PLAY_POS_CHANGED:
		{
			ASSERT(sizeof(DWORD) == pcds->cbData);
			DWORD	playPos = *(DWORD*)pcds->lpData;

			g_Player.m_dwPlayingPos = playPos;

			if (g_Player.IsUseSeekTimeAsPlayingTime())
				break;

			if (g_playerEventDispatcher.GetSmoothTimer().IsRunning())
				g_playerEventDispatcher.GetSmoothTimer().AdjustTimer(playPos);
			else
			{
				g_LyricData.SetPlayElapsedTime(playPos);

				CMPlayerAppBase::GetEventsDispatcher()->DispatchUnsyncEvent((EventType)ET_LYRICS_DRAW_UPDATE);

				DispatchPlayPosEvent(playPos);
			}
		}
		break;

	case M2M_CONFIG:
		{
			ASSERT(sizeof(HWND) == pcds->cbData);
			HWND	hWnd = *(HWND*)pcds->lpData;
			if (!CMPlayerAppBase::GetInstance()->IsRunning())
			{
				MessageBox(hWnd, _T("Sorry, MiniLyrics window is closed, can't configure it now."), SZ_APP_NAME, MB_ICONINFORMATION | MB_OK);
				break;
			}

			ShowPreferenceDialog(CMPlayerAppBase::GetMainWnd());
		}
		break;
	case M2M_ABOUT:
		{
			ASSERT(sizeof(HWND) == pcds->cbData);
			HWND	hWnd = *(HWND*)pcds->lpData;

			if (!CMPlayerAppBase::GetInstance()->IsRunning())
				break;

			CBaseWnd *pWndParent;
			if (hWnd == NULL)
				pWndParent = CMPlayerAppBase::GetMainWnd();
			else
				pWndParent = CBaseWnd::FromHandle(hWnd);

			ShowAboutDialog(pWndParent);
		}
		break;

	case M2M_LOG:
		{
			tstringex		str;
			ConvertStr((LPCWSTR)pcds->lpData, -1, str);
			ERRLOG1(_T("%s"), str.c_str());
		}
		break;
	case M2M_ADD_WND_CLOSE_TO:
		{
			ASSERT(sizeof(WndCloseToML) == pcds->cbData);
			WndCloseToML	*closeTo = (WndCloseToML*)pcds->lpData;
			if (closeTo->hWndCloseTo)
				CMPlayerAppBase::GetMPSkinFactory()->AddWndCloseto(CBaseWnd::FromHandle(closeTo->hWndCloseTo), NULL, NULL);
			else
			{
				TCHAR		szClass[256], szWndName[256];
				ConvertStr2(closeTo->className, -1, szClass, CountOf(szClass));
				ConvertStr2(closeTo->wndName, -1, szWndName, CountOf(szWndName));
				CMPlayerAppBase::GetMPSkinFactory()->AddWndCloseto(NULL, szWndName, szClass);
			}
		}
		break;
	case M2M_PLAYER_ACTIVATED:
		if (g_Profile.GetProfileBool(SZ_SECT_UI, _T("ActivateWithWinamp"), true))
		{
			ASSERT(sizeof(BOOL) == pcds->cbData);
			BOOL bActivate = *(BOOL*)pcds->lpData;
			if (bActivate)
				CMPlayerAppBase::GetMPSkinFactory()->OnSkinWndActivate(NULL);
		}
		break;
	case M2M_PLAYER_MINRESTORE:
		if (g_Profile.GetProfileBool(SZ_SECT_UI, _T("ActivateWithWinamp"), true))
		{
			ASSERT(sizeof(DWORD) == pcds->cbData);
			DWORD state = *(DWORD*)pcds->lpData;
			if (SIZE_MINIMIZED == state)
				CMPlayerAppBase::GetMPSkinFactory()->MinizeAll();
			else if (SIZE_RESTORED == state)
			{
				if (!g_Profile.GetProfileBool(SZ_SECT_UI, _T("AutoHideOnPlayMovie"), true)
					|| !g_Player.HasVideo())
				{
					CMPlayerAppBase::GetMPSkinFactory()->RestoreAll();
					CMPlayerAppBase::GetMPSkinFactory()->OnSkinWndActivate(NULL);
				}
			}
		}
		break;

	case M2M_BEFORE_TRACK_MOVE:
		{
			ASSERT(pcds->cbData > 0 && pcds->lpData);

			HWND *hWndTrackAndChain = (HWND *)pcds->lpData;
			int nCount = pcds->cbData / sizeof(HWND);
			CBaseWnd	*pWndTrackChain[20];
			if (nCount >= CountOf(pWndTrackChain))
				break;
			int			k = 0;
			for (int i = 0; i < nCount; i++)
			{
				if (hWndTrackAndChain[i])
				{
					pWndTrackChain[k] = CBaseWnd::FromHandle(hWndTrackAndChain[i]);
					k++;
				}
			}
			CMPlayerAppBase::GetMPSkinFactory()->BeforeTrackMoveWith(pWndTrackChain + 1, k - 1, pWndTrackChain[0]);
		}
		break;
	case M2M_TRACK_MOVE:
		{
			ASSERT(sizeof(DWORD) == pcds->cbData);
			DWORD	dwXY = *(DWORD*)pcds->lpData;
			CMPlayerAppBase::GetMPSkinFactory()->TrackMoveWith(
				CBaseWnd::FromHandle(g_Player.GetMainWnd()),
				(short)LOWORD(dwXY), (short)HIWORD(dwXY));
		}
		break;

	case M2M_PLAYER_SKIN_CHANGED:
		{
			ASSERT(sizeof(PlayerSkinInfo) == pcds->cbData);
			PlayerSkinInfo *skinInfo = (PlayerSkinInfo *)pcds->lpData;
			g_Player.m_skinInfo = *skinInfo;
			if (CMPlayerAppBase::GetInstance()->IsRunning() && CMPlayerAppBase::GetInstance()->IsEmbeddedMode())
				CMPlayerAppBase::GetInstance()->OnSwitchToEmbeddedSkinMode(true);
		}
		break;
	case M2M_SWITCH_2_EMBEDDED_SKIN:
		{
			ASSERT(sizeof(PlayerSkinInfo) == pcds->cbData);
			PlayerSkinInfo *skinInfo = (PlayerSkinInfo *)pcds->lpData;
			g_Player.m_skinInfo = *skinInfo;
			if (skinInfo->bEmbeddedSkin != CMPlayerAppBase::GetInstance()->IsEmbeddedMode())
			{
				if (skinInfo->bEmbeddedSkin)
					CMPlayerAppBase::GetInstance()->OnSwitchToEmbeddedSkinMode(true);
				else
				{
					CMPlayerAppBase::GetInstance()->EndEmbeddedSkin();
					CMPlayerAppBase::GetInstance()->GetSkinFactory()->ChangeSkin(SZ_DEFSKIN);
				}
			}
		}
		break;
	default:
		return false;
	}

	return true;
}
