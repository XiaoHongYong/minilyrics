#include "stdafx.h"
#include "PlayerImp.h"

CPlayerImp		g_Player;
int				g_UpdateTimeSpan = 50;

///////////////////////////////////////////////////////////////////////////////
// create window to get playback positon timely.
LPCTSTR	szGetPosWndClass = _T("GetPosWnd");
LPCTSTR szGetPosWndName = _T("GetPosWnd");
HWND	g_hWndGetPos = NULL;
const int	TIMER_GETPOS	= 1;

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
		break;
	case WM_TIMER:
		g_Player.OnPlayTimeChanged(g_Player.GetPlayPos());
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_GETPOS);
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
	EmptyStr(m_szArtist);
	EmptyStr(m_szAlbum);
	EmptyStr(m_szTitle);
	EmptyStr(m_szFullTitle);
}

CPlayerImp::~CPlayerImp()
{
}

// Player Info
int CPlayerImp::GetPlayerName(char * lpszBuffer, int nBufSize)
{
	strcpy_safe(lpszBuffer, nBufSize, "Yahoo! Music Jukebox");

	return 1;
}

// get player state and song info
DWORD CPlayerImp::GetPlayerState()
{
	IRMPPlayback	*pPlayBack;
	PLAYER_STATE	playState = PS_STOPED;
	if (m_pMediaPlayer.p != NULL && SUCCEEDED(m_pMediaPlayer->get_Playback((IDispatch * *)&pPlayBack)))
	{
		long		state;
		if (SUCCEEDED(pPlayBack->get_State(&state)))
		{
			if (state == PLAYBACK_STATUS_PAUSED)
				playState = PS_PAUSED;
			else if (state == PLAYBACK_STATUS_PLAYING)
				playState = PS_PLAYING;
			else
				playState = PS_STOPED;
		}
		pPlayBack->Release();
	}

	return playState;
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

int CPlayerImp::GetArtistW(WCHAR *lpszBuffer, int nBufSize)
{
	wcscpy_safe(lpszBuffer, nBufSize, m_szArtist);
	return PR_OK;
}

int CPlayerImp::GetAlbumW(WCHAR *lpszBuffer, int nBufSize)
{
	wcscpy_safe(lpszBuffer, nBufSize, m_szAlbum);
	return PR_OK;
}

int CPlayerImp::GetTitleW(WCHAR *lpszBuffer, int nBufSize)
{
	wcscpy_safe(lpszBuffer, nBufSize, m_szTitle);
	return PR_OK;
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
	double			dbTime = 0;
	IRMPPlayback	*pPlayBack;
	if (m_pMediaPlayer.p != NULL && SUCCEEDED(m_pMediaPlayer->get_Playback((IDispatch **)&pPlayBack)))
	{
		if (SUCCEEDED(pPlayBack->GetPosition(&dbTime)))
		{
		}
		pPlayBack->Release();
	}

	return (DWORD)(dbTime);
}

// 返回歌曲的信息，格式为：artist - title		作者 － 歌曲名
void CPlayerImp::OnSongChanged(LPCWSTR szFullTitle, LPCWSTR szSrcMedia)
{
	if (wcscmp(m_szSrcMedia, szSrcMedia) == 0 && wcscmp(m_szSrcMedia, szSrcMedia) == 0)
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
/*
	IRMPPlayback	*pPlayBack;
	if (m_pMediaPlayer.p != NULL && SUCCEEDED(m_pMediaPlayer->get_Playback((IDispatch **)&pPlayBack)))
	{
		if (SUCCEEDED(pPlayBack->Pause(false)))
		{
		}
		pPlayBack->Release();
	}
*/
	// now use the remote...for example, tell YME to skip to the next song in the playlist:
	if (MakeRemoteCtrlOK())
		m_spRemote->Command(YRC_PLAY, _variant_t(), _variant_t());

	return TRUE;
}

BOOL CPlayerImp::Pause()
{
/*
	IRMPPlayback	*pPlayBack;
	if (m_pMediaPlayer.p != NULL && SUCCEEDED(m_pMediaPlayer->get_Playback((IDispatch **)&pPlayBack)))
	{
		if (SUCCEEDED(pPlayBack->Pause(true)))
		{
		}
		pPlayBack->Release();
	}
*/
	if (MakeRemoteCtrlOK())
		m_spRemote->Command(YRC_PAUSE, _variant_t(), _variant_t());

	return TRUE;
}

BOOL CPlayerImp::PlayPause()
{
	if (GetPlayerState() == PS_PLAYING)
		Pause();
	else
		Play();

	return TRUE;
}

BOOL CPlayerImp::Stop()
{
/*
	IRMPPlayback	*pPlayBack;
	if (m_pMediaPlayer.p != NULL && SUCCEEDED(m_pMediaPlayer->get_Playback((IDispatch **)&pPlayBack)))
	{
		if (SUCCEEDED(pPlayBack->Stop()))
		{
		}
		pPlayBack->Release();
	}
*/
	if (MakeRemoteCtrlOK())
		m_spRemote->Command(YRC_STOP, _variant_t(), _variant_t());

	return TRUE;
}

BOOL CPlayerImp::Prev()
{
	if (MakeRemoteCtrlOK())
		m_spRemote->Command(YRC_SKIP_FWD, _variant_t(), _variant_t());

	return TRUE;
}

BOOL CPlayerImp::Next()
{
	if (MakeRemoteCtrlOK())
		m_spRemote->Command(YRC_SKIP_REV, _variant_t(), _variant_t());

	return TRUE;
}

BOOL CPlayerImp::SeekTo(int nMsPos)
{
/*
	IRMPPlayback	*pPlayBack;
	if (m_pMediaPlayer.p != NULL && SUCCEEDED(m_pMediaPlayer->get_Playback((IDispatch **)&pPlayBack)))
	{
		VARIANT_BOOL	bSucc;
		if (SUCCEEDED(pPlayBack->Seek(double(nMsPos), &bSucc)))
		{
		}
		pPlayBack->Release();
	}
*/
	if (MakeRemoteCtrlOK())
		m_spRemote->Command(YRC_SEEK, _variant_t(nMsPos), _variant_t());

	return TRUE;
}

bool CPlayerImp::MakeRemoteCtrlOK()
{
	if (m_spRemote.GetInterfacePtr() == NULL)
	{
		HRESULT hr = m_spRemote.CreateInstance("YMP.YMPRemote");

		m_remoteCookie = 0; // assigned to your instance of the remote control interface
		// initialize the control before you begin:
		if (SUCCEEDED(hr))
			m_spRemote->Initialize(&m_remoteCookie);
	}

	return m_spRemote.GetInterfacePtr() != NULL;
}

BOOL CPlayerImp::Init()
{
	g_hwndPlayer = FindWindowA("YMPFrame", NULL);
	g_MLService.AddWndCloseto(NULL, "YMPFrame", NULL);

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

	// when you're done, call shutdown:
	if (m_spRemote.GetInterfacePtr() != NULL)
	{
		m_spRemote->Shutdown(m_remoteCookie);
		m_spRemote.Release();
	}
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
