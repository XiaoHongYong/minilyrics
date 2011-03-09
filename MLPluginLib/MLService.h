#pragma once

#include "MiniLyricsPlugin.h "

class CMLService
{
public:
	CMLService()
	{
		memset(&m_mlInfo, 0, sizeof(m_mlInfo));
		m_hWndPlayerService = NULL;
		m_hWndMiniLyrics = NULL;
	}

	void SendPlayerInfo(PlayerInfo &playerInfo)
		{ SendMessage(M2M_PLAYER_INFO, &playerInfo, sizeof(PlayerInfo)); }

	void OnDisconnected()
		{ SendMessage(M2M_DISCONNECTED, NULL, 0); }

	void OnMediaInfoChanged(LPCSTR szMediaUrl, LPCSTR szArtist, LPCSTR szAlbum, LPCSTR szTitle, unsigned long nMediaLength, int hasVideo = R_UNDETERMINED);
	void OnMediaInfoChanged(LPCWSTR szMediaUrl, LPCWSTR szArtist, LPCWSTR szAlbum, LPCWSTR szTitle, unsigned long nMediaLength, int hasVideo = R_UNDETERMINED);

	void OnPlayStateChanged(DWORD playStat)
		{ SendMessage(M2M_PLAYER_STAT_CHANGED, &playStat, sizeof(playStat)); }

	void OnPlayPosChanged(DWORD playPosition)
		{ SendMessage(M2M_PLAY_POS_CHANGED, &playPosition, sizeof(playPosition)); }

	void Config(HWND hWnd)
		{ SendMessage(M2M_CONFIG, hWnd, sizeof(hWnd)); }

	void About(HWND hWnd)
		{ SendMessage(M2M_ABOUT, hWnd, sizeof(hWnd)); }

	void Log(LPCWSTR szMessage)
		{ SendMessage(M2M_LOG, (LPVOID)szMessage, sizeof(WCHAR) * (wcslen(szMessage) + 1)); }

	void AddWndCloseto(HWND hWnd, LPCWSTR wndClass, LPCWSTR wndName);

	void OnNotifyPlayerActivate(BOOL bActivate)
		{ SendMessage(M2M_PLAYER_ACTIVATED, &bActivate, sizeof(bActivate)); }

	void OnNotifyPlayerMinRestore(DWORD uState)
		{ SendMessage(M2M_PLAYER_MINRESTORE, &uState, sizeof(uState)); }

	void OnNotifyBeforeTrackMove(HWND hWndTrackAndChain[], int nCount)
		{ SendMessage(M2M_BEFORE_TRACK_MOVE, hWndTrackAndChain, sizeof(HWND) * nCount); }

	void OnNotifyTrackMove(int x, int y)
	{
		DWORD		dwXY = MAKELONG((WORD)x, (WORD)y);
		SendMessage(M2M_TRACK_MOVE, &dwXY, sizeof(dwXY));
	}

	void OnNotifyPlayerSkinChanged(PlayerSkinInfo &skinInfo)
		{ SendMessage(M2M_PLAYER_SKIN_CHANGED, &skinInfo, sizeof(PlayerSkinInfo)); }

	void SwitchToEmbeddedSkin(PlayerSkinInfo &skinInfo)
		{ SendMessage(M2M_SWITCH_2_EMBEDDED_SKIN, &skinInfo, sizeof(PlayerSkinInfo)); }

	void Log(LPCSTR szMessage)
	{
		WCHAR	szMessageW[512];
		szMessageW[0] = 0;
		MultiByteToWideChar(CP_ACP, 0, szMessage, strlen(szMessage) + 1, szMessageW, CountOf(szMessageW));
		Log(szMessageW);
	}

	void SendMessage(MessageToML message, LPVOID param, size_t sizeParam);

	bool IsValid()
		{ return m_mlInfo.hWndMLService != NULL; }

	HWND GetMainWindow()
		{ return m_hWndMiniLyrics; }

public:
	MLInfo				m_mlInfo;
	HWND				m_hWndMiniLyrics;
	HWND				m_hWndPlayerService;

};
