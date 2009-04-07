#ifndef _ML_PLUGIN_HEADER_
#define _ML_PLUGIN_HEADER_

#include <windows.h>
#include "MiniLyricsPlugin.h"
#include "IPlayer.h"
#include "assert.h"
#include <string>
#include "PlayerInfoQueryBaseWnd.h"

using namespace std;

extern HINSTANCE		g_hInstance;
extern HMODULE			g_hModMiniLyrics;
extern HWND				g_hwndPlayer;
extern class CMLService	g_MLService;
extern PlayerServiceEntry g_PlayerEntry;

#define EmptyStr(str)	(str[0] = '\0')
#define CountOf(arr)	(sizeof(arr) / sizeof(arr[0]))

#define IsEmptyString(str)	(str[0] == '\0')

int UTF8ToUCS2(const char* text, int length, WCHAR *ucs2, int ubuflen);

BOOL GetProgramPath(char szPath[], HINSTANCE hInstance/* = NULL*/);

LPCSTR UrlGetName(LPCSTR szFile);

bool IsFileExist(LPCSTR szFileName);

void DirStringFormat(LPSTR	szDir);

BOOL InitMiniLyricsPlugin(HINSTANCE hInst, IPlayer *pPlayer, DWORD mlStartupFlags = 0);

BOOL WritePrivateProfileInt(
							LPCSTR lpAppName,  // section name
							LPCSTR lpKeyName,  // key name
							int nValue,			// int to add
							LPCSTR lpFileName  // initialization file
							);

char *strcpy_safe(char *strDestination, int nLenMax, const char *strSource);
char *strcat_safe(char * dst, int Size, const char * src);

wchar_t *wcscpy_safe(wchar_t *strDestination, int nLenMax, const wchar_t *strSource);
WCHAR * __cdecl wcscat_safe (
        WCHAR * dst,
		int	Size,
        const WCHAR * src
        );


#ifdef _UNICODE
#define _tcscpy_safe		wcscpy_safe
#define _tcscat_safe		wcscat_safe
#else
#define _tcscpy_safe		strcpy_safe
#define _tcscat_safe		strcat_safe
#endif

int MBCSToUCS2(const char *str, int nLen, WCHAR *strOut, int nOut, int nCodePage = CP_ACP);

long PlayerImpService(int nCommand, MLPARAM Param1, MLPARAM Param2);

class CMLService
{
public:
	enum MLSkinMode
	{
		MLSKIN_PLAYER = 1,
		MLSKIN_ML = 2,
	};

	CMLService() { m_funMLService = NULL; }
	~CMLService() {}

	bool IsValid() { return m_funMLService != NULL; }

	BOOL SetEntryPoint(MLServiceEntry *pMLEntry)
	{
		assert(pMLEntry);
		assert(pMLEntry->uVersion == ML_IF_VERSION);
		assert(pMLEntry->funMLService);

		if (pMLEntry->uVersion != ML_IF_VERSION)
		{
			MessageBoxA(NULL, "Unsupported minilyrics interface version!", "MiniLyrics", MB_OK);
			return FALSE;
		}

		m_funMLService = pMLEntry->funMLService;
		return TRUE;
	}

	void AddWndCloseto(HWND hWnd, LPCSTR szWndClass, LPCSTR szWndName)
	{
		if (m_funMLService)
		{
			if (hWnd)
				m_funMLService(MLAddWndCloseToHWND, (MLPARAM)hWnd, 0);
			else
				m_funMLService(MLAddWndCloseToName, (MLPARAM)szWndClass, (MLPARAM)szWndName);
		}
	}

	long Init(HINSTANCE hInstance, DWORD mlStartupFlags = 0)
	{
		if (m_funMLService)
			return m_funMLService(MLInit, (MLPARAM)hInstance, mlStartupFlags);
		else
			return 0;
	}

	long Quit()
	{
		if (m_funMLService)
			return m_funMLService(MLQuit, 0, 0);
		else
			return 0;
	}

	void Log(const char *szMessage)
	{
		if (m_funMLService)
			m_funMLService(MLLog, (MLPARAM)szMessage, 0);
	}

	void OnPlayStateChanged(DWORD dwPlayState)
	{
		if (m_funMLService)
			m_funMLService(MLPlayerStateChanged, (MLPARAM)dwPlayState, 0);
	}

	void OnPlayTimeChanged(DWORD dwPlayTime)
	{
		if (m_funMLService)
			m_funMLService(MLPlayPosChanged, (MLPARAM)dwPlayTime, 0);
	}

	// notify messages
	HWND GetMainWindow()
	{
		if (m_funMLService)
			return (HWND)m_funMLService(MLGetMainWnd, 0, 0);
		else
			return NULL;
	}

	// WM_ACTIVATEAPP, bActivate = wParam
	void OnNotifyPlayerActivate(BOOL bActivate)
	{
		if (m_funMLService)
			m_funMLService(MLNotifyPlayerActive, bActivate, 0);
	}

	// WM_SIZE, uState = wParam
	void OnNotifyPlayerMinRestore(UINT uState)
	{
		if (m_funMLService)
			m_funMLService(MLNotifyPlayerMinRestore, uState, 0);
	}

	void OnNotifyBeforeTrackMove(HWND hWndTrackAndChain[], int nCount)
	{
		if (m_funMLService)
			m_funMLService(MLNotifyBeforeTrackMove, (long)hWndTrackAndChain, nCount);
	}

	void OnNotifyTrackMove(int x, int y)
	{
		if (m_funMLService)
			m_funMLService(MLNotifyTrackMove, x, y);
	}

	void OnNotifyPlayerSkinChanged()
	{
		m_funMLService(MLNotifySkinChanged, 0, 0);
	}

	void About(HWND hWnd)
	{
		if (m_funMLService)
			m_funMLService(MLAbout, (MLPARAM)hWnd, 0);
	}

	void Config(HWND hWnd)
	{
		if (m_funMLService)
			m_funMLService(MLConfig, (MLPARAM)hWnd, 0);
	}

	MLSkinMode GetSkinMode()
	{
		if (m_funMLService)
			return MLSkinMode(m_funMLService(MLGetSkinMode, 0, 0));
		else
			return MLSKIN_ML;
	}

	void SetWa5Bmp(int nBmpType, HBITMAP hBmp)
	{
		if (m_funMLService)
			m_funMLService(MLSetWa5Bmp, nBmpType, (LPARAM)hBmp);
	}

	void SwitchToEmbeddedSkin(bool bEmbeddedSkin)
	{
		if (m_funMLService)
			m_funMLService(MLSwitchToEmbeddedSkin, (MLPARAM)bEmbeddedSkin, 0);
	}

	void LogW(const WCHAR *szMessage)
	{
		if (m_funMLService)
			m_funMLService(MLLogW, (MLPARAM)szMessage, 0);
	}

	void DestroyMLNotifyWnd()
	{
		if (m_funMLService)
			m_funMLService(MLDestroyNotifyWnd, 0, 0);
	}

	void OnMediaInfoChanged(LPCSTR szMediaUrl, LPCSTR szArtist, LPCSTR szAlbum, LPCSTR szTitle, unsigned long nMediaLength);

	void OnMediaInfoChanged(MLMediaInfo *pMediaInfo)
	{
		if (m_funMLService)
			m_funMLService(MLMediaInfoChanged, (MLPARAM)pMediaInfo, 0);
	}


	//
	// The functions bellow are Deprecated, use OnMediaInfoChanged instead.
	//
/*
	void OnFullTitleChanged(const char *szFullTitle)
	{
		if (m_funMLService)
			m_funMLService(MLTitleChanged, (MLPARAM)szFullTitle, 0);
	}

	void OnFullTitleChangedW(const WCHAR *szFullTitle)
	{
		if (m_funMLService)
			m_funMLService(MLTitleChangedW, (MLPARAM)szFullTitle, 0);
	}*/

public:
	FUNMLService	m_funMLService;

};

#endif // _ML_PLUGIN_HEADER_