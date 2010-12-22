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

static void HMMCopyTabEndText(LPCSTR szText, CHAR szBuff[], int nBuffSize)
{
	int		i;
	for (i = 0; szText[i] != '\0' && szText[i] !='\t' && i < nBuffSize; i++)
		szBuff[i] =  szText[i];

	szBuff[i] = '\0';
}

//////////////////////////////////////////////////////////////////////

CPlayerService::CPlayerService()
{
	m_dwTimeBeg = 0;
	m_hWndPlayer = NULL;
}

CPlayerService::~CPlayerService()
{
}

void CPlayerService::OnGetInfo(PlayerInfo &playerInfo)
{
	//
	// Initialize player information for MiniLyrics
	//
	playerInfo.hWndPlayer = ::FindWindow(_T(HMM_WND_CLASS), NULL);;

	wcscpy_safe(playerInfo.playerName, CountOf(playerInfo.playerName), L"Helium Music Manager");
}

void CPlayerService::OnConnectedDone()
{
	g_MLService.AddWndCloseto(NULL, _T(HMM_WND_CLASS), NULL);

	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);
	SetTimer(TIMER_CHECK_MEDIA, TIME_OUT_UPDATE_MEDIA_INFO);

	::SendMessage(m_hWndPlayer, WM_HMM_IPC, (WPARAM)m_hWnd, IPC_SET_CALLBACK_HWND);
	::SendMessage(m_hWndPlayer, WM_HMM_IPC, (WPARAM)0, IPC_GET_CURRENT_TRACK);

	// Update Media info
	// OnMediaChanged();

	// Update player state
	// g_MLService.OnPlayStateChanged(GetPlayerState());
}

void CPlayerService::OnDisconnect()
{
	KillTimer(TIMER_GETPOS);
	KillTimer(TIMER_CHECK_MEDIA);
}

// control player
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
		OnPlayTimeChangeCheck();
	else if (nTimerID == TIMER_CHECK_MEDIA)
	{
		// update info.
		if (!IsWindow(m_hWndPlayer))
			m_hWndPlayer = ::FindWindow(_T(HMM_WND_CLASS), NULL);
		if (m_hWndPlayer)
			::SendMessage(m_hWndPlayer, WM_HMM_IPC, (WPARAM)0, IPC_GET_CURRENT_TRACK);
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
			char		szSrcMedia[MAX_PATH];
			char		szArtist[MAX_PATH];
			char		szAlbum[MAX_PATH];
			char		szTitle[MAX_PATH];

			nPos = 0;

			// title
			if (str)
				HMMCopyTabEndText(str, szTitle, CountOf(szTitle));
			else
				EmptyStr(szTitle);

			// artist
			if (str)
				str = strchr_ntimes(str, POS_AR);
			if (str)
				HMMCopyTabEndText(str, szArtist, CountOf(szArtist));
			else
				EmptyStr(szArtist);
			nPos = POS_AR;

			// album
			if (str)
				str = strchr_ntimes(str, POS_AL - nPos);
			if (str)
				HMMCopyTabEndText(str, szAlbum, CountOf(szAlbum));
			else
				EmptyStr(szAlbum);
			nPos = POS_AL;

			// media
			if (str)
				str = strchr_ntimes(str, POS_URL - nPos);
			if (str)
				HMMCopyTabEndText(str, szSrcMedia, CountOf(szSrcMedia));
			else
				EmptyStr(szSrcMedia);

			m_dwTimeBeg = GetTickCount() - GetPlayPos();
			unsigned long nMediaLength = ::SendMessage(m_hWndPlayer, WM_HMM_IPC, (WPARAM)0, IPC_GET_DURATION) * 1000;

			g_MLService.OnMediaInfoChanged(szSrcMedia, szArtist, szAlbum, szTitle, nMediaLength);
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

	if (_abs(nPos - nPosQuery) > 1500 || nPosQuery == 0)
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

//////////////////////////////////////////////////////////////////////////

CPlayerPluginAgent::CPlayerPluginAgent() : IPlayerPluginAgent(&g_Player)
{
}

bool CPlayerPluginAgent::IsPlayerRunning()
{
	HWND hWndPlayer = ::FindWindow(_T(HMM_WND_CLASS), NULL);
	return hWndPlayer != NULL;
}

}
