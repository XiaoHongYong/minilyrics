
#pragma once

#include "LicenseManager.h"
#include "MLClientSession.h"

#include "Helper.h"
#include "MPSkinFactory.h"
#include "MPEventsDispatcher.h"
#include "Player.h"
#include "MLCmd.h"
#include "MPSkinMainWnd.h"
#include "LyricsLocalSearch.h"

#ifdef _WIN32
#include "win32/MPHotkey.h"
#include "win32/DlgChooseFont.h"
#endif

#ifdef _LINUX_GTK2
#include "gtk2/MPHotkey.h"
#include "gtk2/DlgChooseFont.h"
#endif

#ifdef _MAC_OSX
#include "mac/MPHotkey.h"
#include "mac/DlgChooseFont.h"
#endif

#include "resource.h"

#ifdef _WIN32_DESKTOP
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
#define SZ_MUTEX_RUNNING	_T("ZikiPlayerRunning")
#define SZ_MAINWND_CLASSNAME	_T("ZikiPlayer")
#ifndef SZ_MINILYRICS_SERVICE_CLASS_NAME
#define SZ_MINILYRICS_SERVICE_CLASS_NAME	SZ_MAINWND_CLASSNAME
#endif
#else
#define SZ_PROFILE_NAME		_T("MiniLyric.ini")
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

// Local lyrics search
extern CLyricsLocalSearch		g_LyricSearch;

extern VersionNumber			g_version;

enum STR_NAME
{
	SN_HTTP_DOMAIN,
	SN_HTTP_REGISTER,
	SN_HTTP_DLSKIN,
	SN_HTTP_DLPLUGIN,
	SN_HTTP_FAQ_INET,
	SN_HTTP_BBS,
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
	SN_HTTP_FEEDBACK,
	SN_EMAIL,
	SN_SUPPORT_MAIL,
	SN_WEBHOME,
};

enum AppMode
{
	SA_UNKNOWN,
	SA_LYRICS_PLAYER,
	SA_IPOD_LYRICS_DOWNLOADER,
};

tstring GetAppNameLong();

LPCTSTR GetStrName(STR_NAME nameId);

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

	virtual void PostQuitMessage();

protected:
	virtual CEventsDispatcher *NewEventPatcher();
	virtual CSkinFactory *NewSkinFactory();

	bool InitLyricsPlayer();
	bool InitiPodLyricsDownloader();

public:
	static CMPlayerApp *GetInstance();

	static CMPSkinFactory *GetMPSkinFactory();
	static CEventsDispatcherBase *GetEventsDispatcher();

	static tstring GetAssociateFileKeyword();
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

	int ChangeSkinByUserCmd(LPCTSTR szSkin);

#ifndef _MPLAYER
public:
	bool					m_bMPSkinMode;
#endif

	AppMode GetCurrentAppMode() const { return m_appMode; }
	AppMode					m_appMode;

	void GetCurLyrDisplaySettingName(bool bFloatingLyr, tstring &strSectionName, EventType &etDispSettings);
	LPCTSTR GetCurLyrDisplaySettingName(bool bFloatingLyr);

protected:
	void SetDefaultSettings();

protected:
	CMPHotkey				m_hotKey;

};

#ifdef _WIN32
#include "win32/MPlayerApp.h"
#endif

#ifdef _LINUX_GTK2
#include "gtk2/MPlayerApp.h"
#endif

#ifdef _MAC_OSX
#include "mac/MPlayerApp.h"
#endif

