#pragma once

#include "../SDK/foobar_SDK_1.0/foobar2000/SDK/foobar2000.h"

class CPlayerImp : public CPlayerServiceBase
{
public:
	CPlayerImp();
	virtual ~CPlayerImp();

public:
	virtual void OnGetInfo(PlayerInfo &playerInfo);

	virtual void OnConnect();
	virtual void OnConnectedDone();
	virtual void OnDisconnect();

	// control player
	virtual void Play();
	virtual void Pause();
	virtual void PlayPause();
	virtual void Stop();
	virtual void Prev();
	virtual void Next();
	virtual void SeekTo(int nMsPos);

public:
	DWORD GetPlayPos();
	void OnTimer(int nTimerID);

public:
	PLAYER_STATE	m_stat;

};
