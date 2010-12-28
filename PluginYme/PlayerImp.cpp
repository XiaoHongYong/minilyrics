#include "stdafx.h"
#include "PlayerImp.h"

CPlayerImp		g_Player;


class CPlayerInfoQueryWnd : public CPlayerInfoQueryBaseWnd
{
public:
	void OnCreate()
	{
		SetTimer(m_hWnd, TIMER_GETPOS, TIME_OUT_UPDATE_POS, NULL);
	}

	void OnDestroy()
	{
		KillTimer(m_hWnd, TIMER_GETPOS);
	}

	void OnTimer(int nTimerID)
	{
		g_MLService.OnPlayTimeChanged(g_Player.GetPlayPos());
	}

};

CPlayerInfoQueryWnd	g_wndInfoQuery;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
}

CPlayerImp::~CPlayerImp()
{
}

// Player Info
int CPlayerImp::GetPlayerName(char * lpszBuffer, int nBufSize)
{
	strcpy_safe(lpszBuffer, nBufSize, "Yahoo! Music Jukebox");

	return 1;
}

DWORD CPlayerImp::GetPlayerState()
{
	IRMPPlayback	*pPlayBack;
	PLAYER_STATE	playState = PS_STOPED;
	if (m_pMediaPlayer.p != NULL && SUCCEEDED(m_pMediaPlayer->get_Playback((IDispatch * *)&pPlayBack)))
	{
		long		state;
		if (SUCCEEDED(pPlayBack->get_State(&state)))
		{
			if (state == PLAYBACK_STATUS_PAUSED)
				playState = PS_PAUSED;
			else if (state == PLAYBACK_STATUS_PLAYING)
				playState = PS_PLAYING;
			else
				playState = PS_STOPED;
		}
		pPlayBack->Release();
	}

	return playState;
}

HWND CPlayerImp::GetMainWnd()
{
	return g_hwndPlayer;
}

DWORD CPlayerImp::GetPlayPos()
{
	double			dbTime = 0;
	IRMPPlayback	*pPlayBack;
	if (m_pMediaPlayer.p != NULL && SUCCEEDED(m_pMediaPlayer->get_Playback((IDispatch **)&pPlayBack)))
	{
		if (SUCCEEDED(pPlayBack->GetPosition(&dbTime)))
		{
		}
		pPlayBack->Release();
	}

	return (DWORD)(dbTime);
}

BOOL CPlayerImp::Play()
{
/*
	IRMPPlayback	*pPlayBack;
	if (m_pMediaPlayer.p != NULL && SUCCEEDED(m_pMediaPlayer->get_Playback((IDispatch **)&pPlayBack)))
	{
		if (SUCCEEDED(pPlayBack->Pause(false)))
		{
		}
		pPlayBack->Release();
	}
*/
	// now use the remote...for example, tell YME to skip to the next song in the playlist:
	if (MakeRemoteCtrlOK())
		m_spRemote->Command(YRC_PLAY, _variant_t(), _variant_t());

	return TRUE;
}

BOOL CPlayerImp::Pause()
{
/*
	IRMPPlayback	*pPlayBack;
	if (m_pMediaPlayer.p != NULL && SUCCEEDED(m_pMediaPlayer->get_Playback((IDispatch **)&pPlayBack)))
	{
		if (SUCCEEDED(pPlayBack->Pause(true)))
		{
		}
		pPlayBack->Release();
	}
*/
	if (MakeRemoteCtrlOK())
		m_spRemote->Command(YRC_PAUSE, _variant_t(), _variant_t());

	return TRUE;
}

BOOL CPlayerImp::PlayPause()
{
	if (GetPlayerState() == PS_PLAYING)
		Pause();
	else
		Play();

	return TRUE;
}

BOOL CPlayerImp::Stop()
{
/*
	IRMPPlayback	*pPlayBack;
	if (m_pMediaPlayer.p != NULL && SUCCEEDED(m_pMediaPlayer->get_Playback((IDispatch **)&pPlayBack)))
	{
		if (SUCCEEDED(pPlayBack->Stop()))
		{
		}
		pPlayBack->Release();
	}
*/
	if (MakeRemoteCtrlOK())
		m_spRemote->Command(YRC_STOP, _variant_t(), _variant_t());

	return TRUE;
}

BOOL CPlayerImp::Prev()
{
	if (MakeRemoteCtrlOK())
		m_spRemote->Command(YRC_SKIP_FWD, _variant_t(), _variant_t());

	return TRUE;
}

BOOL CPlayerImp::Next()
{
	if (MakeRemoteCtrlOK())
		m_spRemote->Command(YRC_SKIP_REV, _variant_t(), _variant_t());

	return TRUE;
}

BOOL CPlayerImp::SeekTo(int nMsPos)
{
/*
	IRMPPlayback	*pPlayBack;
	if (m_pMediaPlayer.p != NULL && SUCCEEDED(m_pMediaPlayer->get_Playback((IDispatch **)&pPlayBack)))
	{
		VARIANT_BOOL	bSucc;
		if (SUCCEEDED(pPlayBack->Seek(double(nMsPos), &bSucc)))
		{
		}
		pPlayBack->Release();
	}
*/
	if (MakeRemoteCtrlOK())
		m_spRemote->Command(YRC_SEEK, _variant_t(nMsPos), _variant_t());

	return TRUE;
}

bool CPlayerImp::MakeRemoteCtrlOK()
{
	if (m_spRemote.GetInterfacePtr() == NULL)
	{
		HRESULT hr = m_spRemote.CreateInstance("YMP.YMPRemote");

		m_remoteCookie = 0; // assigned to your instance of the remote control interface
		// initialize the control before you begin:
		if (SUCCEEDED(hr))
			m_spRemote->Initialize(&m_remoteCookie);
	}

	return m_spRemote.GetInterfacePtr() != NULL;
}

BOOL CPlayerImp::Init()
{
	g_hwndPlayer = FindWindowA("YMPFrame", NULL);
	g_MLService.AddWndCloseto(NULL, "YMPFrame", NULL);

	g_wndInfoQuery.Create(g_hInstance);

	return TRUE;
}

void CPlayerImp::Quit()
{
	g_wndInfoQuery.Destroy();

	// when you're done, call shutdown:
	if (m_spRemote.GetInterfacePtr() != NULL)
	{
		m_spRemote->Shutdown(m_remoteCookie);
		m_spRemote.Release();
	}
}

void CPlayerImp::OnPlayPosUpdateSpanChanged(int nPlayPosUpdateSpan)
{
	g_wndInfoQuery.SetUpdatePosTimer(nPlayPosUpdateSpan);
}
