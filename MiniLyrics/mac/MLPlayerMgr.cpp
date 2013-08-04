#include "StdAfx.h"
#include "MPlayerApp.h"
#include "MLPlayerMgr.h"
#include "PreferenceDlg.h"
#include "DlgAbout.h"
#include "PlayerEventDispatcher.h"
#include "../../MPShared/Player.h"
#include "PlayerPluginInstaller.h"
#include <pthread.h>

#ifdef DEBUG

LPCSTR SZ_PIPE_NAME = "/tmp/ml_+_test.pipe";
pthread_t	g_threadMsgTest = 0;
volatile bool g_bQuitMsgTest = false;

uint32 _msg_id = 5;//0x0102FFFE;
byte _data[4] = { 0xf9, 0x4d, 0xdb, 0x0 };

void Log(LPCTSTR szFormat, ...)
{
	static int _fd = -1;
	if (_fd == -1)
	{
		LPCTSTR SZ_PIPE_NAME = "/tmp/debug.pipe";
		int nRet = mkfifo(SZ_PIPE_NAME, S_IRWXO | S_IRWXG | S_IRWXU);
		if (nRet != 0 && errno != EEXIST)
			return;
		
		_fd = ::open(SZ_PIPE_NAME, O_NONBLOCK | O_WRONLY);
		if (_fd == -1)
			return;
	}
	
	va_list		args;
	va_start(args, szFormat);
	TCHAR buf[1024];
	int nBuf = _vstprintf_s(buf, CountOf(buf), szFormat, args);
	va_end(args);
	
	if (nBuf > 0) {
		::write(_fd, buf, nBuf);
		::write(_fd, "\n", 1);
	}
}

LPVOID ThreadMsgTest(LPVOID param)
{
	CMsgPipe	pipeRead;
	ASSERT(pipeRead.Open(SZ_PIPE_NAME, true));

	string data;
	for (int i = 0; i < 10; i++)
	{
		uint32 message = 0;
		if (!pipeRead.Read(message, data))
			break;
		ASSERT(message == _msg_id);
		ASSERT(data.size() == sizeof(_data));
		ASSERT(memcmp(data.c_str(), _data, CountOf(_data)) == 0);
	}

	return NULL;
}

void TestMsgPipe()
{
	CMsgPipe	pipeWrite;
	ASSERT(pipeWrite.Open(SZ_PIPE_NAME, false));
	
	pthread_create(&g_threadMsgTest, NULL, ThreadMsgTest, NULL);

	for (int i = 0; i < 10; i++)
	{
		Sleep(10000000);
		pipeWrite.Write(_msg_id, _data, sizeof(_data));
	}
	pipeWrite.Close();
}
#endif // DEBUG


CMLPlayerMgr			g_playerMgr;
bool					g_bForceStart = false;

pthread_t	g_threadMsg = 0;
volatile bool g_bQuitMsg = false;


LPVOID CMLPlayerMgr::ThreadMsg(LPVOID param)
{
	CMLPlayerMgr *pMgr = (CMLPlayerMgr *)param;
	ASSERT(pMgr != NULL);
	
	string data;
	while (!g_bQuitMsg)
	{
		uint32 message = 0;
		if (!pMgr->m_msgPipeOfMiniLyrics.Read(message, data))
			break;
		pMgr->OnMsg(message, data.c_str(), data.size());
	}

	return NULL;
}


void CMLPlayerMgr::QueryInfo()
{
	g_playerMgr.m_msgPipeOfPlayer.Write(M2P_QUERY_INFO, NULL, 0);
}

int MessageOut(LPCTSTR lpText, UINT uType, LPCTSTR lpCaption);

bool CMLPlayerMgr::Init(int argc, char *argv[])
{
	DBGLOG0("Enter CMLPlayerMgr::Init");

#ifdef DEBUG
	// TestMsgPipe();
#endif // DEBUG

	m_msgPipeOfMiniLyrics.Open(SZ_PIPE_MINILYRICS_NAME, true);

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
		else if (_tcsicmp(argv[i], _T("/Manual")) == 0)
			g_bForceStart = true;
	}
	
	if (!strPlayerServicePipeName.empty())
	{
		// Check whether MiniLyrics shall start with the player.
		// PipeName: /tmp/MiniLyrics_xxxx.pipe
		LPCTSTR SZ_PREFIX = _T("/tmp/MiniLyrics_");
		LPCTSTR SZ_POSTFIX = _T(".pipe");
		if (StrBeginWith(strPlayerServicePipeName.c_str(), SZ_PREFIX) && StrEndWith(strPlayerServicePipeName.c_str(), SZ_POSTFIX))
		{
			tstring playerName(strPlayerServicePipeName.c_str() + _tcslen(SZ_PREFIX), strPlayerServicePipeName.size() - _tcslen(SZ_PREFIX) - _tcslen(SZ_POSTFIX));
			if (!g_Profile.GetBool(playerName.c_str(), true))
				return false;
		}
	}

	// if (strPlayerServicePipeName.empty())
	{
		CPlayerPluginInstaller installer;
		
		installer.Init();
		for (int i = 0; ; i++) {
			CPlayerPluginInstaller::Player *player = installer.GetPlayerByIndex(i);
			if (player == NULL)
				break;
			player->DetectPlayerPath();
			if (player->IsInstalled()) {
				tstring strErr;
				if (player->CheckPluginInstalled(strErr))
					continue;
				int ret = MessageOut(CStrPrintf("Do you want to install player plugin for %s?", player->szPlayerName).c_str(), MB_YESNO, "MiniLyrics");
				if (ret != IDYES)
					continue;

				int err;
				if (!player->Install(strErr, err)) {
					tstring title = "Failed to install player plugin.";
					MessageOut(strErr.c_str(), MB_OK, title.c_str());
				}
			}
		}

		// On mac only iTunes is supported now.
		if (strPlayerServicePipeName.empty())
			strPlayerServicePipeName = "/tmp/MiniLyrics_iTunes.pipe";
	}
	
	bool ret = m_msgPipeOfPlayer.Open(strPlayerServicePipeName.c_str(), false);
	LOG2(LOG_LVL_INFO, _T("Open player service pipe: %s. Result: %d"), strPlayerServicePipeName.c_str(), ret);

	return true;
}

void CMLPlayerMgr::Quit()
{
	g_bQuitMsg = true;

	m_msgPipeOfPlayer.Close();
	
	if (g_threadMsg != 0)
	{
		LOG0(LOG_LVL_INFO, _T("Close player service pipe."));
		// pthread_kill(g_threadMsg, SIGINT);
		// pthread_join(g_threadMsg, NULL);
		// pthread_wait(g_threadMsg,
	}
}


bool CMLPlayerMgr::ConnectToPlayer()
{
	g_bQuitMsg = false;
	pthread_create(&g_threadMsg, NULL, ThreadMsg, this);
	
	QueryInfo();
	
	g_Player.Connect(g_Player.m_playerInfo);
	
	return true;
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
	g_bQuitMsg = true;

	g_Player.Disconnect();

	memset(&g_Player.m_playerInfo, 0, sizeof(g_Player.m_playerInfo));
	CMPlayerAppBase::GetInstance()->PostQuitMessage();
}


bool CMLPlayerMgr::OnMsg(uint32 msg, LPCVOID lpData, size_t lenData)
{
	// DBGLOG2("Got message from Player: %d, len: %d", msg, lenData);

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
			if (sizeof(DWORD) != lenData)
				break;
			DWORD	playPos = *(DWORD*)lpData;
			
			// DBGLOG1("M2M_PLAY_POS_CHANGED: %d", playPos);

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
		if (g_Profile.GetBool(SZ_SECT_UI, _T("ActivateWithWinamp"), true))
		{
			ASSERT(sizeof(BOOL) == lenData);
			BOOL bActivate = *(BOOL*)lpData;
			if (bActivate)
				CMPlayerAppBase::GetMPSkinFactory()->OnSkinWndActivate(NULL);
		}
		break;
	default:
		DBGLOG2("Unknown message from Player: %d, len: %d", msg, lenData);
		return false;
	}

	return true;
}
