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
	PLAYER_STATE GetPlayerState();

	DWORD GetPlayPos();

	void OnMusicChangeCheck();
	void OnPlayTimeChangeCheck();
	void OnTimer(int nTimerID);

public:
	IMJAutomationPtr	m_pMJ;

	WCHAR				m_szSrcMedia[MAX_PATH];
	DWORD				m_nPlaybackPosBeg;
	DWORD				m_dwTimeBeg;

};
