#pragma once

class CPlayerPluginInstaller
{
public:
	enum Status
	{
		S_PLAYER_INSTALLED		= 1,
		S_PG_INSTALLED			= 1 << 1,
		S_PG_TO_INSTALL			= 1 << 2,
		S_PG_VER_NOT_MATCH		= 1 << 3,		// Plugin version is not match with MiniLyrics
		S_PG_MDT_NOT_MATCH		= 1 << 4,		// Plugin file is not same.
		S_PG_RUNNING			= 1 << 5,		// Plugin is not running
		S_PG_INSTALL_ERROR		= 1 << 6,		// Plugin is NOT installed OK
	};

	struct PlayerExtraInfo
	{
		PlayerExtraInfo()
		{
			dwProcessId = 0;
			hWndPluginService = NULL;
			status = 0;
		}
		DWORD				status;
		HWND				hWndPluginService;
		DWORD				dwProcessId;
		tstring				strPlayerPath;
		tstring				strMsg;
	};

	struct Player
	{
		WORD				verMajor, verMinor, verBuild, verRevision;
		DWORD				dwInstallFlags;	// Combination of InstallFlag
		LPCTSTR				szPlayerName;
		LPCTSTR				szPathRegKeys;
		LPCTSTR				szDefaultProgramFilesDir;
		LPCTSTR				szExeName;
		LPCTSTR				szPluginDirOfPlayer;
		LPCTSTR				szConfigName;
		LPCTSTR				szPluginFiles;	// Separated with ';'
		PlayerExtraInfo		*pExtraInfo;

		bool IsInstalled() { return IsFlagSet(pExtraInfo->status, S_PLAYER_INSTALLED); }

		void DetectPlayerPathByRegistry();

		bool CheckPlayerInstalled(bool bClearPathIfNotInstalled = true);
		bool RefreshPluginInstallationStatus(tstring &strErr);
		void SetToInstallPlugin(bool bInstall);
		bool IsToInstallPlugin() { return IsFlagSet(pExtraInfo->status, S_PG_TO_INSTALL); }
		bool IsPluginInstalled() { return IsFlagSet(pExtraInfo->status, S_PG_INSTALLED); }
		bool IsPlayerRunning() { return pExtraInfo->dwProcessId != 0; }

		bool Install(tstring &strErr);
		bool Uninstall(tstring &strErr);

		int GetStatusImage();
	};

public:
	CPlayerPluginInstaller(void);
	virtual ~CPlayerPluginInstaller(void);

	bool Init();

	bool DetectRunningPlayers();

	void RefreshPluginInstallationStatus();

	bool Install(tstring &strErr);

	Player *GetPlayers();
	Player *GetPlayerByIndex(int i);

};
