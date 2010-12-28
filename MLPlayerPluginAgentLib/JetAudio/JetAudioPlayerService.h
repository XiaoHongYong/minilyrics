#pragma once

#include "IPlayerPluginAgent.h"

namespace JetAudioAgent
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

public:
	PLAYER_STATE GetPlayerState();
	DWORD GetPlayPos();

	void OnPlayTimeChangeCheck();

	void OnTimer(int nTimerID);
	bool OnCopyData(WPARAM wParam, COPYDATASTRUCT *pcds);

public:
	string			m_strSrcMedia;
	HWND			m_hWndPlayer;
	DWORD			m_dwTimeBeg;

};

class CPlayerPluginAgent : public IPlayerPluginAgent
{
public:
	CPlayerPluginAgent();
	
	virtual bool IsPlayerRunning();

};

}
