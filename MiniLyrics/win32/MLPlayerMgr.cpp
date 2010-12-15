#include "StdAfx.h"
#include "MPlayerApp.h"
#include "MLPlayerMgr.h"
#include "LyricsTool.h"
#include "MPMsgWnd.h"
#include "MPMsg.h"
#include "PreferenceDlg.h"
#include "DlgAbout.h"

CMLPlayerMgr			g_playerMgr;

bool ProcessCmdLine(bool bFirstRun, LPCTSTR szCmdLine);

bool CMLPlayerMgr::Create()
{
	return CTinyBaseWnd::Create(GetAppInstance(), SZ_MINILYRICS_SERVICE_CLASS_NAME);
}

void CMLPlayerMgr::OnPlayerInfoReturned(PlayerInfo *playerInfo)
{
	// Add it in the list of players.
	ListPlayerInfo::iterator it = m_listPlayers.begin();
	for (; it != m_listPlayers.end(); ++it)
	{
		PlayerInfo &item = *it;
		if (item.hWndPluginService == playerInfo->hWndPluginService)
			break;
	}
	if (it == m_listPlayers.end())
		m_listPlayers.push_back(*playerInfo);
}

void CMLPlayerMgr::OnPlayerDisconnected()
{
	memset(&g_Player.m_playerInfo, 0, sizeof(g_Player.m_playerInfo));
	CDlgLyricsTool *pLyricsTool = GetRunningLyricsToolWnd();
	if (!pLyricsTool)
		g_mpMsgWnd.PostQuitMPMessage();
}

void CMLPlayerMgr::SetCurPlayer(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_listPlayers.size())
		return;

	PlayerInfo &playerInfo = m_listPlayers[nIndex];
	m_hWndCurPlugin = playerInfo.hWndPluginService;
	g_Player.Connect(playerInfo);

	// Turn off not connected player agent.
	for (int i = 0; i < (int)m_vPlayerPluginAgent.size(); i++)
	{
		IPlayerPluginAgent *pAgent = m_vPlayerPluginAgent[i];
		if (!pAgent->IsConnected())
			pAgent->TurnOff();
	}
}

void CMLPlayerMgr::QueryRunningPlayerInfo()
{
	HWND	hWndPluginService = NULL;

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
	if (wParam == ML_SEND_CMD_LINE && pcds->dwData == ML_SEND_CMD_LINE)
	{
		ProcessCmdLine(false, (LPCTSTR)pcds->lpData);
	}
	else if (wParam == ML_MSG_VERSION && pcds->dwData == M2M_PLAYER_INFO)
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
		g_Player.OnMediaInfoChanged((MLMediaInfo *)pcds->lpData);
		break;
	case M2M_PLAYER_STAT_CHANGED:
		{
			ASSERT(sizeof(DWORD) == pcds->cbData);
			CEventPlayerStatusChanged	*pEvent = new CEventPlayerStatusChanged();
			pEvent->eventType = ET_PLAYER_STATUS_CHANGED;
			g_Player.m_playerState = pEvent->status = (PLAYER_STATE) *(DWORD*)pcds->lpData;
			g_eventDispather.DispatchUnsyncEvent(pEvent);
		}
		break;
	case M2M_PLAY_POS_CHANGED:
		{
			ASSERT(sizeof(DWORD) == pcds->cbData);
			DWORD	playPos = *(DWORD*)pcds->lpData;

			g_Player.m_dwPlayingPos = playPos;

			if (g_Player.IsUseSeekTimeAsPlayingTime())
				break;

			if (g_Player.m_playerSmoothTimer.IsRunning())
				g_Player.m_playerSmoothTimer.AdjustTimer(playPos);
			else
			{
				g_LyricData.SetPlayElapsedTime(playPos);

				g_eventDispather.DispatchUnsyncEvent(ET_LYRICS_DRAW_UPDATE);

				DispatchPlayPosEvent(playPos);
			}
		}
		break;

	case M2M_CONFIG:
		{
			ASSERT(sizeof(HWND) == pcds->cbData);
			HWND	hWnd = *(HWND*)pcds->lpData;
			if (!g_mpApp.IsRunning())
			{
				MessageBox(hWnd, _T("Sorry, MiniLyrics window is closed, can't configure it now."), SZ_APP_NAME, MB_ICONINFORMATION | MB_OK);
				break;
			}

			CBaseWnd *pWndParent;
			if (hWnd == NULL)
				pWndParent = g_mpApp.GetMainWnd();
			else
				pWndParent = CBaseWnd::FromHandle(hWnd);

			CMPSkinDialog::ShowDialog(new CPreferenceDlg(false), pWndParent);
		}
		break;
	case M2M_ABOUT:
		{
			ASSERT(sizeof(HWND) == pcds->cbData);
			HWND	hWnd = *(HWND*)pcds->lpData;

			if (!g_mpApp.IsRunning())
				break;

			CBaseWnd *pWndParent;
			if (hWnd == NULL)
				pWndParent = g_mpApp.GetMainWnd();
			else
				pWndParent = CBaseWnd::FromHandle(hWnd);

			CMPSkinDialog::ShowDialog(new CDlgAbout(), pWndParent);
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
				g_mpApp.GetSkinFactory()->AddWndCloseto(CBaseWnd::FromHandle(closeTo->hWndCloseTo), NULL, NULL);
			else
			{
				TCHAR		szClass[256], szWndName[256];
				ConvertStr2(closeTo->className, -1, szClass, CountOf(szClass));
				ConvertStr2(closeTo->wndName, -1, szWndName, CountOf(szWndName));
				g_mpApp.GetSkinFactory()->AddWndCloseto(NULL, szWndName, szClass);
			}
		}
		break;
	case M2M_PLAYER_ACTIVATED:
		if (g_Profile.GetProfileBool(SZ_SECT_UI, _T("ActivateWithWinamp"), true))
		{
			ASSERT(sizeof(BOOL) == pcds->cbData);
			BOOL bActivate = *(BOOL*)pcds->lpData;
			if (bActivate)
				g_mpApp.GetSkinFactory()->OnSkinWndActivate(NULL);
		}
		break;
	case M2M_PLAYER_MINRESTORE:
		if (g_Profile.GetProfileBool(SZ_SECT_UI, _T("ActivateWithWinamp"), true))
		{
			ASSERT(sizeof(DWORD) == pcds->cbData);
			DWORD state = *(DWORD*)pcds->lpData;
			if (SIZE_MINIMIZED == state)
				g_mpApp.GetSkinFactory()->MinizeAll();
			else if (SIZE_RESTORED == state)
			{
				if (!g_Profile.GetProfileBool(SZ_SECT_UI, _T("AutoHideOnPlayMovie"), true)
					|| !g_Player.HasVideo())
				{
					g_mpApp.GetSkinFactory()->RestoreAll();
					g_mpApp.GetSkinFactory()->OnSkinWndActivate(NULL);
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
			g_mpApp.GetSkinFactory()->BeforeTrackMoveWith(pWndTrackChain + 1, k - 1, pWndTrackChain[0]);
		}
		break;
	case M2M_TRACK_MOVE:
		{
			ASSERT(sizeof(DWORD) == pcds->cbData);
			DWORD	dwXY = *(DWORD*)pcds->lpData;
			g_mpApp.GetSkinFactory()->TrackMoveWith(
				CBaseWnd::FromHandle(g_Player.GetMainWnd()),
				LOWORD(dwXY), HIWORD(dwXY));
		}
		break;

	case M2M_PLAYER_SKIN_CHANGED:
		{
			ASSERT(sizeof(PlayerSkinInfo) == pcds->cbData);
			PlayerSkinInfo *skinInfo = (PlayerSkinInfo *)pcds->lpData;
			g_Player.m_skinInfo = *skinInfo;
			if (g_mpApp.IsRunning() && g_mpApp.IsEmbeddedMode())
				g_mpApp.OnSwitchToEmbeddedSkinMode(true);
		}
		break;
	case M2M_SWITCH_2_EMBEDDED_SKIN:
		{
			ASSERT(sizeof(PlayerSkinInfo) == pcds->cbData);
			PlayerSkinInfo *skinInfo = (PlayerSkinInfo *)pcds->lpData;
			g_Player.m_skinInfo = *skinInfo;
			if (g_mpApp.IsRunning() && skinInfo->bEmbeddedSkin != g_mpApp.IsEmbeddedMode())
			{
				if (skinInfo->bEmbeddedSkin)
					g_mpApp.OnSwitchToEmbeddedSkinMode(true);
				else
					g_mpApp.EndEmbeddedSkin();
			}
		}
		break;
	default:
		return false;
	}

	return true;
}
