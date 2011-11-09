// MPlayerApp.cpp: implementation of the CMPlayerApp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerAppBase.h"
#include "DownloadMgr.h"
#include "VersionUpdate.h"
#include "OnlineSearch.h"
#include "AutoProcessEmbeddedLyrics.h"
#include "MPFloatingLyrWnd.h"
#include "LyricShowObj.h"
#include "../../GfxLite/win32/GdiplusGraphicsLite.h"

#include "../base/win32/ProcessEnum.h"
#ifndef _MPLAYER
#include "../MiniLyrics/win32/MLPlayerMgr.h"
#include "../MiniLyrics/win32/PlayerPluginInstaller.h"
#include "../MiniLyrics/win32/DlgSetupPlayerPlugins.h"
#endif

#include "CrashRptDlg.h"

#include "MPMsg.h"

// #define _MEM_LEAK_CHECK

#if defined(_DEBUG) && defined(_MEM_LEAK_CHECK)
//
// Go link below to check, how to use this
// http://www.codeproject.com/KB/applications/leakfinder.aspx
//
#include "Stackwalker.h"
#endif

bool TerminateAnotherInstance()
{
	CProcessEnum	processEnum;
	DWORD			vProcessID[1024];
	DWORD			nSizeReturned;

	if (!processEnum.Enum(vProcessID, sizeof(vProcessID), &nSizeReturned))
		return false;

	TCHAR	szSelfName[MAX_PATH] = _T("");
	GetModuleFileName(NULL, szSelfName, CountOf(szSelfName));
	DWORD dwCurrentPID = GetCurrentProcessId();

	nSizeReturned /= sizeof(DWORD);
	for (int i = 0; i < nSizeReturned; i++)
	{
		if (vProcessID[i] == dwCurrentPID)
			continue;
		tstring strFile = processEnum.GetProcessFileName(vProcessID[i]);
		if (_tcscmp(strFile.c_str(), szSelfName) == 0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, false, vProcessID[i]);
			if (hProcess != NULL)
			{
				ERRLOG0(_T("Terminate another running instance."));
				TerminateProcess(hProcess, 0);
				CloseHandle(hProcess);
				return true;
			}
		}
	}

	return false;
}

int WINAPI _tWinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPTSTR    lpCmdLine,
					 int       nCmdShow)
{
	SetAppInstance(hInstance);

	InitMiniDumper();

	DBGLOG1(_T("MiniLyrics command line: %s"), lpCmdLine);

	// Only allow one copy of MiniLyrics.exe running, iPod Lyrics Downloader and MiniLyrics can't be running at same time.

	CMPlayerApp *pApp = CMPlayerAppBase::GetInstance();
	if (pApp->IsAnotherInstanceRunning())
	{
		HWND		hWndMsg = FindWindow(MSG_WND_CLASS_NAME, NULL);
		if (hWndMsg)
		{
			SendActivateMainWnd(hWndMsg);

			if (!IsEmptyString(lpCmdLine))
			{
				DBGLOG0(_T("Another Instance is running, send command line to it."));
				SendCommandLine(hWndMsg, lpCmdLine);
			}
			return 0;
		}
		else
			TerminateAnotherInstance();
	}

	if (!CMPlayerAppBase::GetInstance()->Init())
		return 1;

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CMPlayerAppBase::GetInstance()->Quit();

	return msg.wParam;
}


CMPlayerApp::CMPlayerApp()
{
	m_hMutexRuning = NULL;
}

CMPlayerApp::~CMPlayerApp()
{
}

bool CMPlayerApp::IsAnotherInstanceRunning()
{
	ASSERT(!m_hMutexRuning);
	HANDLE hMutexRuning = CreateMutex(NULL, TRUE, SZ_MUTEX_RUNNING);
	int nLastErr = GetLastError();

	CloseHandle(hMutexRuning);

	return nLastErr == ERROR_ALREADY_EXISTS;
}

bool CMPlayerApp::SetRunningFlag()
{
	// Is Product already running?
	ASSERT(!m_hMutexRuning);
	m_hMutexRuning = CreateMutex(NULL, TRUE, SZ_MUTEX_RUNNING);
	if (GetLastError() == ERROR_ALREADY_EXISTS )
	{
		CloseHandle(m_hMutexRuning);
		m_hMutexRuning = NULL;
		return false;
	}

	return true;
}

bool CMPlayerApp::Init()
{
	LPCTSTR lpCmdLine = GetCommandLine();

	if (IsAnotherInstanceRunning())
	{
		HWND hWndMsg = FindWindow(MSG_WND_CLASS_NAME, NULL);
		if (hWndMsg)
		{
			SendActivateMainWnd(hWndMsg);

			if (!IsEmptyString(lpCmdLine))
			{
				DBGLOG0(_T("Another Instance is running, send command line to it."));
				SendCommandLine(hWndMsg, lpCmdLine);
			}
			return false;
		}
		else
			TerminateAnotherInstance();
	}

#if defined(_DEBUG) && defined(_MEM_LEAK_CHECK)
	InitAllocCheck(ACOutput_XML);
#endif

	g_Log.SetSrcRootDir(_T(__FILE__), 2);

	TCHAR		szWorkingFolder[MAX_PATH] = {0};
	GetModulePath(szWorkingFolder, GetAppInstance());
	SetWorkingFolder(szWorkingFolder);

	tstring		strMLIniFile =GetAppIniFile();
	tstring		strMLLogFile = GetAppDataDir();
	strMLLogFile += _T("log.txt");
	InitBaseFrameWork(GetAppInstance(), strMLLogFile.c_str(), strMLIniFile.c_str(), SZ_SECT_UI);

	g_LangTool.SetMacro(SZ_MACRO_PRODUCT_NAME, SZ_APP_NAME);
	g_LangTool.SetMacro(SZ_MACRO_COMPANY_NAME, SZ_COMPANY_NAME);

	LOG1(LOG_LVL_INFO, _T("Start %s."), GetAppNameLong().c_str());

	// Init Winsock, Ole, Common Controls, gdiplus
	WSADATA		WsaData;
	if (WSAStartup(0x0101, &WsaData) != 0)
		ERRLOG1(_T("WSAStartup FAILED: %s"), OSError().Description());

 	INITCOMMONCONTROLSEX	initData;
 	initData.dwSize = sizeof(initData);
 	initData.dwICC = ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES;
 	InitCommonControlsEx(&initData);

	// Init gdiplus
	OleInitialize(NULL);

	CGdiplusGraphicsLite::Startup();

	if (g_Profile.GetProfileBool(SZ_SECT_UI, _T("Antialias"), FALSE))
		GfxRaw::EnableAntialiasFont(TRUE);

#ifndef _MPLAYER
	m_appModeQuitToStart = SA_UNKNOWN;

	if (!ProcessCmdLine(lpCmdLine, m_appMode))
		return false;

	TCHAR szFile[MAX_PATH];
	GetModuleFileName(NULL, szFile, MAX_PATH);
	if (_tcsicmp(FileGetName(szFile), _T("iPodLyricsDownloader")) == 0)
		m_appMode = SA_IPOD_LYRICS_DOWNLOADER;
#endif

	return CMPlayerAppBase::Init();
}

void CMPlayerApp::Quit()
{
	if (!m_hMutexRuning)
		return;

	//
	// 先设置此标志表示退出，其他地方会用到根据此标志判断
	CloseHandle(m_hMutexRuning);
	m_hMutexRuning = NULL;

	CMPlayerAppBase::Quit();

#ifdef _WIN32_DESKTOP
	CGdiplusGraphicsLite::Shutdown();

	OleUninitialize();
#endif

#ifndef _MPLAYER
	if (m_appModeQuitToStart != SA_UNKNOWN)
	{
		TCHAR	szAppExe[MAX_PATH];

		GetModuleFileName(NULL, szAppExe, CountOf(szAppExe));

		if (m_appModeQuitToStart == SA_IPOD_LYRICS_DOWNLOADER)
			Execute(NULL, szAppExe, _T("/iPodLyricsDownloader"));
		else if (m_appModeQuitToStart == SA_LYRICS_PLAYER)
			Execute(NULL, szAppExe, NULL);
	}
#endif
}

#ifndef _MPLAYER

bool CMPlayerApp::IsSupportEmbedded()
{
	return g_Player.IsSupportEmbedded();
}

bool CMPlayerApp::IsEmbeddedImmovable()
{
	return g_Player.IsEmbeddedImmovable();
}

void CMPlayerApp::EndEmbeddedSkin()
{
	if (!m_bMPSkinMode)
	{
		CSkinWnd	*pWnd = GetMainWnd();

		m_bMPSkinMode = true;
		if (::GetParent(pWnd->GetHandle()))
		{
			pWnd->CloseSkin();
			pWnd->SetParent(NULL);
			g_Player.EndEmbeddedSkin();

			pWnd->m_WndDrag.EnableDrag(true);
			pWnd->m_wndResizer.FixedWidth(false);
			pWnd->m_wndResizer.FixedHeight(false);
		}

		GetMainWnd()->GetTrayIcon().ForceShow(false);
	}
}

int CMPlayerApp::OnSwitchToEmbeddedSkinMode(bool bReloadTheme)
{
	// to embedded mode
	ASSERT(IsSupportEmbedded());

	g_Profile.WriteProfileInt(_T("MPSkinMode"), false);

	int nRet;
	if (g_Player.GetPlayerType() == P_WINAMP2)
		nRet = LoadWinamp2Skin(bReloadTheme);
	else
		nRet = LoadGeneralEmbeddedSkin(bReloadTheme);

	if (nRet == ERR_OK)
		m_bMPSkinMode = false;

	return nRet;
}

#ifdef _DEBUG
void VerySkinDirOK(tstring &strDir, LPCTSTR szName)
{
	if (!IsDirExist(strDir.c_str()))
	{
		strDir = GetInstallShareDir();
		strDir += szName;
	}
}
#endif

int CMPlayerApp::LoadGeneralEmbeddedSkin(bool bLoadSkinColorTheme)
{
	//
	// General Embedded Skin
	//
	tstring		strSkinDir;
	int			nRet;
#define GEN_EMBEDDED_SKIN	_T("ml-gen-embedded-skin")

	const PlayerSkinInfo &skinInfo = g_Player.GetPlayerSkinInfo();

	if (!skinInfo.hWndToAttach)
		return ERR_FALSE;

	strSkinDir = GetWorkingFolder();

	strSkinDir += GEN_EMBEDDED_SKIN _T(SZ_DIR_SLASH);
#ifdef _DEBUG
	VerySkinDirOK(strSkinDir, GEN_EMBEDDED_SKIN _T(SZ_DIR_SLASH));
#endif

	nRet = m_pSkinFactory->ChangeSkin(GEN_EMBEDDED_SKIN, strSkinDir.c_str(), _T(""));
	if (nRet != ERR_OK)
		return nRet;

	CSkinWnd	*pWnd = GetMainWnd();

	if (IsLayeredWndSupported())
		::UnSetLayeredWindow(pWnd->GetHandle());

	if (pWnd->IsIconic())
		pWnd->ShowWindow(SW_RESTORE);

	::SetParentByForce(pWnd->GetHandle(), skinInfo.hWndToAttach);

	if (bLoadSkinColorTheme && skinInfo.bClrThemeValid)
	{
		TCHAR		szColor[128];

		ColorToStr(skinInfo.clrFontHighlight, szColor);
		CMPlayerSettings::SetLyricsDisplaySettings(_T("FgColor"), szColor);

		ColorToStr(skinInfo.clrFontLowlight, szColor);
		CMPlayerSettings::SetLyricsDisplaySettings(_T("FgLowColor"), szColor);

		ColorToStr(skinInfo.clrBackground, szColor);
		CMPlayerSettings::SetLyricsDisplaySettings(_T("BgColor"), szColor);

		ColorToStr(skinInfo.clrCurLineBg, szColor);
		CMPlayerSettings::SetLyricsDisplaySettings(_T("FocusLineBgColor"), szColor);
	}

	HWND		hWndParent = ::GetParent(pWnd->GetHandle());
	if (hWndParent)
	{
		pWnd->m_WndDrag.EnableDrag(false);
		pWnd->m_wndResizer.FixedWidth(true);
		pWnd->m_wndResizer.FixedHeight(true);

		if (pWnd->IsIconic())
			pWnd->ShowWindow(SW_RESTORE);

		CRect	rc;
		GetClientRect(skinInfo.hWndToAttach, &rc);
		::MoveWindow(pWnd->GetHandle(), 0, 0, rc.Width(), rc.Height(), false);
	}

	GetMainWnd()->GetTrayIcon().ForceShow(true);

	return ERR_OK;
}

UINT PleditGetPrivateProfileInt(LPCTSTR lpAppName, LPCTSTR lpKeyName, INT nDefault, LPCTSTR lpFileName)
{
	TCHAR	szBuffer[256];

	if (GetPrivateProfileString(lpAppName, lpKeyName, _T(""), szBuffer, 256, lpFileName) == 0)
		return nDefault;

	return GetColorValue(szBuffer, nDefault);
}

int CMPlayerApp::LoadWinamp2Skin(bool bLoadSkinColorTheme)
{
	//
	// 使用 Winamp2 的配套SKIN
	//
	// 主要是设置MiniLyrics的资源搜索路径：
	//   先搜索winamp skin目录中是否有skin，
	//   再搜索迷你歌词中对应的ml-winamp2-skin目录
	TCHAR	szWaSkinPath[MAX_PATH] = _T("");
	tstring	strSkinDir;
	int		nRet = ERR_OK;

	// 取得winamp的skin目录
	g_Player.GetCurSkinPath(szWaSkinPath, MAX_PATH);
	if (IsEmptyString(szWaSkinPath))
	{
		// 使用 minilyrics 的基本skin
		return m_pSkinFactory->ChangeSkin(SZ_DEFSKIN, _T(""), _T(""), true);
	}
	else
	{
		TCHAR	szTestFile[MAX_PATH];

		DirStringFormat(szWaSkinPath);

		_tcscpy_safe(szTestFile, CountOf(szTestFile), szWaSkinPath);
		_tcscat_safe(szTestFile, CountOf(szTestFile), _T("titlebar.bmp"));
		if (!IsFileExist(szTestFile))
		{
			return m_pSkinFactory->ChangeSkin(SZ_DEFSKIN, _T(""), _T(""), true);
		}
		else
		{
			// 取得缺省的skin的目录
			strSkinDir = GetWorkingFolder();
			strSkinDir += _T("ml-winamp2-skin\\");
#ifdef _DEBUG
			VerySkinDirOK(strSkinDir, _T("ml-winamp2-skin\\"));
#endif

			nRet = m_pSkinFactory->ChangeSkin(_T("ml-winamp2-skin"), strSkinDir.c_str(), szWaSkinPath);
			if (nRet != ERR_OK)
				return nRet;
		}
	}

	if (bLoadSkinColorTheme)
	{
		//
		// WINAMP 的skin的配色方案
		//
		TCHAR	szPledit[MAX_PATH];
		COLORREF	clrHigh = 0xFF000000, clrLow = 0xFF000000, clrBg = 0xFF000000, clrFocusLineBg = 0xFF000000;
		TCHAR		szColor[128];


		if (m_pSkinFactory->GetResourceMgr()->GetResourcePathName(szPledit, MAX_PATH, _T("pledit.txt")))
		{
			clrHigh = PleditGetPrivateProfileInt(_T("Text"), _T("Current"), 
				clrHigh, szPledit);

			clrLow = PleditGetPrivateProfileInt(_T("Text"), _T("Normal"), 
				clrLow, szPledit);

			clrBg = PleditGetPrivateProfileInt(_T("Text"), _T("NormalBG"), 
				clrBg, szPledit);

			clrFocusLineBg = PleditGetPrivateProfileInt(_T("Text"), _T("SelectedBG"), 
				clrFocusLineBg, szPledit);

			if (clrHigh != 0xFF000000)
			{
				ColorToStr(clrHigh, szColor);
				CMPlayerSettings::SetLyricsDisplaySettings(_T("FgColor"), szColor);
			}

			if (clrLow != 0xFF000000)
			{
				ColorToStr(clrLow, szColor);
				CMPlayerSettings::SetLyricsDisplaySettings(_T("FgLowColor"), szColor);
			}

			if (clrBg != 0xFF000000)
			{
				ColorToStr(clrBg, szColor);
				CMPlayerSettings::SetLyricsDisplaySettings(_T("BgColor"), szColor);
			}

			if (clrFocusLineBg != 0xFF000000)
			{
				ColorToStr(clrFocusLineBg, szColor);
				CMPlayerSettings::SetLyricsDisplaySettings(_T("FocusLineBgColor"), szColor);
			}
		}
	}

	return nRet;
}

void CMPlayerApp::RestartToAppMode(AppMode appMode)
{
	m_appModeQuitToStart = appMode;

	PostQuitMessage();
}

#endif
