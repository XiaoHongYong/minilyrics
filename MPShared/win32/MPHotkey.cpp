// MPHotkey.cpp: implementation of the CMPHotkey class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "MLCmd.h"
#include "MPHotkey.h"
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
	{ CMD_UNDEFINE, 0, 0, 0, 0 },
};

int				g_mlCmdAccKeyCount = CountOf(g_mlCmdAccKey) - 1;*/


int				g_hcPlayback[] = { CMD_PLAY, CMD_PLAYPAUSE, CMD_STOP, CMD_PREVIOUS, CMD_NEXT, CMD_FORWARD, CMD_BACKWARD, 0 };
int				g_hcGeneral[] = { CMD_QUIT, 0 };
int				g_hcUI[] = { CMD_TOGGLE_MP, CMD_FLOATING_LYRICS, CMD_PREFERENCES, CMD_TOPMOST, 0 };
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
	{ CMD_FLOATING_LYRICS, 'F', MOD_CONTROL | MOD_ALT, 0 },
};

#define SZ_HOTKEY_FILE_HEADER	"HotkeysV1.0"
#define SZ_HOTKEY_FILENAME		_T("hotkeys.cfg")

LPCTSTR MPCMDIDToStr(STRING_ID_MAP idStr[], int nCmd);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPHotkey::CMPHotkey()
{
	m_bEnabled = false;
}

CMPHotkey::~CMPHotkey()
{

}

void CMPHotkey::Init(CBaseWnd *pWnd)
{
	if (LoadSettings() != ERR_OK)
	{
		// load default
		m_vAccKey.clear();
		for (int i = 0; i < CountOf(g_vDefAccKey); i++)
			m_vAccKey.push_back(g_vDefAccKey[i]);
	}

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

void CMPHotkey::Enable(bool bEnable)
{
	g_Profile.WriteProfileInt(_T("EnableGlobalHotkey"), bEnable);
	if (bEnable)
	{
		if (!m_bEnabled)
			RegisterAllHotKeys();
	}
	else
	{
		if (m_bEnabled)
			UnregisterAllHotKeys();
	}

	m_bEnabled = bEnable;
}

void CMPHotkey::RestoreDefaults()
{
	UnregisterAllHotKeys();
	m_vAccKey.clear();

	for (int i = 0; i < CountOf(g_vDefAccKey); i++)
	{
		m_vAccKey.push_back(g_vDefAccKey[i]);
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

		if (nId == cmdKey.idHotKey)
		{
			// execute cmd
			if (g_mpApp.GetSkinFactory()->GetMainWnd())
				g_mpApp.GetSkinFactory()->GetMainWnd()->OnCustomCommand(cmdKey.cmd);
			return;
		}
	}
}

void CMPHotkey::Add(int cmd, int nKey, int fuModifiers)
{
	CmdAccKey	cmdKey;
	cmdKey.cmd = cmd;
	cmdKey.button = nKey;
	cmdKey.fsModifiers = fuModifiers;
	cmdKey.idHotKey = 0xC000 | (cmdKey.fsModifiers << 8) | cmdKey.button;
	m_vAccKey.push_back(cmdKey);

	if (cmdKey.button != 0)
		RegisterHotKey(m_hWnd, cmdKey.idHotKey, cmdKey.fsModifiers, cmdKey.button);

	SaveSettings();
}

bool CMPHotkey::Set(int nIndex, int cmd, int nKey, int fuModifiers)
{
	if (nIndex < 0 || nIndex >= (int)m_vAccKey.size())
		return false;

	CmdAccKey &cmdKey = m_vAccKey[nIndex];
	if (cmdKey.button != 0)
		UnregisterHotKey(m_hWnd, cmdKey.idHotKey);

	cmdKey.cmd = cmd;
	cmdKey.button = nKey;
	cmdKey.fsModifiers = fuModifiers;
	cmdKey.idHotKey = 0xC000 | (cmdKey.fsModifiers << 8) | cmdKey.button;

	if (cmdKey.button != 0)
		RegisterHotKey(m_hWnd, cmdKey.idHotKey, cmdKey.fsModifiers, cmdKey.button);

	SaveSettings();

	return true;
}

bool CMPHotkey::Remove(int nIndex)
{
	if (nIndex < 0 || nIndex >= (int)m_vAccKey.size())
		return false;

	CmdAccKey &cmdKey = m_vAccKey[nIndex];
	if (cmdKey.button != 0)
		UnregisterHotKey(m_hWnd, cmdKey.idHotKey);
	// GlobalDeleteAtom(cmdKey.idHotKey);

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

int CMPHotkey::LoadSettings()
{
	// every line is a hotkey settings.
	// cmd, keycode, modifiers
	FILE		*fp;
	tstring		strFile;
	char		szBuff[512];
	tstringex	strCmd;

	m_vAccKey.clear();

	strFile = GetAppDataDir();
	strFile += SZ_HOTKEY_FILENAME;

	fp = _tfopen(strFile.c_str(), _T("rb"));
	if (!fp)
		return ERR_OPEN_FILE;

	if (!fgets(szBuff, CountOf(szBuff), fp))
	{
		fclose(fp);
		return ERR_READ_FILE;
	}

	if (strncmp(szBuff, SZ_HOTKEY_FILE_HEADER, strlen(SZ_HOTKEY_FILE_HEADER)) != 0)
	{
		fclose(fp);
		return ERR_BAD_FILE_FORMAT;
	}

	while (fgets(szBuff, CountOf(szBuff), fp))
	{
		CmdAccKey	cmdKey;

		char	*p = szBuff;
		char	*pEnd;

		// Cmd
		pEnd = strchr(p, ',');
		if (!pEnd)
			continue;
		*pEnd = '\0';
		ConvertStr(p, -1, strCmd);
		cmdKey.cmd = (ML_CUSTOM_COMMAND)g_mpApp.GetSkinFactory()->GetIDByName(strCmd.c_str());

		// keycode
		p = pEnd + 1;
		pEnd = strchr(p, ',');
		if (!pEnd)
			continue;
		*pEnd = '\0';
		cmdKey.button = (uint16)atoi(p);
		if (cmdKey.button == 0)
			continue;

		// modifier
		p = pEnd + 1;
		cmdKey.fsModifiers = (uint16)atoi(p);

		cmdKey.idHotKey = 0;
		m_vAccKey.push_back(cmdKey);
	}
	fclose(fp);

	return ERR_OK;
}

int CMPHotkey::SaveSettings()
{
	FILE		*fp;
	tstring		strFile;
	char		szBuff[512];
	stringex strCmd;

	strFile = GetAppDataDir();
	strFile += SZ_HOTKEY_FILENAME;

	fp = _tfopen(strFile.c_str(), _T("wb"));
	if (!fp)
		return ERR_OPEN_FILE;

	fwrite(SZ_HOTKEY_FILE_HEADER, 1, strlen(SZ_HOTKEY_FILE_HEADER), fp);
	fwrite("\n", 1, 1, fp);

	for (int i = 0; i < (int)m_vAccKey.size(); i++)
	{
		CmdAccKey	&cmdKey = m_vAccKey[i];
		int			nLen;
		LPCTSTR		szID = MPCMDIDToStr(g_StringIdMap, cmdKey.cmd);

		if (!szID)
			continue;

		ConvertStr(szID, -1, strCmd);
		sprintf(szBuff, "%s,%d,%d\n", strCmd.c_str(), cmdKey.button, cmdKey.fsModifiers);
		nLen = strlen(szBuff);
		if (fwrite(szBuff, 1, nLen, fp) != nLen)
			break;
	}
	fclose(fp);

	return ERR_OK;
}

void CMPHotkey::RegisterAllHotKeys()
{
	for (int i = 0; i < (int)m_vAccKey.size(); i++)
	{
		CmdAccKey		&cmdKey = m_vAccKey[i];

		cmdKey.idHotKey = 0xC000 | (cmdKey.fsModifiers << 8) | cmdKey.button;

		if (cmdKey.button != 0)
			RegisterHotKey(m_hWnd, cmdKey.idHotKey, cmdKey.fsModifiers, cmdKey.button);
	}
}

void CMPHotkey::UnregisterAllHotKeys()
{
	for (int i = 0; i < (int)m_vAccKey.size(); i++)
	{
		CmdAccKey		&cmdKey = m_vAccKey[i];

		if (cmdKey.button != 0)
			UnregisterHotKey(m_hWnd, cmdKey.idHotKey);
	}
}
