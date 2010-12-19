#pragma once

#include "IPlayerPluginAgent.h"

namespace HMMAgent
{

class CPlayerService : public CPlayerServiceBase
{
public:
	CPlayerService();
	virtual ~CPlayerService();

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
	void OnTimer(int nTimerID);
	bool OnCopyData(WPARAM wParam, COPYDATASTRUCT *pcds);

	PLAYER_STATE GetPlayerState();
	DWORD GetPlayPos();

	void OnPlayTimeChangeCheck();

public:
	DWORD			m_dwTimeBeg;
	HWND			m_hWndPlayer;

};

class CMPCPluginAgent : public IPlayerPluginAgent
{
public:
	CMPCPluginAgent();
	
	virtual bool IsPlayerRunning();

};

}
