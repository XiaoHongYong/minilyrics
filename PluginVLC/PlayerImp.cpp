// Code based upon modules\misc\notify\msn.c

#pragma warning (disable: 4311)
#pragma warning (disable: 4996)

#define _WIN32_WINNT  0x0500
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#define HAVE_DIRFD 1
#define HAVE_FDOPENDIR
#define HAVE_GETPID
#define LIBVLC_USE_PTHREAD_CANCEL

typedef unsigned short ssize_t;

#include <Windows.h>
#include <winuser.h>

#include <stdint.h>

#include <vlc_common.h>
#include <vlc_plugin.h>
#include <vlc_interface.h>
#include <vlc_dialog.h>
#include <vlc_meta.h>
#include <vlc_md5.h>
#include <vlc_stream.h>
#include <vlc_url.h>
#include <vlc_network.h>
#include <vlc_playlist.h>

#include "../MLPluginLib/MLPlugin.h"
#include "PlayerImp.h"

/*
#define SENDDEBUGMESSAGE	19771212
#define	DEBUGCLASSNAME		_T("Debug Tracer Class")
#define	DEBUGWNDNAME		_T("Debug Tracer")

static HWND _hDebugWnd;

void LogTrace(LPVOID lpData, int nSize)
{
	COPYDATASTRUCT data;
	LPCSTR	szMsgAssii = (LPCSTR)lpData;

	data.dwData = SENDDEBUGMESSAGE;
	data.lpData = (LPVOID)szMsgAssii;
	data.cbData = strlen(szMsgAssii) + 1;

	if (_hDebugWnd == NULL)
		_hDebugWnd = FindWindow(DEBUGCLASSNAME, DEBUGWNDNAME);

	if (_hDebugWnd)
		::SendMessage(_hDebugWnd, WM_COPYDATA, NULL, (LPARAM)&data);
}

void DBGLOG0(LPCSTR szData)
{
	char		szBuffer[1024];
	SYSTEMTIME	LocalTime;
	int		nLen;
	GetLocalTime(&LocalTime);
	nLen = sprintf(szBuffer, ("|T%d-%d %d:%d:%d|M%s"), LocalTime.wMonth, LocalTime.wDay,
		LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond, szData);

	LogTrace((LPVOID)szBuffer, strlen(szBuffer));
}

class CMethodRecorder
{
public:
	CMethodRecorder(LPCSTR szName, int nLine)
	{
		m_szName = szName;
		m_nLine = nLine;
		char buf[256];
		sprintf(buf, "Enter %s:%d:%d", szName, nLine, GetCurrentThreadId());
		DBGLOG0(buf);
	}
	~CMethodRecorder()
	{
		char buf[256];
		sprintf(buf, "leave %s:%d:%d", m_szName, m_nLine, GetCurrentThreadId());
		DBGLOG0(buf);
	}

	int			m_nLine;
	LPCSTR		m_szName;

};

#define ENTER_METHOD(name)		CMethodRecorder recorder(name, __LINE__);*/

#define ENTER_METHOD(name)

CPlayerImp		g_Player;

HWND GetPlayerWindow()
{
	return FindWindow(_T("QWidget"), NULL);
}

CPlayerImp::CPlayerImp()
{
	m_stat = PS_STOPED;
	m_p_intf = NULL;
}

CPlayerImp::~CPlayerImp()
{
}


void CPlayerImp::OnGetInfo(PlayerInfo &playerInfo)
{
	ENTER_METHOD("OnGetInfo");
	//
	// Initialize player information for MiniLyrics
	//
	playerInfo.hWndPlayer = GetPlayerWindow();
	wcscpy_safe(playerInfo.playerName, CountOf(playerInfo.playerName), L"VLC Media Player");
}

void CPlayerImp::OnConnected()
{
	ENTER_METHOD("OnConnected");
	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);

	MediaChangedCheck();

	// Update player state
	g_MLService.OnPlayStateChanged(m_stat);
}

void CPlayerImp::OnDisconnect()
{
	ENTER_METHOD("OnDisconnect");
	// KillTimer(TIMER_GETPOS);
}

void CPlayerImp::Play()
{
	if (!m_p_intf)
		return;

	playlist_t *p_playlist = pl_Get(m_p_intf);

	playlist_Play(p_playlist);
}

void CPlayerImp::Pause()
{
	if (!m_p_intf)
		return;

	playlist_t *p_playlist = pl_Get(m_p_intf);

	playlist_Pause(p_playlist);
}

void CPlayerImp::PlayPause()
{
	if (!m_p_intf)
		return;

	playlist_t *p_playlist = pl_Get(m_p_intf);

	if (m_stat != PS_PLAYING)
		playlist_Play(p_playlist);
	else
		playlist_Pause(p_playlist);
}

void CPlayerImp::Stop()
{
	if (!m_p_intf)
		return;

	playlist_t *p_playlist = pl_Get(m_p_intf);

	playlist_Stop(p_playlist);
}

void CPlayerImp::Prev()
{
	if (!m_p_intf)
		return;

	playlist_t *p_playlist = pl_Get(m_p_intf);

	playlist_Prev(p_playlist);
}

void CPlayerImp::Next()
{
	if (!m_p_intf)
		return;

	playlist_t *p_playlist = pl_Get(m_p_intf);

	playlist_Next(p_playlist);
}

void CPlayerImp::SeekTo(int nMsPos)
{
	if (!m_p_intf)
		return;

	input_thread_t *p_input;
	playlist_t *p_playlist = pl_Get(m_p_intf);

	p_input = playlist_CurrentInput(p_playlist);

	if (!p_input || p_input->b_dead)
		return;

	int64_t		nPos = (int64_t)nMsPos * 1000;
	input_Control(p_input, INPUT_SET_TIME, nPos);

	vlc_object_release(p_input);
}

void CPlayerImp::OnTimer(int nTimerID)
{
	if (m_stat == PS_PLAYING)
		g_MLService.OnPlayPosChanged(GetPlayPos());
}

DWORD CPlayerImp::GetPlayPos()
{
	ENTER_METHOD("GetPlayPos");
	if (!m_p_intf)
		return 0;

	input_thread_t *p_input;
	playlist_t *p_playlist = pl_Get( m_p_intf );

	p_input = playlist_CurrentInput(p_playlist);

	if (!p_input || p_input->b_dead)
		return 0;

	int64_t		nPos = 0;

	input_Control(p_input, INPUT_GET_TIME, &nPos);

	vlc_object_release(p_input);

	nPos /= 1000;
	if (nPos != m_posLast)
	{
		m_posLast = (DWORD)nPos;
		m_dwTimeBeg = GetTickCount() - (DWORD)nPos;
		return (DWORD)nPos;
	}
	else
		return GetTickCount() - m_dwTimeBeg;
}

int CPlayerImp::HasVideo()
{
	if (!m_p_intf)
		return false;

	input_thread_t *p_input;
	playlist_t *p_playlist = pl_Get(m_p_intf);

	p_input = playlist_CurrentInput(p_playlist);

	if (!p_input || p_input->b_dead)
		return false;

	double	dbFps;

	int ret = input_Control(p_input, INPUT_GET_VIDEO_FPS, &dbFps);

	vlc_object_release(p_input);

	if (ret == VLC_SUCCESS && dbFps > 0.0)
		return true;

	return false;
}

void CPlayerImp::NotifyPlayerStatChanged(PLAYER_STATE newStat)
{
	ENTER_METHOD("NotifyPlayerStatChanged");
	if (newStat != m_stat)
	{
		m_stat = newStat;
		g_MLService.OnPlayStateChanged(m_stat);
	}
}

void CPlayerImp::MediaChangedCheck()
{
	ENTER_METHOD("MediaChangedCheck");
	playlist_t          *p_playlist;
	input_thread_t      *p_input;
	input_item_t		*p_item;
	static char			sz_LastFile[256] = { 0 };

	char *psz_title = NULL;
	char *psz_artist = NULL;
	char *psz_album = NULL;

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

	static char szUriOld[MAX_PATH] = "";
	if (strcmp(p_item->psz_uri, szUriOld) == 0)
		return;
	strcpy_safe(szUriOld, CountOf(szUriOld), p_item->psz_uri);

	EmptyStr(wszTemp);
	Utf8ToUCS2(p_item->psz_uri, -1, wszTemp, CountOf(wszTemp));
	m_strSrcMedia = wszTemp;

	m_strArtist.resize(0);
	m_strAlbum.resize(0);
	m_strTitle.resize(0);

	psz_artist = input_item_GetArtist(p_item);
	if (psz_artist)
	{
		Utf8ToUCS2(psz_artist, -1, wszTemp, CountOf(wszTemp));
		m_strArtist = wszTemp;
	}

	psz_album = input_item_GetAlbum(p_item);
	if (psz_album)
	{
		Utf8ToUCS2(psz_album, -1, wszTemp, CountOf(wszTemp));
		m_strAlbum = wszTemp;
	}

	psz_title = input_item_GetTitle(p_item);
	if (!psz_title)
		psz_title = input_item_GetName(p_item);
	if (psz_title)
	{
		Utf8ToUCS2(psz_title, -1, wszTemp, CountOf(wszTemp));
		m_strTitle = wszTemp;
	}

	int64_t		nLength = 0;
	int	nRet = input_Control(p_input, INPUT_GET_LENGTH, &nLength);

	g_MLService.OnMediaInfoChanged(m_strSrcMedia.c_str(), m_strArtist.c_str(), m_strAlbum.c_str(),
		m_strTitle.c_str(), GetMediaLength());

	vlc_object_release(p_input);
}


//
//////////////////////////////////////////////////////////////////////////
// VLC plugin functions.
//

#define N_(str) (str)

bool		g_bMetaDataRead;


int  Open(vlc_object_t *);
void Close(vlc_object_t *);

int ItemChange(vlc_object_t *, const char *,
		vlc_value_t, vlc_value_t, void *);

int PlayingChange(vlc_object_t *, const char *, vlc_value_t,
		vlc_value_t, void *);

DWORD __stdcall Run(void *data);

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

struct intf_sys_t
{
	DWORD					threadId;
	HANDLE					hThread;
};

int  Open(vlc_object_t * p_this)
{
	ENTER_METHOD("Open");
	intf_thread_t *p_intf = (intf_thread_t *)p_this;
	intf_sys_t    *p_sys = (intf_sys_t *)calloc(1, sizeof(intf_sys_t));

	if (!p_sys)
		return VLC_ENOMEM;

	p_sys->hThread = CreateThread(NULL, 0, Run, p_intf, NULL, &p_sys->threadId);

	p_intf->p_sys = p_sys;

	playlist_t *p_playlist = pl_Get(p_intf);
	var_AddCallback( p_playlist, "item-change", ItemChange, p_intf );
	var_AddCallback(p_playlist, "item-current", ItemChange, p_intf);

	return VLC_SUCCESS;
}


void Close(vlc_object_t * p_this)
{
	ENTER_METHOD("Close");
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

	intf_sys_t *p_sys  = p_intf->p_sys;

	PostThreadMessage(p_sys->threadId, WM_QUIT, 0, 0);
	WaitForSingleObject(p_sys->hThread, INFINITE);
	CloseHandle(p_sys->hThread);

	free(p_sys);
}

DWORD __stdcall Run(void *data)
{
	intf_thread_t *p_intf = (intf_thread_t *)data;
	intf_sys_t *p_sys = p_intf->p_sys;
	ENTER_METHOD("Run");
	g_Player.m_p_intf = p_intf;

	RunMiniLyrics(g_hInstance, GetPlayerWindow(), &g_Player);

	MSG			msg;

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (!vlc_object_alive(p_intf))
			break;
	}

	DisconnectMiniLyrics(&g_Player);

	g_Player.m_p_intf = NULL;

	return NULL;
}

/*****************************************************************************
* PlayingChange: Playing status change callback
*****************************************************************************/
static int PlayingChange(vlc_object_t *p_this, const char *psz_var,
						 vlc_value_t oldval, vlc_value_t newval, void *p_data)
{
	ENTER_METHOD("PlayingChange");

	intf_thread_t   *p_intf = (intf_thread_t*) p_data;

	VLC_UNUSED(psz_var);

	if (newval.i_int == INPUT_EVENT_STATE)
	{
		switch(var_GetInteger((input_thread_t*)p_this, "state"))
		{
		case OPENING_S:
			g_Player.NotifyPlayerStatChanged(PS_PLAYING);
			break;
		case PLAYING_S:
			g_Player.NotifyPlayerStatChanged(PS_PLAYING);
			if (!g_bMetaDataRead && newval.i_int == INPUT_EVENT_ITEM_META)
			{
				g_bMetaDataRead = true;
				g_Player.MediaChangedCheck();
			}
			break;
		case PAUSE_S:
			g_Player.NotifyPlayerStatChanged(PS_PAUSED);
			break;
		default:
			g_Player.NotifyPlayerStatChanged(PS_STOPED);
			break;
		}
	}

	return VLC_SUCCESS;
}

/*****************************************************************************
* ItemChange: Playlist item change callback
*****************************************************************************/
static int ItemChange(vlc_object_t *p_this, const char *psz_var,
					  vlc_value_t oldval, vlc_value_t newval, void *param)
{
	ENTER_METHOD("ItemChange");
	intf_thread_t *p_intf = (intf_thread_t *)param;

    input_thread_t *p_input = playlist_CurrentInput( (playlist_t *) p_this );

	if (!p_input)
		return VLC_SUCCESS;

	if (p_input->b_dead || !input_GetItem(p_input)->psz_name)
	{
		/* Not playing anything ... */
		vlc_object_release(p_input);

		g_MLService.OnMediaInfoChanged(L"", NULL, NULL, NULL, 0);
		return VLC_SUCCESS;
	}

	var_AddCallback(p_input, "intf-event", PlayingChange, p_intf);

	g_bMetaDataRead = false;
	/* Playing something ... */
	if (input_item_IsPreparsed(input_GetItem(p_input)))
	{
		g_bMetaDataRead = true;
		g_Player.MediaChangedCheck();
	}

	vlc_object_release(p_input);

	return VLC_SUCCESS;
}

BOOL APIENTRY DllMain(HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					 )
{
	ENTER_METHOD("DllMain");
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		g_hInstance = hModule;

	return TRUE;
}

DWORD CPlayerImp::GetMediaLength()
{
	if (!m_p_intf)
		return 0;

	input_thread_t *p_input;
	playlist_t *p_playlist = pl_Get(m_p_intf);

	p_input = playlist_CurrentInput(p_playlist);

	if (!p_input || p_input->b_dead)
		return 0;

	int64_t		nPos = 0;

	input_Control(p_input, INPUT_GET_LENGTH, &nPos);

	vlc_object_release(p_input);

	return (DWORD)(nPos / 1000);
}
