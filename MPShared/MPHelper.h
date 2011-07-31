#pragma once

#include "PlayListFile.h"

void GetDefaultPlaylistName(tstring &strPlaylistFile);

bool OnSongOpenFileCmd(CBaseWnd *pWndParent, bool bOpen);

bool OnSongOpenDirCmd(CBaseWnd *pWndParent, bool bOpen);

bool OnCmdSongAddDirToMediaLib(CBaseWnd *pWndParent);

bool OnCmdSongAddFilesToMediaLib(CBaseWnd *pWndParent);

void EnumPlaylists(LPCTSTR szDir, int &nLevel, vector<tstring> &vFiles);

void EnumPlaylistsFast(vector<tstring> &vFiles);

