
#pragma once

// section name
#define SZ_SECT_LYR_DISPLAY			_T("LyrDispaly")
#define SZ_SECT_FLOATING_LYR		_T("FloatingLyr")
#define SZ_SECT_PLAYER				_T("Player")
#define SZ_SECT_LYR_DL				_T("Download")

#ifdef _MPLAYER
#define SZ_SECT_UI			_T("ZikiPlayer")
#else
#define SZ_SECT_UI			_T("MiniLyrics")
#endif

#define SZ_SECT_LYR_DL		_T("Download")

bool inline IsPropertyName(LPCTSTR szName, LPCTSTR szStr) { return _tcsicmp(szName, szStr) == 0; }

#define IsTRUE(szValue)			(_tcsicmp(_T("TRUE"), szValue) == 0 || _tcsicmp(_T("1"), szValue) == 0)

tstring GetAppDataDir();
void GetAppDataDir(TCHAR szDir[]);

tstring GetResourceFile(LPCTSTR szFileName);

tstring GetLyricsFolder();

BOOL SetFileNoReadOnly(LPCTSTR szFile);

