#include "StdAfx.h"
#include "MPlayerApp.h"
#include "MLPlayerMgr.h"
#include "PreferenceDlg.h"
#include "DlgAbout.h"
#include "PlayerEventDispatcher.h"
#include "../../MPShared/Player.h"


CMLPlayerMgr			g_playerMgr;
bool					g_bForceStart = false;

pthread_t	g_threadMsg = 0;
volatile bool g_bQuitMsg = false;


LPVOID CMLPlayerMgr::ThreadMsg(LPVOID param)
{
	CMLPlayerMgr *pMgr = (CMLPlayerMgr *)param;
	ASSERT(pMgr != NULL);
	DBGLOG0("Enter CMLPlayerMgr::ThreadMsg");
	
	string data;
	while (!g_bQuitMsg)
	{
		uint32 message = 0;
		if (!pMgr->m_msgPipeOfPlayer.Read(message, data))
			break;
		pMgr->OnMsg(message, data.c_str(), data.size());
	}
	
	return NULL;
}


void CMLPlayerMgr::QueryInfo(LPCTSTR szPlayerPipeName)
{
	g_playerMgr.m_msgPipeOfPlayer.Write(M2P_QUERY_INFO, NULL, 0);
}

bool CMLPlayerMgr::Init(int argc, char *argv[])
{
	DBGLOG0("Enter CMLPlayerMgr::Init");

	m_msgPipeOfMiniLyrics.Open(SZ_PIPE_MINILYRICS_NAME, true);

	pthread_create(&g_threadMsg, NULL, ThreadMsg, this);
	g_bQuitMsg = false;

	tstring strPlayerServicePipeName;

	for (int i = 0; i < argc; i++)
	{
		if (_tcsicmp(argv[i], _T("/Pipe")) == 0 && i + 1 < argc)
		{
			strPlayerServicePipeName = argv[i + 1];
			i++;
		}
		if (_tcsicmp(argv[i], _T("/Player")) == 0 && i + 1 < argc)
		{
			// g_strPlayerServicePipeName = argv[i + 1];
			i++;
		}
		// else if (_tcsicmp(argv[i], _T("/iPodLyricsDownloader")) == 0)
		//	appMode = SA_IPOD_LYRICS_DOWNLOADER;
		else if (_tcsicmp(argv[i], _T("/Manual")) == 0)
			g_bForceStart = true;
	}

	if (strPlayerServicePipeName.empty())
		return false;

	QueryInfo(strPlayerServicePipeName.c_str());

	g_Player.Connect(g_Player.m_playerInfo);

	return true;
}

bool CMLPlayerMgr::ConnectToPlayer()
{
	return m_msgPipeOfPlayer.IsOK();
}

void CMLPlayerMgr::SendMessageToPlayer(MessageToPlayer message, LPVOID param, size_t sizeParam)
{
	m_msgPipeOfPlayer.Write(message, param, sizeParam);
}


void CMLPlayerMgr::OnPlayerInfoReturned(PlayerInfo *playerInfo)
{
	g_Player.m_playerInfo = *playerInfo;
}

void CMLPlayerMgr::OnPlayerDisconnected()
{
	g_Player.Disconnect();

	memset(&g_Player.m_playerInfo, 0, sizeof(g_Player.m_playerInfo));
	CMPlayerAppBase::GetInstance()->PostQuitMessage();
}


bool CMLPlayerMgr::OnMsg(uint32 msg, LPCVOID lpData, size_t lenData)
{
	DBGLOG1("Got service message from MiniLyrics: %d", msg);

	// MessageToPlayer
	switch (msg)
	{
	case M2M_PLAYER_INFO:
		ASSERT(sizeof(PlayerInfo) == lenData);
		OnPlayerInfoReturned((PlayerInfo *)lpData);
		break;
	case M2M_DISCONNECTED:
		OnPlayerDisconnected();
		break;
	case M2M_MEDIA_CHANGED:
		g_Player.OnMediaInfoChangedOfMiniLyrics((MLMediaInfo *)lpData);
		break;
	case M2M_PLAYER_STAT_CHANGED:
		{
			ASSERT(sizeof(DWORD) == lenData);
			CEventPlayerStatusChanged	*pEvent = new CEventPlayerStatusChanged();
			pEvent->eventType = ET_PLAYER_STATUS_CHANGED;
			g_Player.m_playerState = pEvent->status = (PLAYER_STATE) *(DWORD*)lpData;
			CMPlayerAppBase::GetEventsDispatcher()->DispatchUnsyncEvent(pEvent);
		}
		break;
	case M2M_PLAY_POS_CHANGED:
		{
			ASSERT(sizeof(DWORD) == lenData);
			DWORD	playPos = *(DWORD*)lpData;

			g_Player.m_dwPlayingPos = playPos;

			if (g_Player.IsUseSeekTimeAsPlayingTime())
				break;

			g_LyricData.SetPlayElapsedTime(playPos);

			CMPlayerAppBase::GetEventsDispatcher()->DispatchUnsyncEvent((EventType)ET_LYRICS_DRAW_UPDATE);

			DispatchPlayPosEvent(playPos);
		}
		break;

	case M2M_CONFIG:
		{
			if (!CMPlayerAppBase::GetInstance()->IsRunning())
			{
				break;
			}
			ShowPreferenceDialog(CMPlayerAppBase::GetMainWnd());
		}
		break;
	case M2M_ABOUT:
		{
			if (!CMPlayerAppBase::GetInstance()->IsRunning())
				break;

			ShowAboutDialog(CMPlayerAppBase::GetMainWnd());
		}
		break;

	case M2M_LOG:
		{
			tstringex		str;
			ConvertStr((LPCWSTR)lpData, -1, str);
			ERRLOG1(_T("%s"), str.c_str());
		}
		break;
	case M2M_PLAYER_ACTIVATED:
		if (g_Profile.GetProfileBool(SZ_SECT_UI, _T("ActivateWithWinamp"), true))
		{
			ASSERT(sizeof(BOOL) == lenData);
			BOOL bActivate = *(BOOL*)lpData;
			if (bActivate)
				CMPlayerAppBase::GetMPSkinFactory()->OnSkinWndActivate(NULL);
		}
		break;
	default:
		return false;
	}

	return true;
}
