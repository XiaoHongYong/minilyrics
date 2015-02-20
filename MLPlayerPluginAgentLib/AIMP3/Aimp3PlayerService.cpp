#include "StdAfx.h"
#include "../../MLPluginLib/MLPlugin.h"
#include <tchar.h>
#include <Psapi.h>
#include "Aimp3PlayerService.h"
#include "apiRemote.h"

namespace Aimp3Agent
{

CPlayerService	g_Player;
HWND	g_hWndAimp3 = NULL;
HANDLE	g_hInfo = NULL;
PAIMPRemoteFileInfo g_info = NULL;
LPWSTR g_pInfoCache = NULL;

void ExtractString(LPCWSTR &buf, int length, wstring &str)
{
	str.append(buf, length);
	buf += length;
}

HWND GetPlayerWindow()
{
	return FindWindow(AIMPRemoteAccessClass, NULL);
}

void OpenInfoMapFile() {
	g_hInfo = ::OpenFileMapping(FILE_MAP_READ, false, AIMPRemoteAccessClass);
	if (g_hInfo == NULL)
		return;

	g_info = (PAIMPRemoteFileInfo)MapViewOfFile(g_hInfo, FILE_MAP_READ, 0, 0, AIMPRemoteAccessMapFileSize);
}

void CloseInfoMapFile() {
	if (g_hInfo) {
		UnmapViewOfFile(g_info);
		CloseHandle(g_hInfo);
		g_info = NULL;
		g_hInfo = NULL;
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

//////////////////////////////////////////////////////////////////////

CPlayerService::CPlayerService()
{
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
	playerInfo.hWndPlayer = GetPlayerWindow();

	wcscpy_safe(playerInfo.playerName, CountOf(playerInfo.playerName), L"AIMP3");
}

void CPlayerService::OnConnected()
{
	OpenInfoMapFile();
	g_hWndAimp3 = GetPlayerWindow();

	g_MLService.AddWndCloseto(NULL, L"TAIMPMainForm", NULL);

	m_dwTimeBeg = GetTickCount();

	SetTimer(TIMER_CHECK_PLAYER, 1000);
	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);
	SetTimer(TIMER_CHECK_MEDIA, 200);
}

void CPlayerService::OnDisconnect()
{
	KillTimer(TIMER_GETPOS);
	KillTimer(TIMER_CHECK_PLAYER);
	KillTimer(TIMER_CHECK_MEDIA);

	CloseInfoMapFile();
}

void CPlayerService::Play()
{
	SendCmdToAimp3(AIMP_RA_CMD_PLAY);
}

void CPlayerService::Pause()
{
	SendCmdToAimp3(AIMP_RA_CMD_PAUSE);
}

void CPlayerService::PlayPause()
{
	SendCmdToAimp3(AIMP_RA_CMD_PLAYPAUSE);
}

void CPlayerService::Stop()
{
	SendCmdToAimp3(AIMP_RA_CMD_STOP);
}

void CPlayerService::Prev()
{
	SendCmdToAimp3(AIMP_RA_CMD_PREV);
}

void CPlayerService::Next()
{
	SendCmdToAimp3(AIMP_RA_CMD_NEXT);
}

void CPlayerService::SeekTo(int nMsPos)
{
	if (!g_hWndAimp3)
		return;

	::SendMessage (g_hWndAimp3, WM_AIMP_PROPERTY, (WPARAM)(AIMP_RA_PROPERTY_PLAYER_POSITION | AIMP_RA_PROPVALUE_SET), (LPARAM)nMsPos);
}

void CPlayerService::OnNowPlayingCheck()
{
	if (!g_hWndAimp3)
		return;

	int state = ::SendMessage (g_hWndAimp3, WM_AIMP_PROPERTY, (WPARAM)(AIMP_RA_PROPERTY_PLAYER_STATE | AIMP_RA_PROPVALUE_GET), (LPARAM)0);
	static int statePrev = -1;
	if (statePrev != state) {
		statePrev = state;
		switch (state) {
			case 0: OnPlayStateChanged(PS_STOPED); break;
			case 1: OnPlayStateChanged(PS_PAUSED); break;
			case 2: OnPlayStateChanged(PS_PLAYING); break;
		}
	}

	LPCWSTR buf = (LPCWSTR)((char *)g_info + sizeof(TAIMPRemoteFileInfo));
	static int totalLength = 0;
	if (g_pInfoCache == NULL)
		g_pInfoCache = new WCHAR[AIMPRemoteAccessMapFileSize / sizeof(WCHAR)];

	// Compare whether track changed.
	int totalLengthCur = g_info->AlbumLength + g_info->ArtistLength + g_info->DateLength + g_info->FileNameLength + g_info->GenreLength + g_info->TitleLength;
	if (totalLengthCur == totalLength && memcmp(g_pInfoCache, buf, totalLengthCur) == 0)
		return;

	// Store current.
	memcpy(g_pInfoCache, buf, totalLengthCur);
	totalLength = totalLengthCur;

	// Extract data from the buffer
	wstring album, artist, title, fileName;
	ExtractString(buf, g_info->AlbumLength, album);
	ExtractString(buf, g_info->ArtistLength, artist);
	buf += g_info->DateLength;
	ExtractString(buf, g_info->FileNameLength, fileName);
	buf += g_info->GenreLength;
	ExtractString(buf, g_info->TitleLength, title);

	g_MLService.OnMediaInfoChanged(fileName.c_str(), artist.c_str(), album.c_str(), title.c_str(), g_info->Duration, false);
}

void CPlayerService::OnPlayStateChanged(PLAYER_STATE state)
{
	if (state == PS_PLAYING)
	{
		SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);
	}
	else if (state == PS_PAUSED)
	{
		KillTimer(TIMER_GETPOS);
	}
	else
	{
		KillTimer(TIMER_GETPOS);
	}

	g_MLService.OnPlayStateChanged(state);
}

void CPlayerService::OnPlayTimeChangeCheck()
{
	int pos = (int)::SendMessage (g_hWndAimp3, WM_AIMP_PROPERTY, (WPARAM)(AIMP_RA_PROPERTY_PLAYER_POSITION | AIMP_RA_PROPVALUE_GET), (LPARAM)0);

// 	DWORD dwTimeNew = GetTickCount();
// 	m_nPos = dwTimeNew - m_dwTimeBeg;

	g_MLService.OnPlayPosChanged(pos);
}

void CPlayerService::SendCmdToAimp3(int nCmd)
{
	if (!g_hWndAimp3)
		return;

	::SendMessage (g_hWndAimp3, WM_AIMP_COMMAND, (WPARAM)nCmd, (LPARAM)0);
}

void CPlayerService::OnTimer(int nTimerID)
{
	if (!g_hWndAimp3)
		return;

	if (nTimerID == TIMER_GETPOS)
	{
		// update current play time.
		OnPlayTimeChangeCheck();
	}
	else if (nTimerID == TIMER_CHECK_MEDIA)
	{
		OnNowPlayingCheck();
	}
	else if (nTimerID == TIMER_CHECK_PLAYER)
	{
		if (g_hWndAimp3 != NULL && !IsWindow(g_hWndAimp3))
			g_MLService.OnDisconnected();
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
