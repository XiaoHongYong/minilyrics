#pragma once

#include "iTunesVisualAPI.h"
#include "MLService.h"

#define SZ_MSG_PIPE		"/tmp/MiniLyrics_iTunes.pipe"

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

public:
	void OnTimer(int nTimerID);
	void OnMediaChanged(ITTrackInfo	*pTrackInfo);

public:
	DWORD			m_dwPos;
	bool			m_bWorking;

	PLAYER_STATE	m_playerState;
	DWORD			m_dwBegTickCount;

};

extern CPlayerImp	g_Player;
