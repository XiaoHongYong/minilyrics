#include "stdafx.h"
#include "PlayerImp.h"

CPlayerImp		g_Player;


class CPlayerInfoQueryWnd : public CPlayerInfoQueryBaseWnd
{
public:
	void OnCreate()
	{
		SetTimer(m_hWnd, TIMER_GETPOS, TIME_OUT_UPDATE_POS, NULL);
		SetTimer(m_hWnd, TIMER_CHECK_MEDIA, TIME_OUT_UPDATE_MEDIA_INFO, NULL);
	}

	void OnDestroy()
	{
		KillTimer(m_hWnd, TIMER_GETPOS);
		KillTimer(m_hWnd, TIMER_CHECK_MEDIA);
	}

	void OnTimer(int nTimerID)
	{
		if (nTimerID == TIMER_GETPOS)
			g_Player.OnPlayTimeChangeCheck();
		else if (nTimerID == TIMER_CHECK_MEDIA)
			g_Player.OnMusicChangeCheck();
	}

};

CPlayerInfoQueryWnd	g_wndInfoQuery;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
	EmptyStr(m_szSrcMedia);
}

CPlayerImp::~CPlayerImp()
{
}

// Player Info
int CPlayerImp::GetPlayerName(char * lpszBuffer, int nBufSize)
{
	strcpy_safe(lpszBuffer, nBufSize, "Media JukeBox");

	return 1;
}

// get player state and song info
DWORD CPlayerImp::GetPlayerState()
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

HWND CPlayerImp::GetMainWnd()
{
	return g_hwndPlayer;
}

// 歌曲播放的位置（单位ms)
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
/*

int CPlayerImp::HasVideo()
{
	if (m_pMJ.GetInterfacePtr())
	{
		IMJPlaybackAutomationPtr	playback = m_pMJ->GetPlayback();
		if (playback.GetInterfacePtr())
		{
			if (playback->IsVideo())
				return 1;
		}
	}
	return 0;
}
*/

// control player
BOOL CPlayerImp::Play()
{
	if (m_pMJ.GetInterfacePtr())
	{
		IMJPlaybackAutomationPtr	playback = m_pMJ->GetPlayback();
		if (playback.GetInterfacePtr())
		{
			playback->Play();
		}
	}

	return TRUE;
}

BOOL CPlayerImp::Pause()
{
	if (m_pMJ.GetInterfacePtr())
	{
		IMJPlaybackAutomationPtr	playback = m_pMJ->GetPlayback();
		if (playback.GetInterfacePtr())
		{
			playback->Pause();
		}
	}

	return TRUE;
}

BOOL CPlayerImp::PlayPause()
{
 	Pause();

	return TRUE;
}

BOOL CPlayerImp::Stop()
{
	if (m_pMJ.GetInterfacePtr())
	{
		IMJPlaybackAutomationPtr	playback = m_pMJ->GetPlayback();
		if (playback.GetInterfacePtr())
		{
			playback->Stop();
		}
	}

	return TRUE;
}

BOOL CPlayerImp::Prev()
{
	if (m_pMJ.GetInterfacePtr())
	{
		IMJPlaybackAutomationPtr	playback = m_pMJ->GetPlayback();
		if (playback.GetInterfacePtr())
		{
			playback->Previous();
		}
	}

	return TRUE;
}

BOOL CPlayerImp::Next()
{
	if (m_pMJ.GetInterfacePtr())
	{
		IMJPlaybackAutomationPtr	playback = m_pMJ->GetPlayback();
		if (playback.GetInterfacePtr())
		{
			playback->Next();
		}
	}

	return TRUE;
}

BOOL CPlayerImp::SeekTo(int nMsPos)
{
	if (m_pMJ.GetInterfacePtr())
	{
		IMJPlaybackAutomationPtr	playback = m_pMJ->GetPlayback();
		if (playback.GetInterfacePtr())
		{
			playback->PutPosition(nMsPos / 1000);
		}
	}

	return TRUE;
}

BOOL CPlayerImp::Init()
{
	g_hwndPlayer = FindWindow("MJFrame", NULL);
	g_MLService.AddWndCloseto(NULL, "MJFrame", NULL);

	g_wndInfoQuery.Create(g_hInstance);

	return TRUE;
}

void CPlayerImp::Quit()
{
	g_wndInfoQuery.Destroy();
}

void CPlayerImp::OnPlayPosUpdateSpanChanged(int nPlayPosUpdateSpan)
{
	g_wndInfoQuery.SetUpdatePosTimer(nPlayPosUpdateSpan);
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

				MLMediaInfo		mediaInfo;

				mediaInfo.szMediaUrl = m_szSrcMedia;
				mediaInfo.szArtist = artist;
				mediaInfo.szAlbum = album;
				mediaInfo.szTitle = title;
				mediaInfo.nMediaLength = file->GetDuration() * 1000;

				m_nPlaybackPosBeg = GetPlayPos();
				m_dwTimeBeg = GetTickCount();

				g_MLService.OnMediaInfoChanged(&mediaInfo);
			}
		}
	}
}


int __cdecl _abs(
        int number
        )
{
        return( number>=0 ? number : -number );
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
			g_MLService.OnPlayTimeChanged(nPosQuery);
			return;
		}
		// use Seek or pause? let's adjust it.
		nPos = m_nPlaybackPosBeg = nPosQuery;
		m_dwTimeBeg = GetTickCount();
	}

	g_MLService.OnPlayTimeChanged(nPos);
}
