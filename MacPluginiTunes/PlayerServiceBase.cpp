#include <stdio.h>
#include "../base/base.h"
#include "MLService.h"


pthread_t	g_threadMsg = 0;
volatile bool g_bQuitMsg = false;


LPVOID CPlayerServiceBase::ThreadMsg(LPVOID param)
{
	CPlayerServiceBase *pService = (CPlayerServiceBase *)param;
	ASSERT(pService != NULL);
	Log("Enter CPlayerServiceBase::ThreadMsg");

	string data;
	uint32 msg = 0;
	while (!g_bQuitMsg)
	{
		if (!pService->m_msgPipe.Read(msg, data))
			break;
		pService->OnMsg(msg, data.c_str(), data.size());
	}
	
	return NULL;
}


CPlayerServiceBase::CPlayerServiceBase(void)
{
}

CPlayerServiceBase::~CPlayerServiceBase(void)
{
}

bool CPlayerServiceBase::Create(LPCSTR szPipeName)
{
	Log("CPlayerServiceBase::Create()");

	m_msgPipe.Open(szPipeName, true);

	pthread_create(&g_threadMsg, NULL, ThreadMsg, this);
	g_bQuitMsg = false;

	return true;
}

void CPlayerServiceBase::Destroy()
{
	if (g_threadMsg != 0)
	{
		g_bQuitMsg = true;
		m_msgPipe.Close();
		pthread_kill(g_threadMsg, SIGINT);
		// pthread_join(g_threadMsg, NULL);
		g_threadMsg = 0;
	}
	
	m_msgPipe.Close();
}

bool CPlayerServiceBase::OnMsg(uint32 msg, LPCVOID lpData, size_t lenData)
{
	Log("Got service message from MiniLyrics: %d, len: %d", msg, lenData);

	// MessageToPlayer
	switch (msg)
	{
	case M2P_QUERY_INFO:
		{
			// ASSERT(lenData == sizeof(HWND));

			PlayerInfo playerInfo;
			memset(&playerInfo, 0, sizeof(playerInfo));
			playerInfo.startupFlags = ML_SUPPORT_PLAY_CONTROL | ML_SUPPORT_SEEK_CONTROL;

			OnGetInfo(playerInfo);
			g_MLService.SendPlayerInfo(playerInfo);
		}
		break;
	case M2P_CONNECTED:
		{
			ASSERT(lenData == sizeof(MLInfo));
			g_MLService.m_mlInfo = *(MLInfo*)lpData;

			OnConnected();
		}
		break;
	case M2P_DISCONNECT:
		OnDisconnect();
		memset(&g_MLService.m_mlInfo, 0, sizeof(g_MLService.m_mlInfo));
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
		ASSERT(lenData == sizeof(DWORD));
		SeekTo(*(DWORD*)lpData);
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
