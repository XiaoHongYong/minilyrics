#pragma once

#include "../MLPluginLib/IPlayer.h"
#include "../MLPluginLib/MLPlugin.h"
extern class CPlayerImp		g_Player;

class CPlayerImp : public IPlayer
{
public:
	CPlayerImp();
	virtual ~CPlayerImp();

public:
	// Init and destroy.
	BOOL Init();
	void Quit();

	// Player Info
	int GetPlayerName(char * lpszBuffer, int nBufSize);

	HWND GetMainWnd();

	// get player state and song info
	DWORD GetPlayerState();

	DWORD GetPlayPos();

	// control player
	BOOL Play();
	BOOL Pause();
	BOOL PlayPause();
	BOOL Stop();
	BOOL Prev();
	BOOL Next();
	BOOL SeekTo(int nMsPos);

	void OnPlayPosUpdateSpanChanged(int nPlayPosUpdateSpan);

protected:
	bool MakeRemoteCtrlOK();

public:
	CComQIPtr< IRMPMediaPlayer >	m_pMediaPlayer;
	bool							m_bRemoteOK;
	IRMPRemotePtr					m_spRemote;
	LONG							m_remoteCookie; // assigned to your instance of the remote control interface

	WCHAR			m_szArtist[128], m_szAlbum[128], m_szTitle[128];
	WCHAR			m_szFullTitle[LRC_TITLE_MAX_LEN];
	WCHAR			m_szSrcMedia[MAX_PATH];
	DWORD			m_dwSongLength;

};
