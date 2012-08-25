#include "StdAfx.h"
#include "PlayerPluginInstaller.h"
#include "../../MLPluginLib/MiniLyricsPlugin.h"
#include "MLVersion.h"

enum InstallFlag
{
	INST_COPY_2_PLAYER_PLUGIN_DIR	= 1,
	INST_CREATE_MLPLUGIN_INI		= 1 << 1,
};

#define		SZ_MLPLUGIN_INI		_T("MLPlugin.ini")

CPlayerPluginInstaller::Player		g_playersPlugin[] =
{
	// Players need the plugin to install to proper location.
	{
		ML_MAJOR_VERSION, ML_MINOR_VERSION, 0, AUTO_BUILD_NUM,
		INST_COPY_2_PLAYER_PLUGIN_DIR | INST_CREATE_MLPLUGIN_INI,
		_T("iTunes"),
		_T("iTunes.app"),
		_T("~/Library/iTunes/iTunes Plug-ins/"),
		_T("InstiTunes"),
		_T("MiniLyrics for iTunes.bundle"),
	},
};

tstring getHomeDir()
{
	return [NSHomeDirectory() UTF8String];
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

bool CPlayerPluginInstaller::Init()
{
	CProfile		profile;
	tstring			strFileInstCfg = GetAppResourceDir();
	strFileInstCfg += _T("Install.ini");

	profile.Init(strFileInstCfg.c_str(), _T("INSTALL"));

	for (int i = 0; i < CountOf(g_playersPlugin); i++)
	{
		Player &item = g_playersPlugin[i];

		// Get Player path from config file
		tstring strCfgName = item.szConfigName + tstring(_T("Path"));
		item.pExtraInfo->strPlayerPath = g_Profile.GetProfileString(strCfgName.c_str(), _T(""));
		if (item.pExtraInfo->strPlayerPath.empty())
			item.pExtraInfo->strPlayerPath = profile.GetProfileString(strCfgName.c_str(), _T(""));
	}

	return true;
}

void CPlayerPluginInstaller::Player::DetectPlayerPath()
{
	pExtraInfo->strPlayerPath.clear();

	if (CheckPlayerInstalled())
		return;

	// Detect player by check program files folder
	pExtraInfo->strPlayerPath = "/Applications/";
	DirStringAddSlash(pExtraInfo->strPlayerPath);
	if (CheckPlayerInstalled(false))
		return;

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
		bool ret;
		if (IsDirExist(szSrc))
			ret = ::CopyDir(szSrc, szTarg);
		else
			ret = ::CopyFile(szSrc, szTarg, false);
		if (ret)
			return ERR_OK;

		strErr = CStrPrintf(_TLT("Failed to copy file: %s to: %s."), szSrc, szTarg).c_str();
		return ERR_FAILED;
	}
	
	static bool WriteMLPluginIni(LPCTSTR szFile, tstring &strErr)
	{
		string		strIniContent = "[MiniLyrics]\r\nWorkingFolder=";
		string		strWorkingFolder;
		TStringToMBCS(GetAppResourceDir(), -1, strWorkingFolder);
		strIniContent += strWorkingFolder + "\r\n";

		if (!SaveDataAsFile(szFile, strIniContent.c_str(), strIniContent.size()))
		{
			strErr = CStrPrintf(_TLT("Failed to save file: %s."), szFile).c_str();
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
				strErr = CStrPrintf(_TLT("Failed to delete file: %s."), szTarg).c_str();
				return false;
			}
		}
		else
			ASSERT(0);
		return true;
	}

	static bool WriteMLPluginIni(LPCTSTR szFile, tstring &strErr)
	{
		if (!DeleteFile(szFile))
		{
			strErr = CStrPrintf(_TLT("Failed to delete file: %s."), szFile).c_str();
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

		return ERR_OK;
	}

	static bool WriteMLPluginIni(LPCTSTR szFile, tstring &strErr)
	{
		CProfile	file;
		file.Init(szFile, _T("MiniLyrics"));
		tstring str = file.GetProfileString(_T("WorkingFolder"), _T(""));
		DirStringAddSlash(str);
		if (_tcsicmp(GetAppResourceDir(), str.c_str()) == 0)
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
		if (StrBeginWith(item.szPluginDirOfPlayer, "~/"))
			strDirTarg = DirStringJoint(getHomeDir().c_str(), item.szPluginDirOfPlayer + 2);
		else
			strDirTarg = item.szPluginDirOfPlayer;
		if (!IsDirExist(strDirTarg.c_str()))
			CreateDirectoryAll(strDirTarg.c_str());
		DirStringAddSlash(strDirTarg);
	}

	V_Str	vFiles;
	StrSplit(item.szPluginFiles, ';', vFiles);

	for (int i = 0; i < (int)vFiles.size(); i++)
	{
		// Plugin for players are saved in MiniLyrics "Player-Plugins" dir.
		tstring		strFileSrc = DirStringJoint(GetAppResourceDir(), _T("Player-Plugins/"));
		strFileSrc += vFiles[i];
#ifdef _DEBUG
//		if (!IsFileExist(strFileSrc.c_str()))
//			strFileSrc = DirStringJoint(GetAppResourceDir(), vFiles[i].c_str());
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
