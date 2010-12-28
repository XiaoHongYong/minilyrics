#pragma once

class CPlayerImp : public IPlayer
{
public:
	CPlayerImp();
	virtual ~CPlayerImp();

public:
	// Init and destroy.
	BOOL Init();
	void Quit();

	// Player Info
	int GetPlayerName(char * lpszBuffer, int nBufSize);

	HWND GetMainWnd();

	DWORD GetPlayerState();

	DWORD GetPlayPos();

	virtual int HasVideo();

	BOOL Play();
	BOOL Pause();
	BOOL PlayPause();
	BOOL Stop();
	BOOL Prev();
	BOOL Next();
	BOOL SeekTo(int nMsPos);

	void OnPlayPosUpdateSpanChanged(int nPlayPosUpdateSpan);

public:
	void NotifyPlayerStatChanged(PLAYER_STATE newStat);

	void MediaChangedCheck();

	DWORD GetMediaLength();

public:
	PLAYER_STATE	m_stat;

	intf_thread_t	*m_p_intf;

};
