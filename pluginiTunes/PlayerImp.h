#pragma once

#include "iTunesCOMInterface.h"
#include "iTunesVisualAPI.h"

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

	virtual void SwitchToEmbeddedSkin();
	virtual void EndEmbeddedSkin();

public:
	void OnTimer(int nTimerID);
	void OnMediaChanged(ITTrackInfo	*pTrackInfo);

	LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void OniTunesChangeVisualWnd(HWND hWnd);

	void OniTunesMoveVisualWnd();

	enum 
	{
		TIMER_QUERYITUNES		= TIMER_NEXT + 1,
		WM_SONG_CHANGED			= WM_USER + 1,
		WM_U_ACTIVATEAPP		= WM_USER + 2,
		WM_U_MIN_MAX			= WM_USER + 3
	};

	HWND			m_hWndiTunesVisual;

public:
	DWORD			m_dwPos;
	bool			m_bWorking;

	PLAYER_STATE	m_playerState;
	DWORD			m_dwBegTickCount;

};

extern CPlayerImp		g_Player;
