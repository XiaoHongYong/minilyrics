#pragma once

#include "IPlayerPluginAgent.h"

namespace SpotifyAgent
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
	virtual void OnTimer(int nTimerID);
	void OnNoMedia();

	void OnPlayTimeChangeCheck();

public:
	DWORD		m_dwBeginPlay;
	DWORD		m_dwBeginPaused;
	bool		m_bHasMedia;
	bool		m_bSpotifyTitle;
	WCHAR		m_szWndTitleOld[256];
	int			m_nLenTitleOld;

};

class CPlayerPluginAgent : public IPlayerPluginAgent
{
public:
	CPlayerPluginAgent();

	virtual bool IsPlayerRunning();

};

}
