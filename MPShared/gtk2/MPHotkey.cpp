// MPHotkey.cpp: implementation of the CMPHotkey class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "../MPlayerApp.h"
#include "../MLCmd.h"
#include "MPHotkey.h"


#ifndef MOD_ALT
#define MOD_ALT         0x0001
#define MOD_CONTROL     0x0002
#define MOD_SHIFT       0x0004
#define MOD_WIN         0x0008
#endif

int				g_hcPlayback[] = { CMD_PLAY, CMD_PLAYPAUSE, CMD_STOP, CMD_PREVIOUS, CMD_NEXT, CMD_FORWARD, CMD_BACKWARD, 0 };
int				g_hcGeneral[] = { CMD_QUIT, 0 };
int				g_hcUI[] = { CMD_TOGGLE_MP, CMD_PREFERENCES, CMD_TOPMOST, 0 };
int				g_hcLyrics[] = { CMD_SEARCH_LYRICS, CMD_BACKWARD_LYRICS, CMD_FORWARD_LYRICS, 0 };

MPHotKeySection		g_vHotkeySections[] = 
{
	{ _T("Playback:"), g_hcPlayback },
	{ _T("General:"), g_hcGeneral },
	{ _T("UI:"), g_hcUI },
	{ _T("Lyrics:"), g_hcLyrics },
	{ NULL, NULL }
};

CMPHotkey::CmdAccKey			g_vDefAccKey[] = 
{
	{ CMD_BACKWARD_LYRICS, VK_UP, MOD_CONTROL | MOD_SHIFT, 0 },
	{ CMD_FORWARD_LYRICS, VK_DOWN, MOD_CONTROL | MOD_SHIFT, 0 },
	{ CMD_TOPMOST, 'T', MOD_CONTROL | MOD_SHIFT, 0 },
	{ CMD_TOGGLE_MP, 'M', MOD_CONTROL | MOD_ALT, 0 },
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPHotkey::CMPHotkey()
{
}

CMPHotkey::~CMPHotkey()
{

}

void CMPHotkey::Init(CBaseWnd *pWnd)
{
}

void CMPHotkey::Quit()
{
}

void CMPHotkey::Enable(bool bEnable)
{
}

void CMPHotkey::RestoreDefaults()
{
}

void CMPHotkey::Add(int cmd, int nKey, int fuModifiers)
{
}

bool CMPHotkey::Set(int nIndex, int cmd, int nKey, int fuModifiers)
{
	return true;
}

bool CMPHotkey::Remove(int nIndex)
{
}

CMPHotkey::CmdAccKey *CMPHotkey::Get(int nIndex)
{
	return NULL;
}

int CMPHotkey::LoadSettings()
{
	return ERR_OK;
}

int CMPHotkey::SaveSettings()
{
	return ERR_OK;
}

void CMPHotkey::RegisterAllHotKeys()
{
}

void CMPHotkey::UnregisterAllHotKeys()
{
}
