#pragma once

#include "../MLPluginLib/MLPlugin.h"

class CPlayerImp : public CPlayerServiceBase
{
public:
	CPlayerImp();
	virtual ~CPlayerImp();

public:
	virtual void OnGetInfo(PlayerInfo &playerInfo);

	virtual void OnConnected();
	virtual void OnDisconnect();

	virtual void Play();
	virtual void Pause();
	virtual void PlayPause();
	virtual void Stop();
	virtual void Prev();
	virtual void Next();
	virtual void SeekTo(int nMsPos);

protected:
	virtual void OnTimer(int nTimerID);

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

extern CPlayerImp g_Player;