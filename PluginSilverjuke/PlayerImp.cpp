#include "../MLPluginLib/MLPlugin.h"
#include <tchar.h>
#include "PlayerImp.h"

#include "sj_api.h"


CPlayerImp		g_Player;


// some globals
SjInterface interf;

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

			//
			// Initial load MiniLyrics
			//

			g_hwndPlayer = FindWindow(_T("wxWindowClass"), NULL);

			if (!InitMiniLyricsPlugin(g_hInstance, &g_Player))
				return 1;

			// now we can use other g_MLService functions.
			char	szMsg[256];

			wsprintf(szMsg, "Start Silverjuke plugin for MiniLyrics. HINSTANCE: %#X", g_hInstance);
			g_MLService.Log(szMsg);
		}
		return 1; // success

	case SJ_PLUGIN_CALL:
		// this is the "script to C" bridge
		if( strcmp((char*)param1, "ID_ML_onAbout") == 0 && param2)
		{
			g_MLService.About(g_hwndPlayer);
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

			UTF8ToUCS2((char*)param2, -1, szUCS2, CountOf(szUCS2));

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

			MLMediaInfo		mediaInfo;
			mediaInfo.szMediaUrl = szData[I_URL];
			mediaInfo.szArtist = szData[I_ARTIST];
			mediaInfo.szAlbum = szData[I_ALBUM];
			mediaInfo.szTitle = szData[I_TITLE];
			mediaInfo.nMediaLength = interf->CallMaster(interf, SJ_EXECUTE, (LPARAM)"player.duration", 0, 0);

			g_MLService.OnMediaInfoChanged(&mediaInfo);
		}
		return 0;
	case SJ_PLUGIN_EXIT:
		g_MLService.Quit();
		return 1;
	}
	return 0;
}


// The Silverjuke API entry point - this symbol needs to be exported
SjInterface* SjGetInterface(void)
{
	interf.CallPlugin = CallPlugin;
	return &interf;
}


//////////////////////////////////////////////////////////////////////////

class CPlayerInfoQueryWnd : public CPlayerInfoQueryBaseWnd
{
public:
	enum { TIMER_QUERY_STAT = TIMER_NEXT + 1 };

	void OnCreate()
	{
		SetTimer(m_hWnd, TIMER_GETPOS, TIME_OUT_UPDATE_POS, NULL);
		SetTimer(m_hWnd, TIMER_QUERY_STAT, TIME_OUT_UPDATE_MEDIA_INFO, NULL);
	}

	void OnDestroy()
	{
		KillTimer(m_hWnd, TIMER_GETPOS);
		KillTimer(m_hWnd, TIMER_QUERY_STAT);
	}

	void OnTimer(int nTimerID)
	{
		if (nTimerID == TIMER_GETPOS)
			g_MLService.OnPlayTimeChanged(g_Player.GetPlayPos());
		else if (nTimerID == TIMER_QUERY_STAT)
		{
			PLAYER_STATE	newState = (PLAYER_STATE)g_Player.GetPlayerState();
			if (newState != m_state)
			{
				m_state = newState;
				g_MLService.OnPlayStateChanged(newState);
			}
		}
	}

	PLAYER_STATE		m_state;

};

CPlayerInfoQueryWnd	g_wndInfoQuery;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPlayerImp::CPlayerImp()
{
}

CPlayerImp::~CPlayerImp()
{
}

// Player Info
int CPlayerImp::GetPlayerName(char * lpszBuffer, int nBufSize)
{
	strcpy_safe(lpszBuffer, nBufSize, "Silverjuke");

	return PR_OK;
}

DWORD CPlayerImp::GetPlayerState()
{
	if (interf.CallMaster(&interf, SJ_EXECUTE, (LPARAM)"player.isPlaying()", 0, 0))
		return PS_PLAYING;
	else if (interf.CallMaster(&interf, SJ_EXECUTE, (LPARAM)"player.isPaused()", 0, 0))
		return PS_PAUSED;
	return PS_STOPED;
}

HWND CPlayerImp::GetMainWnd()
{
	return g_hwndPlayer;
}

DWORD CPlayerImp::GetPlayPos()
{
	return interf.CallMaster(&interf, SJ_EXECUTE, (LPARAM)"player.time", 0, 0);
}

BOOL CPlayerImp::Play()
{
	interf.CallMaster(&interf, SJ_EXECUTE, (LPARAM)"player.play()", 0, 0);
	return TRUE;
}

BOOL CPlayerImp::Pause()
{
	interf.CallMaster(&interf, SJ_EXECUTE, (LPARAM)"player.pause()", 0, 0);
	return TRUE;
}

BOOL CPlayerImp::PlayPause()
{
	interf.CallMaster(&interf, SJ_EXECUTE, (LPARAM)"if (player.isPlaying()) player.pause(); else player.play();", 0, 0);
	return TRUE;
}

BOOL CPlayerImp::Stop()
{
	interf.CallMaster(&interf, SJ_EXECUTE, (LPARAM)"player.stop()", 0, 0);
	return TRUE;
}

BOOL CPlayerImp::Prev()
{
	interf.CallMaster(&interf, SJ_EXECUTE, (LPARAM)"player.prev()", 0, 0);
	return TRUE;
}

BOOL CPlayerImp::Next()
{
	interf.CallMaster(&interf, SJ_EXECUTE, (LPARAM)"player.next()", 0, 0);
	return TRUE;
}

BOOL CPlayerImp::SeekTo(int nMsPos)
{
	char	szScript[256];

	sprintf(szScript, "player.time=%d", nMsPos);
	interf.CallMaster(&interf, SJ_EXECUTE, (LPARAM)szScript, 0, 0);
	return TRUE;
}

BOOL CPlayerImp::Init()
{
	g_MLService.AddWndCloseto(NULL, "WMPlayerApp", NULL);

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
