// MPlayerApp.cpp: implementation of the CMPlayerApp class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../MPlayerApp.h"
#include "MPMsgWnd.h"
#include "DownloadMgr.h"
#include "VersionUpdate.h"
#include "OnlineSearch.h"
#include "AutoProcessEmbeddedLyrics.h"
#include "MPFloatingLyrWnd.h"
#include "MPlayerAppGtk2.h"

#ifdef _MPLAYER
int main(int argc, char *argv[])
{
	if (!CMPlayerAppBase::GetInstance()->Init(argc, argv))
		return 0;

	gtk_main();

	CMPlayerAppBase::GetInstance()->Quit();

	return 0;
}
#endif // #ifdef _MPLAYER


CMPlayerApp::CMPlayerApp()
{
}

CMPlayerApp::~CMPlayerApp()
{
}

bool CMPlayerApp::Init(int argc, char *argv[])
{

	TCHAR		szWorkingFolder[MAX_PATH];
	tstring		strMLIniFile, strMLLogFile;

	//
	// Init Base frame, save log and settings in app dir
#ifdef _WIN32_WCE
	strMLIniFile = _T("HKEY_LOCAL_MACHINE\\Software\\ZikiPlayer\\");
#else
	strMLIniFile = GetAppDataDir();
	strMLIniFile += SZ_PROFILE_NAME;
	SetFileNoReadOnly(strMLIniFile.c_str());
#endif

	if (!IsFileExist(strMLIniFile.c_str()))
		g_bEngEdition = true;

	g_Log.SetSrcRootDir(_T(__FILE__), 2);

	strMLLogFile = GetAppDataDir();
	strMLLogFile += _T("log.txt");
	InitBaseFrameWork(argc, argv, strMLLogFile.c_str(), strMLIniFile.c_str(), SZ_SECT_UI);

	MLWidgetInit(argc, argv);

	int nRet = CResourceMgr::Load("resource.xml");
	if (nRet != ERR_OK)
	{
		ERRLOG1("Load resouce Error: %s", (LPCTSTR)Error2Str(nRet));
		return false;
	}

#if (defined _WIN32_DESKTOP && defined _DEBUG_OUTPUT)
	DBGLOG1(_T("Working Folder: %s"), GetAppResourceDir());
	DBGLOG1(_T("App Data Folder: %s"), GetAppDataDir());
#endif

#ifndef _WIN32_WCE
	LOG1(LOG_LVL_EVENT_IMP, _T("Start %s"), GetAppNameLong().c_str());
#endif

	{
		//
		// 检查版本
		//
#ifdef _DEBUG
		CProfile	file;
		tstring		strEng;
		file.Init(_T("install.ini"), _T("INSTALL"));
		strEng = file.GetString(_T("Edition"), _T(""));
		if (_tcscmp(strEng.c_str(), _T("english")) == 0)
			g_bEngEdition = TRUE;
		else if (_tcscmp(strEng.c_str(), _T("chinese")) == 0)
			g_bEngEdition = FALSE;
		else 
#endif
		{
			// 根据 location 来设置中英文版本差别
		}
	}

	return CMPlayerAppBase::Init();
}

void CMPlayerApp::Quit()
{
	CMPlayerAppBase::Quit();


	gtk_main_quit();
}
