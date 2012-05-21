#pragma once

#include "../MLPluginLib/MiniLyricsPlugin.h"
#include "PlayerServiceBase.h"
#include "MsgPipe.h"

BOOL RunMiniLyrics(LPCSTR pipeName, CPlayerServiceBase *pPlayerService, bool bManual = false);

void DisconnectMiniLyrics(CPlayerServiceBase *pPlayerService);

void Log(LPCTSTR szFormat, ...);

class CMLService
{
public:
	CMLService()
	{
		memset(&m_mlInfo, 0, sizeof(m_mlInfo));
	}
	
	bool Init(LPCSTR pipeName);

	void SendPlayerInfo(PlayerInfo &playerInfo)
		{ SendMessage(M2M_PLAYER_INFO, &playerInfo, sizeof(PlayerInfo)); }

	void OnDisconnected()
		{ SendMessage(M2M_DISCONNECTED, NULL, 0); }

	void OnMediaInfoChanged(LPCSTR szMediaUrl, LPCSTR szArtist, LPCSTR szAlbum, LPCSTR szTitle, unsigned long nMediaLength, int hasVideo = R_UNDETERMINED);

	void OnPlayStateChanged(DWORD playStat)
		{ SendMessage(M2M_PLAYER_STAT_CHANGED, &playStat, sizeof(playStat)); }

	void OnPlayPosChanged(DWORD playPosition)
		{ SendMessage(M2M_PLAY_POS_CHANGED, &playPosition, sizeof(playPosition)); }

	void Log(LPCSTR szMessage)
		{ SendMessage(M2M_LOG, (LPVOID)szMessage, sizeof(char) * (strlen(szMessage) + 1)); }

	void OnNotifyPlayerActivate(BOOL bActivate)
		{ SendMessage(M2M_PLAYER_ACTIVATED, &bActivate, sizeof(bActivate)); }

	void OnNotifyPlayerMinRestore(DWORD uState)
		{ SendMessage(M2M_PLAYER_MINRESTORE, &uState, sizeof(uState)); }

/*	void OnNotifyBeforeTrackMove(HWND hWndTrackAndChain[], int nCount)
		{ SendMessage(M2M_BEFORE_TRACK_MOVE, hWndTrackAndChain, sizeof(HWND) * nCount); }

	void OnNotifyTrackMove(int x, int y)
	{
		DWORD		dwXY = MAKELONG((short)x, (short)y);
		SendMessage(M2M_TRACK_MOVE, &dwXY, sizeof(dwXY));
	}*/

	void OnNotifyPlayerSkinChanged(PlayerSkinInfo &skinInfo)
		{ SendMessage(M2M_PLAYER_SKIN_CHANGED, &skinInfo, sizeof(PlayerSkinInfo)); }

	void SendMessage(MessageToML message, LPVOID param, size_t sizeParam);

	bool IsValid()
		{ return m_msgPipeMiniLyrics.IsOK(); }

public:
	MLInfo				m_mlInfo;
	CMsgPipe			m_msgPipeMiniLyrics;

};

extern class CMLService	g_MLService;

