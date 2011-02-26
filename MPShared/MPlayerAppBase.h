
#pragma once

#include "Helper.h"
#include "MPSkinFactory.h"
#include "../MPlayerLib/IMPlayer.h"
#include "MPEventsDispatcher.h"
#include "Player.h"
#include "MLCmd.h"
#include "MPSkinMainWnd.h"
#include "LyricsLocalSearch.h"

#ifdef _WIN32
#include "win32/MPHotkey.h"
#endif

#ifdef _LINUX_GTK2
#include "gtk2/MPHotkey.h"
#endif

#include "resource.h"

#ifdef _WIN32_DESKTOP
#include "win32/CECommandBar.h"
#include "win32/DlgChooseFont.h"
#endif

#ifdef _WIN32_WCE
#include "win32/wce/wce.h"
#include "win32/wce/CECommandBar.h"
#include "win32/wce/DlgChooseFont.h"
#endif

#ifdef _LINUX_GTK2
#include "gtk2/CECommandBar.h"
#include "gtk2/DlgChooseFont.h"
#endif

#ifdef _MPLAYER
#define SZ_APP_NAME			_T("ZikiPlayer")
#define SZ_COMPANY_NAME		_T("")
#else
#define SZ_APP_NAME			_T("MiniLyrics")
#define SZ_COMPANY_NAME		_T("CrintSoft")
#endif

#define SZ_MACRO_PRODUCT_NAME				_T("$Product$")
#define SZ_MACRO_COMPANY_NAME				_T("$Company$")


#ifdef _MPLAYER
#define SZ_PROFILE_NAME		_T("ZikiPlayer.ini")
#define SZ_TRANS_PREFIX		_T("zp")
#define SZ_MUTEX_RUNNING	_T("ZikiPlayerRunning")
#define SZ_MAINWND_CLASSNAME	_T("ZikiPlayer")
#else
#define SZ_PROFILE_NAME		_T("MiniLyric.ini")
#define SZ_TRANS_PREFIX		_T("ml")
#define SZ_MUTEX_RUNNING	_T("MiniLyricsRunning")
#define SZ_MAINWND_CLASSNAME	_T("MiniLyrics")
#endif

class CDownloadMgr;
class CMLClientSession;
class CPreferenceDlg;
class CDownloadTask;
class CMPlayerApp;
class CMPEventsDispatcher;

// lyrics data
extern CMLData					g_LyricData;

// lyrics download manager
extern CDownloadMgr				g_LyricsDownloader;

// Session of lyrics upload and login
extern CMLClientSession			g_sessionUpload;

// Local lyrics search
extern CLyricsLocalSearch		g_LyricSearch;

extern DWORD		MAJOR_VERSION;
extern DWORD		MINOR_VERSION;
extern DWORD		BUILD_NUMBER;

enum STR_NAME
{
	SN_HTTP_DOMAIN,
	SN_HTTP_REGISTER,
	SN_HTTP_DLSKIN,
	SN_HTTP_DLPLUGIN,
	SN_HTTP_FAQ_INET,
	SN_HTTP_BBS,
	SN_HTTP_USER_REG,
	SN_HTTP_GDIPLUS,
	SN_HTTP_ML_VER,
	// SN_HTTP_AD,
	SN_HTTP_SIGNUP,
	SN_HTTP_LOGIN,
	SN_HTTP_RATE_LRC,
	SN_HTTP_HELP,
	SN_HTTP_HELP_EMBEDDED_LYR,
	SN_HTTP_HELP_UPLOAD_LYR,
	SN_HTTP_HELP_EDIT_LYR,
	SN_HTTP_HELP_STATIC_LYR,
	SN_HTTP_HELP_G15_LCD,
	SN_EMAIL,
	SN_SUPPORT_MAIL,
	SN_WEBHOME,
};

tstring GetAppNameLong();

LPCTSTR GetStrName(STR_NAME nameId);

inline bool IsRegistered()
{
	TCHAR		szRegCode[MAX_PATH];
	g_Profile.EncryptGetProfileString(SZ_REGCODE, _T(""), szRegCode, CountOf(szRegCode));
	return (g_LicenseType == LT_REGISTERED && !IsEmptyString(szRegCode));
}

LPCTSTR GetAppDataDir();

void GetAppDataDir(TCHAR *szAppDir);

tstring GetAppIniFile();

void DispatchPlayPosEvent(int nPlayPos);

class CMPlayerAppBase : public CSkinApp
{
public:
	CMPlayerAppBase();
	virtual ~CMPlayerAppBase();

	virtual bool Init();
	virtual void Quit();

	virtual void OnEvent(const IEvent *pEvent);

protected:
	virtual CEventsDispatcher *NewEventPatcher();
	virtual CSkinFactory *NewSkinFactory();

public:
	static CMPlayerApp *GetInstance();

	static CMPSkinFactory *GetMPSkinFactory();
	static CEventsDispatcherBase *GetEventsDispatcher();

	static void GetAssociateFileKeyword(LPTSTR szAssociateFileKeyword, int nMaxLen);
	static CMPSkinMainWnd *GetMainWnd();
	static CMPHotkey &GetHotkey();

	void DispatchInfoText(LPCTSTR szInfo, LPCTSTR szInfoName = NULL);
	void DispatchLongErrorText(LPCTSTR szError);

	void DispatchResearchLyrics();
	void DispatchLyricsChangedSyncEvent();

	bool InitLoadSkin();

	void OnMediaChanged();

	void OnLanguageChanged();

	void OnOnlineSearchEnd();

	void OnDownloadLyricsFailed(CDownloadTask *pTask);

	void NewLyrics();

	int OpenLyrics(LPCTSTR szAssociateKeyword, LPCTSTR szLrcSource);

	bool OnLyricsChangingSavePrompt();

	int MessageOut(LPCTSTR lpText, UINT uType = MB_ICONINFORMATION | MB_OK, LPCTSTR lpCaption = NULL);

	int ChangeSkin(LPCTSTR szSkin, bool bLoadSkinColorTheme);

public:

#ifndef _MPLAYER
	bool					m_bMPSkinMode;
#endif

protected:
	void SetDefaultSettings();

public:
#ifdef _CPPUNIT_TEST
	void RunUnitTest();
#endif // _CPPUNIT_TEST

protected:
	CMPHotkey				m_hotKey;

};

#ifdef _WIN32
#include "win32/MPlayerApp.h"
#endif

#ifdef _LINUX_GTK2
#include "gtk2/MPlayerApp.h"
#endif
