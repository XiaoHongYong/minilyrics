#include "../MLPluginLib/MLPlugin.h"
#include <tchar.h>
#include "PlayerImp.h"

#include "sj_api.h"

CPlayerImp		g_Player;
SjInterface		interf;

HWND GetPlayerWindow()
{
	return FindWindow(_T("wxWindowClass"), NULL);
}

// The main DLL entry point, may be needed for special purposes
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance = (HMODULE)hModule;
	return TRUE;
}

// Receiving messages from Silverjuke
LPARAM CALLBACK CallPlugin(SjInterface* interf, UINT msg, LPARAM param1, LPARAM param2, LPARAM param3)
{
	switch( msg )
	{
	case SJ_PLUGIN_INIT:
		{
			// embed the plugin for demonstration purposes to the main menu.
			interf->CallMaster(interf, SJ_EXECUTE, (LPARAM)
				"program.addMenuEntry('About MiniLyrics...', function() { program.callPlugin('ID_ML_onAbout'); })"
				, 0, 0);

			interf->CallMaster(interf, SJ_EXECUTE, (LPARAM)
				"function myPlayerNotificationHandler() { program.callExported('ML_onTrackChange'); }"
				"player.onTrackChange = myPlayerNotificationHandler;"
				, 0, 0);

			// add a superglobal function that can be called by other scripts using "program.callExported('foo_exported');"
			interf->CallMaster(interf, SJ_EXECUTE, (LPARAM)
				"program.exportFunction(ML_onTrackChange);"
				"function ML_onTrackChange() {"
					"program.callPlugin('ID_ML_onTrackChange', "
					"player.getUrlAtPos(player.queuePos) + '*****' + "
					"player.getArtistAtPos(player.queuePos) + '*****' + "
					"player.getAlbumAtPos(player.queuePos) + '*****' + "
					"player.getTitleAtPos(player.queuePos)); } "
				, 0, 0);

			RunMiniLyrics(g_hInstance, GetPlayerWindow(), &g_Player);
		}
		return 1; // success

	case SJ_PLUGIN_CALL:
		// this is the "script to C" bridge
		if( strcmp((char*)param1, "ID_ML_onAbout") == 0 && param2)
		{
			g_MLService.About(GetPlayerWindow());
		}
		else if( strcmp((char*)param1, "ID_ML_onTrackChange") == 0 && param2)
		{
			enum 
			{
				I_URL,
				I_ARTIST,
				I_ALBUM,
				I_TITLE,
				I_COUNT
			};
			LPCWSTR szSeperator = L"*****";
			size_t		nLenSep = wcslen(szSeperator);
			wchar_t	szUCS2[1024];
			LPWSTR	szData[I_COUNT], p;

			Utf8ToUCS2((char*)param2, -1, szUCS2, CountOf(szUCS2));

			// Split music tag, by *****
			szData[0] = szUCS2;
			p = szUCS2;
			for (int i = 0; i < CountOf(szData) - 1; ++i)
			{
				while (*p)
				{
					if (*p == '*')
					{
						if (wcsncmp(p, szSeperator, nLenSep) == 0)
						{
							*p = '\0';
							p += nLenSep;
							break;
						}
					}

					p++;
				}
				szData[i + 1] = p;
			}

			g_MLService.OnMediaInfoChanged(szData[I_URL], szData[I_ARTIST], szData[I_ALBUM], szData[I_TITLE],
				interf->CallMaster(interf, SJ_EXECUTE, (LPARAM)"player.duration", 0, 0));
		}
		return 0;
	case SJ_PLUGIN_EXIT:
		g_MLService.OnDisconnected();
		return 1;
	}
	return 0;
}

DWORD GetPlayerState()
{
	if (interf.CallMaster(&interf, SJ_EXECUTE, (LPARAM)"player.isPlaying()", 0, 0))
		return PS_PLAYING;
	else if (interf.CallMaster(&interf, SJ_EXECUTE, (LPARAM)"player.isPaused()", 0, 0))
		return PS_PAUSED;
	return PS_STOPED;
}

DWORD GetPlayPos()
{
	return interf.CallMaster(&interf, SJ_EXECUTE, (LPARAM)"player.time", 0, 0);
}

// The Silverjuke API entry point - this symbol needs to be exported
SjInterface* SjGetInterface(void)
{
	interf.CallPlugin = CallPlugin;
	return &interf;
}


//////////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
	m_state = PS_STOPED;
}

CPlayerImp::~CPlayerImp()
{
}

void CPlayerImp::OnGetInfo(PlayerInfo &playerInfo)
{
	//
	// Initialize player information for MiniLyrics
	//
	playerInfo.hWndPlayer = GetPlayerWindow();
	wcscpy_safe(playerInfo.playerName, CountOf(playerInfo.playerName), L"Silverjuke");
}

void CPlayerImp::OnConnected()
{
	g_MLService.AddWndCloseto(GetPlayerWindow(), NULL, NULL);

	SetTimer(TIMER_GETPOS, TIME_OUT_UPDATE_POS);
	SetTimer(TIMER_QUERY_STAT, TIME_OUT_UPDATE_MEDIA_INFO);

	// Update player state
	g_MLService.OnPlayStateChanged(GetPlayerState());
}

void CPlayerImp::OnDisconnect()
{
	KillTimer(TIMER_GETPOS);
	KillTimer(TIMER_QUERY_STAT);
}

void CPlayerImp::Play()
{
	interf.CallMaster(&interf, SJ_EXECUTE, (LPARAM)"player.play()", 0, 0);
}

void CPlayerImp::Pause()
{
	interf.CallMaster(&interf, SJ_EXECUTE, (LPARAM)"player.pause()", 0, 0);
}

void CPlayerImp::PlayPause()
{
	interf.CallMaster(&interf, SJ_EXECUTE, (LPARAM)"if (player.isPlaying()) player.pause(); else player.play();", 0, 0);
}

void CPlayerImp::Stop()
{
	interf.CallMaster(&interf, SJ_EXECUTE, (LPARAM)"player.stop()", 0, 0);
}

void CPlayerImp::Prev()
{
	interf.CallMaster(&interf, SJ_EXECUTE, (LPARAM)"player.prev()", 0, 0);
}

void CPlayerImp::Next()
{
	interf.CallMaster(&interf, SJ_EXECUTE, (LPARAM)"player.next()", 0, 0);
}

void CPlayerImp::SeekTo(int nMsPos)
{
	char	szScript[256];

	sprintf(szScript, "player.time=%d", nMsPos);
	interf.CallMaster(&interf, SJ_EXECUTE, (LPARAM)szScript, 0, 0);
}

void CPlayerImp::OnTimer(int nTimerID)
{
	if (nTimerID == TIMER_GETPOS)
		g_MLService.OnPlayPosChanged(GetPlayPos());
	else if (nTimerID == TIMER_QUERY_STAT)
	{
		PLAYER_STATE	newState = (PLAYER_STATE)GetPlayerState();
		if (newState != m_state)
		{
			m_state = newState;
			g_MLService.OnPlayStateChanged(newState);
		}
	}
}
