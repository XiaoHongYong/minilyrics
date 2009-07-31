// MPHotkey.cpp: implementation of the CMPHotkey class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "MLCmd.h"
#include "MPHotkey.h"
#include "HotkeyCtrlEx.h"

/*
// for pocket pc

MLCmdAccKey		g_mlCmdAccKey[] = 
{
	{ CMD_PLAYPAUSE, VK_RETURN, 0, VK_RETURN, 0 },
	{ CMD_PLAYPAUSE, 0, 0, 0, 0 },
	{ CMD_STOP, 0, 0, 0 },
	{ CMD_PLAY, 0, 0, 0 },
	{ CMD_PAUSE, 0, 0, 0 },
	{ CMD_PREVIOUS, VK_LEFT, 0, VK_LEFT, 0 },
	{ CMD_NEXT, VK_RIGHT, 0, VK_RIGHT, 0 },
	{ CMD_VOL_INC, VK_UP, 0, VK_UP, 0 },
	{ CMD_VOL_DEC, VK_DOWN, 0, VK_DOWN, 0 },
	{ CMD_BACKWARD, 0, 0, 0, 0 },
	{ CMD_FORWARD, 0, 0, 0, 0 },
	{ CMD_MUTE, '#', 0, '#', 0 },
	{ CMD_BACKWARD_LYRICS, 0, 0 , 0, 0 },
	{ CMD_FORWARD_LYRICS, 0, 0, 0, 0 },
	{ CMD_TOGGLE_SCREEEN, 0, 0, 0, 0 },
	{ CMD_SHUFFLE, 0, 0, 0, 0 },
	{ CMD_LOOP, 0, 0, 0, 0 },
};

int				g_mlCmdAccKeyCount = CountOf(g_mlCmdAccKey) - 1;*/


// CMD_OPEN_MEDIA
int				g_hcPlayback[] = { CMD_PLAY, CMD_PLAYPAUSE, CMD_STOP, CMD_PREVIOUS, CMD_NEXT, CMD_FORWARD, CMD_BACKWARD, CMD_J_PREV_LINE, CMD_J_NEXT_LINE, 0 };
int				g_hcGeneral[] = { CMD_HELP, CMD_QUIT, CMD_CLOSE, CMD_MENU, CMD_PREFERENCES, CMD_DISPLAY_OPT, CMD_COPY_SONG_FILE_LYR, 0 };
int				g_hcUI[] = { CMD_FONT_SIZE_INC, CMD_FONT_SIZE_DEC, CMD_CLR_PREV_HUE, CMD_CLR_NEXT_HUE, CMD_TOGGLE_MP, CMD_MINIMIZE, CMD_TOPMOST, CMD_CLICK_THROUGH, 0 };
int				g_hcLyrics[] = { CMD_SEARCH_LYRICS, CMD_BACKWARD_LYRICS, CMD_FORWARD_LYRICS, CMD_RELOAD_LYR, 0 };
int				g_hcLyrEditor[] = { CMD_LYR_EDITOR, CMD_NEW_LRC, CMD_OPEN_LRC, CMD_SAVE_LRC, CMD_SAVE_LRC_AS, CMD_SAVE_LYR_IN_SONG_FILE, CMD_INSERTTAG_DOWN, CMD_INSERTTAG, CMD_DEL_TAG, CMD_AUTO_FILL_LYR_INFO, CMD_JUMP, CMD_FORWARD_CUR_LINE, CMD_BACKWARD_CUR_LINE, CMD_FORWARD_REMAIN_LINES, CMD_BACKWARD_REMAIN_LINES, CMD_EDIT_TAG, CMD_REMOVE_ALL_TAG, CMD_REMOVE_BLANK_LINE, CMD_TRIM_WHITESPACE, CMD_REMOVE_UNSYNC_LINES, CMD_CAPITALIZE_LEADING_LETTER, CMD_LYRICS_TO_LOWERCASE, 0 };

MPHotKeySection		g_vHotkeySections[] = 
{
	{ _T("Playback:"), g_hcPlayback },
	{ _T("General:"), g_hcGeneral },
	{ _T("UI:"), g_hcUI },
	{ _T("Lyrics:"), g_hcLyrics },
	{ _T("Lyrics Editor:"), g_hcLyrEditor },
	{ NULL, NULL }
};

CMPHotkey::CmdAccKey			g_vDefAccKey[] = 
{
	{ CMD_BACKWARD_LYRICS, true, VK_UP, MOD_CONTROL | MOD_SHIFT, 0 },
	{ CMD_FORWARD_LYRICS, true, VK_DOWN, MOD_CONTROL | MOD_SHIFT, 0 },
	{ CMD_TOPMOST, true, 'T', MOD_CONTROL | MOD_SHIFT, 0 },
	{ CMD_TOGGLE_MP, true, 'M', MOD_CONTROL | MOD_ALT, 0 },

	{ CMD_HELP, false, VK_F1, 0, 0 },

	{ CMD_SEARCH_LYRICS, false, 'D', MOD_CONTROL, 0 },
	{ CMD_RELOAD_LYR, false, VK_F5, 0, 0 },

	{ CMD_LYR_EDITOR, false, 'E', MOD_CONTROL, 0 },
	{ CMD_NEW_LRC, false, 'N', MOD_CONTROL, 0 },
	{ CMD_OPEN_LRC, false, 'O', MOD_CONTROL, 0 },
	{ CMD_SAVE_LRC, false, 'S', MOD_CONTROL, 0 },
	{ CMD_JUMP, false, 'J', MOD_CONTROL, 0 },
	{ CMD_FORWARD_REMAIN_LINES, false, VK_F11, 0, 0 },
	{ CMD_BACKWARD_REMAIN_LINES, false, VK_F12, 0, 0 },
	{ CMD_INSERTTAG_DOWN, false, VK_F7, 0, 0 },
	{ CMD_INSERTTAG, false, VK_F7, MOD_CONTROL, 0 },
	{ CMD_EDIT_TAG, false, VK_F8, 0, 0 },
	{ CMD_FORWARD_CUR_LINE, false, VK_ADD, MOD_CONTROL, 0 },
	{ CMD_FORWARD_CUR_LINE, false, 187, MOD_CONTROL, 0 },
	{ CMD_BACKWARD_CUR_LINE, false, VK_SUBTRACT, MOD_CONTROL, 0 },
	{ CMD_BACKWARD_CUR_LINE, false, 189, MOD_CONTROL, 0 },

	{ CMD_FORWARD, false, VK_NEXT, MOD_CONTROL, 0 },
	{ CMD_BACKWARD, false, VK_PRIOR, MOD_CONTROL, 0 },
	{ CMD_PLAYPAUSE, false, 'P', MOD_CONTROL, 0 },
	{ CMD_PLAYPAUSE, false, VK_F3, 0, 0 },
	{ CMD_STOP, false, VK_F3, MOD_CONTROL, 0 },

	{ CMD_MINIMIZE, false, 'M', MOD_CONTROL, 0 },
	{ CMD_TOPMOST, false, 'T', MOD_CONTROL, 0 },

	{ CMD_PREFERENCES, false, 'P', MOD_CONTROL, 0 },
	{ CMD_COPY_SONG_FILE_LYR, false, VK_F12, MOD_CONTROL, 0 },

};

#define SZ_HOTKEY_FILE_HEADER_V1	_T("HotkeysV1.0")
#define SZ_HOTKEY_FILE_HEADER		_T("HotkeysV1.1")
#define SZ_HOTKEY_FILENAME			_T("hotkeys.cfg")

inline int MakeHotkeyID(int nModifier, int button)
{
	return 0xC000 | (nModifier << 8) | button;
}

int		g_vMenuIDToCmdID[] = 
{
	IDC_DOWNLOAD_CURRENT_LYRICS,	CMD_SEARCH_LYRICS,

	IDC_RATE_LYRICS,	CMD_RATE_LYR,
	IDC_OPEN_LYRICS,	CMD_OPEN_LRC,
	IDC_SAVE_LYRICS,	CMD_SAVE_LRC,
	IDC_SAVE_LYRICS_AS,	CMD_SAVE_LRC_AS,
	IDC_EDIT_SAVE,		CMD_SAVE_LRC,
	IDC_ADVANCED_SAVE_LYR, CMD_ADVANCED_SAVE_LYR,
	IDC_UPLOAD_LYRICS,	CMD_UPLOAD_LYR,
	IDC_EXTERNAL_LYR_EDIT, CMD_EXTERNAL_LYR_EDIT,
	IDC_COPYTO,			CMD_COPY_SONG_FILE_LYR,
	IDC_FORWARD_LYRICS, CMD_FORWARD_LYRICS,
	IDC_BACKWARD_LYRICS, CMD_BACKWARD_LYRICS,
	IDC_SAVE_LYRICS_TO_MP3, CMD_SAVE_LYR_IN_SONG_FILE,

	IDC_LYR_DISPLAY_OPT,CMD_DISPLAY_OPT,
	IDC_CLICK_THROUGH,	CMD_CLICK_THROUGH,

	IDC_PREFERENCE,		CMD_PREFERENCES,
	IDC_FLOATING_LYRICS,CMD_FLOATING_LYRICS,
	IDC_MINIMIZE,		CMD_MINIMIZE,
	IDC_SETTOPMOST,		CMD_TOPMOST,
	IDC_CLOSE,			CMD_CLOSE,

	IDC_FORWARD_REMAIN_LINES, CMD_FORWARD_REMAIN_LINES,
	IDC_BACKWARD_REMAIN_LINES, CMD_BACKWARD_REMAIN_LINES,
	IDC_FORWARD_CUR_LINE, CMD_FORWARD_CUR_LINE,
	IDC_BACKWARD_CUR_LINE, CMD_BACKWARD_CUR_LINE,
	IDC_TO_LOWER_CASE, CMD_LYRICS_TO_LOWERCASE,
	IDC_CAPITALIZE_LEADING_LETTER, CMD_CAPITALIZE_LEADING_LETTER,
	IDC_REMOVE_UNSYNC_LINE, CMD_REMOVE_UNSYNC_LINES,
	IDC_EDIT_LRC_TAG, CMD_EDIT_TAG,
// 	IDC_EDIT_UNDO, CMD_EDIT_UNDO
// 	IDC_EDIT_REDO, CMD_EDIT_REDO,
// 	IDC_EDIT_CUT, CMD_EDIT_CUT,
// 	IDC_EDIT_COPY, CMD_EDIT_COPY,
// 	IDC_EDIT_PASTE, CMD_EDIT_PASTE,
// 	IDC_EDIT_DELETE, CMD_EDIT_DELETE,
	IDC_AUTO_FILL_LYRICS_INFO, CMD_AUTO_FILL_LYR_INFO,
	IDC_REMOVE_ALL_TAG, CMD_REMOVE_ALL_TAG,
	IDC_REMOVE_BLANK_LINE, CMD_REMOVE_BLANK_LINE,
	IDC_TRIM_SPACE, CMD_TRIM_WHITESPACE,

	IDC_PLAY,			CMD_PLAY,
	IDC_PREV,			CMD_PREVIOUS,
	IDC_PAUSE,			CMD_PAUSE,
	IDC_PLAYPAUSE,		CMD_PLAYPAUSE,
	IDC_STOP,			CMD_STOP,
	IDC_NEXT,			CMD_NEXT,

#ifdef _MPLAYER
	IDC_SHUFFLE,		CMD_SHUFFLE,
//	IDC_OPENFILE,		CMD_OPEN_MEDIA,
	IDC_OPENFILE,		CMD_PL_OPEN_FILE,
	IDC_OPENDIR,		CMD_PL_OPEN_DIR,
	IDC_ADDFILE,		CMD_PL_ADD_FILE,
	IDC_ADDDIR,			CMD_PL_ADD_DIR,
#endif

	IDC_ML_HELP,		CMD_HELP,
	IDC_EXIT,			CMD_QUIT,
};

int MenuIDToCmdID(int nMenuID)
{
	for (int i = 0; i < CountOf(g_vMenuIDToCmdID); i += 2)
	{
		if (g_vMenuIDToCmdID[i] == nMenuID)
			return g_vMenuIDToCmdID[i + 1];
	}

	return CMD_UNDEFINE;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPHotkey::CMPHotkey()
{
	m_bEnabled = false;
	m_hWnd = NULL;
}

CMPHotkey::~CMPHotkey()
{

}

void CMPHotkey::Init()
{
	if (LoadSettings() != ERR_OK)
	{
		// load default
		m_vAccKey.clear();
		for (int i = 0; i < CountOf(g_vDefAccKey); i++)
		{
			m_vAccKey.push_back(g_vDefAccKey[i]);
			m_vAccKey.back().idHotKey = MakeHotkeyID(g_vDefAccKey[i].fsModifiers, g_vDefAccKey[i].button);
		}
	}
}

void CMPHotkey::SetEventWnd(CBaseWnd *pWnd)
{
	m_hWnd = pWnd->GetHandle();

	m_bEnabled = g_Profile.GetProfileBool(_T("EnableGlobalHotkey"), false);
	if (!m_bEnabled)
		return;

	RegisterAllHotKeys();
}

void CMPHotkey::Quit()
{
	// free hotkeys...
	UnregisterAllHotKeys();
	m_vAccKey.clear();
}

void CMPHotkey::EnableGlobalHotkey(bool bEnable)
{
	if (m_bEnabled == bEnable)
		return;

	g_Profile.WriteProfileInt(_T("EnableGlobalHotkey"), bEnable);

	m_bEnabled = bEnable;

	if (m_bEnabled)
		RegisterAllHotKeys();
	else
		UnregisterAllHotKeys();
}

void CMPHotkey::RestoreDefaults()
{
	UnregisterAllHotKeys();
	m_vAccKey.clear();

	for (int i = 0; i < CountOf(g_vDefAccKey); i++)
	{
		m_vAccKey.push_back(g_vDefAccKey[i]);
		m_vAccKey.back().idHotKey = MakeHotkeyID(g_vDefAccKey[i].fsModifiers, g_vDefAccKey[i].button);
	}

	if (m_bEnabled)
		RegisterAllHotKeys();

	SaveSettings();
}

void CMPHotkey::OnHotKey(int nId, UINT fuModifiers, UINT uVirtKey)
{
	for (int i = 0; i < (int)m_vAccKey.size(); i++)
	{
		CmdAccKey		&cmdKey = m_vAccKey[i];

		if (cmdKey.bGlobal && nId == cmdKey.idHotKey)
		{
			// execute cmd
			if (g_mpApp.GetSkinFactory()->GetMainWnd())
				g_mpApp.GetSkinFactory()->GetMainWnd()->OnCustomCommand(cmdKey.cmd);
			return;
		}
	}
}

bool CMPHotkey::OnKeyDown(CMPSkinWnd *pWnd, UINT nChar, UINT nFlags)
{
	ASSERT(pWnd);

	bool ctrl = IsKeyPressed(VK_CONTROL);
	bool shift = IsKeyPressed(VK_SHIFT);
	bool alt = IsKeyPressed(VK_MENU);

	int		nModifiers = 0;
	int		nIDKeyPressed;

	if (ctrl)
		nModifiers |= MOD_CONTROL;
	if (shift)
		nModifiers |= MOD_SHIFT;
	if (alt)
		nModifiers |= MOD_ALT;

	nIDKeyPressed = MakeHotkeyID(nModifiers, nChar);

	for (int i = 0; i < (int)m_vAccKey.size(); i++)
	{
		CmdAccKey		&cmdKey = m_vAccKey[i];

		if (!cmdKey.bGlobal && nIDKeyPressed == cmdKey.idHotKey)
		{
			// execute cmd
			pWnd->OnCustomCommand(cmdKey.cmd);
			return true;
		}
	}

	return false;
}

void CMPHotkey::Add(int cmd, bool bGlobal, int nKey, int fuModifiers)
{
	CmdAccKey	cmdKey;
	cmdKey.cmd = cmd;
	cmdKey.bGlobal = bGlobal;
	cmdKey.button = nKey;
	cmdKey.fsModifiers = fuModifiers;
	cmdKey.idHotKey = MakeHotkeyID(cmdKey.fsModifiers, cmdKey.button);

	if (bGlobal && cmdKey.button != 0)
		RegisterHotKey(m_hWnd, cmdKey.idHotKey, cmdKey.fsModifiers, cmdKey.button);

	m_vAccKey.push_back(cmdKey);

	SaveSettings();
}

bool CMPHotkey::Set(int nIndex, int cmd, bool bGlobal, int nKey, int fuModifiers)
{
	if (nIndex < 0 || nIndex >= (int)m_vAccKey.size())
		return false;

	CmdAccKey &cmdKey = m_vAccKey[nIndex];
	if (cmdKey.button != 0 && cmdKey.bGlobal)
		UnregisterHotKey(m_hWnd, cmdKey.idHotKey);

	cmdKey.cmd = cmd;
	cmdKey.bGlobal = bGlobal;
	cmdKey.button = nKey;
	cmdKey.fsModifiers = fuModifiers;
	cmdKey.idHotKey = MakeHotkeyID(cmdKey.fsModifiers, cmdKey.button);

	if (cmdKey.bGlobal && cmdKey.button != 0)
		RegisterHotKey(m_hWnd, cmdKey.idHotKey, cmdKey.fsModifiers, cmdKey.button);

	SaveSettings();

	return true;
}

bool CMPHotkey::Remove(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_vAccKey.size())
		return false;

	CmdAccKey &cmdKey = m_vAccKey[nIndex];
	if (cmdKey.button != 0 && cmdKey.bGlobal)
		UnregisterHotKey(m_hWnd, cmdKey.idHotKey);

	m_vAccKey.erase(m_vAccKey.begin() + nIndex);

	SaveSettings();

	return true;
}

CMPHotkey::CmdAccKey *CMPHotkey::Get(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_vAccKey.size())
		return NULL;

	return &(m_vAccKey[nIndex]);
}

int CMPHotkey::GetByKey(int nKey, int nModifiers)
{
	int  nIDKey = MakeHotkeyID(nModifiers, nKey);

	for (int i = 0; i < (int)m_vAccKey.size(); i++)
	{
		CmdAccKey		&cmdKey = m_vAccKey[i];

		if (nIDKey == cmdKey.idHotKey)
		{
			return i;
		}
	}

	return -1;
}

bool CMPHotkey::GetHotkeyText(int cmd, tstring &strKey)
{
	for (int i = 0; i < (int)m_vAccKey.size(); i++)
	{
		CmdAccKey		&cmdKey = m_vAccKey[i];

		if (cmdKey.cmd == cmd)
		{
			FormatHotkeyText(strKey, cmdKey.button, cmdKey.fsModifiers);
			return true;
		}
	}

	return false;
}


int CMPHotkey::LoadSettings()
{
	// every line is a hotkey settings.
	// cmd, keycode, modifiers
	tstring		strFile;
	CCSVFile	file;
	tstring		str;
	bool		bFileVer1 = false;

	m_vAccKey.clear();

	strFile = GetAppDataDir();
	strFile += SZ_HOTKEY_FILENAME;

	if (!file.Open(strFile.c_str(), CCSVFile::OM_READ))
		return ERR_OPEN_FILE;

	// File header correct?
	if (!file.ReadLine() || !file.ReadFieldString(str))
		return ERR_BAD_FILE_FORMAT;
	if (_tcscmp(str.c_str(), SZ_HOTKEY_FILE_HEADER) != 0)
	{
		if (_tcscmp(str.c_str(), SZ_HOTKEY_FILE_HEADER_V1) != 0)
			return ERR_BAD_FILE_FORMAT;
		bFileVer1 = true;
	}

	while (file.ReadLine())
	{
		CmdAccKey	cmdKey;
		int			nValue;

		if (!file.ReadFieldString(str))
			continue;

		// Cmd
		cmdKey.cmd = MPCMDStrToID(str.c_str());
		if (cmdKey.cmd == SKIN_UIOBJ_ID_UNDEFINE)
			continue;

		// Global flag
		if (!bFileVer1)
		{
			if (!file.ReadFieldInt(nValue))
				continue;
			cmdKey.bGlobal = Tobool(nValue);
		}

		// keycode
		if (!file.ReadFieldInt(nValue))
			continue;
		cmdKey.button = (uint16)nValue;

		// modifier
		if (!file.ReadFieldInt(nValue))
			continue;
		cmdKey.fsModifiers = (uint16)nValue;

		cmdKey.idHotKey = MakeHotkeyID(cmdKey.fsModifiers, cmdKey.button);
		m_vAccKey.push_back(cmdKey);
	}

	return ERR_OK;
}

int CMPHotkey::SaveSettings()
{
	tstring		strFile;
	CCSVFile	file;

	strFile = GetAppDataDir();
	strFile += SZ_HOTKEY_FILENAME;

	if (!file.Open(strFile.c_str(), CCSVFile::OM_WRITE))
		return ERR_OPEN_FILE;

	file.WriteFieldString(SZ_HOTKEY_FILE_HEADER);
	file.WriteLine();

	for (int i = 0; i < (int)m_vAccKey.size(); i++)
	{
		CmdAccKey	&cmdKey = m_vAccKey[i];
		LPCTSTR		szID = MPCMDIDToStr(cmdKey.cmd);

		if (!szID)
			continue;

		file.WriteFieldString(szID);
		file.WriteFieldInt(cmdKey.bGlobal);
		file.WriteFieldInt(cmdKey.button);
		file.WriteFieldInt(cmdKey.fsModifiers);
		file.WriteLine();
	}

	return ERR_OK;
}

void CMPHotkey::RegisterAllHotKeys()
{
	for (int i = 0; i < (int)m_vAccKey.size(); i++)
	{
		CmdAccKey		&cmdKey = m_vAccKey[i];

		cmdKey.idHotKey = MakeHotkeyID(cmdKey.fsModifiers, cmdKey.button);
		if (cmdKey.bGlobal && cmdKey.button != 0)
			RegisterHotKey(m_hWnd, cmdKey.idHotKey, cmdKey.fsModifiers, cmdKey.button);
	}
}

void CMPHotkey::UnregisterAllHotKeys()
{
	for (int i = 0; i < (int)m_vAccKey.size(); i++)
	{
		CmdAccKey		&cmdKey = m_vAccKey[i];

		if (cmdKey.button != 0 && cmdKey.bGlobal)
			UnregisterHotKey(m_hWnd, cmdKey.idHotKey);
	}
}
