#pragma once

extern class CPlayerImp		g_Player;

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
	void OnMediaChanged();
	void OnTimer(int nTimerID);

	PLAYER_STATE	m_playerState;
	DWORD			m_dwLength;

};
