// MPHelper.cpp: implementation of the CMPHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerAppBase.h"
#include "MPHelper.h"
#include "MLCmd.h"
#include "MediaDetectionService.h"

void GetDefaultPlaylistName(tstring &strPlaylistFile)
{
	strPlaylistFile = GetAppDataDir();
	strPlaylistFile += _T("DefPlaylist.m3u");
}

bool OnSongOpenFileCmd(CBaseWnd *pWndParent, bool bOpen)
{
	tstring				strExtentions;
	g_Player.GetFileOpenDlgExtention(strExtentions);
	CFileOpenDlg		dlg(_T("Add Music File"), _T(""), strExtentions.c_str(), 0, true);

	if (dlg.DoModal(pWndParent) == IDOK)
	{
		vector<tstring>		vFiles;
		tstring				strFile;

		dlg.GetOpenFile(vFiles);

		if (bOpen)
			g_Player.ClearPlaylist();
		else
			g_Player.SetPlaylistModified(true);

		for (int i = 0; i < (int)vFiles.size(); i++)
		{
			g_Player.AddToPlaylist(vFiles[i].c_str());
		}
		g_Player.SaveCurrentPlaylist();
		if (bOpen)
			g_Player.Play();

		return true;
	}

	return false;
}

bool OnSongOpenDirCmd(CBaseWnd *pWndParent, bool bOpen)
{
	tstring				strFolder;
	
	CFolderDialog	dlg;
	dlg.SetInitFolder(g_Profile.GetProfileString(_T("Last Open Dir"), _T("")));
	if (dlg.DoBrowse(pWndParent) == IDOK)
	{
		strFolder = dlg.GetFolder();

		g_Profile.WriteProfileString(_T("Last Open Dir"), strFolder.c_str());

		if (bOpen)
			g_Player.ClearPlaylist();
		else
			g_Player.SetPlaylistModified(true);

		g_Player.AddDirToPlaylist(strFolder.c_str(), true);
		g_Player.SaveCurrentPlaylist();
		if (bOpen)
			g_Player.Play();

		return true;
	}

	return false;
}

bool OnCmdSongAddDirToMediaLib(CBaseWnd *pWndParent)
{
	tstring				strFolder;

#ifdef _WIN32_WCE
	bool				bIncSubfolder = true;
	CFolderBrowseDlg	dlg;
	if (dlg.DoModal(pWndParent, g_Profile.GetProfileString(_T("Last Open Dir"), _T("")), 
		g_Profile.GetProfileBool(_T("AddIncSubFolder"), true)) == IDOK)
	{
		bIncSubfolder = dlg.IsIncSubfolder();
#else
	CFolderDialog	dlg;
	dlg.SetInitFolder(g_Profile.GetProfileString(_T("Last Open Dir"), _T("")));
	if (dlg.DoBrowse(pWndParent) == IDOK)
	{
#endif
		strFolder = dlg.GetFolder();

		g_Profile.WriteProfileString(_T("Last Open Dir"), strFolder.c_str());

		pWndParent->MessageOut(_T("The music in the folder will added in the background."));
		g_mediaDetectionService.AddMediaInDir(strFolder.c_str());

		return true;
#ifdef _WIN32_WCE
	}
#else
	}
#endif

	return false;
}

bool OnCmdSongAddFilesToMediaLib(CBaseWnd *pWndParent)
{
	tstring				strExtentions;
	g_Player.GetFileOpenDlgExtention(strExtentions);
	CFileOpenDlg		dlg(_T("Add Music File"), _T(""), strExtentions.c_str(), 0, true);

	if (dlg.DoModal(pWndParent) == IDOK)
	{
		pWndParent->MessageOut(_T("The music in the folder will added in the background."));
		vector<tstring> vFiles;
		dlg.GetOpenFile(vFiles);
		g_mediaDetectionService.AddMedia(vFiles);

		return true;
	}

	return false;
}

void EnumPlaylists(LPCTSTR szDir, int &nLevel, vector<tstring> &vFiles)
{
	CMLFileFind		finder;
	tstring			strDir, strFile;

	if (!finder.OpenDir(szDir))
		return;

	strDir = szDir;
	DirStringFormat(strDir);

	nLevel--;

	while (finder.FindNext())
	{
		if (finder.IsCurDir())
		{
			if (nLevel >= 0 &&
				_tcscmp(finder.GetCurName(), _T(".")) != 0 &&
				_tcscmp(finder.GetCurName(), _T("..")) != 0)
			{
				strFile = strDir + finder.GetCurName();
				EnumPlaylists(strFile.c_str(), nLevel, vFiles);
			}
		}
		else
		{
			if (CPlayer::IsExtPlaylistFile(FileGetExt(finder.GetCurName())))
			{
				strFile = strDir + finder.GetCurName();
				vFiles.push_back(strFile);
			}
		}
	}

	nLevel++;
}

void EnumPlaylistsFast(vector<tstring> &vFiles)
{
	int nLevel = 0;
	EnumPlaylists(GetAppDataDir(), nLevel, vFiles);

#ifdef _WIN32
	TCHAR		szDir[MAX_PATH];
	if (SUCCEEDED(SHGetSpecialFolderPath(NULL, szDir, CSIDL_PERSONAL, FALSE)))
	{
		nLevel = 1;
		EnumPlaylists(szDir, nLevel, vFiles);
	}

	if (SUCCEEDED(SHGetSpecialFolderPath(NULL, szDir, CSIDL_DESKTOPDIRECTORY , FALSE)))
	{
		nLevel = 0;
		EnumPlaylists(szDir, nLevel, vFiles);
	}
#endif
}
