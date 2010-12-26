#include "StdAfx.h"
#include "../../MLPluginLib/MLPlugin.h"
#include "ZunePlayerService.h"

namespace ZuneAgent
{

CPlayerService		g_Player;

HWND GetPlayerWindow()
{
	return FindWindow(_T("UIX Render Window"), _T("Zune"));
}

class CFakeMsnMessageWnd : public CTinyBaseWnd
{
protected:
	bool		m_bHasMedia;

public:
	CFakeMsnMessageWnd()
	{
		m_bHasMedia = false;
	}

	bool OnCopyData(WPARAM wParam, COPYDATASTRUCT *pcds)
	{
		LPCWSTR		szInfo = (LPCWSTR)pcds->lpData;

		// "ZUNE\0Music\01\0{0} - {1}\0I Got That (Boom Boom)\0Britney Spears\0In the Zone\0zune:{373fd701-0100-11db-89ca-0019b92a3933}\0"	wchar_t *
		LPCWSTR		SZ_SPLLITOR = L"\\0";
		const int	LEN_SZ_SPLLITOR = wcslen(SZ_SPLLITOR);
		LPCWSTR		SZ_HEAD = L"ZUNE\\0Music";
		if (wcsncmp(szInfo, SZ_HEAD, wcslen(SZ_HEAD)) != 0)
			return false;

		const int	MAX_TAG = 10;
		LPCWSTR		vszTag[10] = { 0 };
		int			nLen = wcslen(szInfo);
		wchar_t		*szSplitedInfo = new wchar_t[nLen + 1];
		if (!szSplitedInfo)
			return false;

		wcscpy_safe(szSplitedInfo, nLen + 1, szInfo + wcslen(SZ_HEAD));

		LPWSTR		szCur;

		szCur = szSplitedInfo;
		for (int i = 0; i < MAX_TAG; i++)
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

		LPCWSTR szTitle = vszTag[3];
		LPCWSTR szArtist = vszTag[4];
		LPCWSTR szAlbum = vszTag[5];
		// info.szMediaUrl = vszTag[6];
		int nMediaLength = 0;

		if (szTitle == NULL || IsEmptyString(szTitle))
			m_bHasMedia = false;
		else
		{
			m_bHasMedia = true;
			g_Player.m_dwBeginPlay = GetTickCount();
		}

		g_MLService.OnMediaInfoChanged(NULL, szArtist, szAlbum, szTitle, nMediaLength);

		delete[] szSplitedInfo;

		return true;
	}

};

CFakeMsnMessageWnd		g_wndFakeMsnWnd;

///////////////////////////////////////////////////////////////////////////////

CPlayerService::CPlayerService()
{
	m_dwBeginPlay = 0;
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

	wcscpy_safe(playerInfo.playerName, CountOf(playerInfo.playerName), L"Zune");
}

void CPlayerService::OnConnected()
{
	g_wndFakeMsnWnd.Create(GetAppInstance(), _T("MsnMsgrUIManager"), _T(""));

	g_MLService.AddWndCloseto(NULL, L"UIX Render Window", L"Zune");

	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);
}

void CPlayerService::OnDisconnect()
{
	g_wndFakeMsnWnd.Destroy();
	KillTimer(TIMER_GETPOS);
}

// control player
void CPlayerService::Play()
{
}

void CPlayerService::Pause()
{
}

void CPlayerService::PlayPause()
{
}

void CPlayerService::Stop()
{
}

void CPlayerService::Prev()
{
}

void CPlayerService::Next()
{
}

void CPlayerService::SeekTo(int nMsPos)
{
}

void CPlayerService::OnTimer(int nTimerID)
{
	if (nTimerID == TIMER_GETPOS)
	{
		g_MLService.OnPlayPosChanged(GetTickCount() - m_dwBeginPlay);
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
