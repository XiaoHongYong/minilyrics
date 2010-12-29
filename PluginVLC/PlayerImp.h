#pragma once

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
	virtual void OnTimer(int nTimerID);
	DWORD GetPlayPos();
	int HasVideo();
	void NotifyPlayerStatChanged(PLAYER_STATE newStat);

	void MediaChangedCheck();

	DWORD GetMediaLength();

public:
	PLAYER_STATE	m_stat;

	intf_thread_t	*m_p_intf;

};
