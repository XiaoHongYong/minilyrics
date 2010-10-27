#pragma warning (disable: 4311)

#pragma warning (disable: 4996)

#define _WIN32_WINNT  0x0500

#include <Windows.h>
#include <vlc/libvlc.h>
#include <vlc/libvlc_structures.h>
#include <vlc/libvlc_media.h>
#include <vlc/libvlc_media_player.h>
// #include <vlc/libvlc_events.h>
#include <vlc_common.h>

#include <vlc_plugin.h>
#include <vlc_interface.h>
#include <vlc_meta.h>

VLC_EXPORT(void, __pl_Release, (vlc_object_t *));

#include <vlc_playlist.h>


#include "../MLPluginLib/MLPlugin.h"
#include "PlayerImp.h"
#include <tchar.h>

CPlayerImp		g_Player;


class CPlayerInfoQueryWnd : public CPlayerInfoQueryBaseWnd
{
public:
	enum NotifyMessage
	{
		WM_PLAYER_STAT_CHANGED			= WM_USER + 1,
		WM_PLAYER_NOW_PLAYING_CHANGED,
	};

	void PostNotifyMsgToML(NotifyMessage msg)
	{
		PostMessage(m_hWnd, msg, 0, 0);
	}

	void OnCreate()
	{
		SetTimer(m_hWnd, TIMER_GETPOS, TIME_OUT_UPDATE_POS, NULL);
	}

	void OnDestroy()
	{
		KillTimer(m_hWnd, TIMER_GETPOS);
	}

	void OnTimer(int nTimerID)
	{
		if (nTimerID == TIMER_GETPOS)
			g_MLService.OnPlayTimeChanged(g_Player.GetPlayPos());
	}

	virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_PLAYER_STAT_CHANGED:
			g_MLService.OnPlayStateChanged(g_Player.m_stat);
			break;
		case WM_PLAYER_NOW_PLAYING_CHANGED:
			g_Player.MediaChangedCheck();
			break;
		default:
			return CPlayerInfoQueryBaseWnd::WndProc(uMsg, wParam, lParam);
		}
		return 1;
	}

};

CPlayerInfoQueryWnd	g_wndInfoQuery;


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

// 歌曲播放的位置（单位ms)
DWORD CPlayerImp::GetPlayPos()
{
	if (!m_p_intf)
		return 0;

	input_thread_t *p_input;
	playlist_t *p_playlist = pl_Get( m_p_intf );

	p_input = playlist_CurrentInput(p_playlist);

	if (!p_input || p_input->b_dead)
	{
		return 0;
	}

	int64_t		nPos = 0;

	input_Control(p_input, INPUT_GET_TIME, &nPos);

	vlc_object_release(p_input);

	return nPos / 1000;
}

int CPlayerImp::HasVideo()
{
	if (!m_p_intf)
		return 0;

	input_thread_t *p_input;
	playlist_t *p_playlist = pl_Get(m_p_intf);

	p_input = playlist_CurrentInput(p_playlist);

	if (!p_input || p_input->b_dead)
	{
		return 0;
	}

	double	dbFps;

	int ret = input_Control(p_input, INPUT_GET_VIDEO_FPS, &dbFps);

	vlc_object_release(p_input);

	if (ret == VLC_SUCCESS && dbFps > 0.0)
		return PR_OK;

	return PR_UNIMPLEMENTED;
}

// control player
BOOL CPlayerImp::Play()
{
	if (!m_p_intf)
		return 0;

	playlist_t *p_playlist = pl_Get(m_p_intf);

	playlist_Play(p_playlist);

	return TRUE;
}

BOOL CPlayerImp::Pause()
{
	if (!m_p_intf)
		return 0;

	playlist_t *p_playlist = pl_Get(m_p_intf);

	playlist_Pause(p_playlist);

	return TRUE;
}

BOOL CPlayerImp::PlayPause()
{
	if (!m_p_intf)
		return 0;

	playlist_t *p_playlist = pl_Get(m_p_intf);

	if (m_stat != PS_PLAYING)
		playlist_Play(p_playlist);
	else
		playlist_Pause(p_playlist);

	return TRUE;
}

BOOL CPlayerImp::Stop()
{
	if (!m_p_intf)
		return 0;

	playlist_t *p_playlist = pl_Get(m_p_intf);

	playlist_Stop(p_playlist);

	return TRUE;
}

BOOL CPlayerImp::Prev()
{
	if (!m_p_intf)
		return 0;

	playlist_t *p_playlist = pl_Get(m_p_intf);

	playlist_Prev(p_playlist);

	return TRUE;
}

BOOL CPlayerImp::Next()
{
	if (!m_p_intf)
		return 0;

	playlist_t *p_playlist = pl_Get(m_p_intf);

	playlist_Next(p_playlist);

	return TRUE;
}

BOOL CPlayerImp::SeekTo(int nMsPos)
{
	if (!m_p_intf)
		return 0;

	input_thread_t *p_input;
	playlist_t *p_playlist = pl_Get(m_p_intf);

	p_input = playlist_CurrentInput(p_playlist);

	if (!p_input || p_input->b_dead)
	{
		return 0;
	}

	input_Control(p_input, INPUT_SET_TIME, nMsPos * 1000);

	vlc_object_release(p_input);

	return TRUE;
}

BOOL CPlayerImp::Init()
{
	g_wndInfoQuery.Create(g_hInstance);

	return TRUE;
}

void CPlayerImp::Quit()
{
	g_wndInfoQuery.Destroy();
}

void CPlayerImp::OnPlayPosUpdateSpanChanged(int nPlayPosUpdateSpan)
{
	g_wndInfoQuery.SetUpdatePosTimer(nPlayPosUpdateSpan);
}

void CPlayerImp::NotifyPlayerStatChanged(PLAYER_STATE newStat)
{
	if (newStat != m_stat)
	{
		m_stat = newStat;
		g_wndInfoQuery.PostNotifyMsgToML(CPlayerInfoQueryWnd::WM_PLAYER_STAT_CHANGED);
	}
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

	p_playlist = pl_Get(m_p_intf);
	PL_LOCK;
	p_input = playlist_CurrentInput(p_playlist);
	if (!p_input)
	{
		PL_UNLOCK;
		return;
	}
	PL_UNLOCK;

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
	wstring		m_strArtist, m_strAlbum, m_strTitle, m_strSrcMedia;


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

	MLMediaInfo		mediaInfo;
	int64_t		nLength = 0;

	int	nRet = input_Control(p_input, INPUT_GET_LENGTH, &nLength);

	mediaInfo.szMediaUrl = m_strSrcMedia.c_str();
	mediaInfo.szArtist = m_strArtist.c_str();
	mediaInfo.szAlbum = m_strAlbum.c_str();
	mediaInfo.szTitle = m_strTitle.c_str();
	mediaInfo.nMediaLength = GetMediaLength();

	g_MLService.OnMediaInfoChanged(&mediaInfo);

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
set_shortname(N_(MODULE_NAME));
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

	p_intf->p_sys = new (intf_sys_t);
	if( !p_intf->p_sys )
		return VLC_ENOMEM;

	p_playlist = pl_Get(p_intf);
	PL_LOCK;
	var_AddCallback( p_playlist, "item-change", ItemChange, p_intf );
	var_AddCallback(p_playlist, "item-current", ItemChange, p_intf);
	PL_UNLOCK;

	g_hwndPlayer = NULL;

	p_intf->pf_run = Run;

	g_hEventOK2Quit = CreateEvent(NULL, false, false, NULL);

	return VLC_SUCCESS;
}


void Close(vlc_object_t * p_this)
{
	intf_thread_t *p_intf = (intf_thread_t *)p_this;
	playlist_t *p_playlist = pl_Get(p_this);
	input_thread_t              *p_input;

	var_DelCallback( p_playlist, "item-change", ItemChange, p_intf );
	var_DelCallback( p_playlist, "item-current", ItemChange, p_intf );

	p_input = playlist_CurrentInput( p_playlist );
	if ( p_input )
	{
		var_DelCallback(p_input, "intf-event", PlayingChange, p_intf);

		vlc_object_release( p_input );
	}

	PostThreadMessage(g_dwMLThreadID, WM_QUIT, 0, 0);
	WaitForSingleObject(g_hEventOK2Quit, INFINITE);
	CloseHandle(g_hEventOK2Quit);

	/* Destroy structure */
	delete p_intf->p_sys;
}

void Run(intf_thread_t *p_intf)
{
	g_Player.m_p_intf = p_intf;

	if (!InitMiniLyricsPlugin(g_hInstance, &g_Player))
		return;

	g_dwMLThreadID = GetCurrentThreadId();

	// now we can use other g_MLService functions.
	char	szMsg[256];

	wsprintfA(szMsg, "Start VLC Media Player plugin for minilyrics. HINSTANCE: %#X", g_hInstance);
	g_MLService.Log(szMsg);

	MSG			msg;

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (!vlc_object_alive(p_intf))
		{
			break;
		}
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
		g_wndInfoQuery.PostNotifyMsgToML(CPlayerInfoQueryWnd::WM_PLAYER_NOW_PLAYING_CHANGED);
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
	playlist_t *p_playlist = pl_Get(p_this);
	intf_sys_t          *p_sys = p_intf->p_sys;

	p_input = playlist_CurrentInput(p_playlist);

	if (!p_input)
		return VLC_SUCCESS;

	if (p_input->b_dead || !input_GetItem(p_input)->psz_name)
	{
		/* Not playing anything ... */
		vlc_object_release(p_input);

		g_MLService.OnMediaInfoChanged(NULL);
		return VLC_SUCCESS;
	}

	var_AddCallback(p_input, "intf-event", PlayingChange, p_intf);

	p_sys->bMetaDataRead = false;
	/* Playing something ... */
	if (input_item_IsPreparsed(input_GetItem(p_input)))
	{
		p_sys->bMetaDataRead = true;
		g_wndInfoQuery.PostNotifyMsgToML(CPlayerInfoQueryWnd::WM_PLAYER_NOW_PLAYING_CHANGED);
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


// 歌曲的长度（单位ms）
DWORD CPlayerImp::GetMediaLength()
{
	if (!m_p_intf)
		return 0;

	input_thread_t *p_input;
	playlist_t *p_playlist = pl_Get(m_p_intf);

	p_input = playlist_CurrentInput(p_playlist);

	if (!p_input || p_input->b_dead)
	{
		return 0;
	}

	int64_t		nPos = 0;

	input_Control(p_input, INPUT_GET_LENGTH, &nPos);

	vlc_object_release(p_input);

	return nPos / 1000;
}
