#include "StdAfx.h"
#include "../../MLPluginLib/MLPlugin.h"
#include "SpotifyPlayerService.h"

namespace SpotifyAgent
{

CPlayerService		g_Player;
HWND				g_hwndPlayer = NULL;

HWND GetPlayerWindow()
{
	return FindWindow(_T("SpotifyMainWindow"), NULL);
}

///////////////////////////////////////////////////////////////////////////////

CPlayerService::CPlayerService()
{
	m_szWndTitleOld[0] = 0;
	m_bSpotifyTitle = true;
	m_bHasMedia = false;
	m_nLenTitleOld = 0;
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
	playerInfo.hWndPluginService = GetHandle();

	wcscpy_safe(playerInfo.playerName, CountOf(playerInfo.playerName), L"Spotify");
}

void CPlayerService::OnConnected()
{
	g_MLService.AddWndCloseto(NULL, L"SpotifyMainWindow", NULL);
	g_hwndPlayer = GetPlayerWindow();

	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);
	SetTimer(TIMER_CHECK_MEDIA, 100);
}

void CPlayerService::OnDisconnect()
{
	KillTimer(TIMER_GETPOS);
	KillTimer(TIMER_CHECK_MEDIA);
}

void CPlayerService::Play()
{
	PlayPause();
}

void CPlayerService::Pause()
{
	PlayPause();
}

void CPlayerService::PlayPause()
{
	SendMessage(g_hwndPlayer, WM_KEYDOWN, VK_SPACE, 0);
	SendMessage(g_hwndPlayer, WM_KEYUP, VK_SPACE, 0);
}

void CPlayerService::Stop()
{
	PlayPause();
}

void CPlayerService::Prev()
{
	keybd_event(VK_CONTROL, 0x1D, 0, 0);
	SendMessage(g_hwndPlayer, WM_KEYDOWN, VK_LEFT, 0);
	SendMessage(g_hwndPlayer, WM_KEYUP, VK_LEFT, 0);
	Sleep(100);
	keybd_event(VK_CONTROL, 0x1D, KEYEVENTF_KEYUP, 0);

	m_dwBeginPlay = GetTickCount();
}

void CPlayerService::Next()
{
	keybd_event(VK_CONTROL, 0x1D, 0, 0);
	SendMessage(g_hwndPlayer, WM_KEYDOWN, VK_RIGHT, 0);
	SendMessage(g_hwndPlayer, WM_KEYUP, VK_RIGHT, 0);
	Sleep(100);
	keybd_event(VK_CONTROL, 0x1D, KEYEVENTF_KEYUP, 0);
}

void CPlayerService::SeekTo(int nMsPos)
{
}

void CPlayerService::OnTimer(int nTimerID)
{
	if (nTimerID == TIMER_GETPOS)
	{
		if (m_bHasMedia && !m_bSpotifyTitle)
			g_MLService.OnPlayPosChanged(GetTickCount() - m_dwBeginPlay);
	}
	else if (nTimerID == TIMER_CHECK_MEDIA)
	{
		if (!IsWindow(g_hwndPlayer))
			g_hwndPlayer = FindWindow(_T("SpotifyMainWindow"), NULL);
		if (!g_hwndPlayer)
		{
			g_MLService.OnDisconnected();
			return;
		}

		WCHAR		szWndTitle[256];
		int			nLen, i;
		nLen = GetWindowTextW(g_hwndPlayer, szWndTitle, CountOf(szWndTitle));
		if (nLen <= 0)
		{
			OnNoMedia();
			return;
		}

		if (m_nLenTitleOld == nLen 
			&& lstrcmpW(m_szWndTitleOld, szWndTitle) == 0)
		{
			if (m_bSpotifyTitle)
			{
				m_bSpotifyTitle = false;
				m_dwBeginPlay += GetTickCount() - m_dwBeginPaused;
			}
			return;
		}

		if (lstrcmpW(szWndTitle, L"Spotify") == 0)
		{
			if (!m_bSpotifyTitle)
			{
				m_bSpotifyTitle = true;
				m_dwBeginPaused = GetTickCount();
			}
			return;
		}

		m_bSpotifyTitle = false;

		lstrcpyW(m_szWndTitleOld, szWndTitle);
		m_nLenTitleOld = nLen;

		const int	MAX_TAG = 10;
		LPCWSTR		vszTag[10] = { 0 };
		LPWSTR		szCur;
		LPCWSTR		SZ_SPLLITOR = L" - ";
		const int	LEN_SZ_SPLLITOR = wcslen(SZ_SPLLITOR);

		// Replace the separator '¨C' with '-'
		for (i = 0; i < nLen; i++)
		{
			if (szWndTitle[i] == 0x2013) // L'¨C'
				szWndTitle[i] = L'-';
		}

		// Split title.
		// Spotify - Britney Spears ¨C Overprotected - Album Version (Radio Edit)
		szCur = szWndTitle;
		for (i = 0; i < MAX_TAG; i++)
		{
			vszTag[i] = szCur;
			szCur = wcsstr(szCur, SZ_SPLLITOR);
			if (szCur)
			{
				szCur[0] = '\0';
				szCur += LEN_SZ_SPLLITOR;
			}
			else
				break;
		}

		LPCTSTR szTitle = vszTag[2];
		LPCTSTR szArtist = vszTag[1];
		LPCTSTR szAlbum = vszTag[3];

		if (szTitle == NULL || IsEmptyString(szTitle))
			m_bHasMedia = false;
		else
		{
			m_bHasMedia = true;
			m_dwBeginPlay = GetTickCount() + 2500;
		}

		g_MLService.OnMediaInfoChanged(NULL, szArtist, szAlbum, szTitle, 0);
	}
}

void CPlayerService::OnNoMedia()
{
	m_bHasMedia = false;
	g_MLService.OnMediaInfoChanged(L"", NULL, NULL, NULL, 0);
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
