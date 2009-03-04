#pragma warning (disable: 4311)

#pragma warning (disable: 4996)

#define _WIN32_WINNT  0x0500

#include <Windows.h>
#include <vlc/libvlc_structures.h>
#include <vlc/libvlc_events.h>
#include <vlc/mediacontrol.h>
#include <vlc_common.h>

#include <vlc_plugin.h>
#include <vlc_interface.h>
#include <vlc_meta.h>

VLC_EXPORT(void, __pl_Release, (vlc_object_t *));

#include <vlc_playlist.h>

int ReadMetaData(intf_thread_t *p_this);


#include "../MLPluginLib/MLPlugin.h"
#include "PlayerImp.h"
#include <tchar.h>

CPlayerImp		g_Player;
int				g_UpdateTimeSpan = 50;

///////////////////////////////////////////////////////////////////////////////
// create window to get playback pos of foobar timely.
LPCTSTR	szGetPosWndClass = _T("GetPosWnd");
LPCTSTR szGetPosWndName = _T("GetPosWnd");
HWND	g_hWndGetPos = NULL;
const int	TIMER_GETPOS	= 0;

enum NotifyMessage
{
	WM_PLAYER_STAT_CHANGED			= WM_USER + 1,
	WM_PLAYER_NOW_PLAYING_CHANGED,
};

void PostNotifyMsgToML(NotifyMessage msg)
{
	PostMessage(g_hWndGetPos, msg, 0, 0);
}

LRESULT CALLBACK GetPosWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL GetPosWndRegisterClass(HINSTANCE hInstance)
{
	WNDCLASS wc;

	if (GetClassInfo(hInstance, szGetPosWndClass, &wc))
		return TRUE;

	// regiszter pane class
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
		g_MLService.OnPlayTimeChanged(g_Player.GetPlayPos());
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		KillTimer(hWnd, TIMER_GETPOS);
		g_hWndGetPos = NULL;
		break;
	case WM_PLAYER_STAT_CHANGED:
		g_MLService.OnPlayStateChanged(g_Player.m_stat);
		break;
	case WM_PLAYER_NOW_PLAYING_CHANGED:
		g_Player.MediaChangedCheck();
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

// End of GetPosWindow
////////////////////////////////////////////////////////////////////////////////////


CPlayerImp::CPlayerImp()
{
	m_stat = PS_STOPED;
	m_p_intf = NULL;
}

CPlayerImp::~CPlayerImp()
{
}

// Player Info
int CPlayerImp::GetPlayerName(char * lpszBuffer, int nBufSize)
{
	strcpy_safe(lpszBuffer, nBufSize, "VLC Media Player");

	return 1;
}

// get player state and song info
DWORD CPlayerImp::GetPlayerState()
{
	return m_stat;
}

HWND CPlayerImp::GetMainWnd()
{
	return g_hwndPlayer;
}

int CPlayerImp::GetArtistW(WCHAR * lpszBuffer, int nBufSize)
{
	wcscpy_safe(lpszBuffer, nBufSize, m_strArtist.c_str());
	return PR_OK;
}

int CPlayerImp::GetAlbumW(WCHAR * lpszBuffer, int nBufSize)
{
	wcscpy_safe(lpszBuffer, nBufSize, m_strAlbum.c_str());
	return PR_OK;
}

int CPlayerImp::GetTitleW(WCHAR * lpszBuffer, int nBufSize)
{
	wcscpy_safe(lpszBuffer, nBufSize, m_strTitle.c_str());
	return PR_OK;
}

// 返回歌曲的全文件名（如c:\\...., httP;//wwww......)
int CPlayerImp::GetSourceMediaW(WCHAR * lpszSourceMedia, int nBufSize)
{
	wcscpy_safe(lpszSourceMedia, nBufSize, m_strSrcMedia.c_str());

	return 1;
}

// 歌曲的长度（单位ms）
DWORD CPlayerImp::GetMediaLength()
{
	if (!m_p_intf)
		return 0;

	input_thread_t *p_input;
	playlist_t *p_playlist = pl_Yield(m_p_intf);

	p_input = p_playlist->p_input;

	if (!p_input || p_input->b_dead)
	{
		pl_Release(p_playlist);
		return 0;
	}

	vlc_object_yield(p_input);

	int64_t		nLength = 0;

	input_Control(p_input, INPUT_GET_LENGTH, &nLength);

	vlc_object_release(p_input);
	pl_Release(m_p_intf);

	return nLength / 1000;
}

// 歌曲播放的位置（单位ms)
DWORD CPlayerImp::GetPlayPos()
{
	if (!m_p_intf)
		return 0;

	input_thread_t *p_input;
	playlist_t *p_playlist = pl_Yield(m_p_intf);

	p_input = p_playlist->p_input;

	if (!p_input || p_input->b_dead)
	{
		pl_Release(m_p_intf);
		return 0;
	}

	vlc_object_yield(p_input);

	int64_t		nPos = 0;

	input_Control(p_input, INPUT_GET_TIME, &nPos);

	vlc_object_release(p_input);
	pl_Release(m_p_intf);

	return nPos / 1000;
}

int CPlayerImp::HasVideo()
{
	if (!m_p_intf)
		return 0;

	input_thread_t *p_input;
	playlist_t *p_playlist = pl_Yield(m_p_intf);

	p_input = p_playlist->p_input;

	if (!p_input || p_input->b_dead)
	{
		pl_Release(m_p_intf);
		return 0;
	}

	vlc_object_yield(p_input);

	double	dbFps;

	int ret = input_Control(p_input, INPUT_GET_VIDEO_FPS, &dbFps);

	vlc_object_release(p_input);
	pl_Release(m_p_intf);

	if (ret == VLC_SUCCESS && dbFps > 0.0)
		return PR_OK;

	return PR_UNIMPLEMENTED;
}

// control player
BOOL CPlayerImp::Play()
{
	if (!m_p_intf)
		return 0;

	playlist_t *p_playlist = pl_Yield(m_p_intf);

	playlist_Play(p_playlist);

	pl_Release(m_p_intf);

	return TRUE;
}

BOOL CPlayerImp::Pause()
{
	if (!m_p_intf)
		return 0;

	playlist_t *p_playlist = pl_Yield(m_p_intf);

	playlist_Pause(p_playlist);

	pl_Release(m_p_intf);

	return TRUE;
}

BOOL CPlayerImp::PlayPause()
{
	if (!m_p_intf)
		return 0;

	playlist_t *p_playlist = pl_Yield(m_p_intf);

	if (m_stat != PS_PLAYING)
		playlist_Play(p_playlist);
	else
		playlist_Pause(p_playlist);

	pl_Release(m_p_intf);

	return TRUE;
}

BOOL CPlayerImp::Stop()
{
	if (!m_p_intf)
		return 0;

	playlist_t *p_playlist = pl_Yield(m_p_intf);

	playlist_Stop(p_playlist);

	pl_Release(m_p_intf);

	return TRUE;
}

BOOL CPlayerImp::Prev()
{
	if (!m_p_intf)
		return 0;

	playlist_t *p_playlist = pl_Yield(m_p_intf);

	playlist_Prev(p_playlist);

	pl_Release(m_p_intf);

	return TRUE;
}

BOOL CPlayerImp::Next()
{
	if (!m_p_intf)
		return 0;

	playlist_t *p_playlist = pl_Yield(m_p_intf);

	playlist_Next(p_playlist);

	pl_Release(m_p_intf);

	return TRUE;
}

BOOL CPlayerImp::SeekTo(int nMsPos)
{
	if (!m_p_intf)
		return 0;

	input_thread_t *p_input;
	playlist_t *p_playlist = pl_Yield(m_p_intf);

	p_input = p_playlist->p_input;

	if (!p_input || p_input->b_dead)
	{
		pl_Release(m_p_intf);
		return 0;
	}

	vlc_object_yield(p_input);

	input_Control(p_input, INPUT_SET_TIME, nMsPos * 1000);

	vlc_object_release(p_input);
	pl_Release(m_p_intf);

	return TRUE;
}

BOOL CPlayerImp::Init()
{
	//
	// Create Window to get playback pos timely.
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
	{
		DestroyWindow(g_hWndGetPos);
		g_hWndGetPos = NULL;
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

void CPlayerImp::NotifyPlayerStatChanged(PLAYER_STATE newStat)
{
	m_stat = newStat;
	PostNotifyMsgToML(WM_PLAYER_STAT_CHANGED);
}

void CPlayerImp::MediaChangedCheck()
{
	playlist_t          *p_playlist;
	input_thread_t      *p_input;
	input_item_t		*p_item;
	static char			sz_LastFile[256] = { 0 };

	char *psz_title = NULL;
	char *psz_artist = NULL;
	char *psz_album = NULL;

	intf_sys_t          *p_sys = m_p_intf->p_sys;

	p_playlist = pl_Yield(m_p_intf);
	PL_LOCK;
	p_input = p_playlist->p_input;
	if (!p_input)
	{
		PL_UNLOCK;
		pl_Release(m_p_intf);
		return;
	}

	vlc_object_yield(p_input);
	PL_UNLOCK;
	pl_Release(m_p_intf);

	if (p_input->b_dead || !input_GetItem(p_input)->psz_name)
	{
		/* Not playing anything ... */
		vlc_object_release(p_input);
		return;
	}

	p_item = input_GetItem(p_input);
	if (strcmp(sz_LastFile, p_item->psz_uri) == 0)
	{
		// song file Not changed.
		vlc_object_release(p_input);
		return;
	}

	wchar_t		wszTemp[260];

	EmptyStr(wszTemp);
	UTF8ToUCS2(p_item->psz_uri, -1, wszTemp, CountOf(wszTemp));
	m_strSrcMedia = wszTemp;

	m_strArtist.resize(0);
	m_strAlbum.resize(0);
	m_strTitle.resize(0);

	psz_artist = input_item_GetArtist(p_item);
	if (psz_artist)
	{
		UTF8ToUCS2(psz_artist, -1, wszTemp, CountOf(wszTemp));
		m_strArtist = wszTemp;
	}

	psz_album = input_item_GetAlbum(p_item);
	if (psz_album)
	{
		UTF8ToUCS2(psz_album, -1, wszTemp, CountOf(wszTemp));
		m_strAlbum = wszTemp;
	}

	psz_title = input_item_GetTitle(p_item);
	if (!psz_title)
		psz_title = input_item_GetName(p_item);
	if (psz_title)
	{
		UTF8ToUCS2(psz_title, -1, wszTemp, CountOf(wszTemp));
		m_strTitle = wszTemp;
	}

	wstring		strFullTitle;

	FormatFullTitle(m_strArtist.c_str(), m_strTitle.c_str(), m_strSrcMedia.c_str(), strFullTitle);

	g_MLService.OnFullTitleChangedW(strFullTitle.c_str());

	vlc_object_release(p_input);
}


//
//////////////////////////////////////////////////////////////////////////
// VLC plugin functions.
//


#define N_(str) (str)

struct intf_sys_t
{
	bool		bMetaDataRead;
};


int  Open(vlc_object_t *);
void Close(vlc_object_t *);

int ItemChange(vlc_object_t *, const char *,
		vlc_value_t, vlc_value_t, void *);

int PlayingChange(vlc_object_t *, const char *, vlc_value_t,
		vlc_value_t, void *);

void Run(intf_thread_t *p_intf);

#define MODULE_NAME  "MiniLyrics"
#define MODULE_STRING  "MiniLyrics"
#define MODULE_NAME_IS_  "MiniLyrics"

vlc_module_begin();
set_category(CAT_INTERFACE);
set_subcategory(SUBCAT_INTERFACE_CONTROL);
set_shortname(N_("MiniLyrics"));
set_description(N_("MiniLyrics - Auto display lyrics for current playing song"));
// add_submodule();
set_capability("interface", 0);
set_callbacks(Open, Close);
vlc_module_end();

static DWORD	g_dwMLThreadID = 0;
HANDLE			g_hEventOK2Quit = NULL;

int  Open(vlc_object_t * p_this)
{
	intf_thread_t *p_intf = (intf_thread_t *)p_this;
	playlist_t *p_playlist;

	MALLOC_ERR(p_intf->p_sys, intf_sys_t);

	p_playlist = pl_Yield(p_intf);
	var_AddCallback(p_playlist, "item-change", ItemChange, p_intf);
	var_AddCallback(p_playlist, "playlist-current", ItemChange, p_intf);
	pl_Release(p_intf);

	g_hwndPlayer = NULL;

	p_intf->pf_run = Run;

	g_hEventOK2Quit = CreateEvent(NULL, false, false, NULL);

	return VLC_SUCCESS;
}


void Close(vlc_object_t * p_this)
{
	intf_thread_t *p_intf = (intf_thread_t *)p_this;
	playlist_t *p_playlist = pl_Yield(p_this);
	input_thread_t              *p_input;

	var_DelCallback(p_playlist, "item-change", ItemChange, p_intf);
	var_DelCallback(p_playlist, "playlist-current", ItemChange, p_intf);

	p_input = p_playlist->p_input;
	if (p_input)
	{
		vlc_object_yield(p_input);
		var_DelCallback(p_input, "state", PlayingChange, p_intf);
		vlc_object_release(p_input);
	}

	pl_Release(p_this);

	PostThreadMessage(g_dwMLThreadID, WM_QUIT, 0, 0);
	WaitForSingleObject(g_hEventOK2Quit, INFINITE);
	CloseHandle(g_hEventOK2Quit);

	/* Destroy structure */
	free(p_intf->p_sys);
}

void Run(intf_thread_t *p_intf)
{
	g_Player.m_p_intf = p_intf;

	if (!InitMiniLyricsPlugin(g_hInstance, &g_Player))
		return;

	g_dwMLThreadID = GetCurrentThreadId();

	// now we can use other g_MLService functions.
	char	szMsg[256];

	wsprintfA(szMsg, "Start Foobar2000 plugin for minilyrics. HINSTANCE: %#X", g_hInstance);
	g_MLService.Log(szMsg);

	MSG			msg;

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		vlc_object_lock(p_intf);
		if (!vlc_object_alive(p_intf))
		{
			vlc_object_unlock(p_intf);
			break;
		}
		vlc_object_unlock(p_intf);
	}

	g_MLService.Quit();

	SetEvent(g_hEventOK2Quit);
}

/*****************************************************************************
* PlayingChange: Playing status change callback
*****************************************************************************/
static int PlayingChange(vlc_object_t *p_this, const char *psz_var,
						 vlc_value_t oldval, vlc_value_t newval, void *p_data)
{
	intf_thread_t   *p_intf = (intf_thread_t*) p_data;
	intf_sys_t      *p_sys  = p_intf->p_sys;

	VLC_UNUSED(p_this); VLC_UNUSED(psz_var);

	if (!p_sys->bMetaDataRead && newval.i_int >= PLAYING_S)
	{
		p_sys->bMetaDataRead = true;
		PostNotifyMsgToML(WM_PLAYER_NOW_PLAYING_CHANGED);
	}

	if (newval.i_int >= END_S)
	{
		g_Player.NotifyPlayerStatChanged(PS_STOPED);
	}
	else if (newval.i_int == PAUSE_S)
	{
		g_Player.NotifyPlayerStatChanged(PS_PAUSED);
	}
	else if (newval.i_int == PLAYING_S)
	{
		g_Player.NotifyPlayerStatChanged(PS_PLAYING);
	}

	return VLC_SUCCESS;
}

/*****************************************************************************
* ItemChange: Playlist item change callback
*****************************************************************************/
static int ItemChange(vlc_object_t *p_this, const char *psz_var,
					  vlc_value_t oldval, vlc_value_t newval, void *param)
{
	(void)psz_var;    (void)oldval;    (void)newval;
	intf_thread_t *p_intf = (intf_thread_t *)param;
	input_thread_t *p_input;
	playlist_t *p_playlist = pl_Yield(p_this);
	intf_sys_t          *p_sys = p_intf->p_sys;

	p_input = p_playlist->p_input;
	pl_Release(p_this);

	if (!p_input)
		return VLC_SUCCESS;
	vlc_object_yield(p_input);

	if (p_input->b_dead || !input_GetItem(p_input)->psz_name)
	{
		/* Not playing anything ... */
		vlc_object_release(p_input);

		g_Player.m_strArtist.resize(0);
		g_Player.m_strAlbum.resize(0);
		g_Player.m_strTitle.resize(0);
		g_Player.m_strSrcMedia.resize(0);
		g_MLService.OnFullTitleChangedW(L"");
		return VLC_SUCCESS;
	}

	var_AddCallback(p_input, "state", PlayingChange, p_intf);

	p_sys->bMetaDataRead = false;
	/* Playing something ... */
	if (input_item_IsPreparsed(input_GetItem(p_input)))
	{
		p_sys->bMetaDataRead = true;
		PostNotifyMsgToML(WM_PLAYER_NOW_PLAYING_CHANGED);
	}

	vlc_object_release(p_input);

	return VLC_SUCCESS;
}

BOOL APIENTRY DllMain(HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					 )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		g_hInstance = hModule;

	return TRUE;
}

