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
	void OnTimer(int nTimerID);
	bool OnCopyData(WPARAM wParam, COPYDATASTRUCT *pcds);

	PLAYER_STATE GetPlayerState();
	DWORD GetPlayPos();

	void UpdatePlayerState();

protected:
	tstring			m_strSrcMedia;
	DWORD			m_dwTimeBeg;
	HWND			m_hWndPlayer;
	PLAYER_STATE	m_playerState, m_playerStateOld;

};

class CPlayerPluginAgent : public IPlayerPluginAgent
{
public:
	CPlayerPluginAgent();
	
	virtual bool IsPlayerRunning();

};

}
