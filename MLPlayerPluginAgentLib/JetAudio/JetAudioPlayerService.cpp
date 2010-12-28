#include "StdAfx.h"
#include "../../MLPluginLib/MLPlugin.h"
#include "JetAudioPlayerService.h"
#include "JetAudio6_API.h"
#include <MMSystem.h>

namespace JetAudioAgent
{

CPlayerService		g_Player;

HWND FindPlayerWnd()
{
	return ::FindWindow(_T("COWON Jet-Audio Remocon Class"), _T("Jet-Audio Remote Control"));
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerService::CPlayerService()
{
}

CPlayerService::~CPlayerService()
{
}


void CPlayerService::OnGetInfo(PlayerInfo &playerInfo)
{
	//
	// Initialize player information for MiniLyrics
	//
	playerInfo.hWndPlayer = FindPlayerWnd();

	wcscpy_safe(playerInfo.playerName, CountOf(playerInfo.playerName), L"jetAudio");
}

void CPlayerService::OnConnected()
{
	m_hWndPlayer = FindPlayerWnd();

	g_MLService.AddWndCloseto(NULL, L"COWON Jet-Audio MainWnd Class", NULL);
	g_MLService.AddWndCloseto(NULL, NULL, L"jetAudio Lyric Viewer");
	g_MLService.AddWndCloseto(NULL, NULL, L"jetAudio Playlist");

	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);
	SetTimer(TIMER_CHECK_MEDIA, 1000);

	// Update Media info
	// OnMediaChanged();

	// Update player state
	g_MLService.OnPlayStateChanged(GetPlayerState());
}

void CPlayerService::OnDisconnect()
{
	KillTimer(TIMER_GETPOS);
	KillTimer(TIMER_CHECK_MEDIA);
}

void CPlayerService::Play()
{
	if (GetPlayerState() != PS_PLAYING)
		::SendMessage(m_hWndPlayer, WM_REMOCON_SENDCOMMAND, 0, JRC_ID_PLAY);
}

void CPlayerService::Pause()
{
	if (GetPlayerState() == PS_PLAYING)
		::SendMessage(m_hWndPlayer, WM_REMOCON_SENDCOMMAND, 0, JRC_ID_PLAY);
}

void CPlayerService::PlayPause()
{
	::SendMessage(m_hWndPlayer, WM_REMOCON_SENDCOMMAND, 0, JRC_ID_PLAY);
}

void CPlayerService::Stop()
{
	::SendMessage(m_hWndPlayer, WM_REMOCON_SENDCOMMAND, 0, JRC_ID_STOP);
}

void CPlayerService::Prev()
{
	::SendMessage(m_hWndPlayer, WM_REMOCON_SENDCOMMAND, 0, JRC_ID_PREV_TRACK);
}

void CPlayerService::Next()
{
	::SendMessage(m_hWndPlayer, WM_REMOCON_SENDCOMMAND, 0, JRC_ID_NEXT_TRACK);
}

void CPlayerService::SeekTo(int nMsPos)
{
	::SendMessage(m_hWndPlayer, WM_REMOCON_SENDCOMMAND, 0, MAKELPARAM(JRC_ID_SEEK, nMsPos / 1000));
}

PLAYER_STATE CPlayerService::GetPlayerState()
{
	int			n;
	n = ::SendMessage(m_hWndPlayer, WM_REMOCON_GETSTATUS, (WPARAM)m_hWnd, GET_STATUS_STATUS);
	if (n == MCI_MODE_PLAY)
		return PS_PLAYING;
	else if (n == MCI_MODE_PAUSE)
		return PS_PAUSED;
	else
		return PS_STOPED;
}

DWORD CPlayerService::GetPlayPos()
{
	return ::SendMessage(m_hWndPlayer, WM_REMOCON_GETSTATUS, (WPARAM)m_hWnd, GET_STATUS_CUR_TIME);
}

void CPlayerService::OnPlayTimeChangeCheck()
{
	DWORD		nPos, nPosQuery;
	DWORD		dwTimeNew;

	static		bool bPauseCheck = false;
	static		DWORD nPosOld = 0;

	dwTimeNew = GetTickCount();
	nPos = dwTimeNew - m_dwTimeBeg;
	nPosQuery = GetPlayPos();

	if (bPauseCheck)
	{
		if (nPosQuery != nPosOld)
		{
			bPauseCheck = false;
			m_dwTimeBeg = dwTimeNew - nPosQuery;
			g_MLService.OnPlayPosChanged(nPosQuery);
		}
		return;
	}

	if (_abs(nPos - nPosQuery) > 500 || nPosQuery == 0)
	{
		// use Seek or pause? let's adjust it.
		m_dwTimeBeg = dwTimeNew - nPosQuery;
		g_MLService.OnPlayPosChanged(nPosQuery);
		nPosOld = nPosQuery;
		bPauseCheck = true;
	}
	else
		g_MLService.OnPlayPosChanged(nPos);
}

void CPlayerService::OnTimer(int nTimerID)
{
	if (nTimerID == TIMER_GETPOS)
		OnPlayTimeChangeCheck();
	else if (nTimerID == TIMER_CHECK_MEDIA)
	{
		// update info.
		if (!IsWindow(m_hWndPlayer))
			m_hWndPlayer = FindPlayerWnd();
		if (m_hWndPlayer)
		{
			::SendMessage(m_hWndPlayer, WM_REMOCON_GETSTATUS, (WPARAM)m_hWnd, GET_STATUS_TRACK_INFO);
		}
		else
			g_MLService.OnMediaInfoChanged(L"", NULL, NULL, NULL, 0);
	}
}

bool CPlayerService::OnCopyData(WPARAM wParam, COPYDATASTRUCT *pcds)
{
	if (CPlayerServiceBase::OnCopyData(wParam, pcds))
		return true;

	LPCSTR str = (LPCSTR)pcds->lpData;
	switch (pcds->dwData)
	{
	case JRC_COPYDATA_ID_TRACK_INFO:
		{
			// Is changed?
			if (strcmp(m_strSrcMedia.c_str(), str) == 0)
				break;
			m_strSrcMedia = str;

			LPCSTR		szInfo[10];
			int			i = 0;
			while (*str && i < CountOf(szInfo))
			{
				szInfo[i] = str;
				str += (strlen(str) + 1);
				i++;
			}
			for (; i < CountOf(szInfo); i++)
				szInfo[i] = "";
			DWORD	dwLength = 	::SendMessage(m_hWndPlayer, WM_REMOCON_GETSTATUS, (WPARAM)m_hWnd, GET_STATUS_MAX_TIME);
			g_MLService.OnMediaInfoChanged(szInfo[0], szInfo[2], "", szInfo[1], dwLength);
		}
		break;
	default:
		return false;
	}

	return true;
}

CPlayerPluginAgent::CPlayerPluginAgent() : IPlayerPluginAgent(&g_Player)
{
}

bool CPlayerPluginAgent::IsPlayerRunning()
{
	HWND hWndPlayer = FindPlayerWnd();
	return hWndPlayer != NULL;
}

}
