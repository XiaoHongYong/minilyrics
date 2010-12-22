#include "StdAfx.h"
#include "../../MLPluginLib/MLPlugin.h"
#include <tchar.h>
#include <Psapi.h>
#include "MpcPlayerService.h"
#include "MpcApi.h"

namespace MPCAgent
{

CPlayerService	g_Player;
HWND	g_hWndMPC = NULL;

#define WM_ON_CONNECT	(WM_USER + 1)

LPCTSTR GetMPCCommandName(MPCAPI_COMMAND nCmd)
{
	switch (nCmd)
	{
	case CMD_CONNECT :
		return _T("CMD_CONNECT");
	case CMD_STATE :
		return _T("CMD_STATE");
	case CMD_PLAYMODE :
		return _T("CMD_PLAYMODE");
	case CMD_NOWPLAYING :
		return _T("CMD_NOWPLAYING");
	case CMD_LISTSUBTITLETRACKS :
		return _T("CMD_LISTSUBTITLETRACKS");
	case CMD_LISTAUDIOTRACKS :
		return _T("CMD_LISTAUDIOTRACKS");
	case CMD_PLAYLIST :
		return _T("CMD_PLAYLIST");
	default :
		return _T("CMD_UNK");
	}
}

//////////////////////////////////////////////////////////////////////////

bool GetProcessName(TCHAR szFile[], DWORD nProcessId)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, nProcessId);
	if (!hProcess)
		return false;

	bool bRet = (GetModuleFileNameEx(hProcess, NULL, szFile, MAX_PATH) > 0);

	CloseHandle(hProcess);

	return bRet;
}

bool GetMPCHCProcessName(TCHAR szFile[])
{
	HWND hWndPlayer = FindWindow(_T("MediaPlayerClassicW"), NULL);
	if (hWndPlayer)
	{
		DWORD		dwProcessId = 0;
		GetWindowThreadProcessId(hWndPlayer, &dwProcessId);
		if (dwProcessId > 0)
			return GetProcessName(szFile, dwProcessId);
	}

	return false;
}


int StrSplitInfo(LPTSTR szStr, LPCTSTR szArray[])
{
	if (IsEmptyString(szStr))
		return 0;

	TCHAR		chSeperator = '|';
	LPTSTR		szPos, szPrev;
	int			nCount = 0;

	szPrev = szPos = szStr;
	while (szPos && *szPos)
	{
		szPos = _tcschr(szPos, chSeperator);
		if (szPos)
		{
			if (szPos > szStr)
			{
				if (*(szPos - 1) == '\\')
				{
					*(szPos - 1) = ' ';
					szPos++;
					continue;
				}
			}
			szArray[nCount] = szPrev;
			nCount++;
			*szPos = '\0';
			szPos++;
		}
		else
			break;

		szPrev = szPos;
	}

	szArray[nCount] = szPrev;
	nCount++;
	return nCount;
}

void RemoveYearInfoInTitle(LPTSTR szTitle)
{
	int		n = wcslen(szTitle) - 1;
	if (n <= 0)
		return;

	if (szTitle[n] == ')')
	{
		for (; n >= 0; n--)
		{
			if (szTitle[n] == '(')
			{
				szTitle[n] = '\0';
				return;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////

CPlayerService::CPlayerService()
{
	m_state = PS_STOPED;
	m_nPos = 0;
}

CPlayerService::~CPlayerService()
{
}

void CPlayerService::OnGetInfo(PlayerInfo &playerInfo)
{
	//
	// Initialize player information for MiniLyrics
	//
	playerInfo.hWndPlayer = FindWindow(L"MediaPlayerClassicW", NULL);

	wcscpy_safe(playerInfo.playerName, CountOf(playerInfo.playerName), L"Media Player Classic");
}

void CPlayerService::OnConnectedDone()
{
	g_MLService.AddWndCloseto(NULL, L"MediaPlayerClassicW", NULL);

	ConnectToMpc();

	SetTimer(TIMER_CHECK_PLAYER, 15 * 1000);
	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);

	// Update Media info
	// OnMediaChanged();

	// Update player state
	// g_MLService.OnPlayStateChanged(GetPlayerState());
}

void CPlayerService::OnDisconnect()
{
	KillTimer(TIMER_GETPOS);
	KillTimer(TIMER_CHECK_PLAYER);
}

// control player
void CPlayerService::Play()
{
	SendMpcData(CMD_PLAYPAUSE);
}

void CPlayerService::Pause()
{
	SendMpcData(CMD_PLAYPAUSE);
}

void CPlayerService::PlayPause()
{
	SendMpcData(CMD_PLAYPAUSE);
}

void CPlayerService::Stop()
{
	SendMpcData(CMD_STOP);
}

void CPlayerService::Prev()
{
	SendMpcData(CMD_JUMPBACKWARDMED);
}

void CPlayerService::Next()
{
	SendMpcData(CMD_JUMPFORWARDMED);
}

void CPlayerService::SeekTo(int nMsPos)
{
	TCHAR	szParam[256];
	wsprintf(szParam, _T("%d"), nMsPos / 1000);
	SendMpcData(CMD_SETPOSITION, szParam);
	m_dwTimeBeg = GetTickCount() - nMsPos;
}

void CPlayerService::OnNowPlayingChanged(LPCTSTR szInfo)
{
	// Send after opening a new file
	// Par 1 : title
	// Par 2 : author
	// Par 3 : description
	// Par 4 : complete filename (path included)
	// Par 5 : duration in seconds
	LPCTSTR		vInfo[64];

	m_dwTimeBeg = GetTickCount();
	m_nPos = 0;

	int nCount = StrSplitInfo((LPTSTR)szInfo, vInfo);
	if (nCount < 5)
	{
		g_MLService.OnMediaInfoChanged(L"", NULL, NULL, NULL, 0);
		return;
	}

	RemoveYearInfoInTitle((LPTSTR)vInfo[0]);

	g_MLService.OnMediaInfoChanged(vInfo[3], vInfo[1], NULL, vInfo[0], _ttoi(vInfo[4]) * 1000);
}

void CPlayerService::OnPlayStateChanged(int newState)
{
	MPC_PLAYSTATE state = (MPC_PLAYSTATE)newState;
	if (state == PS_PLAY)
	{
		if (m_state == PS_STOPED)
			m_dwTimeBeg = GetTickCount();
		else
			m_dwTimeBeg = GetTickCount() - m_nPos;

		m_state = PS_PLAYING;
		SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);
	}
	else if (state == PS_PAUSE)
	{
		m_state = PS_PAUSED;
		KillTimer(TIMER_GETPOS);
	}
	else
	{
		m_state = PS_STOPED;
		m_nPos = 0;
		KillTimer(TIMER_GETPOS);
	}

	g_MLService.OnPlayStateChanged(m_state);
}

void CPlayerService::OnPlayTimeChangeCheck()
{
	DWORD		dwTimeNew;

	dwTimeNew = GetTickCount();
	m_nPos = dwTimeNew - m_dwTimeBeg;

	g_MLService.OnPlayPosChanged(m_nPos);
}

void CPlayerService::SendMpcData(int nCmd, LPCWSTR strCommand)
{
	if (g_hWndMPC)
	{
		COPYDATASTRUCT		MyCDS;

		MyCDS.dwData	= nCmd;
		MyCDS.cbData	= (wcslen (strCommand) + 1) * sizeof(WCHAR);
		MyCDS.lpData	= (LPVOID) strCommand;

		::SendMessage (g_hWndMPC, WM_COPYDATA, (WPARAM)GetHandle(), (LPARAM)&MyCDS);
	}
}

bool CPlayerService::ConnectToMpc()
{
	TCHAR	szFile[MAX_PATH];

	if (!GetMPCHCProcessName(szFile))
		return false;

	STARTUPINFO				StartupInfo;
	PROCESS_INFORMATION		ProcessInfo;
	TCHAR					szCmd[512];

	wsprintf(szCmd, _T("%s  /slave %d"), szFile, GetHandle());

	memset (&StartupInfo,		0, sizeof(StartupInfo));
	StartupInfo.cb			= sizeof(StartupInfo);
	GetStartupInfo(&StartupInfo);
	if (CreateProcess(NULL, (LPTSTR)szCmd, NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInfo))
	{
		CloseHandle(ProcessInfo.hProcess);
		CloseHandle(ProcessInfo.hThread);
	}

	return true;
}

void CPlayerService::OnTimer(int nTimerID)
{
	if (nTimerID == TIMER_GETPOS)
	{
		// update current play time.
		OnPlayTimeChangeCheck();
	}
	else if (nTimerID == TIMER_CHECK_PLAYER)
	{
		if (g_hWndMPC == NULL || !IsWindow(g_hWndMPC))
		{
			g_hWndMPC = NULL;
			ConnectToMpc();
		}
	}
}

bool CPlayerService::OnCopyData(WPARAM wParam, COPYDATASTRUCT *pcds)
{
	if (CPlayerServiceBase::OnCopyData(wParam, pcds))
		return true;

	LPCTSTR		szData = (LPCTSTR)pcds->lpData;

	/*TCHAR	szMsg[512];

	wsprintf(szMsg, _T("%s : %s"), GetMPCCommandName ((MPCAPI_COMMAND)pcds->dwData), (LPCTSTR)szData);

	g_MLService.LogW(szMsg);*/

	switch (pcds->dwData)
	{
	// Send after connection
	// Par 1 : MPC window handle (command should be send to this HWnd)
	case CMD_CONNECT:
		g_hWndMPC = (HWND)_wtol(szData);
		PostMessage(m_hWnd, WM_ON_CONNECT, 0, 0);
		break;

	// Send when opening or closing file
	// Par 1 : current state (see MPC_LOADSTATE enum)
	case CMD_STATE:
		break;

	// Send when playing, pausing or closing file
	// Par 1 : current play mode (see MPC_PLAYSTATE enum)
	case CMD_PLAYMODE:
		{
			OnPlayStateChanged(_ttoi(szData));
		}
		break;

	case CMD_NOWPLAYING:
		OnNowPlayingChanged(szData);
		break;

	// Send current playback position in response
	// of CMD_GETCURRENTPOSITION.
	// Par 1 : current position in seconds
/*		case CMD_CURRENTPOSITION:
		m_nPos = _ttoi(szData) * 1000;
		break;
*/
	// Send the current playback position after a jump.
	// (Automatically sent after a seek event).
	// Par 1 : new playback position (in seconds).
	case CMD_NOTIFYSEEK:
		// This message won't be sent to host.
		m_nPos = _ttoi(szData) * 1000;
		m_dwTimeBeg = GetTickCount() - m_nPos;
		break;

	// Notify the end of current playback
	// (Automatically sent).
	// Par 1 : none.
	case CMD_NOTIFYENDOFSTREAM:
		KillTimer(TIMER_GETPOS);
		break;
	default:
		return false;
	}

	return true;
}

LRESULT CPlayerService::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_ON_CONNECT)
	{
		SendMpcData(CMD_GETNOWPLAYING);
		return 1;
	}
	else
		return CPlayerServiceBase::WndProc(uMsg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////

CPlayerPluginAgent::CPlayerPluginAgent() : IPlayerPluginAgent(&g_Player)
{
}

bool CPlayerPluginAgent::IsPlayerRunning()
{
	HWND hWndPlayer = ::FindWindow(L"MediaPlayerClassicW", NULL);
	return hWndPlayer != NULL;
}

}
