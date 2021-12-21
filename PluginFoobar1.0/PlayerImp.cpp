#include "../MLPluginLib/MLPlugin.h"
#include "PlayerImp.h"

CPlayerImp		g_Player;

DWORD			g_dwThreadIDMain = 0;

inline bool IsInMainThread()
{
	return GetCurrentThreadId() == g_dwThreadIDMain;
}

class initquit_ui : public initquit
{
	virtual void on_init()
	{
		HWND hWndPlayer = FindWindowA("{DA7CD0DE-1602-45e6-89A1-C2CA151E008E}/1", NULL);

		g_dwThreadIDMain = GetCurrentThreadId();

		RunMiniLyrics(core_api::get_my_instance(), hWndPlayer, &g_Player);
	}
	virtual void on_quit()
	{
		g_MLService.OnDisconnected();
	}
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
		wchar_t			wszArtist[MAX_PATH], wszAlbum[MAX_PATH], wszTitle[MAX_PATH], wszUrl[MAX_PATH];

		EmptyStr(wszArtist);
		EmptyStr(wszAlbum);
		EmptyStr(wszTitle);
		EmptyStr(wszUrl);

		Utf8ToUCS2((p_track->get_location()).get_path(), -1, wszUrl, CountOf(wszUrl));

		if (p_track->get_info(outInfo))
		{
			LPCSTR		szString;

			szString = outInfo.meta_get("artist", 0);
			if (szString && !IsEmptyString(szString))
				Utf8ToUCS2(szString, -1, wszArtist, CountOf(wszArtist));

			szString = outInfo.meta_get("title", 0);
			if (szString && !IsEmptyString(szString))
				Utf8ToUCS2(szString, -1, wszTitle, CountOf(wszTitle));

			szString = outInfo.meta_get("album", 0);
			if (szString && !IsEmptyString(szString))
				Utf8ToUCS2(szString, -1, wszAlbum, CountOf(wszAlbum));
		}

		double length = 0;
		static_api_ptr_t<playback_control>	api_playback;
		length = api_playback->playback_get_length();

		g_MLService.OnMediaInfoChanged(wszUrl, wszArtist, wszAlbum, wszTitle, (unsigned long)(length * 1000));
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


void CPlayerImp::OnGetInfo(PlayerInfo &playerInfo)
{
	//
	// Initialize player information for MiniLyrics
	//
	playerInfo.hWndPlayer = FindWindowA("{DA7CD0DE-1602-45e6-89A1-C2CA151E008E}/1", NULL);

	wcscpy_safe(playerInfo.playerName, CountOf(playerInfo.playerName), L"Foobar2000");
}

void CPlayerImp::OnConnected()
{
	assert(IsInMainThread());
	static_api_ptr_t<play_callback_manager>	pcmgr;
	pcmgr->register_callback(&ml_play_callback, play_callback::flag_on_playback_all, true);

	g_MLService.AddWndCloseto(NULL, L"{97E27FAA-C0B3-4b8e-A693-ED7881E99FC1}", NULL);

	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);

	// Update Media info
	// OnMediaChanged();

	// Update player state
	g_MLService.OnPlayStateChanged(m_stat);
}

void CPlayerImp::OnDisconnect()
{
	KillTimer(TIMER_GETPOS);

	assert(IsInMainThread());
	static_api_ptr_t<play_callback_manager>	pcmgr;
	pcmgr->unregister_callback(&ml_play_callback);
}

void CPlayerImp::Play()
{
	assert(IsInMainThread());
	if (IsInMainThread())
	{
		static_api_ptr_t<playback_control>	api_playback;
		api_playback->play_start();
	}
}

void CPlayerImp::Pause()
{
	assert(IsInMainThread());
	if (IsInMainThread())
	{
		static_api_ptr_t<playback_control>	api_playback;
		if (api_playback->is_playing())
			api_playback->pause(true);
	}
}

void CPlayerImp::PlayPause()
{
	assert(IsInMainThread());
	if (IsInMainThread())
	{
		static_api_ptr_t<playback_control>	api_playback;
		api_playback->play_or_pause();
	}
}

void CPlayerImp::Stop()
{
	assert(IsInMainThread());
	if (IsInMainThread())
	{
		static_api_ptr_t<playback_control>	api_playback;
		api_playback->stop();
	}
}

void CPlayerImp::Prev()
{
	assert(IsInMainThread());
	if (IsInMainThread())
	{
		static_api_ptr_t<playback_control>	api_playback;
		api_playback->play_start(playback_control::track_command_prev);
	}
}

void CPlayerImp::Next()
{
	assert(IsInMainThread());
	if (IsInMainThread())
	{
		static_api_ptr_t<playback_control>	api_playback;
		api_playback->play_start(playback_control::track_command_next);
	}
}

void CPlayerImp::SeekTo(int nMsPos)
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
		}
	}
}

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
		return 0;
}

void CPlayerImp::OnTimer(int nTimerID)
{
	if (nTimerID == TIMER_GETPOS)
	{
		// update current play time.
		g_MLService.OnPlayPosChanged(g_Player.GetPlayPos());
	}
}
