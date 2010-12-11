
#pragma once

#include <windows.h>
#include <assert.h>
#include <tchar.h>

#include <string>
using namespace std;

#define ASSERT		assert

#include "PluginHelper.h"
#include "MiniLyricsPlugin.h"
#include "IPlayer.h"
#include "TinyBaseWnd.h"
#include "PlayerServiceWndBase.h"

extern HINSTANCE		g_hInstance;
extern class CMLService	g_MLService;
extern IPlayer			*g_pPlayer;
extern CPlayerServiceWndBase *g_pPlayerServiceWnd;

BOOL RunMiniLyrics(HINSTANCE hInst, HWND hWndPlayer, IPlayer *pPlayer, CPlayerServiceWndBase *pPlayerServiceWnd);

void DisconnectMiniLyrics();

