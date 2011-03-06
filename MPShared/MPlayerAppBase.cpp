// MPlayerApp.cpp: implementation of the CMPlayerAppBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerAppBase.h"
#include "MLCmd.h"
#include "DownloadMgr.h"
#include "MPEventsDispatcher.h"
#include "VersionUpdate.h"
#include "AutoProcessEmbeddedLyrics.h"
#include "MPCommonCmdHandler.h"
#include "OnlineSearch.h"
#include "ErrorResolveDlg.h"
#include "LyricShowObj.h"

#ifdef _LINUX_GTK2
#include "../Skin/gtk2/SplashScreenWnd.h"
#include "gtk2/LyricsOutPluginMgr.h"
#endif

#ifdef _WIN32
#include "../Skin/win32/SplashScreenWnd.h"
#include "win32/LyricsOutPluginMgr.h"
#endif

#ifdef _WIN32_DESKTOP
#include "win32/MPFloatingLyrWnd.h"
#ifndef _MPLAYER
#include "../MiniLyrics/win32/MLPlayerMgr.h"
#endif
#else
#include "MPFloatingLyrWnd.h"
#endif

CMLData					g_LyricData;

CDownloadMgr			g_LyricsDownloader;

CMLClientSession		g_sessionUpload;

CLyricsLocalSearch		g_LyricSearch;

tstring GetAppNameLong()
{
#ifdef _MPLAYER
	return SZ_APP_NAME;
#else
	// ASSERT(!IsEmptyString(g_Player.GetPlayerName()));
	return tstring(SZ_APP_NAME) + tstring(_T(" for ")) + g_Player.GetPlayerName();
#endif
}

#ifdef _WIN32_DESKTOP
LPCTSTR GetRegnowBuyLink()
{
	static	TCHAR szBuyNowLink[MAX_PATH] = _T("");

	if (g_Profile.GetProfileInt(_T("UseAffiliate"), true))
	{
		RegGetProfileString(HKEY_LOCAL_MACHINE,
			_T("SOFTWARE\\Digital River\\SoftwarePassport\\Crintsoft\\MiniLyrics\\0"), _T("BuyURL"), _T(""), szBuyNowLink, CountOf(szBuyNowLink));
	}

	return szBuyNowLink;
}
#endif

LPCTSTR GetStrName(STR_NAME nameId)
{
#ifdef _MPLAYER
	switch (nameId)
	{
	case SN_HTTP_DOMAIN:	return _T("http://www.zikiplayer.com");
#ifdef _WIN32_DESKTOP
	case SN_HTTP_REGISTER:
		{
			LPCTSTR		szLink;
			szLink = GetRegnowBuyLink();
			if (IsEmptyString(szLink))
				return _T("http://www.zikiplayer.com/buy.htm");
			else
				return szLink;
		}
#endif
	case SN_HTTP_DLSKIN:	return _T("http://www.zikiplayer.com/skins.htm");
	case SN_HTTP_BBS:		return _T("http://forum.zikiplayer.com");
#ifdef _WIN32_WCE
	case SN_HTTP_ML_VER:	return _T("http://www.zikiplayer.com/zpppcver.xml");
#else
	case SN_HTTP_ML_VER:	return _T("http://www.zikiplayer.com/zpver.xml");
	case SN_HTTP_USER_REG:	return _T("http://search.crintsoft.com/signup.htm");
	case SN_HTTP_GDIPLUS:	return _T("http://www.zikiplayer.com/gdiplus/");
//	case SN_HTTP_AD:		return _T("http://www.zikiplayer.com/mlad2.xml");
	case SN_HTTP_SIGNUP:	return _T("http://search.crintsoft.com/eng/signup.htm");
	case SN_HTTP_LOGIN:		return _T("http://search.crintsoft.com/eng/login.htm");
	case SN_HTTP_RATE_LRC:	return _T("http://viewlyrics.com/lyrics/rate.aspx?fid=");
	case SN_HTTP_HELP:				return _T("http://www.zikiplayer.com/help.htm");
	case SN_HTTP_HELP_EMBEDDED_LYR:	return _T("http://www.zikiplayer.com/mlfaq-embedlyrics.htm");
	case SN_HTTP_HELP_UPLOAD_LYR:	return _T("http://www.zikiplayer.com/mlfaq-upload.htm");
	case SN_HTTP_HELP_EDIT_LYR:		return _T("http://www.zikiplayer.com/mlfaq-editlrc.htm");
	case SN_HTTP_HELP_STATIC_LYR:	return _T("http://www.zikiplayer.com/help-lyrscrollaction.htm");
	case SN_HTTP_HELP_G15_LCD:	return _T("http://www.zikiplayer.com/help-lyrscrollaction.htm");

	case SN_HTTP_DLPLUGIN:	return _T("http://www.zikiplayer.com/plugins.htm");
#endif
	case SN_EMAIL:			return _T("mailto:support@zikiplayer.com");
	case SN_SUPPORT_MAIL:	return _T("support@zikiplayer.com");
	case SN_WEBHOME:		return _T("www.zikiplayer.com");
	default: ASSERT(0);		return _T("");
	}
#else // _MPLAYER
//	if (g_bEngEdition)
	{
		switch (nameId)
		{
		case SN_HTTP_DOMAIN:	return _T("http://www.crintsoft.com");
#ifdef _WIN32_DESKTOP
		case SN_HTTP_REGISTER:
			{
				LPCTSTR		szLink;
				szLink = GetRegnowBuyLink();
				if (IsEmptyString(szLink))
					return _T("http://www.crintsoft.com/mlbuy.htm");
				else
					return szLink;
			}
#endif
		case SN_HTTP_DLSKIN:	return _T("http://www.crintsoft.com/mlskin.htm");
		case SN_HTTP_BBS:		return _T("http://forum.viewlyrics.com");
#ifdef _WIN32_WCE
		case SN_HTTP_ML_VER:	return _T("http://viewlyrics.com/mlppcver.xml");
#else
		case SN_HTTP_ML_VER:	return _T("http://viewlyrics.com/mlver.xml");
		case SN_HTTP_USER_REG:	return _T("http://search.crintsoft.com/signup.htm");
		case SN_HTTP_GDIPLUS:	return _T("http://viewlyrics.com/gdiplus/");
//		case SN_HTTP_AD:		return _T("http://viewlyrics.com/mlad2.xml");
		case SN_HTTP_SIGNUP:	return _T("http://search.crintsoft.com/eng/signup.htm");
		case SN_HTTP_LOGIN:		return _T("http://search.crintsoft.com/eng/login.htm");
		case SN_HTTP_RATE_LRC:	return _T("http://viewlyrics.com/lyrics/rate.aspx?fid=");
		case SN_HTTP_HELP:				return _T("http://www.crintsoft.com/help.htm");
		case SN_HTTP_HELP_EMBEDDED_LYR:	return _T("http://www.crintsoft.com/mlfaq-embedlyrics.htm");
		case SN_HTTP_HELP_UPLOAD_LYR:	return _T("http://www.crintsoft.com/mlfaq-upload.htm");
		case SN_HTTP_HELP_EDIT_LYR:		return _T("http://www.crintsoft.com/mlfaq-editlrc.htm");
		case SN_HTTP_HELP_STATIC_LYR:	return _T("http://www.crintsoft.com/help-lyrscrollaction.htm");
		case SN_HTTP_DLPLUGIN:	return _T("http://www.crintsoft.com/mlplugin.htm");
		case SN_HTTP_HELP_G15_LCD:	return _T("http://www.crintsoft.com/help-logitech-lcd.htm");
#endif
		case SN_EMAIL:			return _T("mailto:support@crintsoft.com");
		case SN_SUPPORT_MAIL:	return _T("support@crintsoft.com");
		case SN_WEBHOME:		return _T("www.crintsoft.com");
		default: ASSERT(0);		return _T("");
		}
	}
/*	else
	{
		switch (nameId)
		{
		case SN_HTTP_DOMAIN:	return _T("http://www.minilyrics.com");
		case SN_HTTP_REGISTER:	return _T("http://www.minilyrics.com/buy.htm");
		case SN_HTTP_DLSKIN:	return _T("http://www.minilyrics.com");
		case SN_HTTP_BBS:		return _T("http://bbs.minilyrics.com");
#ifdef _WIN32_WCE
		case SN_HTTP_ML_VER:	return _T("http://www.minilyrics.com/mlppcverchs.xml");
#else
		case SN_HTTP_ML_VER:	return _T("http://www.minilyrics.com/mlverchs.xml");
		case SN_HTTP_USER_REG:	return _T("http://www.minilyrics.com/bbs/");
		case SN_HTTP_GDIPLUS:	return _T("http://www.minilyrics.com/gdiplus/");
		case SN_HTTP_AD:		return _T("http://www.minilyrics.com/mlad2.xml");
		case SN_HTTP_SIGNUP:	return _T("http://www.minilyrics.com:1212/chs/signup.htm");
		case SN_HTTP_LOGIN:		return _T("http://www.minilyrics.com:1212/chs/login.htm");
		case SN_HTTP_RATE_LRC:	return _T("http://www.minilyrics.com/lyrics/rate.aspx?fid=");
		case SN_HTTP_DLPLUGIN:	return _T("http://www.minilyrics.com");
#endif
		case SN_EMAIL:			return _T("mailto:minilyrics@163.com");
		case SN_SUPPORT_MAIL:	return _T("minilyrics@163.com");
		case SN_WEBHOME:		return _T("www.minilyrics.com");
		default: ASSERT(0);		return _T("");
		}
	}*/
#endif // _MPLAYER
}

LPCTSTR GetAppDataDir()
{
	static TCHAR			g_szAppDataDir[MAX_PATH] = _T("");

	if (IsEmptyString(g_szAppDataDir))
	{
#ifdef _WIN32
		OSVERSIONINFO	osvi;
		bool			bInUserDir = false;

		osvi.dwOSVersionInfoSize = sizeof(osvi);
		if (GetVersionEx(&osvi))
		{
			if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && osvi.dwMajorVersion >= 5)
				bInUserDir = true;
		}
		if (bInUserDir)
		{
			tstring		strFile;

			//
			// PerUserSettings?
			//
			strFile = GetWorkingFolder();
			strFile += _T("install.ini");
			if (IsFileExist(strFile.c_str()))
			{
				CProfile	file;
				file.Init(strFile.c_str(), _T("install"));
				bInUserDir = file.GetProfileBool(_T("PerUserSettings"), true);
				if (!bInUserDir)
				{
					// To verify that it is writable.
					if (!CanWriteInDir(GetWorkingFolder()))
						bInUserDir = true;
				}
			}
		}

		if (!bInUserDir)
		{
			GetWorkingFolder(g_szAppDataDir);
		}
		else
		{
			if (SUCCEEDED(SHGetSpecialFolderPath(NULL, g_szAppDataDir, CSIDL_APPDATA, FALSE)))
			{
				DirStringFormat(g_szAppDataDir);
				_tcscat_safe(g_szAppDataDir, CountOf(g_szAppDataDir), SZ_APP_NAME _T(SZ_DIR_SLASH));
				if (!IsDirExist(g_szAppDataDir))
				{
					CreateDirectory(g_szAppDataDir, NULL);

					LPCTSTR		szToMoveFiles[] = { _T("MiniLyric.ini"), _T("MLLyrics.S2L"), _T("FontColorCustom.the"), 
						_T("dbSearchCache.db"), _T("dbLyrUpload.db"), _T("LocalLrcDb.ds") };
					tstring		strMLIniOld, strMLIniNew;

					for (int i = 0; i < CountOf(szToMoveFiles); i++)
					{
						strMLIniOld = GetWorkingFolder();
						strMLIniOld += szToMoveFiles[i];
						strMLIniNew = g_szAppDataDir;
						strMLIniNew += szToMoveFiles[i];
						if (IsFileExist(strMLIniOld.c_str()))
							MoveFile(strMLIniOld.c_str(), strMLIniNew.c_str());
					}
				}
			}
			else
				GetWorkingFolder(g_szAppDataDir);
		}
#else
		GetWorkingFolder(g_szAppDataDir);
#endif
	}

	return g_szAppDataDir;
}

void GetAppDataDir(TCHAR *szAppDir)
{
	_tcscpy_safe(szAppDir, MAX_PATH, GetAppDataDir());
}

void DispatchPlayPosEvent(int nPlayPos)
{
	static int nPlayPosLast = 0;
	nPlayPos %= 500;
	if (nPlayPos != nPlayPosLast)
	{
		nPlayPosLast = nPlayPos;
		CMPlayerAppBase::GetEventsDispatcher()->DispatchSyncEvent(ET_PLAYER_POS_UPDATE);
	}
}

int SaveDialogBox(CBaseWnd *pWnd)
{
	static bool bInModal = FALSE;
	if (!bInModal)
	{
		bInModal = TRUE;

		tstring		strMessage = CStrPrintf(TOLOCAL(_T("The lyrics of the %s file have changed.")), 
			FileGetName(g_LyricData.GetSongFileName())).c_str();
		strMessage += _T("\r\n\r\n");
		strMessage += TOLOCAL(_T("Do you want to save the changes?"));

		int nRet = pWnd->MessageOut(strMessage.c_str(), MB_YESNOCANCEL | MB_ICONINFORMATION,
			TOLOCAL(_T("Confirm to save the lyrics file")));

		bInModal = FALSE;
		return nRet;
	}

	return IDCANCEL;
}

tstring GetAppIniFile()
{
	tstring	strFile;
	//
	// Init Base frame, save log and settings in app dir
#ifdef _WIN32_WCE
	strFile = _T("HKEY_LOCAL_MACHINE\\Software\\ZikiPlayer\\");
#else
	strFile = GetAppDataDir();
	strFile += SZ_PROFILE_NAME;
	SetFileNoReadOnly(strFile.c_str());
#endif

	return strFile;
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifndef _WIN32_WCE
bool				g_bAutoMinimized = FALSE;
#endif

CMPlayerAppBase::CMPlayerAppBase()
{
#ifndef _MPLAYER
	m_bMPSkinMode = true;
#endif
}

CMPlayerAppBase::~CMPlayerAppBase()
{

}

CMPSkinFactory *CMPlayerAppBase::GetMPSkinFactory()
{
	ASSERT(m_pInstance);
	return (CMPSkinFactory *)m_pInstance->GetSkinFactory();
}

CEventsDispatcherBase *CMPlayerAppBase::GetEventsDispatcher()
{
	ASSERT(m_pInstance);
	return m_pInstance->GetEventPatcher();
}

void InitEdition()
{
	//
	// 检查版本
	//
#ifdef _DEBUG
	CProfile	file;
	tstring		strEng;
	file.Init(_T("install.ini"), _T("INSTALL"));
	strEng = file.GetProfileString(_T("Edition"), _T(""));
	if (_tcscmp(strEng.c_str(), _T("english")) == 0)
		g_bEngEdition = TRUE;
	else if (_tcscmp(strEng.c_str(), _T("chinese")) == 0)
		g_bEngEdition = FALSE;
	else 
#endif
#if defined(_WIN32)
	{
		// 根据 location 来设置中英文版本差别
		LCID	lcid;
		TCHAR	szBuff[32];

		g_bEngEdition = TRUE;
		lcid = GetUserDefaultLCID();
		if (GetLocaleInfo(lcid, LOCALE_ICOUNTRY, szBuff, CountOf(szBuff)) > 0)
		{
			if (_tcscmp(szBuff, _T("86")) == 0)
				g_bEngEdition = FALSE;
		}
	}
#else
	{
	}
#endif
}

bool CMPlayerAppBase::Init()
{
	CLanguageTool::SetTransFilePrefix(SZ_TRANS_PREFIX);

	g_LangTool.SetMacro(SZ_MACRO_PRODUCT_NAME, SZ_APP_NAME);
	g_LangTool.SetMacro(SZ_MACRO_COMPANY_NAME, SZ_COMPANY_NAME);

	SetDefaultSettings();

	InitEdition();

	if (!CSkinApp::Init())
		return false;

#ifndef _MPLAYER
	if (!MLConnectToPlayer())
		return false;
#endif

#ifdef _CPPUNIT_TEST
	RunUnitTest();
#endif // _CPPUNIT_TEST

	int			nRet = 0;

	g_LyricSearch.Init();

	g_LyricsDownloader.Init();

	g_LyricData.m_nGlobalOffsetTime = g_Profile.GetProfileInt(SZ_SECT_LYR_DISPLAY, _T("LyrDelayTime"), 0);

	g_OnlineSearch.Init();

	m_pEventDispatcher->RegisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
	m_pEventDispatcher->RegisterHanlder(ET_LYRICS_SEARCH_END, this);
	m_pEventDispatcher->RegisterHanlder(ET_DOWNLOAD_END, this);

	// Init player.
	g_Player.OnInit();

	m_hotKey.Init();

	if (!InitLoadSkin())
		return false;

	OnMediaChanged();

	GetMainWnd()->ShowWindow(SW_SHOW);

	m_hotKey.SetEventWnd(GetMainWnd());

#ifndef _WIN32_WCE
	// Restore iconic status
	if (g_Profile.GetProfileBool(_T("Minimized"), false))
	{
		if (GetMainWnd()->IsToolWindow())
			GetMainWnd()->ShowWindow(SW_HIDE);
		GetMainWnd()->ShowWindow(SW_SHOWMINNOACTIVE);
	}
#endif

#ifdef _WIN32_DESKTOP
	if (g_Profile.GetProfileBool(SZ_SECT_UI, _T("ClickThrough"), false))
	{
		GetMPSkinFactory()->SetClickThrough(true);
	}
#endif

#ifndef _MPLAYER
	if (g_Profile.GetProfileBool(_T("FloatingLyr"), false))
		g_wndFloatingLyr.Create();
#endif

	//
	// 检查是否注册以及过期
	//
	if (IsRegisterOK())
		g_LicenseType = LT_REGISTERED;

	ShowRegisterNoticeSafely();

	// 检查升级
	if (g_Profile.GetProfileBool(SZ_SECT_UI, _T("CheckNewVer"), true))
	{
		CVersionUpdate		VerUp;
		VerUp.CheckNewVersion();
	}

	g_lyrOutPlguinMgr.Init();

	return true;
}

bool CMPlayerAppBase::InitLoadSkin()
{
	int nRet;

	// create main skin window.
#ifndef _MPLAYER
	m_pSkinFactory->SetSkinFileName(_T("minilyrics.xml"));
	bool bMPSkinMode = g_Profile.GetProfileBool(_T("MPSkinMode"), true);
	if (!bMPSkinMode)
	{
		nRet = g_Player.SwitchToEmbeddedSkin();
		if (!m_pSkinFactory->IsOpened())
		{

		}
	}
	else
		nRet = LoadDefaultSkin(SZ_DEFSKIN);
#else // _MPLAYER
	nRet = LoadDefaultSkin(SZ_DEFSKIN);
#endif // _MPLAYER

	if (nRet != ERR_OK)
	{
#ifndef _MPLAYER
		m_bMPSkinMode = true;
		g_Profile.WriteProfileInt(_T("MPSkinMode"), true);
#endif
		g_Profile.WriteProfileString(_T("DefaultSkin"), SZ_DEFSKIN);
		g_Profile.GetProfileString(_T("DefaultSkinWnd"), _T(""));
		nRet = LoadDefaultSkin(SZ_DEFSKIN);
		if (nRet != ERR_OK)
		{
			MessageOut(TOLOCAL(_T("$Product$ can't open latest skin, please try other skins.")), MB_OK | MB_ICONERROR);
			nRet = GetMPSkinFactory()->OpenSimpliestSkin();
			if (nRet != ERR_OK)
				return false;
		}
	}

	return true;
}

void CMPlayerAppBase::Quit()
{
	OnLyricsChangingSavePrompt();

	g_autoProcessEmbeddedLyrics.OnQuit();

	g_LyricSearch.Quit();
	g_LyricsDownloader.Quit();


	g_lyrOutPlguinMgr.Quit();
	g_OnlineSearch.Quit();

	m_pEventDispatcher->UnregisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
	m_pEventDispatcher->UnregisterHanlder(ET_LYRICS_SEARCH_END, this);
	m_pEventDispatcher->UnregisterHanlder(ET_DOWNLOAD_END, this);
	m_pEventDispatcher->Quit();

	GetHotkey().Quit();

	g_wndFloatingLyr.Destroy();
	m_pSkinFactory->Quit();
	g_Player.OnQuit();

	// Quit SimpleJVM environment
	CSjvmSkinSystem::Quit();

	g_LyricData.Close();

	g_Profile.Close();
	g_Log.Close();
	g_LangTool.Close();

#if defined(_DEBUG) && defined(_MEM_LEAK_CHECK)
	DeInitAllocCheck();
#endif
}

void CMPlayerAppBase::OnEvent(const IEvent *pEvent)
{
	if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED)
	{
		OnMediaChanged();
	}
	else if (pEvent->eventType == ET_LYRICS_SEARCH_END)
	{
		OnOnlineSearchEnd();
	}
	else if (pEvent->eventType == ET_DOWNLOAD_END)
	{
		CEventDownloadEnd		*pDlEvt = (CEventDownloadEnd *)pEvent;
		if (pDlEvt->downloadType == CEventDownloadEnd::DT_DL_LRC_FAILED)
			OnDownloadLyricsFailed(pDlEvt->pTask);
		else if (pDlEvt->downloadType == CEventDownloadEnd::DT_DL_CHECK_NEW_VERSION_OK)
		{
			CVersionUpdate		verCheck;
			verCheck.OnDownloadOK(pDlEvt->pTask);
		}
	}
}

CEventsDispatcher *CMPlayerAppBase::NewEventPatcher()
{
	return new CMPEventsDispatcher();
}

CSkinFactory *CMPlayerAppBase::NewSkinFactory()
{
	return new CMPSkinFactory(this, g_uidDefinition);
}

void CMPlayerAppBase::OnMediaChanged()
{
	// 判断是否要保存歌词
	if (!OnLyricsChangingSavePrompt())
		return;

	// auto save embedded lyrics
	g_autoProcessEmbeddedLyrics.OnSongChanged();

	g_LyricData.Close();

	//
	// 没有歌曲信息
	//
	if (!g_Player.IsMediaOpened())
	{
		DispatchLyricsChangedSyncEvent();
		return;
	}

	TCHAR	szLyricFile[MAX_PATH];
	TCHAR	szAssociateKeyword[MAX_PATH];
	BOOL	bLyricsOpened = FALSE;
	BOOL	bAssociateNone = FALSE;
	int		nRet;

	GetAssociateFileKeyword(szAssociateKeyword, CountOf(szAssociateKeyword));

	if (g_LyricSearch.IsAssociatedWithNoneLyrics(szAssociateKeyword))
		bAssociateNone = TRUE;
	else
	{
		if (g_LyricSearch.GetBestMatchLyrics(g_Player.GetSrcMedia(), g_Player.GetArtist(), g_Player.GetTitle(), szLyricFile))
		{
			nRet = OpenLyrics(szAssociateKeyword, szLyricFile);
			if (nRet == ERR_OK)
				bLyricsOpened = TRUE;
			else
				DispatchLongErrorText(ERROR2STR_LOCAL(nRet));
		}

	}

	if (bLyricsOpened)
	{
		if (g_bAutoMinimized)
		{
			GetMainWnd()->ShowWindow(SW_SHOWNOACTIVATE);
			g_bAutoMinimized = FALSE;
		}

		if (g_LyricsDownloader.AutoDownloadEnabled() &&
			g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("DownLrcEvenIfHasTxt"), true))
		{
			if (g_LyricData.GetLyrContentType() == LCT_TXT &&
				!g_LyricSearch.IsAssociatedLyrics(g_Player.GetSrcMedia(), g_Player.GetFullTitle()))
			{
				//
				// continue to search online .lrc, if .txt lyrics is opened
				//
				if (!g_LyricsDownloader.SearchInCacheResult(false))
					g_OnlineSearch.AutoSearch(g_Player.GetArtist(), g_Player.GetTitle());
			}
		}

		return;
	}

	// Add Media Info:
	V_Str		vMediaInfo;
	if (!IsEmptyString(g_Player.GetArtist()))
		vMediaInfo.push_back(tstring(TOLOCAL(_T("Artist:"))) + _T(" ") + g_Player.GetArtist());
	if (!IsEmptyString(g_Player.GetTitle()))
		vMediaInfo.push_back(tstring(TOLOCAL(_T("Title:"))) + _T(" ") + g_Player.GetTitle());
	if (!IsEmptyString(g_Player.GetAlbum()))
		vMediaInfo.push_back(tstring(TOLOCAL(_T("Album:"))) + _T(" ") + g_Player.GetAlbum());
	g_LyricData.AddTextInLyrics(vMediaInfo);

	DispatchLyricsChangedSyncEvent();

#if !defined(_MPLAYER) && defined(_WIN32_DESKTOP)
	// 隐藏歌词显示窗口
	if (g_Profile.GetProfileBool(SZ_SECT_UI, _T("AutoHideIfNoLyrics"), false))
	{
		HWND	hWnd = GetForegroundWindow();
		if (!hWnd || GetWindowThreadProcessId(hWnd, NULL) != GetWindowThreadProcessId(GetMainWnd()->GetHandle(), NULL)
			&& !::GetParent(GetMainWnd()->GetHandle()))
		{
			CMPlayerAppBase::GetMPSkinFactory()->MinizeAll(true);
			g_bAutoMinimized = TRUE;
		}
	}

	//
	// 播放MOVIE的时候隐藏窗口
	//
	if (g_Profile.GetProfileBool(SZ_SECT_UI, _T("AutoHideOnPlayMovie"), true)
		&& g_Player.HasVideo() && !::GetParent(GetMainWnd()->GetHandle()))
	{
		CMPlayerAppBase::GetMPSkinFactory()->MinizeAll(true);
		g_bAutoMinimized = TRUE;
		return;
	}
#endif

	if (bAssociateNone)
		return;

	if (_tcslen(g_Player.GetTitle()) >= 3)
	{
		// title at least 3 char long
		g_LyricsDownloader.OnSongChanged();
	}
}

void CMPlayerAppBase::OnLanguageChanged()
{
	g_LangTool.OnLanguageChanged();

	m_pSkinFactory->OnLanguageChanged();

#ifdef _WIN32_DESKTOP
	if (g_wndFloatingLyr.IsValid())
		g_wndFloatingLyr.OnLanguageChanged();
#endif
}

void CMPlayerAppBase::OnOnlineSearchEnd()
{
	// If lyrics already associated, do not download auto.
	if (g_LyricSearch.IsAssociatedLyrics(g_Player.GetSrcMedia(), g_Player.GetFullTitle()))
		return;

	// 在线自动搜索的通知消息，表示该歌词的结果已经搜索到了
	g_LyricsDownloader.SearchInCacheResult(true);
}

CMPSkinMainWnd *CMPlayerAppBase::GetMainWnd()
{
	ASSERT(m_pInstance);

	CSkinWnd	*pWnd = m_pInstance->GetSkinFactory()->GetMainWnd();
	if (pWnd)
		return (CMPSkinMainWnd *)pWnd;
	else
	{
		ASSERT(0 && "Main window should have been created already.");
		static CMPSkinMainWnd wnd;
		wnd.m_WndDrag.Init(&wnd, NULL);
		return &wnd;
	}
}

CMPHotkey &CMPlayerAppBase::GetHotkey()
{
	ASSERT(m_pInstance);

	return ((CMPlayerAppBase*)m_pInstance)->m_hotKey;
}

CMPlayerApp *CMPlayerAppBase::GetInstance()
{
	if (m_pInstance)
		return (CMPlayerApp *)m_pInstance;

	m_mutex.Acquire();
	if (!m_pInstance)
		m_pInstance = new CMPlayerApp();
	m_mutex.Release();

	return (CMPlayerApp*)m_pInstance;
}

void CMPlayerAppBase::DispatchInfoText(LPCTSTR szInfo, LPCTSTR szInfoName)
{
	IEvent		*pEvent = new IEvent;
	pEvent->eventType = ET_UI_INFO_TEXT;
	pEvent->strValue = szInfo;
	if (szInfoName)
		pEvent->strName = szInfoName;

	GetEventsDispatcher()->DispatchUnsyncEvent(pEvent);
}

void CMPlayerAppBase::DispatchLongErrorText(LPCTSTR szError)
{
	IEvent		*pEvent = new IEvent;
	pEvent->eventType = ET_UI_LONG_ERROR_TEXT;
	pEvent->strValue = szError;

	GetEventsDispatcher()->DispatchUnsyncEvent(pEvent);
}

void CMPlayerAppBase::DispatchResearchLyrics()
{
	IEvent	*pEvent = new IEvent();
	pEvent->eventType = ET_PLAYER_CUR_MEDIA_CHANGED;
	GetEventsDispatcher()->DispatchUnsyncEvent(pEvent);
}

void CMPlayerAppBase::DispatchLyricsChangedSyncEvent()
{
	IEvent		*pEvent = new IEvent();

	pEvent->eventType = ET_LYRICS_CHANGED;
	GetEventsDispatcher()->DispatchSyncEvent(pEvent);
}

void CMPlayerAppBase::OnDownloadLyricsFailed(CDownloadTask *pTask)
{
	if (pTask->m_errResult != ERR_OK)
		ShowErrorDlg(GetMainWnd(), pTask->m_errResult);
	else if (pTask->m_nHttpRetCode != 200)
	{
		CStrPrintf	str;

		str.printf(_T("URL:  %s\r\nCode: %d, %s"), pTask->m_strURL.c_str(), 
			pTask->m_nHttpRetCode, HttpErrorCodeToStr(pTask->m_nHttpRetCode));

		GetMainWnd()->MessageOut(str.c_str());
	}
}

// 取得关联时的关键信息
void CMPlayerAppBase::GetAssociateFileKeyword(LPTSTR szAssociateFileKeyword, int nMaxLen)
{
	CLyricsLocalSearch::GetAssociateFileKeyword(g_Player.GetSrcMedia(), g_Player.GetFullTitle(), szAssociateFileKeyword, nMaxLen);
}

void CMPlayerAppBase::NewLyrics()
{
	// 判断是否要保存歌词
	if (!OnLyricsChangingSavePrompt())
		return;

	g_LyricData.Close();

	DispatchLyricsChangedSyncEvent();
}

int CMPlayerAppBase::OpenLyrics(LPCTSTR szAssociateKeyword, LPCTSTR szLrcSource)
{
	int		nRet;

	nRet = g_LyricData.OpenLyrics(szAssociateKeyword, g_Player.GetMediaLength(), szLrcSource);
	if (nRet != ERR_OK)
		return nRet;

	//
	// Show rate link in the lyrics
	//
// #ifndef _MPLAYER
	if (g_Profile.GetProfileBool(SZ_SECT_UI, _T("HideRateLink"), false))
	{
		if (g_LyricData.Properties().m_strId.size())
		{
//			g_LyricData.AddAdInLyrics(g_LyricData.GetPlayElapsedTime() + 3 * 1000, TOLOCAL(_T("Are these lyrics correct to the song? Rate them!")), 
//				(tstring(_T("cmd://rate/")) + GetStrName(SN_HTTP_RATE_LRC) + g_LyricData.Properties().m_strId).c_str());
		}
	}
// #endif

	DispatchLyricsChangedSyncEvent();

	return nRet;
}

// Before lyrics changing, Show Save Prompt dialog, 
// if return true, cancel changes.
bool CMPlayerAppBase::OnLyricsChangingSavePrompt()
{
	int			nRet;

	// 判断是否要保存歌词
	m_pEventDispatcher->DispatchSyncEvent(ET_LYRICS_ON_SAVE_EDIT);

	if (g_LyricData.HasLyricsOpened() && g_LyricData.IsModified())
	{
		if (g_LyricData.IsOnlyAddLyrScrollActions())
			nRet = IDYES;
		else
			nRet = SaveDialogBox(GetMainWnd());
		if (nRet == IDYES)
		{
			if (g_LyricData.DoesChooseNewFileName())
			{
				nRet = CMPCommonCmdHandler::SaveAsLyricsFile(GetMainWnd(),
					LyricsConentTypeToFileType(g_LyricData.GetLyrContentType()), false);

				// 如果保存失败（或用户选择取消），则不改变歌词。
				if (!nRet)
					return false;
			}
			else
			{
				nRet = g_LyricData.Save();
				if (nRet != ERR_OK)
				{
					if (g_LyricData.GetLyricsSourceType() != LST_FILE)
					{
						tstring		str;
						if (g_LyricData.ToString(str, FT_LYRICS_LRC, TRUE))
						{
							g_autoProcessEmbeddedLyrics.SaveEmbeddedLyrics(g_LyricData.GetSongFileName(), 
							_T(""), str.c_str(), g_LyricData.GetLyricsSourceType());
						}
					}
					if (GetMainWnd())
						GetMainWnd()->MessageOut(CStrPrintf(_T("%s\n%s"), ERROR2STR_LOCAL(nRet), TOLOCAL(_T("Failed to Save embedded lyrics, $Product$ will auto try again later."))).c_str());
				}
			}
		}
		else if (nRet == IDCANCEL)
		{
			return false;
		}
	}

	return true;
}


int CMPlayerAppBase::MessageOut(LPCTSTR lpText, UINT uType, LPCTSTR lpCaption)
{
	CBaseWnd	*pWnd;

	pWnd = GetMainWnd();
	if (!pWnd)
		return IDCANCEL;

	if (lpCaption == NULL)
		lpCaption = SZ_APP_NAME;

	return pWnd->MessageOut(lpText, uType, lpCaption);
}

int CMPlayerAppBase::ChangeSkin(LPCTSTR szSkin, bool bLoadSkinColorTheme)
{
	return m_pSkinFactory->ChangeSkin(szSkin, _T(""), _T(""), bLoadSkinColorTheme);
}

void CMPlayerAppBase::SetDefaultSettings()
{
	g_Profile.DoCache();

#ifdef _WIN32
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("Font"), _T("Verdana, 16, bold, 0, 0, Tahoma"));
#else
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("Font"), _T(", 13, bold, 0, 0, "));
#endif
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("FgColor"), _T("#FFFFFF"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("FgLowColor"), _T("#B2B2B2"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("BgColor"), _T("#576C91"));

	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("Ed_HighColor"), _T("#0000AE"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("Ed_LowColor"), _T("#000000"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("Ed_BgColor"), _T("#FFFFFF"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("Ed_TagColor"), _T("#FF8000"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("Ed_FocusLineBgColor"), _T("#C0C0C0"));

	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("HilightBorderColor"), _T("#000000"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("LowlightBorderColor"), _T("#000000"));

	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("HilightOBM"), OBM_COLOR);
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("HilightGradient1"), _T("#FFFF80"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("HilightGradient2"), _T("#FF2D2D"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("HilightGradient3"), _T("#AE0000"));

	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("FgColor"), _T("#FF0000"));

	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("LowlightOBM"), OBM_COLOR);
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("LowlightGradient1"), _T("#0000A0"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("LowlightGradient2"), _T("#0080FF"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("LowlightGradient3"), _T("#0000FF"));

	g_Profile.SetDefaultIfNotExist(SZ_SECT_LYR_DISPLAY, _T("FgLowColor"), _T("#0000FF"));

	//
	// Floating lyrics display settings
	//
#ifdef _WIN32
	g_Profile.SetDefaultIfNotExist(SZ_SECT_FLOATING_LYR, _T("Font"), _T("Verdana, 35, bold, 0, 0, Tahoma"));
#else
	g_Profile.SetDefaultIfNotExist(SZ_SECT_FLOATING_LYR, _T("Font"), _T(", 35, bold, 0, 0, "));
#endif
	g_Profile.SetDefaultIfNotExist(SZ_SECT_FLOATING_LYR, _T("FgColor"), _T("#0000FF"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_FLOATING_LYR, _T("FgLowColor"), _T("#8080FF"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_FLOATING_LYR, _T("BgColor"), _T("#576C91"));

	g_Profile.SetDefaultIfNotExist(SZ_SECT_FLOATING_LYR, _T("LyrDrawOpt"), _T("fadeout"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_FLOATING_LYR, _T("OutlineLyrText"), true);

	g_Profile.SetDefaultIfNotExist(SZ_SECT_FLOATING_LYR, _T("HilightBorderColor"), _T("#000000"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_FLOATING_LYR, _T("LowlightBorderColor"), _T("#000000"));

	g_Profile.SetDefaultIfNotExist(SZ_SECT_FLOATING_LYR, _T("HilightOBM"), OBM_GRADIENT_COLOR);
	g_Profile.SetDefaultIfNotExist(SZ_SECT_FLOATING_LYR, _T("HilightGradient1"), _T("#FFFFA4"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_FLOATING_LYR, _T("HilightGradient2"), _T("#FFFF00"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_FLOATING_LYR, _T("HilightGradient3"), _T("#FE6001"));

	g_Profile.SetDefaultIfNotExist(SZ_SECT_FLOATING_LYR, _T("FgColor"), _T("#FFF900"));

	g_Profile.SetDefaultIfNotExist(SZ_SECT_FLOATING_LYR, _T("LowlightOBM"), OBM_GRADIENT_COLOR);
	g_Profile.SetDefaultIfNotExist(SZ_SECT_FLOATING_LYR, _T("LowlightGradient1"), _T("#912F00"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_FLOATING_LYR, _T("LowlightGradient2"), _T("#FF8040"));
	g_Profile.SetDefaultIfNotExist(SZ_SECT_FLOATING_LYR, _T("LowlightGradient3"), _T("#973100"));

	g_Profile.SetDefaultIfNotExist(SZ_SECT_FLOATING_LYR, _T("FgLowColor"), _T("#FF8040"));
}


#ifdef _CPPUNIT_TEST

void CMPlayerAppBase::RunUnitTest()
{
	if (!g_Profile.GetProfileBool(_T("RunUnitTest"), true))
		return;

	CreateUnitResultWnd(GetAppInstance());

	::RunUnitTest();
}

#endif // _CPPUNIT_TEST
