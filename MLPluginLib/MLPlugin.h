
#pragma once

#include <windows.h>
#include <assert.h>
#include <tchar.h>

#include <string>
using namespace std;

#define ASSERT		assert

#include "PluginHelper.h"
#include "MiniLyricsPlugin.h"
#include "MLService.h"
#include "TinyBaseWnd.h"
#include "PlayerServiceBase.h"

extern HINSTANCE		g_hInstance;
extern class CMLService	g_MLService;

BOOL RunMiniLyrics(HINSTANCE hInst, HWND hWndPlayer, CPlayerServiceBase *pPlayerService, bool bManual = false);

void DisconnectMiniLyrics(CPlayerServiceBase *pPlayerService);

