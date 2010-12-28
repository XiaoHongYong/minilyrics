#pragma once

#include "IPlayerPluginAgent.h"

namespace MPCAgent
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
	void OnNowPlayingChanged(LPCTSTR szInfo);
	void OnPlayStateChanged(int newState);
	void OnPlayTimeChangeCheck();

	bool ConnectToMpc();

	void SendMpcData(int nCmd, LPCWSTR strCommand = _T(""));

	void OnTimer(int nTimerID);
	bool OnCopyData(WPARAM wParam, COPYDATASTRUCT *pcds);
	LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	DWORD			m_dwTimeBeg;
	DWORD			m_nPos;

	PLAYER_STATE	m_state;

};

class CPlayerPluginAgent : public IPlayerPluginAgent
{
public:
	CPlayerPluginAgent();
	
	virtual bool IsPlayerRunning();

};

}
