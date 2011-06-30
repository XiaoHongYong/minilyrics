#include "StdAfx.h"
#include "PlayerPluginInstaller.h"
#include "../MLPluginLib/MiniLyricsPlugin.h"
#include "MLVersion.h"
#include "FileVersionInfo.h"
#include "ProcessEnum.h"

enum InstallFlag
{
	INST_COPY_2_PLAYER_PLUGIN_DIR	= 1,
	INST_REG_DLL					= 1 << 1,
	INST_CREATE_MLPLUGIN_INI		= 1 << 2,
};

#define		SZ_MLPLUGIN_INI		_T("MLPlugin.ini")

CPlayerPluginInstaller::Player		g_playersPlugin[] =
{
	// Players need the plugin to install to proper location.
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		INST_COPY_2_PLAYER_PLUGIN_DIR | INST_CREATE_MLPLUGIN_INI,
		_T("iTunes"),
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{81063354-9060-42B2-A000-1EBE96778AA9}\\InstallLocation"),
		_T("iTunes\\"), _T("iTunes.exe"),
		_T("Plug-ins"),
		_T("InstiTunes"),
		_T("iTunesMinilyrics.dll"),
	},
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		INST_COPY_2_PLAYER_PLUGIN_DIR | INST_CREATE_MLPLUGIN_INI,
		_T("Winamp"),
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Winamp\\UninstallString"),
		_T("Winamp\\"), _T("winamp.exe"),
		_T("Plugins"),
		_T("InstWinamp5"),
		_T("gen_MiniLyrics.dll"),
	},
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		INST_COPY_2_PLAYER_PLUGIN_DIR | INST_CREATE_MLPLUGIN_INI,
		_T("MediaMonkey"),
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\MediaMonkey_is1\\DisplayIcon"),
		_T("MediaMonkey\\"), _T("MediaMonkey.exe"),
		_T("Plugins"),
		_T("InstMediaMonkey"),
		_T("gen_MiniLyrics.dll"),
	},
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		INST_COPY_2_PLAYER_PLUGIN_DIR | INST_CREATE_MLPLUGIN_INI,
		_T("foobar2000"),
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\foobar2000\\InstallLocation"),
		_T("foobar2000\\"), _T("foobar2000.exe"),
		_T("components"),
		_T("InstFoobar"),
		_T("foo_ui_minilyrics.dll"),
	},
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		INST_COPY_2_PLAYER_PLUGIN_DIR | INST_CREATE_MLPLUGIN_INI,
		_T("VLC"),
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\VLC media player\\DisplayIcon"),
		_T("VideoLAN\\VLC\\"), _T("vlc.exe"),
		_T("plugins"),
		_T("InstVLC"),
		_T("libminilyrics_plugin.dll"),
	},
	{
		0, 0, 0, 0,
		INST_COPY_2_PLAYER_PLUGIN_DIR | INST_CREATE_MLPLUGIN_INI,
		_T("Songbird"),
		_T(""),
		_T("Songbird\\"), _T("songbird.exe"),
		_T("components"),
		_T("InstSB"),
		_T("sbMLSongbird.dll;sbMLSongbird.xpt")
	},
/*	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		INST_COPY_2_PLAYER_PLUGIN_DIR | INST_CREATE_MLPLUGIN_INI,
		_T("RealPlayer"),
		_T(""),
		_T("Common Files\\Real\\Plugins\\"), _T(""),
		_T(""),
		_T("InstRealOne"),
		_T("PluginRealone.dll"),
	},*/
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		INST_COPY_2_PLAYER_PLUGIN_DIR | INST_CREATE_MLPLUGIN_INI,
		_T("XMPlay"),
		_T(""),
		_T("xmplay\\"), _T("xmplay.exe"),
		_T(""),
		_T("InstXM"),
		_T("xmp-ml.dll"),
	},
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		INST_COPY_2_PLAYER_PLUGIN_DIR | INST_CREATE_MLPLUGIN_INI,
		_T("AIMP2"),
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\AIMP2\\UninstallString"),
		_T("AIMP2\\"), _T("AIMP2.exe"),
		_T("Plugins"),
		_T("InstAIMP"),
		_T("gen_MiniLyrics.dll"),
	},
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		INST_COPY_2_PLAYER_PLUGIN_DIR | INST_CREATE_MLPLUGIN_INI,
		_T("AIMP3"),
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\AIMP3\\UninstallString"),
		_T("AIMP3\\"), _T("AIMP3.exe"),
		_T("Plugins"),
		_T("InstAIMP3"),
		_T("gen_MiniLyrics.dll"),
	},
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		INST_COPY_2_PLAYER_PLUGIN_DIR | INST_CREATE_MLPLUGIN_INI,
		_T("KMPlayer"),
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\The KMPlayer\\UninstallString"),
		_T("The KMPlayer\\"), _T("KMPlayer.exe"),
		_T("Plugins"),
		_T("InstKMPlayer"),
		_T("gen_MiniLyrics.dll"),
	},
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		INST_COPY_2_PLAYER_PLUGIN_DIR | INST_CREATE_MLPLUGIN_INI,
		_T("Quintessential Player"),
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Quintessential Player\\UninstallString"),
		_T("Quintessential Player\\"), _T("QCDPlayer.exe"),
		_T("Plugins"),
		_T("InstQCD"),
		_T("QCDMinilyrics.dll"),
	},
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		INST_COPY_2_PLAYER_PLUGIN_DIR | INST_CREATE_MLPLUGIN_INI,
		_T("Quintessential Media Player"),
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Quintessential Media Player\\UninstallString"),
		_T("Quintessential Media Player\\"), _T("QMPlayer.exe"),
		_T("Plugins"),
		_T("InstQMP"),
		_T("QCDMinilyrics.dll"),
	},
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		INST_COPY_2_PLAYER_PLUGIN_DIR | INST_CREATE_MLPLUGIN_INI,
		_T("Silverjuke"),
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Silverjuke\\UninstallString"),
		_T("Silverjuke\\"), _T("Silverjuke.exe"),
		_T(""),
		_T("InstSJ"),
		_T("PluginSilverjuke.dll")
	},
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		INST_COPY_2_PLAYER_PLUGIN_DIR | INST_CREATE_MLPLUGIN_INI,
		_T("MusicBee"),
		_T(""),
		_T("MusicBee\\"), _T("MusicBee.exe"),
		_T("Plugins"),
		_T("InstMB"),
		_T("MB_MiniLyrics.dll")
	},
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		INST_COPY_2_PLAYER_PLUGIN_DIR | INST_CREATE_MLPLUGIN_INI,
		_T("BSPlayer"),
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\BSPlayer1\\UninstallString"),
		_T("Webteh\\BSplayer\\"), _T("bsplayer.exe"),
		_T("plugins"),
		_T("InstBSPlayer"),
		_T("MLPluginBsp.dll"),
	},

	// The MiniLyrics plugin needn't to install to player folder.
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		INST_REG_DLL,
		_T("Windows Media Player"),
		NULL,
		_T("Windows Media Player\\"), _T("wmplayer.exe"),
		NULL,
		_T("InstWMPlayer"),
		_T("PluginWMPVis.dll;WMPLyrics.dll"),
	},
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		0,
		_T("Media Player Classic HC"),
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\mpc-hc.exe\\"),
		_T("K-Lite Codec Pack\\Media Player Classic\\"), _T("mpc-hc.exe"),
		NULL,
		_T("InstMPC"),
		NULL,
	},
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		0,
		_T("Zune Software"),
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Zune\\InstallLocation"),
		_T("Zune\\"), _T("Zune.exe"),
		NULL,
		_T("InstZUNE"),
		NULL,
	},
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		0,
		_T("Spotify"),
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Spotify\\InstallLocation"),
		_T("Spotify\\"), _T("Spotify.exe"),
		NULL,
		_T("InstSpotify"),
		NULL,
	},
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		INST_REG_DLL,
		_T("J. River Media Jukebox"),
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Media Jukebox 14\\UninstallString"),
		_T("J River\\Media Jukebox 14\\"), _T("Media Jukebox 14.exe"),
		NULL,
		_T("InstMJ"),
		_T("PluginMJ.dll"),
	},
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		INST_REG_DLL,
		_T("J. River Media Center"),
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Media Center 16\\UninstallString"),
		_T("J River\\Media Center 16\\"), _T("Media Center 16.exe"),
		NULL,
		_T("InstMJ"),
		_T("PluginMJ.dll"),
	},
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		0,
		_T("Helium Music Manager"),
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{E320F1E2-4E3C-43B3-8F5E-5D08AA5C71F0}}_is1\\InstallLocation"),
		_T("Intermedia Design\\Helium 2007\\"), _T("helium2007.exe"),
		NULL,
		_T("InstHMM"),
		NULL,
	},
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		0,
		_T("JetAudio"),
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{DF8195AF-8E6F-4487-A0EE-196F7E3F4B8A}\\InstallLocation"),
		_T("JetAudio\\"), _T("JetAudio.exe"),
		NULL,
		_T("InstJA"),
		NULL,
	},
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		0,
		_T("AlbumPlayer"),
		_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\AlbumPlayer_is1\\InstallLocation"),
		_T("AlbumPlayer\\"), _T("AlbumPlayer.exe"),
		NULL,
		_T("InstAP"),
		NULL,
	},
};

//////////////////////////////////////////////////////////////////////////

bool IsFileSame(LPCTSTR szFile1, LPCTSTR szFile2)
{
	if (!IsFileExist(szFile1) || !IsFileExist(szFile2))
		return false;

	byte	digist1[MD5_LEN], digist2[MD5_LEN];

	if (!MDFile(szFile1, digist1))
		return false;

	if (!MDFile(szFile2, digist2))
		return false;

	return memcmp(digist1, digist2, MD5_LEN) == 0;
}

//////////////////////////////////////////////////////////////////////////

void UpdatePlayerPath(LPCTSTR szFile, DWORD dwProcessId)
{
	LPCTSTR	szFileName = FileGetName(szFile);

	for (int i = 0; i < CountOf(g_playersPlugin); i++)
	{
		CPlayerPluginInstaller::Player &item = g_playersPlugin[i];
		if (item.szExeName && _tcsicmp(szFileName, item.szExeName) == 0)
		{
			FileGetPath(szFile, item.pExtraInfo->strPlayerPath);
			item.pExtraInfo->dwProcessId = dwProcessId;
			DBGLOG2(_T("Player: %s is running: %d"), szFile, dwProcessId);
			return;
		}
	}
}

bool UpdatePluginService(HWND hWndPluginService, DWORD dwProcessId)
{
	for (int i = 0; i < CountOf(g_playersPlugin); i++)
	{
		CPlayerPluginInstaller::Player &item = g_playersPlugin[i];
		if (dwProcessId == item.pExtraInfo->dwProcessId)
		{
			item.pExtraInfo->hWndPluginService = hWndPluginService;
			item.pExtraInfo->status |= CPlayerPluginInstaller::S_PG_RUNNING;
			DBGLOG2(_T("Player of Plugin Server: %s is running: %d"), item.szPlayerName, dwProcessId);
			return true;
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////

static int __nInstance = 0;

CPlayerPluginInstaller::CPlayerPluginInstaller(void)
{
	if (__nInstance == 0)
	{
		// Alloc pExtraInfo
		for (int i = 0; i < CountOf(g_playersPlugin); i++)
		{
			Player &item = g_playersPlugin[i];
			if (!item.pExtraInfo)
				item.pExtraInfo = new PlayerExtraInfo();
		}
	}
	__nInstance++;
}

CPlayerPluginInstaller::~CPlayerPluginInstaller(void)
{
	__nInstance--;
	if (__nInstance == 0)
	{
		// Free pExtraInfo
		for (int i = 0; i < CountOf(g_playersPlugin); i++)
		{
			if (g_playersPlugin[i].pExtraInfo)
			{
				delete g_playersPlugin[i].pExtraInfo;
				g_playersPlugin[i].pExtraInfo = NULL;
			}
		}
	}
}

tstring GetProgramFilesPath()
{
#define CSIDL_PROGRAM_FILES             0x0026        // C:\Program Files
	TCHAR		szPath[MAX_PATH];
	EmptyStr(szPath);

	SHGetSpecialFolderPath(NULL, szPath, CSIDL_PROGRAM_FILES, false);

	return szPath;
}

bool CPlayerPluginInstaller::Init()
{
	CProfile		profile;
	tstring			strFileInstCfg = GetWorkingFolder();
	strFileInstCfg += _T("Install.ini");

	profile.Init(strFileInstCfg.c_str(), _T("INSTALL"));

	for (int i = 0; i < CountOf(g_playersPlugin); i++)
	{
		Player &item = g_playersPlugin[i];

#ifdef _DEBUG
		if (IsFlagSet(item.dwInstallFlags, INST_REG_DLL))
		{
			ASSERT(!IsFlagSet(item.dwInstallFlags, INST_COPY_2_PLAYER_PLUGIN_DIR));
			ASSERT(!IsFlagSet(item.dwInstallFlags, INST_CREATE_MLPLUGIN_INI));
		}
#endif

		// Get Player path from config file
		tstring strCfgName = item.szConfigName + tstring(_T("Path"));
		item.pExtraInfo->strPlayerPath = g_Profile.GetProfileString(strCfgName.c_str(), _T(""));
		if (item.pExtraInfo->strPlayerPath.empty())
			item.pExtraInfo->strPlayerPath = profile.GetProfileString(strCfgName.c_str(), _T(""));

		// Get Player path from registry
		if (!item.CheckPlayerInstalled())
			item.DetectPlayerPath();
	}

	// Update running player path
	RefreshRunningPlayers();

	// Update running plugin service
	RefreshRunningPluginService();

	return true;
}

void CPlayerPluginInstaller::RefreshRunningPluginService()
{
	for (int i = 0; i < CountOf(g_playersPlugin); i++)
	{
		Player &item = g_playersPlugin[i];
		item.pExtraInfo->hWndPluginService = NULL;
	}

	HWND	hWndPluginService = NULL;

	while (1)
	{
		hWndPluginService = FindWindowEx(NULL, hWndPluginService, SZ_PLUGIN_SERVICE_CLASS_NAME, NULL);
		if (hWndPluginService == NULL)
			break;

		DWORD	dwProcessId = 0;
		GetWindowThreadProcessId(hWndPluginService, &dwProcessId);
		if (dwProcessId != 0)
		{
			if (!UpdatePluginService(hWndPluginService, dwProcessId))
				ERRLOG0(_T("Failed to UpdatePluginService"));
		}
	}
}

void CPlayerPluginInstaller::RefreshRunningPlayers()
{
	for (int i = 0; i < CountOf(g_playersPlugin); i++)
	{
		Player &item = g_playersPlugin[i];
		item.pExtraInfo->dwProcessId = 0;
	}

	CProcessEnum	processEnum;

	DWORD aProcesses[1024], cbNeeded;
	if (!processEnum.Enum(aProcesses, sizeof(aProcesses), &cbNeeded))
		return;

	for (int i = cbNeeded / sizeof(DWORD) - 1; i >= 0; i--)
	{
		tstring strFile = processEnum.GetProcessFileName(aProcesses[i]);
		if (strFile.size())
			UpdatePlayerPath(strFile.c_str(), aProcesses[i]);
	}
}

bool RegisterDll(LPCTSTR szDll, tstring &strErr)
{
	if (!IsFileExist(szDll))
	{
		strErr = _T("Dll does not exist.");
		strErr += _T("\r\n");
		strErr += szDll;
		return false;
	}

	DWORD		dwExitCode = -1;
	tstring		strCmd = _T("regsvr32 /s \"");
	strCmd += szDll;
	strCmd += _T("\"");
	if (ExecuteCmdAndWait(strCmd.c_str(), 5 * 1000, &dwExitCode) && dwExitCode == 0)
		return true;

	strErr = TOLOCAL_T("Failed to register dll: ");
	strErr += szDll;
	return false;
}

bool UnregisterDll(LPCTSTR szDll, tstring &strErr)
{
	if (!IsFileExist(szDll))
		return true;

	DWORD		dwExitCode = -1;
	tstring		strCmd = _T("regsvr32 /s /u \"");
	strCmd += szDll;
	strCmd += _T("\"");
	if (ExecuteCmdAndWait(strCmd.c_str(), 5 * 1000, &dwExitCode) && dwExitCode == 0)
		return true;

	strErr = TOLOCAL_T("Failed to unregister dll: ");
	strErr += szDll;
	return false;
}

void CPlayerPluginInstaller::Player::DetectPlayerPath()
{
	pExtraInfo->strPlayerPath.clear();

	if (szPathRegKeys && !IsEmptyString(szPathRegKeys))
	{
		// Detect player by registry key
		LPCTSTR szValueName = _tcsrchr(szPathRegKeys, '\\');
		if (szValueName)
		{
			tstring		strKey;
			strKey.append(szPathRegKeys, szValueName);
			szValueName++;

			TCHAR	szPath[MAX_PATH];
			RegGetProfileString(HKEY_LOCAL_MACHINE, strKey.c_str(), szValueName, _T(""), szPath, MAX_PATH);

			TrimStr(szPath, '"');
			if (!IsEmptyString(szPath))
				FileGetPath(szPath, pExtraInfo->strPlayerPath);
		}
	}

	if (CheckPlayerInstalled())
		return;

	// Detect player by check program files folder
	pExtraInfo->strPlayerPath = DirStringJoint(GetProgramFilesPath().c_str(), szDefaultProgramFilesDir);
	DirStringAddSlash(pExtraInfo->strPlayerPath);
	if (CheckPlayerInstalled(false))
		return;

	DWORD dwDriveMask = GetLogicalDrives();
	TCHAR chDrive = 'A';
	TCHAR szDrive[] = _T("C:\\");
	for (int i = 0; i < 6; i++)
	{
		szDrive[0] = chDrive;
		if (GetDriveType(szDrive) == DRIVE_FIXED && dwDriveMask & (1 << i))
		{
			pExtraInfo->strPlayerPath[0] = chDrive;
			if (CheckPlayerInstalled(false))
				return;
		}
		chDrive++;
	}

	pExtraInfo->strPlayerPath.clear();
}

void CPlayerPluginInstaller::Player::SavePlayerPath()
{
	g_Profile.WriteProfileString((szConfigName + tstring(_T("Path"))).c_str(), pExtraInfo->strPlayerPath.c_str());
}

void CPlayerPluginInstaller::Player::SetToInstallPlugin(bool bInstall)
{
	if (bInstall)
		pExtraInfo->status |= S_PG_TO_INSTALL;
	else
		pExtraInfo->status &= ~S_PG_TO_INSTALL;
}

bool CPlayerPluginInstaller::Player::CheckPlayerInstalled(bool bClearPathIfNotInstalled)
{
	if (szExeName == NULL)
	{
		pExtraInfo->status |= CPlayerPluginInstaller::S_PLAYER_INSTALLED;
		return true;
	}

	ASSERT(pExtraInfo && szExeName);

	DirStringAddSlash(pExtraInfo->strPlayerPath);

	tstring		strPath = pExtraInfo->strPlayerPath;
	strPath += szExeName;

	pExtraInfo->status &= ~CPlayerPluginInstaller::S_PLAYER_INSTALLED;
	if (!IsFileExist(strPath.c_str()))
	{
		if (bClearPathIfNotInstalled)
			pExtraInfo->strPlayerPath.clear();
		return false;
	}

	pExtraInfo->status |= CPlayerPluginInstaller::S_PLAYER_INSTALLED;
	return true;
}

tstring CPlayerPluginInstaller::Player::GetPlayerExeFile()
{
	if (szExeName != NULL)
	{
		ASSERT(pExtraInfo && szExeName);

		DirStringAddSlash(pExtraInfo->strPlayerPath);

		tstring		strExeFile = DirStringJoint(pExtraInfo->strPlayerPath.c_str(), szExeName);
		if (IsFileExist(strExeFile.c_str()))
			return strExeFile;
	}

	return _T("");
}

class PluginInstaller
{
public:
	static int CopyFile(LPCTSTR szSrc, LPCTSTR szTarg, tstring &strErr, CPlayerPluginInstaller::Player &item)
	{
		if (!::CopyFile(szSrc, szTarg, false))
		{
			int nErr = GetLastError();
			strErr = CStrPrintf(TOLOCAL_T("Failed to copy file: %s to: %s."), szSrc, szTarg).c_str();
			strErr += _T("\n");
			strErr += CWin32Error(nErr).Description();
			return nErr;
		}

		return ERR_OK;
	}

	static bool RegisterDll(LPCTSTR szFile, tstring &strErr)
	{
		return ::RegisterDll(szFile, strErr);
	}

	static bool WriteMLPluginIni(LPCTSTR szFile, tstring &strErr)
	{
		string		strIniContent = "[MiniLyrics]\r\nWorkingFolder=";
		string		strWorkingFolder;
		TStringToMBCS(GetWorkingFolder(), -1, strWorkingFolder);
		strIniContent += strWorkingFolder + "\r\n";

		if (!SaveDataAsFile(szFile, strIniContent.c_str(), strIniContent.size()))
		{
			int nErr = GetLastError();
			strErr = CStrPrintf(TOLOCAL_T("Failed to save file: %s."), szFile).c_str();
			strErr += _T("\n");
			strErr += CWin32Error(nErr).Description();
			return false;
		}

		return true;
	}

};

class PluginUninstaller
{
public:
	static bool CopyFile(LPCTSTR szSrc, LPCTSTR szTarg, tstring &strErr, CPlayerPluginInstaller::Player &item)
	{
		if (_tcsicmp(szSrc, szTarg) != 0)
		{
			if (!DeleteFile(szTarg))
			{
				int nErr = GetLastError();
				strErr = CStrPrintf(TOLOCAL_T("Failed to delete file: %s."), szTarg).c_str();
				strErr += _T("\n");
				strErr += CWin32Error(nErr).Description();
				return false;
			}
		}
		else
			ASSERT(0);
		return true;
	}

	static bool RegisterDll(LPCTSTR szFile, tstring &strErr)
	{
		return ::UnregisterDll(szFile, strErr);
	}

	static bool WriteMLPluginIni(LPCTSTR szFile, tstring &strErr)
	{
		if (!DeleteFile(szFile))
		{
			int nErr = GetLastError();
			strErr = CStrPrintf(TOLOCAL_T("Failed to delete file: %s."), szFile).c_str();
			strErr += _T("\n");
			strErr += CWin32Error(nErr).Description();
			return false;
		}

		return true;
	}

};

class PluginInstallerVerify
{
public:
	static int CopyFile(LPCTSTR szSrc, LPCTSTR szTarg, tstring &strErr, CPlayerPluginInstaller::Player &item)
	{
		if (!IsFileExist(szTarg))
		{
			item.pExtraInfo->strMsg = _T("File does not exist.");
			item.pExtraInfo->strMsg += _T("\n");
			item.pExtraInfo->strMsg += szTarg;
			strErr = item.pExtraInfo->strMsg;
			return ERR_FILE_NOT_EXIST;
		}
		if (item.verMajor == 0)
			return ERR_OK;

		// Is version match?
		CFileVersionInfo	fileVer;
		int nMajor = 0, nMinor = 0, nBuild = 0, nRevision = 0;
		if (fileVer.Open(szTarg))
			fileVer.GetFileVersion(nMajor, nMinor, nBuild, nRevision);

		if (nMajor != item.verMajor || nMinor != item.verMinor || nRevision != item.verRevision)
		{
			item.pExtraInfo->status |= CPlayerPluginInstaller::S_PG_VER_NOT_MATCH;
			item.pExtraInfo->strMsg = TOLOCAL(_T("File version doesn't match the current version."));
			item.pExtraInfo->strMsg += _T("\r\n");
			item.pExtraInfo->strMsg += szTarg;
			strErr = item.pExtraInfo->strMsg;
			return ERR_FALSE;
		}

		return ERR_OK;
	}

	static bool RegisterDll(LPCTSTR szFile, tstring &strErr)
	{
		return true;
	}

	static bool WriteMLPluginIni(LPCTSTR szFile, tstring &strErr)
	{
		CProfile	file;
		file.Init(szFile, _T("MiniLyrics"));
		tstring str = file.GetProfileString(_T("WorkingFolder"), _T(""));
		DirStringAddSlash(str);
		if (_tcsicmp(GetWorkingFolder(), str.c_str()) == 0)
			return true;

		return false;
	}

};

template<class _Installer>
int Install(CPlayerPluginInstaller::Player &item, tstring &strErr)
{
	if (!IsFlagSet(item.pExtraInfo->status, CPlayerPluginInstaller::S_PLAYER_INSTALLED))
		return ERR_FALSE;

	if (item.szPluginFiles == NULL)
		return ERR_OK;

	tstring	strDirTarg;
	if (item.szPluginDirOfPlayer)
	{
		strDirTarg = item.pExtraInfo->strPlayerPath;
		strDirTarg += item.szPluginDirOfPlayer;
		if (!IsDirExist(strDirTarg.c_str()))
			CreateDirectoryAll(strDirTarg.c_str());
		DirStringAddSlash(strDirTarg);
	}

	V_Str	vFiles;
	StrSplit(item.szPluginFiles, ';', vFiles);

	for (int i = 0; i < (int)vFiles.size(); i++)
	{
		// Plugin for players are saved in MiniLyrics "Player-Plugins" dir.
		tstring		strFileSrc = DirStringJoint(GetWorkingFolder(), _T("Player-Plugins\\"));
		strFileSrc += vFiles[i];
#ifdef _DEBUG
		if (!IsFileExist(strFileSrc.c_str()))
			strFileSrc = DirStringJoint(GetWorkingFolder(), vFiles[i].c_str());
#endif

		tstring		strFileTarg;
		if (item.szPluginDirOfPlayer)
			strFileTarg = strDirTarg + vFiles[i];

		if (IsFlagSet(item.dwInstallFlags, INST_COPY_2_PLAYER_PLUGIN_DIR))
		{
			ASSERT(strFileTarg.size());
			if (strFileTarg.empty())
			{
				ERRLOG1(_T("No targ file is specified for player: %s"), item.szPlayerName);
				return ERR_FALSE;
			}

			ASSERT(strFileSrc != strFileTarg);
			DBGLOG2(_T("CopyFile: %s, to: %s"), strFileSrc.c_str(), strFileTarg.c_str());
			int nRet = _Installer::CopyFile(strFileSrc.c_str(), strFileTarg.c_str(), strErr, item);
			if (nRet != ERR_OK)
				return nRet;
		}

		if (IsFlagSet(item.dwInstallFlags, INST_REG_DLL))
		{
			DBGLOG1(_T("RegisterDll: %s"), strFileSrc.c_str());
			if (!_Installer::RegisterDll(strFileSrc.c_str(), strErr))
				return ERR_FALSE;
		}
	}

	if (IsFlagSet(item.dwInstallFlags, INST_CREATE_MLPLUGIN_INI))
	{
		tstring		strIniFile = strDirTarg + SZ_MLPLUGIN_INI;
		if (!_Installer::WriteMLPluginIni(strIniFile.c_str(), strErr))
			return ERR_FALSE;
	}

	return ERR_OK;
}

bool CPlayerPluginInstaller::Player::Install(tstring &strErr, int &err)
{
	ASSERT(IsFlagSet(pExtraInfo->status, S_PLAYER_INSTALLED));

	err = ::Install<PluginInstaller>(*this, strErr);
	if (err != ERR_OK)
		return false;

	pExtraInfo->status |= S_PG_INSTALLED;

	return true;
}


bool CPlayerPluginInstaller::Player::CheckPluginInstalled(tstring &strErr)
{
	pExtraInfo->status &= ~(S_PG_VER_NOT_MATCH | S_PG_MDT_NOT_MATCH | S_PG_INSTALL_ERROR);

	int nRet = ::Install<PluginInstallerVerify>(*this, strErr);
	if (nRet != ERR_OK)
	{
		pExtraInfo->status &= ~S_PG_INSTALLED;
		return false;
	}

	pExtraInfo->status |= S_PG_INSTALLED;

	return true;
}

bool CPlayerPluginInstaller::Player::Uninstall(tstring &strErr)
{
	ASSERT(IsFlagSet(pExtraInfo->status, S_PLAYER_INSTALLED));

	int nRet = ::Install<PluginUninstaller>(*this, strErr);
	if (nRet != ERR_OK)
		return false;

	pExtraInfo->status &= ~S_PG_INSTALLED;

	return true;
}

int CPlayerPluginInstaller::Player::GetStatusImage()
{
	if (IsFlagSet(pExtraInfo->status, S_PG_INSTALLED))
	{
		if (pExtraInfo->status & (S_PG_VER_NOT_MATCH | S_PG_MDT_NOT_MATCH | S_PG_INSTALL_ERROR))
			return 4; // Plugin is not installed propertly
		else
			return 1;
	}
	else if (IsFlagSet(pExtraInfo->status, S_PLAYER_INSTALLED))
		return 3;
	else
		return 2;
}

bool CPlayerPluginInstaller::Uninstall(tstring &strErr)
{
	bool	bRet = true;

	for (int i = 0; i < CountOf(g_playersPlugin); i++)
	{
		Player &item = g_playersPlugin[i];

		if (!IsFlagSet(item.pExtraInfo->status, S_PLAYER_INSTALLED))
			continue;

		if (!item.Uninstall(strErr))
			bRet = false;
	}

	return bRet;
}

CPlayerPluginInstaller::Player *CPlayerPluginInstaller::GetPlayerByIndex(int i)
{
	if (i < 0 || i >= CountOf(g_playersPlugin))
		return NULL;

	return g_playersPlugin + i;
}
