 /********************************************************************
	Created  :	2002/02/21	2:11
	FileName :	DownloadMgr.cpp
	Author   :	xhy
	
	Purpose  :	下载任务的管理类
*********************************************************************/

#include "stdafx.h"
#include "MPlayerApp.h"
#include "DownloadMgr.h"
#include "../MLProtocol/HttpClient.h"
#include "OnlineSearch.h"
#include "AutoProcessEmbeddedLyrics.h"

#define SZ_USER_AGENT_IE5 _T("Mozilla/4.0 (compatible; MSIE 5.00; Windows 98)")

CDownloadMgr::CDownloadMgr() : m_eventShutDown(true, false)
{
	m_bAutoDownload = true;
}


CDownloadMgr::~CDownloadMgr()
{

}


int CDownloadMgr::Init()
{
	m_eventShutDown.Reset();

	g_nPortDownloadServer = g_Profile.GetProfileInt(_T("PortDownloadServer"), 80);
	g_nPortSearchServer = g_Profile.GetProfileInt(_T("PortSearchServer"), 80);

#ifdef _WIN32_DESKTOP
	if (g_Profile.GetProfileInt(_T("ProxyType"), 0xFF) == 0xFF)
	{
		// 第一次运行，自动加载代理设置
		bool		bUseProxy;
		tstring		strSvr;
		int			nPort;

		if (LoadProxySvrFromIE(bUseProxy, strSvr, nPort))
		{
			if (bUseProxy)
				g_Profile.WriteProfileInt(_T("ProxyType"), HTTP_PROXY_OURS);
			else
				g_Profile.WriteProfileInt(_T("ProxyType"), HTTP_PROXY_NONE);
			g_Profile.WriteProfileString(_T("ProxyServer"), strSvr.c_str());
			g_Profile.WriteProfileInt(_T("ProxyPort"), nPort);
		}
		else
			g_Profile.WriteProfileInt(_T("ProxyType"), HTTP_PROXY_NONE);
	}
#endif

	// 缺省下载路径
	m_strDefSavePath = CMLProfile::GetDir(SZ_SECT_LYR_DL, _T("LyricsDownPath"), _T(""));
	if (m_strDefSavePath.empty())
		CreateDefaultLyricsDir(m_strDefSavePath);
	
	DBGLOG1(_T("Lyrics download dir: %s"), m_strDefSavePath.c_str());

	// 自动下载
	m_bAutoDownload = g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("EnableAuoDownload"), true);

	return ERR_OK;
}


void CDownloadMgr::Quit()
{
	g_Profile.WriteProfileInt(_T("PortDownloadServer"), g_nPortDownloadServer);
	g_Profile.GetProfileInt(_T("PortSearchServer"), g_nPortSearchServer);

	m_eventShutDown.Set();
	m_threadDownload.Join();

	m_mutexAccess.Acquire();
	for (LIST_TASK::iterator it = m_listTasks.begin(); it != m_listTasks.end(); ++it)
	{
		CDownloadTask	*pTask = *it;
		delete pTask;
	}
	m_listTasks.clear();
	m_mutexAccess.Release();
}

void CDownloadMgr::OnSongChanged()
{
	if (!m_bAutoDownload || g_LyricData.HasLyricsOpened())
		return;

	if (!SearchInCacheResult(false))
		g_OnlineSearch.AutoSearch(g_Player.GetArtist(), g_Player.GetTitle());
}

BOOL CDownloadMgr::SearchInCacheResult(bool bShowInfoText)
{
	V_LRCSEARCHRESULT		vLrcSearchResult;

	if (!g_OnlineSearch.SearchCacheForCur(&vLrcSearchResult))
	{
		static	int nFailedCount = 0;
		nFailedCount++;
		if (bShowInfoText && nFailedCount > 2)
		{
			tstring		str;
			str = _TLT("Failed to get searching lyrics results.");
			str += _T(" ");
			str += _TLT("Please contact us to report this issue.");
			CMPlayerAppBase::GetInstance()->DispatchLongErrorText(str.c_str());
		}
		return FALSE;
	}

	if (vLrcSearchResult.size() == 0)
	{
		if (bShowInfoText)
		{
			tstring		str;
			str = _TLT("Search returned no results.");
			CMPlayerAppBase::GetInstance()->DispatchLongErrorText(str.c_str());
		}
		return FALSE;
	}

	float	nValueMax = 0;
	BOOL	bUserSelDown;
	V_LRCSEARCHRESULT::iterator itBest;

	bUserSelDown = g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("DownLrcUserSelect"), false);

	//
	// 分值超过 MATCH_VALUE_OK
	itBest = vLrcSearchResult.GetTheBestMatchLyrics(FALSE);
	if (itBest != vLrcSearchResult.end())
	{
		LrcSearchResult		&result = *itBest;
		nValueMax = result.nMatchValue;
		if (nValueMax >= MATCH_VALUE_OK)
		{
			// Do not download .txt lyrics
			if (FileIsExtSame(result.strUrl.c_str(), _T(".txt"))
				&& g_Profile.GetProfileInt(SZ_SECT_LYR_DL, _T("OnlyDlSyncLyr"), false))
			{
				CMPlayerAppBase::GetInstance()->DispatchLongErrorText(_TLT("None of the lyrics are .lrc extension lyrics, so they are not downloaded."));
				return TRUE;
			}

			// If current lyrics is text lyrics, and the match lyrics is also text
			// lyrics, do NOT download it.
			if (g_LyricData.GetLyrContentType() != LCT_UNKNOWN
				&& result.nMatchValue < MATCH_VALUE_BETTER)
				return TRUE;

			// 就下载它了
			tstring		str;
			str = _TLT("Downloading lyrics:");
			str += _T(" ");
			str += result.strSaveFileName.c_str();
			CMPlayerAppBase::GetInstance()->DispatchLongErrorText(str.c_str());

			DownloadLyrics(CMPlayerAppBase::GetAssociateFileKeyword().c_str(),
				g_Player.GetSrcMedia(), result.strUrl.c_str(), result.strSaveFileName.c_str());
			return TRUE;
		}
	}

	// If any lyrics is opened, do not continue to download.
	if (g_LyricData.HasLyricsOpened())
		return true;

	// 由用户选择？
	if (bUserSelDown)
	{
		// if all lyrics is txt, return directly.
		if (g_Profile.GetProfileInt(SZ_SECT_LYR_DL, _T("OnlyDlSyncLyr"), false)
			&& vLrcSearchResult.IsAllTxtLyrics())
		{
			CMPlayerAppBase::GetInstance()->DispatchLongErrorText(_TLT("None of the lyrics are .lrc extension lyrics, so they are not downloaded."));
			return true;
		}

		if (bShowInfoText)
		{
			CMPlayerAppBase::GetInstance()->DispatchLongErrorText(
				_TLT("Found lyrics, please select the lyrics in the popup dialog."));
		}

		CMPlayerAppBase::GetMainWnd()->OnCustomCommand(CMD_OPEN_LRC);
		return TRUE;
	}
	else
	{
		if (bShowInfoText)
		{
			CMPlayerAppBase::GetInstance()->DispatchLongErrorText(
				_TLT("Found some similar lyrics, please choose them in 'Search Lyrics' window."));
		}
	}

	return TRUE;
}

void CDownloadMgr::DownloadLyrics(LPCTSTR szMediaKey, LPCTSTR szSongFile, LPCTSTR szUrl, LPCTSTR szFileName)
{
	CDownloadTask *pTask = new CDownloadTask(DTT_LYRICS, szUrl);
	pTask->strMediaKey = szMediaKey;
	pTask->m_strLyrFileName = szFileName;

	AddTask(pTask);
}

void CDownloadMgr::DownloadVersionFile(LPCTSTR szUrl, bool bNoUI)
{
	CDownloadTask *pTask = new CDownloadTask(bNoUI ? DTT_CHECK_VERSION_NOUI : DTT_CHECK_VERSION, szUrl);
	AddTask(pTask);
}

void CDownloadMgr::AddTask(CDownloadTask *pTask)
{
	if (IsTaskExist(pTask->m_strURL.c_str()))
	{
		delete pTask;
		return;
	}

	m_mutexAccess.Acquire();
	m_listTasks.push_back(pTask);

	if (!m_threadDownload.IsRunning())
	{
		if (!m_threadDownload.Create(DownloadThreadProc, this))
		{
			ERRLOG0(_T("FAILED to create downloading thread."));
		}
	}
	m_mutexAccess.Release();
}

bool CDownloadMgr::IsTaskExist(LPCTSTR szURL)
{
	CMutexAutolock	lock(m_mutexAccess);

	LIST_TASK::iterator	it;

	for (it = m_listTasks.begin(); it != m_listTasks.end(); ++it)
	{
		CDownloadTask	*pTask = *it;
		if (_tcsicmp(pTask->m_strURL.c_str(), szURL) == 0)
			return true;
	}

	for (it = m_listRunningTasks.begin(); it != m_listRunningTasks.end(); ++it)
	{
		CDownloadTask	*pTask = *it;
		if (_tcsicmp(pTask->m_strURL.c_str(), szURL) == 0)
			return true;
	}

	return false;
}

void CDownloadMgr::CreateDefaultLyricsDir(tstring &strDefSavePath)
{
#ifdef _WIN32_DESKTOP
	// Retrieve the root Dir.
	strDefSavePath = GetWorkingFolder();
	strDefSavePath.resize(3);
	if (!CanWriteInDir(strDefSavePath.c_str()))
	{
		TCHAR		szDir[MAX_PATH];

		// Try to save in windows drive
		GetWindowsDirectory(szDir, CountOf(szDir));
		strDefSavePath = szDir;
		strDefSavePath.resize(3);
		if (!CanWriteInDir(strDefSavePath.c_str()))
			strDefSavePath = GetAppDataDir();
	}
#else
	strDefSavePath = GetAppDataDir();
#endif
	strDefSavePath += _T("Lyrics");
	DirStringAddSlash(strDefSavePath);

	if (!IsDirExist(strDefSavePath.c_str()))
		CreateDirectoryAll(strDefSavePath.c_str());

	if (IsEmptyString(g_Profile.GetProfileString(SZ_SECT_LYR_DL, _T("LyricsDownPath"), _T(""))))
		CMLProfile::WriteDir(SZ_SECT_LYR_DL, _T("LyricsDownPath"), strDefSavePath.c_str());
}

tstring CDownloadMgr::GetSaveLyricsFile(LPCTSTR szSongFile, LPCTSTR szFileName)
{
	DOWN_SAVE_DIR		DownSaveDir;
	DOWN_SAVE_NAME		DownSaveName;

	DownSaveDir = (DOWN_SAVE_DIR)g_Profile.GetProfileInt(SZ_SECT_LYR_DL, _T("DownSaveInSongDir"), DOWN_SAVE_IN_CUSTOM_DIR);
	DownSaveName = (DOWN_SAVE_NAME)g_Profile.GetProfileInt(SZ_SECT_LYR_DL, _T("DownSaveName"), DOWN_SAVE_NAME_KEEP);

	return GetSaveLyricsFile(szSongFile, szFileName, DownSaveDir, DownSaveName);
}

tstring CDownloadMgr::GetSaveLyricsFile(LPCTSTR szSongFile, LPCTSTR szFileName, DOWN_SAVE_DIR DownSaveDir, DOWN_SAVE_NAME DownSaveName)
{
	szFileName = FileGetName(szFileName);

	tstring				strLyricsFile;
	if (DownSaveDir == DOWN_SAVE_IN_SONG_DIR)
		strLyricsFile = FileGetPath(szSongFile);

	if (strLyricsFile.empty())
		strLyricsFile = m_strDefSavePath.c_str();

	DirStringAddSlash(strLyricsFile);

	// Save lyrics in sub folder by ABC order?
	if (DownSaveDir != DOWN_SAVE_IN_SONG_DIR 
		&& g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("DownSaveByABC"), false))
	{
		TCHAR			c = *szFileName;
		if (IsAlpha(c))
			strLyricsFile += toupper(c);
		else if (c >= '0' && c <='9')
			strLyricsFile += '#';

		DirStringAddSlash(strLyricsFile);
	}

	TCHAR				szName[MAX_PATH] = _T("");
	if (DownSaveName == DOWN_SAVE_NAME_AS_SONG_NAME)
	{
		_tcscpy_safe(szName, CountOf(szName), UrlGetTitle(szSongFile).c_str());
		if (!IsEmptyString(szName))
			_tcscat_safe(szName, CountOf(szName), FileGetExt(szFileName));
	}
	if (IsEmptyString(szName) && szFileName)
		_tcscpy_safe(szName, CountOf(szName), szFileName);

	strLyricsFile += szName;

	return strLyricsFile;
}

void CDownloadMgr::OnEndDownload(CDownloadTask *pTask)
{
	if (pTask->m_errResult != ERR_OK)
	{
		if (pTask->taskType == DTT_LYRICS)
		{
			CEventDownloadEnd	*pEvent = new CEventDownloadEnd;
			pEvent->eventType = ET_DOWNLOAD_END;
			pEvent->pTask = pTask;
			pEvent->downloadType = CEventDownloadEnd::DT_DL_LRC_FAILED;
			CMPlayerAppBase::GetEventsDispatcher()->DispatchSyncEventByNoUIThread(pEvent);
		}
		else if (pTask->taskType == DTT_CHECK_VERSION_NOUI
			|| pTask->taskType == DTT_CHECK_VERSION)
		{
			CEventDownloadEnd		*pEvent = new CEventDownloadEnd;
			pEvent->eventType = ET_DOWNLOAD_END;
			pEvent->pTask = pTask;
			pEvent->downloadType = CEventDownloadEnd::DT_DL_CHECK_NEW_VERSION_OK;
			CMPlayerAppBase::GetEventsDispatcher()->DispatchSyncEventByNoUIThread(pEvent);
		}
		return;
	}

	switch (pTask->taskType)
	{
	case DTT_LYRICS:
		//
		// 本地链接下载任务完成时的处理
		//
		{
			if (SaveDownloadedLyrics(pTask->strMediaKey.c_str(), pTask->m_strLyrFileName.c_str(), 
				pTask->buffContent.c_str(), pTask->buffContent.size()) == ERR_OK)
				CMPlayerAppBase::GetInstance()->DispatchResearchLyrics();

			CSkinApp::GetInstance()->GetEventPatcher()->DispatchUnsyncEvent(ET_UI_ACTIVATE_NOTICE);
		}
		break;
	case DTT_CHECK_VERSION:
	case DTT_CHECK_VERSION_NOUI:
		{
			CEventDownloadEnd		*pEvent = new CEventDownloadEnd;
			pEvent->eventType = ET_DOWNLOAD_END;
			pEvent->pTask = pTask;
			pEvent->downloadType = CEventDownloadEnd::DT_DL_CHECK_NEW_VERSION_OK;
			CMPlayerAppBase::GetEventsDispatcher()->DispatchSyncEventByNoUIThread(pEvent);
		}
		break;
	default:
		break;
	}
}

void CDownloadMgr::DownloadThreadProc(LPVOID lpParam)
{
	CDownloadMgr	*pThis = (CDownloadMgr *)lpParam;

	pThis->DownloadThread();
}


void CDownloadMgr::DownloadThread()
{
	while (!IsQuiting())
	{
		CDownloadTask	*pTask = NULL;

		// Pick up one task
		m_mutexAccess.Acquire();
		if (m_listTasks.empty())
		{
			m_mutexAccess.Release();
			return;
		}
		pTask = m_listTasks.front();
		m_listTasks.pop_front();
		m_listRunningTasks.push_back(pTask);
		m_mutexAccess.Release();

		if (IsQuiting())
			return;

		if (_tcsnicmp(pTask->m_strURL.c_str(), _T("http://"), 7) == 0)
		{
			RunHttpTask(pTask);
		}

		if (IsQuiting())
			return;

		OnEndDownload(pTask);

		// Remove task from queue
		m_mutexAccess.Acquire();
		for (LIST_TASK::iterator it = m_listRunningTasks.begin(); it != m_listRunningTasks.end(); ++it)
		{
			if (*it == pTask)
			{
				delete pTask;
				m_listRunningTasks.erase(it);
				break;
			}
		}
		m_mutexAccess.Release();
	}
}

int CDownloadMgr::TryToConnect(CHttpClient &httpClient)
{
	int		nRet;

	// Latest port
	nRet = httpClient.Connect();
	if (nRet == ERR_OK)
		return nRet;

	if (IsQuiting())
		return ERR_FALSE;

	// 80
	if (g_nPortDownloadServer != 80)
	{
		httpClient.SetHostPort(80);
		nRet = httpClient.Connect();
		if (nRet == ERR_OK)
		{
			g_nPortDownloadServer = 80;
			return ERR_OK;
		}
	}

	// 8000 ~ 8080
	if (g_nPortDownloadServer < SERVER_PORT_BEGIN)
		g_nPortDownloadServer = SERVER_PORT_BEGIN - 1;
	for (int i = SERVER_PORT_BEGIN; i <= SERVER_PORT_END; i++)
	{
		if (IsQuiting())
			return ERR_FALSE;

		g_nPortDownloadServer++;
		if (g_nPortDownloadServer > SERVER_PORT_END)
			g_nPortDownloadServer = SERVER_PORT_BEGIN;
		httpClient.SetHostPort(g_nPortDownloadServer);
		nRet = httpClient.Connect();
		if (nRet == ERR_OK)
			break;
	}

	return nRet;
}

int CDownloadMgr::RunHttpTask(CDownloadTask *pTask)
{
	CHttpClient			httpClient;
	int					nHttpCode = 0;
	CCharBuffer			&buffRet = pTask->buffContent;
	int					nRet;
	int					iRetry;

	int					nProxyPort = 80;
	TCHAR				szProxyServer[256];
	bool bUseProxy = CMLProfile::InetGetProxy(szProxyServer, nProxyPort);
	tstring strProxyPwd = CMLProfile::InetGetBase64ProxyUserPass();

	// LOG1(LOG_LVL_INFO, _T("Enter lyrics download thread: %s"), pTask->m_strURL.c_str());
	// LOG2(LOG_LVL_INFO, _T("Proxy settings: %d, %s"), bUseProxy, szProxyPwd);

	bool	bMLServer = false;

	httpClient.SetUserAgent(GetAppNameLong().c_str());
	nRet = httpClient.Init(pTask->m_strURL.c_str(), NULL);
	if (nRet != ERR_OK)
	{
		CStrPrintf	str(_T("Failed to Init http url: %s"),pTask->m_strURL.c_str());
		ERRLOG1(_T("%s"), str.c_str());
		nRet = ERR_HTTP_BAD_REQUEST;
		nHttpCode = 0;
		goto RETURN_END;
	}

	if (httpClient.GetHostPort() == 80)
	{
		bMLServer = true;
		httpClient.SetHostPort(g_nPortDownloadServer);
	}

	httpClient.SetProxy(bUseProxy, szProxyServer, nProxyPort, strProxyPwd.c_str());

	for (iRetry = 1; iRetry <= 2 && !IsQuiting(); iRetry++)
	{
		if (iRetry != 1)
		{
			//
			// Wait for 10 sec to try again
			if (pTask->taskType == DTT_LYRICS)
				CMPlayerAppBase::GetInstance()->DispatchLongErrorText(_TLT("Wait 10 seconds and retry."));

			if (m_eventShutDown.Acquire(10 * 1000))
				break;
		}

		if (bUseProxy || !bMLServer)
			nRet = httpClient.Connect();
		else
			nRet = TryToConnect(httpClient);
		if (nRet != ERR_OK)
		{
			tstring		str;
			str = _TLT("Failed to download lyrics with error:");
			str += _T("\r\n");
			str += (LPCTSTR)ERROR2STR_LOCAL(nRet);
			if (pTask->taskType == DTT_LYRICS)
				CMPlayerAppBase::GetInstance()->DispatchLongErrorText(str.c_str());
			DBGLOG1(_T("%s"), str.c_str());

			continue;
		}

		if (IsQuiting())
			break;

		nRet = httpClient.SendRequest();
		if (nRet != ERR_OK)
		{
			tstring		str;
			str = _TLT("Failed to download lyrics with error:");
			str += _T("\r\n");
			str += (LPCTSTR)ERROR2STR_LOCAL(nRet);
			if (pTask->taskType == DTT_LYRICS)
				CMPlayerAppBase::GetInstance()->DispatchLongErrorText(str.c_str());
			DBGLOG1(_T("%s"), str.c_str());
			httpClient.Close();
			continue;
		}

		if (IsQuiting())
			break;

		nRet = httpClient.GetRespond(nHttpCode, buffRet);
		if (nRet != ERR_OK)
		{
			tstring		str;
			str = _TLT("Failed to download lyrics with error:");
			str += _T("\r\n");
			str += (LPCTSTR)ERROR2STR_LOCAL(nRet);
			if (pTask->taskType == DTT_LYRICS)
				CMPlayerAppBase::GetInstance()->DispatchLongErrorText(str.c_str());
			DBGLOG1(_T("%s"), str.c_str());
			httpClient.Close();
			if (nHttpCode != 0)
				break;
			continue;
		}

		httpClient.Close();

		break;
	}

RETURN_END:
	pTask->m_nHttpRetCode = nHttpCode;
	pTask->m_errResult = nRet;
	if (!(nRet == ERR_OK && nHttpCode == 200))
		DBGLOG3(_T("Downloaded Failed, Error: %d, httpcode: %d, %s"), nRet, nHttpCode, pTask->m_strURL.c_str());

	CMPlayerApp::GetAppAnalytics().IncreaseNumberOfResult(SZ_UD_DOWNLOAD_LYRICS_DATA, nRet);

	return ERR_OK;
}

int CDownloadMgr::SaveDownloadedLyrics(LPCTSTR szMediaFile, LPCTSTR szLyrFileName, LPCVOID lpData, int nSize, uint32 nLyrSaveFlag)
{
	ASSERT(lpData && nSize > 0 && nSize < 1024 * 200);
	VecString vEmbeddedLyrNames;

	if (nLyrSaveFlag == LST_NONE)
	{
		// Load default settings
		if (g_Profile.GetProfileInt(SZ_SECT_LYR_DL, _T("DownSaveInSongDir"), DOWN_SAVE_IN_CUSTOM_DIR)
			!= DOWN_SAVE_NO_FILE)
			nLyrSaveFlag = LST_FILE;

		// Save lyrics in song file?
		if (g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("DownSaveEmbeded"), false))
		{
			if (g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("DownSaveId3v2Sylt"), true))
				vEmbeddedLyrNames.push_back(SZ_SONG_ID3V2_SYLT);
			if (g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("DownSaveId3v2Uslt"), true))
				vEmbeddedLyrNames.push_back(SZ_SONG_ID3V2_USLT);
			if (g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("DownSaveLyrics3v2"), false))
				vEmbeddedLyrNames.push_back(SZ_SONG_LYRICS3V2);
			if (g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("DownSaveLyricsM4a"), false))
				vEmbeddedLyrNames.push_back(SZ_SONG_M4A_LYRICS);
		}
	}

	bool	bLrcSaved = false;
	int		nRet = ERR_OK;

	if (vEmbeddedLyrNames.size() > 0
		&& CTagLib::IsEmbeddedLyricsSupported(szMediaFile))
	{
		CCharBuffer bufLyrics;
		bufLyrics.append((LPCSTR)lpData, nSize);
		nRet = g_autoProcessEmbeddedLyrics.SaveEmbeddedLyrics(szMediaFile, NULL, &bufLyrics, vEmbeddedLyrNames);
		if (nRet == ERR_OK)
			bLrcSaved = true;
	}

	if (nLyrSaveFlag & LST_FILE || !bLrcSaved)
	{
		tstring strLyrFile = GetSaveLyricsFile(szMediaFile, szLyrFileName);
		DBGLOG1(_T("Save downloaded lyrics: %s"), strLyrFile.c_str());
		nRet = SaveDownloadedLyrAsFile(strLyrFile, lpData, nSize);
		if (nRet == ERR_OK)
			g_LyricSearch.AssociateLyrics(szMediaFile, strLyrFile.c_str());
	}

	return nRet;
}

int CDownloadMgr::SaveDownloadedLyrAsFile(tstring &strFile, LPCVOID lpData, int nSize)
{
	if (IsFileExist(strFile.c_str()))
	{
		SetFileNoReadOnly(strFile.c_str());
	}

	strFile = FileNameFilterInvalidChars(strFile.c_str());

	if (!SaveDataAsFile(strFile.c_str(), lpData, nSize))
	{
		tstring strDir = FileGetPath(strFile.c_str());
		CreateDirectoryAll(strDir.c_str());

		if (!SaveDataAsFile(strFile.c_str(), lpData, nSize))
		{
			if (m_strDefSavePath.empty() || !CanWriteInDir(m_strDefSavePath.c_str()))
				CreateDefaultLyricsDir(m_strDefSavePath);

			// save in the default directory
			tstring	strSaveFile = m_strDefSavePath;
			DirStringFormat(strSaveFile);
			strSaveFile += FileGetName(strFile.c_str());
			strFile = strSaveFile;

			strDir = FileGetPath(strFile.c_str());
			CreateDirectoryAll(strDir.c_str());
			if (!SaveDataAsFile(strFile.c_str(), lpData, nSize))
			{
				ERRLOG1(_T("Failed to save downloaded lyrics file: %s."), strFile.c_str());
				return ERR_OPEN_FILE;
			}
		}
	}

	return ERR_OK;
}
