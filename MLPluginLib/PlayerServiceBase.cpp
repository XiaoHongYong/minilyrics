#include "StdAfx.h"
#include "MLPlugin.h"
#include "PlayerServiceBase.h"

CPlayerServiceBase::CPlayerServiceBase(void)
{
}

CPlayerServiceBase::~CPlayerServiceBase(void)
{
}

bool CPlayerServiceBase::OnCopyData(WPARAM wParam, COPYDATASTRUCT *pcds)
{
	if (wParam != ML_MSG_VERSION)
		return false;

	// MessageToPlayer
	switch (pcds->dwData)
	{
	case M2P_QUERY_INFO:
		{
			ASSERT(pcds->cbData == sizeof(DWORD));

			// Backup m_mlInfo.hWndMLService.
			HWND	hWndMLServiceBackup = (HWND)g_MLService.m_mlInfo.hWndMLService;
			g_MLService.m_mlInfo.hWndMLService = (*(WND_HANDLE*)pcds->lpData);

			PlayerInfo playerInfo;
			memset(&playerInfo, 0, sizeof(playerInfo));
			playerInfo.startupFlags = ML_SUPPORT_PLAY_CONTROL | ML_SUPPORT_SEEK_CONTROL;
			playerInfo.hWndPluginService = (WND_HANDLE)m_hWnd;

			OnGetInfo(playerInfo);
			g_MLService.SendPlayerInfo(playerInfo);

			// Restore m_mlInfo.hWndMLService.
			g_MLService.m_mlInfo.hWndMLService = (WND_HANDLE)hWndMLServiceBackup;
		}
		break;
	case M2P_CONNECTED:
		{
			if (IsWindow((HWND)g_MLService.m_mlInfo.hWndMLService))
				break;

			ASSERT(pcds->cbData == sizeof(MLInfo));
			g_MLService.m_mlInfo = *(MLInfo*)pcds->lpData;

			g_MLService.m_hWndPlayerService = m_hWnd;

			OnConnected();
		}
		break;
	case M2P_DISCONNECT:
		OnDisconnect();
		memset(&g_MLService.m_mlInfo, 0, sizeof(g_MLService.m_mlInfo));
		break;
	case M2P_MAINWND_HANDLE:
		ASSERT(pcds->cbData == sizeof(WND_HANDLE));
		g_MLService.m_hWndMiniLyrics = (HWND)*(WND_HANDLE*)pcds->lpData;
		break;
	case M2P_PLAY:
		Play();
		break;
	case M2P_PAUSE:
		Pause();
		break;
	case M2P_PLAY_PAUSE:
		PlayPause();
		break;
	case M2P_STOP:
		Stop();
		break;
	case M2P_PREV:
		Prev();
		break;
	case M2P_NEXT:
		Next();
		break;
	case M2P_SEEK:
		ASSERT(pcds->cbData == sizeof(DWORD));
		SeekTo(*(DWORD*)pcds->lpData);
		break;
	case M2P_SWITCH_2_EMBEDDED_SKIN:
		SwitchToEmbeddedSkin();
		break;
	case M2P_END_EMBEDDED_SKIN:
		EndEmbeddedSkin();
		break;
	}

	return true;
}
