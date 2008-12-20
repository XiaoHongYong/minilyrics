#include "stdafx.h"
#include "PlayerImp.h"

CPlayerImp		g_Player;
int				g_UpdateTimeSpan = 50;

///////////////////////////////////////////////////////////////////////////////
// create window to get playback positon timely.
LPCTSTR		szGetPosWndClass = _T("GetPosWnd");
LPCTSTR		szGetPosWndName = _T("GetPosWnd");
HWND		g_hWndGetPos = NULL;
const int	TIMER_GETPOS		= 1;
const int	TIMER_QUERYTITLE	= 2;

LRESULT CALLBACK GetPosWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL GetPosWndRegisterClass(HINSTANCE hInstance)
{
	WNDCLASS wc;

	if (GetClassInfo(hInstance, szGetPosWndClass, &wc))
		return TRUE;

	// register pane class
	wc.style = 0;
	wc.lpfnWndProc = (WNDPROC)GetPosWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon		= NULL;
	wc.hCursor		= NULL;
	wc.hbrBackground	= (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName	= NULL;
	wc.lpszClassName	= szGetPosWndClass;

	int r = RegisterClass(&wc);
	if (r == 0)
	{
		int res = GetLastError();
		if (res != ERROR_CLASS_ALREADY_EXISTS)
		{
			return FALSE;
		}
	}

	return TRUE;
}

BOOL GetPosWndCreate(HINSTANCE hInstance)
{
	hInstance; // Store instance handle in our global variable

	g_hWndGetPos = CreateWindow(szGetPosWndClass, szGetPosWndName, WS_OVERLAPPEDWINDOW,
		0, 0, 100, 100, NULL, NULL, hInstance, NULL);

	if (!g_hWndGetPos)
	{
		return FALSE;
	}

	ShowWindow(g_hWndGetPos, SW_MINIMIZE);
	ShowWindow(g_hWndGetPos, SW_HIDE);

	return TRUE;
}

LRESULT CALLBACK GetPosWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
	case WM_CREATE:
		SetTimer(hWnd, TIMER_GETPOS, g_UpdateTimeSpan, NULL);
		SetTimer(hWnd, TIMER_QUERYTITLE, 500, NULL);
		break;
	case WM_TIMER:
		if (wParam == TIMER_QUERYTITLE)
		{
			g_Player.OnMusicChangeCheck();
		}
		else if (wParam == TIMER_GETPOS)
			g_Player.OnPlayTimeChangeCheck();
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_GETPOS);
		KillTimer(hWnd, TIMER_QUERYTITLE);
		g_hWndGetPos = NULL;
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// End of GetPosWindow
////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
	EmptyStr(m_szFullTitle);
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

int CPlayerImp::GetCurSkinPath(char * lpszBuffer, int nBufSize)
{
	EmptyStr(lpszBuffer);

	return 1;
}

int CPlayerImp::GetProxy(char * lpszBuffer, int nBufSize)
{
	EmptyStr(lpszBuffer);

	return 1;
}

HWND CPlayerImp::GetMainWnd()
{
	return g_hwndPlayer;
}

// 返回歌曲的信息，格式为：artist - title		作者 － 歌曲名
int CPlayerImp::GetFullTitleW(WCHAR * lpszBuffer, int nBufSize)
{
	wcscpy_safe(lpszBuffer, nBufSize, m_szFullTitle);

	return 1;
}

// 返回歌曲的全文件名（如c:\\...., httP://wwww......)
int CPlayerImp::GetSourceMediaW(WCHAR * lpszBuffer, int nBufSize)
{
	wcscpy_safe(lpszBuffer, nBufSize, m_szSrcMedia);

	return 1;
}

// 歌曲的长度（单位ms）
DWORD CPlayerImp::GetMediaLength()
{
	return m_dwSongLength;
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

// 返回歌曲的信息，格式为：artist - title		作者 － 歌曲名
void CPlayerImp::OnSongChanged(LPCWSTR szFullTitle, LPCWSTR szSrcMedia)
{
	if (wcscmp(m_szSrcMedia, szSrcMedia) == 0)
		return;

	wcscpy_safe(m_szFullTitle, CountOf(m_szFullTitle), szFullTitle);
	wcscpy_safe(m_szSrcMedia, CountOf(m_szSrcMedia), szSrcMedia);

	g_MLService.OnFullTitleChangedW(szFullTitle);
}

// 歌曲播放位置改变了
void CPlayerImp::OnPlayTimeChanged(DWORD dwTimeCur)
{
	g_MLService.OnPlayTimeChanged(dwTimeCur);
}

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

	//
	// Create Window to get playback positon timely.
	//
	GetPosWndRegisterClass(g_hInstance);
	GetPosWndCreate(g_hInstance);

	return TRUE;
}

// COMMENT:
//		关闭WINAMP 状态查询线程
void CPlayerImp::Quit()
{
	if (g_hWndGetPos)
		DestroyWindow(g_hWndGetPos);
}

void CPlayerImp::SetTimeUpdateFreq(int nFreq)
{
	if (nFreq == 0)
		g_UpdateTimeSpan = 100;
	else if (nFreq == 1)
		g_UpdateTimeSpan = 50;
	else
		g_UpdateTimeSpan = 10;

	KillTimer(g_hWndGetPos, TIMER_GETPOS);
	SetTimer(g_hWndGetPos, TIMER_GETPOS, g_UpdateTimeSpan, NULL);
}

int CPlayerImp::GetArtistW(WCHAR * lpszBuffer, int nBufSize)
{
	lpszBuffer[0] = '\0';
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
				_bstr_t		str;
				str = file->GetArtist();
				wcscpy_safe(lpszBuffer, nBufSize, str);
			}
		}
	}
	return PR_OK;
}

int CPlayerImp::GetTitleW(WCHAR * lpszBuffer, int nBufSize)
{
	lpszBuffer[0] = '\0';
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
				_bstr_t		str;
				str = file->GetName();
				wcscpy_safe(lpszBuffer, nBufSize, str);
			}
		}
	}
	return PR_OK;
}

int CPlayerImp::GetAlbumW(WCHAR * lpszBuffer, int nBufSize)
{
	lpszBuffer[0] = '\0';
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
				_bstr_t		str;
				str = file->GetAlbum();
				wcscpy_safe(lpszBuffer, nBufSize, str);
			}
		}
	}
	return PR_OK;
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

				_bstr_t	title, artist;
				title = file->GetName();
				artist = file->GetArtist();

				wcscpy(m_szFullTitle, artist);
				wcscat(m_szFullTitle, L" - ");
				wcscat(m_szFullTitle, title);

				m_dwSongLength = file->GetDuration() * 1000;

				m_nPlaybackPosBeg = GetPlayPos();
				m_dwTimeBeg = GetTickCount();

				g_MLService.OnFullTitleChangedW(m_szFullTitle);
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
