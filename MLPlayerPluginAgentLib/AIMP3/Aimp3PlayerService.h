#pragma once

#include "IPlayerPluginAgent.h"

namespace Aimp3Agent
{

class CPlayerService : public CPlayerServiceBase
{
public:
	enum { TIMER_CHECK_PLAYER = TIMER_NEXT + 1 };

	CPlayerService();
	virtual ~CPlayerService();

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
	void OnNowPlayingCheck();
	void OnPlayStateChanged(PLAYER_STATE state);
	void OnPlayTimeChangeCheck();

	void SendCmdToAimp3(int nCmd);

	void OnTimer(int nTimerID);

public:
	DWORD			m_dwTimeBeg;
	DWORD			m_nPos;

};

class CPlayerPluginAgent : public IPlayerPluginAgent
{
public:
	CPlayerPluginAgent();
	
	virtual bool IsPlayerRunning();

};

}
