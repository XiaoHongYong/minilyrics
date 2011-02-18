#include "stdafx.h"
#include "../MLPluginLib/MLPlugin.h"
#include "PlayerImp.h"

CPlayerImp		g_Player;

HWND GetPlayerWindow()
{
	return FindWindow("MJFrame", NULL);
}

void InitMJ(HINSTANCE hInstance, LPDISPATCH pDisp)
{
	g_Player.m_pMJ = pDisp;

	RunMiniLyrics(hInstance, GetPlayerWindow(), &g_Player);
}

CPlayerImp::CPlayerImp()
{
	EmptyStr(m_szSrcMedia);
}

CPlayerImp::~CPlayerImp()
{
}

void CPlayerImp::OnGetInfo(PlayerInfo &playerInfo)
{
	//
	// Initialize player information for MiniLyrics
	//
	playerInfo.hWndPlayer = GetPlayerWindow();
	wcscpy_safe(playerInfo.playerName, CountOf(playerInfo.playerName), L"Media JukeBox");
}

void CPlayerImp::OnConnected()
{
	g_MLService.AddWndCloseto(NULL, L"MJFrame", NULL);

	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);
	SetTimer(TIMER_CHECK_MEDIA, TIME_OUT_UPDATE_MEDIA_INFO);

	// Update player state
	g_MLService.OnPlayStateChanged(GetPlayerState());

	OnMusicChangeCheck();
}

void CPlayerImp::OnDisconnect()
{
	KillTimer(TIMER_GETPOS);
	KillTimer(TIMER_CHECK_MEDIA);
}

void CPlayerImp::Play()
{
	if (m_pMJ.GetInterfacePtr())
	{
		IMJPlaybackAutomationPtr	playback = m_pMJ->GetPlayback();
		if (playback.GetInterfacePtr())
		{
			playback->Play();
		}
	}
}

void CPlayerImp::Pause()
{
	if (m_pMJ.GetInterfacePtr())
	{
		IMJPlaybackAutomationPtr	playback = m_pMJ->GetPlayback();
		if (playback.GetInterfacePtr())
		{
			playback->Pause();
		}
	}
}

void CPlayerImp::PlayPause()
{
	Pause();
}

void CPlayerImp::Stop()
{
	if (m_pMJ.GetInterfacePtr())
	{
		IMJPlaybackAutomationPtr	playback = m_pMJ->GetPlayback();
		if (playback.GetInterfacePtr())
		{
			playback->Stop();
		}
	}
}

void CPlayerImp::Prev()
{
	if (m_pMJ.GetInterfacePtr())
	{
		IMJPlaybackAutomationPtr	playback = m_pMJ->GetPlayback();
		if (playback.GetInterfacePtr())
		{
			playback->Previous();
		}
	}
}

void CPlayerImp::Next()
{
	if (m_pMJ.GetInterfacePtr())
	{
		IMJPlaybackAutomationPtr	playback = m_pMJ->GetPlayback();
		if (playback.GetInterfacePtr())
		{
			playback->Next();
		}
	}
}

void CPlayerImp::SeekTo(int nMsPos)
{
	if (m_pMJ.GetInterfacePtr())
	{
		IMJPlaybackAutomationPtr	playback = m_pMJ->GetPlayback();
		if (playback.GetInterfacePtr())
		{
			playback->PutPosition(nMsPos / 1000);
		}
	}
}

PLAYER_STATE CPlayerImp::GetPlayerState()
{
	if (m_pMJ.GetInterfacePtr())
	{
		IMJPlaybackAutomationPtr	playback = m_pMJ->GetPlayback();
		if (playback.GetInterfacePtr())
		{
			MJPlaybackStates	state = playback->GetState();
			if (state == PLAYSTATE_PLAYING)
				return PS_PLAYING;
			else if (state == PLAYSTATE_PAUSED)
				return PS_PAUSED;
			else
				return PS_STOPED;
		}
	}

	return PS_STOPED;
}

DWORD CPlayerImp::GetPlayPos()
{
	long	pos = 0;

	if (m_pMJ.GetInterfacePtr())
	{
		IMJPlaybackAutomationPtr	playback = m_pMJ->GetPlayback();
		if (playback.GetInterfacePtr())
		{
			pos = playback->GetPosition() * 1000;
			if (pos < 0)
				pos = 0;
			else
				pos += 500;
		}
	}

	return (DWORD)pos;
}

void CPlayerImp::OnTimer(int nTimerID)
{
	if (nTimerID == TIMER_GETPOS)
		OnPlayTimeChangeCheck();
	else if (nTimerID == TIMER_CHECK_MEDIA)
		OnMusicChangeCheck();
}

void CPlayerImp::OnMusicChangeCheck()
{
	if (m_pMJ.GetInterfacePtr())
	{
		IMJCurPlaylistAutomationPtr	curplaylist = m_pMJ->GetCurPlaylist();
		if (curplaylist.GetInterfacePtr())
		{
			int		nFile;
			nFile = curplaylist->GetPosition();
			IMJFileAutomationPtr	file = curplaylist->GetFile(nFile);
			if (file.GetInterfacePtr())
			{
				_bstr_t	filename = file->GetFilename();
				if (wcscmp(m_szSrcMedia, filename) == 0)
					return;

				// song changed.
				wcscpy(m_szSrcMedia, filename);

				_bstr_t	title, artist, album;
				title = file->GetName();
				artist = file->GetArtist();
				album = file->GetAlbum();

				g_MLService.OnMediaInfoChanged(m_szSrcMedia, artist, album, title, file->GetDuration() * 1000);

				m_nPlaybackPosBeg = GetPlayPos();
				m_dwTimeBeg = GetTickCount();
			}
		}
	}
}

void CPlayerImp::OnPlayTimeChangeCheck()
{
	DWORD		nPos, nPosQuery;

	nPos = GetTickCount() - m_dwTimeBeg + m_nPlaybackPosBeg;

	nPosQuery = GetPlayPos();
	if (_abs(nPos - nPosQuery) > 2000 || nPosQuery == 0)
	{
		if (GetPlayerState() != PS_PLAYING)
		{
			g_MLService.OnPlayPosChanged(nPosQuery);
			return;
		}
		// use Seek or pause? let's adjust it.
		nPos = m_nPlaybackPosBeg = nPosQuery;
		m_dwTimeBeg = GetTickCount();
	}

	g_MLService.OnPlayPosChanged(nPos);
}
