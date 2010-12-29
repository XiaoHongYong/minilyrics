#include "stdafx.h"
#include "PlayerImp.h"

CPlayerImp		g_Player;

//////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
}

CPlayerImp::~CPlayerImp()
{
}

void CPlayerImp::OnGetInfo(PlayerInfo &playerInfo)
{
	//
	// Initialize player information for MiniLyrics
	//
	playerInfo.hWndPlayer = FindWindowA("YMPFrame", NULL);

	wcscpy_safe(playerInfo.playerName, CountOf(playerInfo.playerName), L"Yahoo! Music Jukebox");
}

void CPlayerImp::OnConnected()
{
	g_MLService.AddWndCloseto(NULL, L"YMPFrame", NULL);

	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);

	// Update Media info
	CheckMediaUpdate(true);

	// Update player state
	g_MLService.OnPlayStateChanged(GetPlayerState());
}

void CPlayerImp::OnDisconnect()
{
	KillTimer(TIMER_GETPOS);

	// when you're done, call shutdown:
	if (m_spRemote.GetInterfacePtr() != NULL)
	{
		m_spRemote->Shutdown(m_remoteCookie);
		m_spRemote.Release();
	}
}

void CPlayerImp::Play()
{
	if (MakeRemoteCtrlOK())
		m_spRemote->Command(YRC_PLAY, _variant_t(), _variant_t());
}

void CPlayerImp::Pause()
{
	if (MakeRemoteCtrlOK())
		m_spRemote->Command(YRC_PAUSE, _variant_t(), _variant_t());
}

void CPlayerImp::PlayPause()
{
	if (GetPlayerState() == PS_PLAYING)
		Pause();
	else
		Play();
}

void CPlayerImp::Stop()
{
	if (MakeRemoteCtrlOK())
		m_spRemote->Command(YRC_STOP, _variant_t(), _variant_t());
}

void CPlayerImp::Prev()
{
	if (MakeRemoteCtrlOK())
		m_spRemote->Command(YRC_SKIP_FWD, _variant_t(), _variant_t());
}

void CPlayerImp::Next()
{
	if (MakeRemoteCtrlOK())
		m_spRemote->Command(YRC_SKIP_REV, _variant_t(), _variant_t());
}

void CPlayerImp::SeekTo(int nMsPos)
{
	if (MakeRemoteCtrlOK())
		m_spRemote->Command(YRC_SEEK, _variant_t(nMsPos), _variant_t());
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

void CPlayerImp::OnTimer(int nTimerID)
{
	g_MLService.OnPlayPosChanged(g_Player.GetPlayPos());
}
