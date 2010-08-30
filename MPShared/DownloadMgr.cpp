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

void ShowRegisterNoticeSafely();

#define SZ_USER_AGENT_IE5 _T("Mozilla/4.0 (compatible; MSIE 5.00; Windows 98)")

//	COMMENT:
//		在URL中取得服务器的名字
//		URL的格式都是类似于:
//		http_URL	   = "http:" "//" host [ ":" port ] [ abs_path ]
//		http://127.0.0.1/README.TXT
BOOL AnalysisHttpUrl(const TCHAR szURL[], TCHAR szHostBuff[], int nHostBuff, int &nOutPort, TCHAR szAbsPath[], int nAbsPath)
{
	assert(szURL);
	assert(szHostBuff);
	assert(szAbsPath);

	const TCHAR	*szHostBeg, *szHostEnd1, *szHostEnd2;
	const TCHAR	*szAbsPathBeg = NULL;
	int 	nLen;
	const TCHAR		*szHttp = _T("HTTP://");

	if (_tcsnicmp(szURL, szHttp, _tcslen(szHttp)) != 0)
		return FALSE;

	// 找到主机名开始的位置
	szHostBeg = szURL + _tcslen(szHttp);

	// 找到下一个 / 开始的位置
	szAbsPathBeg = szHostEnd1 = _tcschr(szHostBeg, '/');

	if (szHostEnd1 == NULL) // 如果没有 / 则
		nLen = _tcslen(szURL) - int(szURL - szHostBeg);
	else
		nLen = szHostEnd1 - szHostBeg;

	nOutPort = __HTTP_PORT;

	// 找到下一个 : 开始的位置
	szHostEnd2 = _tcschr(szHostBeg, ':');
	if (szHostEnd2) // 如果有 : 则
	{
		if (nLen > szHostEnd2 - szHostBeg)
		{
			nLen = szHostEnd2 - szHostBeg;		// 长度
			nOutPort = _ttoi(szHostEnd2 + 1);	// 端口号
		}
	}

	// 复制服务器名
	_tcsncpy_safe(szHostBuff, nHostBuff, szHostBeg, nLen);
	szHostBuff[nLen] = '\0';

	// 服务器上的路径
	if (szAbsPathBeg == NULL)
		szAbsPath[0] = '\0';
	else
		_tcscpy_safe(szAbsPath, nAbsPath, szAbsPathBeg);

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////

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
	if (m_strDefSavePath.empty() || !CanWriteInDir(m_strDefSavePath.c_str()))
	{
		CreateDefaultLyricsDir(m_strDefSavePath);
	}

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

// COMMENT:
//		根据条件判断是否需要到歌词服务器上搜索新的歌词.
void CDownloadMgr::OnSongChanged()
{
	// LOG0(LOG_LVL_EVENT_IMP, _T("Enter auto download proc"));

	// 用户选择了自动下载?
	if (!m_bAutoDownload)
	{
		// LOG0(LOG_LVL_EVENT_IMP, _T("Auto download is disabled."));
		return;
	}

	//
	// 如果当前状态下打开有歌词，则不再下载
	//
	if (_tcslen(g_LyricData.GetLyricsFileName()) != 0)
	{
		// LOG1(LOG_LVL_EVENT_IMP, _T("Lyrics: %s was opened, quit auto download."), g_LyricData.GetLyricsFileName());
		return;
	}

	if (!SearchInCacheResult(false))
	{
		// 使用在线检索，搜索歌词
		g_OnlineSearch.AutoSearch(g_Player.GetArtist(), g_Player.GetTitle());
	}
	else
	{
		// LOG0(LOG_LVL_EVENT_IMP, _T("Lyrics in cached data were returned, quit auto search."));
	}
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
			str = TOLOCAL(_T("Failed to get searching lyrics results."));
			str += _T(" ");
			str += TOLOCAL(_T("Please contact us to report this issue."));
			g_eventDispather.DispatchLongErrorText(str.c_str());
		}
		return FALSE;
	}

	if (vLrcSearchResult.size() == 0)
	{
		if (bShowInfoText)
		{
			tstring		str;
			str = TOLOCAL(_T("Search returned no results."));
			g_eventDispather.DispatchLongErrorText(str.c_str());
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
			TCHAR		szAssociateKeyword[MAX_PATH] = _T("");

			// Do not download .txt lyrics
			if (FileIsExtSame(result.strUrl.c_str(), _T(".txt"))
				&& g_Profile.GetProfileInt(SZ_SECT_LYR_DL, _T("OnlyDlSyncLyr"), false))
			{
				g_eventDispather.DispatchLongErrorText(TOLOCAL(_T("None of the lyrics are .lrc extension lyrics, so they are not downloaded.")));
				return TRUE;
			}

			// If current lyrics is text lyrics, and the match lyrics is also text
			// lyrics, do NOT download it.
			if (g_LyricData.GetLyrContentType() != LCT_UNKNOWN
				&& result.nMatchValue < MATCH_VALUE_BETTER)
				return TRUE;

			g_mpApp.GetAssociateFileKeyword(szAssociateKeyword, MAX_PATH);

			// 就下载它了
			tstring		str;
			str = TOLOCAL(_T("Downloading lyrics:"));
			str += _T(" ");
			str += result.strSaveFileName.c_str();
			g_eventDispather.DispatchLongErrorText(str.c_str());

			DownloadLyrics(szAssociateKeyword, g_Player.GetSrcMedia(), result.strUrl.c_str(), result.strSaveFileName.c_str());
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
			g_eventDispather.DispatchLongErrorText(TOLOCAL(_T("None of the lyrics are .lrc extension lyrics, so they are not downloaded.")));
			return true;
		}

		if (bShowInfoText)
		{
			g_eventDispather.DispatchLongErrorText(
				TOLOCAL(_T("Found lyrics, please select the lyrics in the popup dialog.")));
		}

		g_mpApp.GetMainWnd()->OnCustomCommand(CMD_OPEN_LRC);
		return TRUE;
	}
	else
	{
		if (bShowInfoText)
		{
			g_eventDispather.DispatchLongErrorText(
				TOLOCAL(_T("Found some similar lyrics, please choose them in 'Search Lyrics' window.")));
		}
	}

	return TRUE;
}

void CDownloadMgr::DownloadLyrics(LPCTSTR szMediaKey, LPCTSTR szSongFile, LPCTSTR szUrl, LPCTSTR szFileName)
{
	CDownloadTask *pTask = new CDownloadTask(DTT_LYRICS, szUrl);
	pTask->strMediaKey = szMediaKey;
	pTask->m_strSaveFile = GetSaveLyricsFile(szSongFile, szFileName);

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

	int					nRet = ERR_OK;

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
	strDefSavePath += _T("Lyrics\\");

	if (!IsDirExist(strDefSavePath.c_str()))
		CreateDirectoryAll(strDefSavePath.c_str());

	if (IsEmptyString(g_Profile.GetProfileString(SZ_SECT_LYR_DL, _T("LyricsDownPath"), _T(""))))
		CMLProfile::WriteDir(SZ_SECT_LYR_DL, _T("LyricsDownPath"), strDefSavePath.c_str());
}

tstring CDownloadMgr::GetSaveLyricsFile(LPCTSTR szSongFile, LPCTSTR szFileName)
{
	DOWN_SAVE_DIR		DownSaveDir;
	DOWN_SAVE_NAME		DownSaveName;
	tstring				strLyricsFile;
	TCHAR				szName[MAX_PATH] = _T("");

	DownSaveDir = (DOWN_SAVE_DIR)g_Profile.GetProfileInt(SZ_SECT_LYR_DL, _T("DownSaveInSongDir"), DOWN_SAVE_IN_CUSTOM_DIR);
	if (DownSaveDir == DOWN_SAVE_IN_SONG_DIR)
	{
		FileGetPath(szSongFile, strLyricsFile);
	}

	if (strLyricsFile.empty())
		strLyricsFile = m_strDefSavePath.c_str();

	// Save lyrics in sub folder by ABC order?
	if (DownSaveDir != DOWN_SAVE_IN_SONG_DIR 
		&& g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("DownSaveByABC"), false))
	{
		TCHAR			c = *szFileName;
		if (c >= 'a' && c <='z' || c >= 'A' && c <='Z')
		{
			c = toupper(c);
			strLyricsFile += c;
			strLyricsFile += _T(SZ_DIR_SLASH);
		}
		else if (c >= '0' && c <='9')
		{
			strLyricsFile += '#';
			strLyricsFile += _T(SZ_DIR_SLASH);
		}
	}

	DownSaveName = (DOWN_SAVE_NAME)g_Profile.GetProfileInt(SZ_SECT_LYR_DL, _T("DownSaveName"), DOWN_SAVE_NAME_KEEP);
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
			g_eventDispather.DispatchSyncEventByNoUIThread(pEvent);
		}
		else if (pTask->taskType == DTT_CHECK_VERSION_NOUI
			|| pTask->taskType == DTT_CHECK_VERSION)
		{
			CEventDownloadEnd		*pEvent = new CEventDownloadEnd;
			pEvent->eventType = ET_DOWNLOAD_END;
			pEvent->pTask = pTask;
			pEvent->downloadType = CEventDownloadEnd::DT_DL_CHECK_NEW_VERSION_OK;
			g_eventDispather.DispatchSyncEventByNoUIThread(pEvent);
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
			map<DWORD, tstring>::iterator	it;
			BOOL			bSaveToMp3 = FALSE;

			// Get md5 of the lyrics, and remember it
			TCHAR		szMD5[64];
			if (MD5FileStr(pTask->m_strSaveFile.c_str(), szMD5))
				g_mpApp.SetCachedLyricsMD5(pTask->strMediaKey.c_str(), szMD5);

			if (g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("DownSaveEmbeded"), false)
				&& FileIsExtSame(pTask->strMediaKey.c_str(), _T(".mp3")))
			{
				UINT			uEmbeddedLyrics = 0;

				if (g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("DownSaveId3v2Sylt"), true))
					uEmbeddedLyrics |= LST_ID3V2_SYLT;
				if (g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("DownSaveId3v2Uslt"), true))
					uEmbeddedLyrics |= LST_ID3V2_USLT;
				if (g_Profile.GetProfileBool(SZ_SECT_LYR_DL, _T("DownSaveLyrics3v2"), false))
					uEmbeddedLyrics |= LST_LYRICS3V2;

				g_autoProcessEmbeddedLyrics.SaveEmbeddedLyrics(pTask->strMediaKey.c_str(), pTask->m_strSaveFile.c_str(), _T(""), uEmbeddedLyrics);
			}

			g_LyricSearch.AssociateLyrics(pTask->strMediaKey.c_str(), pTask->m_strSaveFile.c_str());

			g_mpApp.ResearchLyrics();

#ifdef _WIN32
			if (!IsRegistered())
			{
				static int	dwDownloadedCount = 0;
				static bool bShowRegNoticeNext = false;

				dwDownloadedCount++;
				if (dwDownloadedCount % 6 == 1 || bShowRegNoticeNext)
				{
					// Show registration dialog every xxx time.
					if (::GetForegroundWindow() == g_mpApp.GetMainWnd()->GetHandle())
					{
						ShowRegisterNoticeSafely();
						bShowRegNoticeNext = false;
					}
					else
						bShowRegNoticeNext = true;
				}
			}
#endif
		}
		break;
	case DTT_CHECK_VERSION:
	case DTT_CHECK_VERSION_NOUI:
		{
			CEventDownloadEnd		*pEvent = new CEventDownloadEnd;
			pEvent->eventType = ET_DOWNLOAD_END;
			pEvent->pTask = pTask;
			pEvent->downloadType = CEventDownloadEnd::DT_DL_CHECK_NEW_VERSION_OK;
			g_eventDispather.DispatchSyncEventByNoUIThread(pEvent);
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
		int				nRet = ERR_FALSE;

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
			nRet = RunHttpTask(pTask);
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
	BOOL				bUseProxy = false;
	int					nProxyPort = 80;
	TCHAR				szProxyServer[256];
	LPCTSTR				szProxyPwd;

	int					nHttpCode = 0;
	CCharBuffer			&buffRet = pTask->buffContent;
	int					nRet;
	TCHAR				szHost[256];
	TCHAR				szUri[512];
	int					nPort;
	int					iRetry;
	stringex			strProxyServer, strProxyPwd;

	bUseProxy = CMLProfile::InetGetProxy(szProxyServer, nProxyPort);
	szProxyPwd = CMLProfile::InetGetBase64ProxyUserPass();

	// LOG1(LOG_LVL_EVENT_IMP, _T("Enter lyrics download thread: %s"), pTask->m_strURL.c_str());
	// LOG2(LOG_LVL_EVENT_IMP, _T("Proxy settings: %d, %s"), bUseProxy, szProxyPwd);

	if (!AnalysisHttpUrl(pTask->m_strURL.c_str(), szHost, CountOf(szHost), nPort, szUri, CountOf(szUri)))
	{
		CStrPrintf	str(_T("Invalid http url: %s"),pTask->m_strURL.c_str());
		if (pTask->taskType == DTT_LYRICS)
			g_eventDispather.DispatchLongErrorText(str.c_str());
		ERRLOG1(_T("%s"), str.c_str());
		nRet = ERR_HTTP_BAD_REQUEST;
		nHttpCode = 0;
		goto RETURN_END;
	}
	bool	bMLServer = false;
	if (nPort == 80)
	{
		bMLServer = true;
		nPort = g_nPortDownloadServer;
	}

	httpClient.SetUserAgent(SZ_ML_VERSION);
	nRet = httpClient.Init(szHost, szUri, NULL, nPort);

#ifdef _UNICODE
	ConvertStr(szProxyServer, -1, strProxyServer);
	ConvertStr(szProxyPwd, -1, strProxyPwd);
	httpClient.SetProxy(bUseProxy, strProxyServer.c_str(), nProxyPort, strProxyPwd.c_str());
#else
	httpClient.SetProxy(bUseProxy, szProxyServer, nProxyPort, szProxyPwd);
#endif

	const int nRetryCount = 2;
	for (iRetry = 1; iRetry <= 2 && !IsQuiting(); iRetry++)
	{
		if (iRetry != 1)
		{
			//
			// Wait for 10 sec to try again
			if (pTask->taskType == DTT_LYRICS)
				g_eventDispather.DispatchLongErrorText(TOLOCAL(_T("Wait 10 seconds and retry.")));

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
			str = TOLOCAL(_T("Failed to download lyrics with error:"));
			str += _T("\r\n");
			str += (LPCTSTR)ERROR2STR_LOCAL(nRet);
			if (pTask->taskType == DTT_LYRICS)
				g_eventDispather.DispatchLongErrorText(str.c_str());
			DBGLOG1(_T("%s"), str.c_str());

			continue;
		}

		if (IsQuiting())
			break;

		nRet = httpClient.SendRequest();
		if (nRet != ERR_OK)
		{
			tstring		str;
			str = TOLOCAL(_T("Failed to download lyrics with error:"));
			str += _T("\r\n");
			str += (LPCTSTR)ERROR2STR_LOCAL(nRet);
			if (pTask->taskType == DTT_LYRICS)
				g_eventDispather.DispatchLongErrorText(str.c_str());
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
			str = TOLOCAL(_T("Failed to download lyrics with error:"));
			str += _T("\r\n");
			str += (LPCTSTR)ERROR2STR_LOCAL(nRet);
			if (pTask->taskType == DTT_LYRICS)
				g_eventDispather.DispatchLongErrorText(str.c_str());
			DBGLOG1(_T("%s"), str.c_str());
			httpClient.Close();
			continue;
		}

		if (IsQuiting())
			break;

		if (nHttpCode != 200 && pTask->taskType == DTT_LYRICS)
		{
			tstringex	strBuff;

			ConvertStr(buffRet.c_str(), buffRet.size(), strBuff);
			FilterAllHTMLTags(strBuff.data());
			strBuff.resize(_tcslen(strBuff.c_str()));

			CStrPrintf	str(_T("Url: %s\r\nHttp %d error!\r\n%s"), pTask->m_strURL.c_str(), nHttpCode, strBuff.c_str());
			g_eventDispather.DispatchLongErrorText(str.c_str());
			ERRLOG1(_T("%s"), str.c_str());
			break;
		}

		httpClient.Close();

		break;
	}

RETURN_END:
	pTask->m_nHttpRetCode = nHttpCode;
	pTask->m_errResult = nRet;
	if (nRet == ERR_OK && nHttpCode == 200)
	{
		if (pTask->taskType == DTT_LYRICS)
			SaveDownloadedFile(pTask->m_strSaveFile, buffRet.data(), buffRet.size());
	}
	else
	{
		DBGLOG3(_T("Downloaded Failed, Error: %d, httpcode: %d, %s"), nRet, nHttpCode, pTask->m_strURL.c_str());
	}

	return ERR_OK;
}


int CDownloadMgr::SaveDownloadedFile(tstring &strFile, LPCVOID lpData, int nSize)
{
	if (IsFileExist(strFile.c_str()))
	{
		SetFileNoReadOnly(strFile.c_str());
	}

	if (!SaveDataAsFile(strFile.c_str(), lpData, nSize))
	{
		TCHAR	szPath[MAX_PATH];

		FileGetPath(szPath, CountOf(szPath), strFile.c_str());
		CreateDirectoryAll(szPath);

		if (!SaveDataAsFile(strFile.c_str(), lpData, nSize))
		{
			// save in the default directory
			tstring	strSaveFile = m_strDefSavePath;
			DirStringFormat(strSaveFile);
			strSaveFile += FileGetName(strFile.c_str());
			strFile = strSaveFile;

			FileGetPath(szPath, CountOf(szPath), strFile.c_str());
			CreateDirectoryAll(szPath);
			if (!SaveDataAsFile(strFile.c_str(), lpData, nSize))
			{
				ERRLOG1(_T("Failed to save downloaded lyrics file: %s."), strFile.c_str());
				return ERR_OPEN_FILE;
			}
		}
	}

	return ERR_OK;
}
