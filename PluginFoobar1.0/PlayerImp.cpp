#include "../MLPluginLib/MLPlugin.h"
#include "PlayerImp.h"

CPlayerImp		g_Player;

DWORD			g_dwThreadIDMain = 0;

inline bool IsInMainThread()
{
	return GetCurrentThreadId() == g_dwThreadIDMain;
}

#define	MSG_PLAYER_API		(WM_USER + 1)

enum PLAYER_API
{
//	PA_GET_STATE,
	PA_IS_PLAYING,
	PA_IS_PAUSED,
	PA_GET_MEDIA_LENGTH,
	PA_GET_POS,
};

///////////////////////////////////////////////////////////////////////////////


class CPlayerInfoQueryWnd : public CPlayerInfoQueryBaseWnd
{
public:
	DWORD SendPlayerAPIMsg(PLAYER_API playerApi)
	{
		DWORD		dwResult;

		if (SendMessageTimeout(m_hWnd, MSG_PLAYER_API, playerApi, 0, SMTO_BLOCK, 2000, &dwResult))
			return dwResult;
		else
			return 0;
	}

	LRESULT OnMsgPlayerApi(PLAYER_API playerApi, LPARAM lParam)
	{
		assert(IsInMainThread());

		switch (playerApi)
		{
			/*	case PA_GET_STATE:
			{
			static_api_ptr_t<playback_control>	api_playback;
			if (api_playback->is_playing())
			return PS_PLAYING;
			else if (api_playback->is_paused())
			return PS_PAUSED;
			else
			return PS_STOPED;
			}
			break;*/
		case PA_IS_PLAYING:
			{
				static_api_ptr_t<playback_control>	api_playback;
				return api_playback->is_playing();
			}
			break;
		case PA_IS_PAUSED:
			{
				static_api_ptr_t<playback_control>	api_playback;
				return api_playback->is_paused();
			}
			break;
		case PA_GET_MEDIA_LENGTH:
			{
				double length = 0;
				static_api_ptr_t<playback_control>	api_playback;
				length = api_playback->playback_get_length();
				return (DWORD)(length * 1000);
			}
			break;
		case PA_GET_POS:
			{
				double time = 0;
				static_api_ptr_t<playback_control>	api_playback;
				time = api_playback->playback_get_position();
				return (DWORD)(time * 1000);
			}
			break;
		}

		return 0;
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
		if (uMsg == MSG_PLAYER_API)
		{
			OnMsgPlayerApi(PLAYER_API(wParam), lParam);
			return 1;
		}
		else
			return CPlayerInfoQueryBaseWnd::WndProc(uMsg, wParam, lParam);
	}

};

CPlayerInfoQueryWnd	g_wndInfoQuery;

void GetMediaTitle(LPSTR szBuff, int nBuffLen, LPCSTR szMediaSrc)
{
	assert(szMediaSrc);

	LPCSTR		szBeg1, szBeg2;

	szBeg1 = strrchr(szMediaSrc, '\\');
	if (szBeg1 == NULL)
		szBeg1 = szMediaSrc;
	else
		szBeg1++;

	szBeg2 = strrchr(szMediaSrc, '/');
	if (szBeg2 == NULL)
		szBeg2 = szMediaSrc;
	else
		szBeg2++;

	if (szBeg2 > szBeg1)
		szBeg1 = szBeg2;

	strcpy_safe(szBuff, nBuffLen, szBeg1);
}

class initquit_ui : public initquit
{
	virtual void on_init()
	{
		g_hInstance = core_api::get_my_instance();

		// g_hwndPlayer = FindWindow("{DA7CD0DE-1602-45e6-89A1-C2CA151E008E}", NULL);
		g_hwndPlayer = FindWindowA("{DA7CD0DE-1602-45e6-89A1-C2CA151E008E}/1", NULL);

		g_dwThreadIDMain = GetCurrentThreadId();

		if (!InitMiniLyricsPlugin(g_hInstance, &g_Player))
			return ;

		// now we can use other g_MLService functions.
		char	szMsg[256];

		wsprintfA(szMsg, "Start Foobar2000 plugin for minilyrics. HINSTANCE: %#X", g_hInstance);
		g_MLService.Log(szMsg);
	}
	virtual void on_quit()
	{
		g_MLService.Quit();
	}
/*	virtual int service_release()
	{
	}*/
};

class play_callback_ui : public play_callback
{
	virtual void on_playback_starting(play_control::t_track_command p_command,bool p_paused)
	{
		g_Player.m_stat = PS_PLAYING;
		g_MLService.OnPlayStateChanged(PS_PLAYING);
	}
	virtual void on_playback_new_track(metadb_handle_ptr p_track)
	{
		file_info_impl	outInfo;
		MLMediaInfo		mediaInfo;
		wchar_t			wszArtist[MAX_PATH], wszAlbum[MAX_PATH], wszTitle[MAX_PATH], wszUrl[MAX_PATH];

		EmptyStr(wszArtist);
		EmptyStr(wszAlbum);
		EmptyStr(wszTitle);
		EmptyStr(wszUrl);
		mediaInfo.szArtist = wszArtist;
		mediaInfo.szAlbum = wszAlbum;
		mediaInfo.szTitle = wszTitle;
		mediaInfo.szMediaUrl = wszUrl;

		UTF8ToUCS2((p_track->get_location()).get_path(), -1, wszUrl, CountOf(wszUrl));

		if (p_track->get_info(outInfo))
		{
			LPCSTR		szString;

			szString = outInfo.meta_get("artist", 0);
			if (szString && !IsEmptyString(szString))
				UTF8ToUCS2(szString, -1, wszArtist, CountOf(wszArtist));

			szString = outInfo.meta_get("title", 0);
			if (szString && !IsEmptyString(szString))
				UTF8ToUCS2(szString, -1, wszTitle, CountOf(wszTitle));

			szString = outInfo.meta_get("album", 0);
			if (szString && !IsEmptyString(szString))
				UTF8ToUCS2(szString, -1, wszAlbum, CountOf(wszAlbum));
		}

		double length = 0;
		static_api_ptr_t<playback_control>	api_playback;
		length = api_playback->playback_get_length();
		mediaInfo.nMediaLength = (unsigned long)(length * 1000);

		g_MLService.OnMediaInfoChanged(&mediaInfo);
	}
	virtual void on_playback_stop(play_control::t_stop_reason p_reason)
	{
		g_Player.m_stat = PS_STOPED;
		g_MLService.OnPlayStateChanged(PS_STOPED);
	}
	virtual void on_playback_seek(double time)
	{
	}
	virtual void on_playback_pause(bool p_state)
	{
		g_Player.m_stat = p_state ? PS_PAUSED : PS_PLAYING;
		g_MLService.OnPlayStateChanged(g_Player.m_stat);
	}
	virtual void on_playback_edited(metadb_handle_ptr p_track) //currently played file got edited
	{
	}
	virtual void on_playback_dynamic_info(const file_info & p_info)
	{
	}
	virtual void on_playback_dynamic_info_track(const file_info & p_info)
	{
	}
	virtual void on_playback_time(double p_time)
	{
	}
	virtual void on_volume_change(float p_new_val)
	{
	}

	// virtual void on_volume_change(int new_val) {};//may also happen when not playing
};

static service_factory_single_t<initquit_ui> foo;
//
// service.h(239) : error C2259: 'play_callback_ui' : cannot instantiate abstract class
// 遇到此错误，检查play_callback_ui类里面是否有纯虚函数继承下来了……
// static service_factory_single_t<play_callback_ui> foo2;
play_callback_ui		ml_play_callback;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
	m_stat = PS_STOPED;
}

CPlayerImp::~CPlayerImp()
{
}

// Player Info
int CPlayerImp::GetPlayerName(char * lpszBuffer, int nBufSize)
{
	strcpy_safe(lpszBuffer, nBufSize, "Foobar2000");

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
	assert(IsInMainThread());
	if (IsInMainThread())
	{
		double time = 0;
		static_api_ptr_t<playback_control>	api_playback;
		time = api_playback->playback_get_position();
		return (DWORD)(time * 1000);
	}
	else
	{
		return g_wndInfoQuery.SendPlayerAPIMsg(PA_GET_POS);
	}
}

// control player
BOOL CPlayerImp::Play()
{
	assert(IsInMainThread());
	if (IsInMainThread())
	{
		static_api_ptr_t<playback_control>	api_playback;
		api_playback->play_start();//command(play_control::PLAYBACK_PLAY,0);
	}
	else
	{
	}

	return TRUE;
}

BOOL CPlayerImp::Pause()
{
	assert(IsInMainThread());
	if (IsInMainThread())
	{
		static_api_ptr_t<playback_control>	api_playback;
		if (api_playback->is_playing())
			api_playback->pause(true);
	}
	else
	{
	}

	return TRUE;
}

BOOL CPlayerImp::PlayPause()
{
	assert(IsInMainThread());
	if (IsInMainThread())
	{
		static_api_ptr_t<playback_control>	api_playback;
		api_playback->play_or_pause();
	}
	else
	{
	}

	return TRUE;
}

BOOL CPlayerImp::Stop()
{
	assert(IsInMainThread());
	if (IsInMainThread())
	{
		static_api_ptr_t<playback_control>	api_playback;
		api_playback->stop();
	}
	else
	{
	}

	return TRUE;
}

BOOL CPlayerImp::Prev()
{
	assert(IsInMainThread());
	if (IsInMainThread())
	{
		static_api_ptr_t<playback_control>	api_playback;
		api_playback->play_start(playback_control::track_command_prev);
	}
	else
	{
	}

	return TRUE;
}

BOOL CPlayerImp::Next()
{
	assert(IsInMainThread());
	if (IsInMainThread())
	{
		static_api_ptr_t<playback_control>	api_playback;
		api_playback->play_start(playback_control::track_command_next);
	}
	else
	{
	}

	return TRUE;
}

BOOL CPlayerImp::SeekTo(int nMsPos)
{
	assert(IsInMainThread());
	if (IsInMainThread())
	{
		static_api_ptr_t<playback_control>	api_playback;
		if (api_playback->is_playing())
		{
			double newpos;
			double length = 0;
			length = api_playback->playback_get_length();
			if (length > 0)
			{
				newpos = (double)nMsPos / (double)1000;
				api_playback->playback_seek(newpos);
			}
			return TRUE;
		}
	}
	else
	{
	}
	return FALSE;
}

BOOL CPlayerImp::Init()
{
	assert(IsInMainThread());
	static_api_ptr_t<play_callback_manager>	pcmgr;
	pcmgr->register_callback(&ml_play_callback, play_callback::flag_on_playback_all, true);

	g_wndInfoQuery.Create(g_hInstance);

	g_MLService.AddWndCloseto(NULL, "{97E27FAA-C0B3-4b8e-A693-ED7881E99FC1}", NULL);

	return TRUE;
}

void CPlayerImp::Quit()
{
	assert(IsInMainThread());
	static_api_ptr_t<play_callback_manager>	pcmgr;
	pcmgr->unregister_callback(&ml_play_callback);

	g_wndInfoQuery.Destroy();
}

void CPlayerImp::OnPlayPosUpdateSpanChanged(int nPlayPosUpdateSpan)
{
	g_wndInfoQuery.SetUpdatePosTimer(nPlayPosUpdateSpan);
}
