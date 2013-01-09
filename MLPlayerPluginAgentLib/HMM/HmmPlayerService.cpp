#include "StdAfx.h"
#include "../../MLPluginLib/MLPlugin.h"
#include "HmmPlayerService.h"
#include "HMM_API.h"

namespace HMMAgent
{

CPlayerService		g_Player;

#define POS_AR		1
#define POS_TI		0
#define POS_AL		2
#define POS_URL		8

LPCSTR strchr_ntimes(LPCSTR szStr, int nTimes)
{
	for (; szStr && nTimes > 0; nTimes--)
	{
		szStr = strchr(szStr, '\t');
		if (szStr)
			szStr++;
	}

	return szStr;
}

static void HMMCopyTabEndText(LPCSTR szText, tstring &strOut)
{
	int		i = 0;
	while (szText[i] != '\0' && szText[i] !='\t')
		 i++;

	UTF8ToTString(szText, i, strOut);
}

HWND GetPlayerWindow()
{
	return FindWindow(_T(HMM_WND_CLASS), NULL);
}

//////////////////////////////////////////////////////////////////////

CPlayerService::CPlayerService()
{
	m_dwTimeBeg = 0;
	m_hWndPlayer = NULL;
	m_playerStateOld = m_playerState = PS_STOPED;
}

CPlayerService::~CPlayerService()
{
}

void CPlayerService::OnGetInfo(PlayerInfo &playerInfo)
{
	//
	// Initialize player information for MiniLyrics
	//
	playerInfo.hWndPlayer = GetPlayerWindow();

	wcscpy_safe(playerInfo.playerName, CountOf(playerInfo.playerName), L"Helium Music Manager");
}

void CPlayerService::OnConnected()
{
	m_hWndPlayer = GetPlayerWindow();

	g_MLService.AddWndCloseto(NULL, _T(HMM_WND_CLASS), NULL);

	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);
	SetTimer(TIMER_CHECK_MEDIA, TIME_OUT_UPDATE_MEDIA_INFO);

	::SendMessage(m_hWndPlayer, WM_HMM_IPC, (WPARAM)m_hWnd, IPC_SET_CALLBACK_HWND);

	// Update Media info
	::SendMessage(m_hWndPlayer, WM_HMM_IPC, (WPARAM)0, IPC_GET_CURRENT_TRACK);

	// Update player state
	UpdatePlayerState();
}

void CPlayerService::OnDisconnect()
{
	KillTimer(TIMER_GETPOS);
	KillTimer(TIMER_CHECK_MEDIA);
}

void CPlayerService::Play()
{
	if (GetPlayerState() != PS_PLAYING)
		::SendMessage(m_hWndPlayer, WM_HMM_IPC, (WPARAM)0, IPC_PLAY);
}

void CPlayerService::Pause()
{
	if (GetPlayerState() == PS_PLAYING)
		::SendMessage(m_hWndPlayer, WM_HMM_IPC, (WPARAM)0, IPC_FORCEPAUSE);
}

void CPlayerService::PlayPause()
{
	::SendMessage(m_hWndPlayer, WM_HMM_IPC, (WPARAM)0, IPC_PLAYPAUSE);
}

void CPlayerService::Stop()
{
	::SendMessage(m_hWndPlayer, WM_HMM_IPC, (WPARAM)0, IPC_STOP);
}

void CPlayerService::Prev()
{
	::SendMessage(m_hWndPlayer, WM_HMM_IPC, (WPARAM)0, IPC_PREVIOUS);
}

void CPlayerService::Next()
{
	::SendMessage(m_hWndPlayer, WM_HMM_IPC, (WPARAM)0, IPC_NEXT);
}

void CPlayerService::SeekTo(int nMsPos)
{
	::SendMessage(m_hWndPlayer, WM_HMM_IPC, (WPARAM)nMsPos / 1000, IPC_SET_POSITION);
}

void CPlayerService::OnTimer(int nTimerID)
{
	if (nTimerID == TIMER_GETPOS)
	{
		if (m_playerState == PS_PAUSED || m_playerState == PS_STOPED)
			return;

		g_MLService.OnPlayPosChanged(GetTickCount() - m_dwTimeBeg);
	}
	else if (nTimerID == TIMER_CHECK_MEDIA)
	{
		// update info.
		if (!IsWindow(m_hWndPlayer))
			g_MLService.OnDisconnected();
		else
		{
			::SendMessage(m_hWndPlayer, WM_HMM_IPC, (WPARAM)0, IPC_GET_CURRENT_TRACK);

			// Call GetPlayerState to update the state.
			UpdatePlayerState();
		}
	}
}

bool CPlayerService::OnCopyData(WPARAM wParam, COPYDATASTRUCT *pcds)
{
	if (CPlayerServiceBase::OnCopyData(wParam, pcds))
		return true;

	LPCSTR		str;
	int			nPos;

	switch (pcds->dwData)
	{
	case IPC_GET_CURRENT_TRACK:
	case IPC_TRACK_CHANGED_NOTIFICATION:
		{
			str = (LPCSTR)pcds->lpData;
			tstring		strSrcMedia, strArtist, strAlbum, strTitle;

			nPos = 0;

			// title
			if (str)
				HMMCopyTabEndText(str, strTitle);

			// artist
			if (str)
				str = strchr_ntimes(str, POS_AR);
			if (str)
				HMMCopyTabEndText(str, strArtist);
			nPos = POS_AR;

			// album
			if (str)
				str = strchr_ntimes(str, POS_AL - nPos);
			if (str)
				HMMCopyTabEndText(str, strAlbum);
			nPos = POS_AL;

			// media
			if (str)
				str = strchr_ntimes(str, POS_URL - nPos);
			if (str)
				HMMCopyTabEndText(str, strSrcMedia);

			// Is changed?
			if (_tcscmp(m_strSrcMedia.c_str(), strSrcMedia.c_str()) == 0)
				break;
			m_strSrcMedia = strSrcMedia;

			unsigned long nMediaLength = ::SendMessage(m_hWndPlayer, WM_HMM_IPC, (WPARAM)0, IPC_GET_DURATION) * 1000;

			g_MLService.OnMediaInfoChanged(strSrcMedia.c_str(), strArtist.c_str(), strAlbum.c_str(), strTitle.c_str(), nMediaLength);
		}
		break;
	case IPC_SHUTDOWN_NOTIFICATION:
		break;
	case IPC_PLAYER_STATE_CHANGED_NOTIFICATION:
		break;
	default:
		return false;
	}

	return true;
}

PLAYER_STATE CPlayerService::GetPlayerState()
{
	int			n;
	n = ::SendMessage(m_hWndPlayer, WM_HMM_IPC, (WPARAM)0, IPC_GET_PLAYER_STATE);
	if (n == 1)
		return PS_PLAYING;
	else if (n == 2)
		return PS_PAUSED;
	else
		return PS_STOPED;
}

DWORD CPlayerService::GetPlayPos()
{
	return ::SendMessage(m_hWndPlayer, WM_HMM_IPC, (WPARAM)0, IPC_GET_POSITION) * 1000;
}

void CPlayerService::UpdatePlayerState()
{
	m_playerState = GetPlayerState();

	if (m_playerStateOld != m_playerState)
	{
		m_dwTimeBeg = GetTickCount() - GetPlayPos();
		m_playerStateOld = m_playerState;
		g_MLService.OnPlayStateChanged(m_playerState);
		g_MLService.OnPlayPosChanged(GetTickCount() - m_dwTimeBeg);
	}
}

//////////////////////////////////////////////////////////////////////////

CPlayerPluginAgent::CPlayerPluginAgent() : IPlayerPluginAgent(&g_Player)
{
}

bool CPlayerPluginAgent::IsPlayerRunning()
{
	HWND hWndPlayer = GetPlayerWindow();
	return hWndPlayer != NULL;
}

}
