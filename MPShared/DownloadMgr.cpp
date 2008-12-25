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
	_tcsncpy(szHostBuff, szHostBeg, nLen);
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
	m_dwTaskIDNext = 0;
	m_nRetryCount = 2;
	m_bQuit = false;
	m_strUserAgent = SZ_USER_AGENT_IE5;
}


CDownloadMgr::~CDownloadMgr()
{

}


int CDownloadMgr::Init()
{
	m_bQuit = false;
	m_eventShutDown.Reset();

	return ERR_OK;
}


void CDownloadMgr::Quit()
{
	m_bQuit = true;

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


int CDownloadMgr::AddTask(LPCTSTR szURL, LPCTSTR szRefUrl, LPCTSTR szFile, DWORD dwDownOkID, DWORD *pdwTaskID, bool bReportErr, bool bHideTask)
{
	CDownloadTask		*pTask;

	if (IsTaskExist(szURL))
		return ERR_EXIST;

	pTask = new CDownloadTask;
	pTask->m_strURL = szURL;
	if (szRefUrl)
		pTask->m_strReferUrl = szRefUrl;
	pTask->m_strSaveFile = szFile;
	pTask->m_dwDownOkID = dwDownOkID;
	pTask->m_bReportErr = bReportErr;
	pTask->m_bHideTask = bHideTask;
	if (pdwTaskID)
	{
		pTask->m_dwID = ++m_dwTaskIDNext;
		*pdwTaskID = m_dwTaskIDNext;
	}

	m_mutexAccess.Acquire();
	m_listTasks.push_back(pTask);

	if (!m_threadDownload.IsRunning())
	{
		if (!m_threadDownload.Create(DownloadThreadProc, this))
			ERRLOG0(_T("FAILED to create downloading thread."));
	}
	m_mutexAccess.Release();

	return ERR_OK;
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


void CDownloadMgr::OnEndDownload(CDownloadTask *pTask)
{
}


void CDownloadMgr::DownloadThreadProc(LPVOID lpParam)
{
	ASSERT(lpParam);

	CDownloadMgr	*pThis = (CDownloadMgr *)lpParam;

	pThis->DownloadThread();
}


void CDownloadMgr::DownloadThread()
{
	while (!m_bQuit)
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

		if (m_bQuit)
			return;

		if (_tcsnicmp(pTask->m_strURL.c_str(), _T("http://"), 7) == 0)
		{
			nRet = RunHttpTask(pTask);
		}

		if (m_bQuit)
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


int CDownloadMgr::RunHttpTask(CDownloadTask *pTask)
{
	CHttpClient			httpClient;
	BOOL				bUseProxy = false;
	int					nProxyPort = 80;
	TCHAR				szProxyServer[256];
	LPCTSTR				szProxyPwd;

	int					nHttpCode = 0;
	CCharBuffer			buffRet;
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
		if (!pTask->m_bHideTask)
			g_eventDispather.DispatchLongErrorText(str.c_str());
		ERRLOG1(_T("%s"), str.c_str());
		nRet = ERR_HTTP_BAD_REQUEST;
		nHttpCode = 0;
		goto RETURN_END;
	}

	httpClient.SetUserAgent(m_strUserAgent.c_str());
	nRet = httpClient.Init(szHost, szUri, pTask->m_strReferUrl.c_str(), nPort);

#ifdef _UNICODE
	ConvertStr(szProxyServer, -1, strProxyServer);
	ConvertStr(szProxyPwd, -1, strProxyPwd);
	httpClient.SetProxy(bUseProxy, strProxyServer.c_str(), nProxyPort, strProxyPwd.c_str());
#else
	httpClient.SetProxy(bUseProxy, szProxyServer, nProxyPort, szProxyPwd);
#endif

	if (m_nRetryCount <= 0)
		m_nRetryCount = 1;

	for (iRetry = 1; iRetry <= m_nRetryCount && !m_bQuit; iRetry++)
	{
		nRet = httpClient.Connect();
		if (nRet != ERR_OK)
		{
			tstring		str;
			str = TOLOCAL(_T("Download lyrics: connected to server FAILED."));
			str += _T("\r\n");
			str += TOLOCAL(_T("Error: "));
			str += (LPCTSTR)ERROR2STR_LOCAL(nRet);
			if (!pTask->m_bHideTask)
				g_eventDispather.DispatchLongErrorText(str.c_str());
			ERRLOG1(_T("%s"), str.c_str());
			continue;
		}

		if (m_bQuit)
			break;

		nRet = httpClient.SendRequest();
		if (nRet != ERR_OK)
		{
			tstring		str;
			str = _T("Download lyrics: sent data FAILED.\r\n");
			str += TOLOCAL(_T("Error: "));
			str += (LPCTSTR)ERROR2STR_LOCAL(nRet);
			if (!pTask->m_bHideTask)
				g_eventDispather.DispatchLongErrorText(str.c_str());
			ERRLOG1(_T("%s"), str.c_str());
			httpClient.Close();
			continue;
		}

		if (m_bQuit)
			break;

		nRet = httpClient.GetRespond(nHttpCode, buffRet);
		if (nRet != ERR_OK)
		{
			tstring		str;
			str = _T("Download lyrics: got response FAILED.\r\n");
			str += TOLOCAL(_T("Error: "));
			str += (LPCTSTR)ERROR2STR_LOCAL(nRet);
			if (!pTask->m_bHideTask)
				g_eventDispather.DispatchLongErrorText(str.c_str());
			ERRLOG1(_T("%s"), str.c_str());
			httpClient.Close();

			if (iRetry < m_nRetryCount)
			{
				//
				// Wait for 10 sec to try again
				if (!pTask->m_bHideTask)
					g_eventDispather.DispatchLongErrorText(TOLOCAL(_T("Wait 10 seconds and retry.")));

				if (m_eventShutDown.Acquire(10 * 1000))
					break;
			}

			continue;
		}

		if (m_bQuit)
			break;

		if (nHttpCode != 200 && !pTask->m_bHideTask)
		{
			tstringex	strBuff;

			ConvertStr(buffRet.c_str(), buffRet.size(), strBuff);
			FilterAllHTMLTags(strBuff.data());
			strBuff.resize(_tcslen(strBuff.c_str()));

			CStrPrintf	str(_T("Url: %s\r\nHttp %d error!\r\n%s"), pTask->m_strURL.c_str(), nHttpCode, strBuff.c_str());
			g_eventDispather.DispatchLongErrorText(str.c_str());
			ERRLOG1(_T("%s"), str.c_str());
		}

		httpClient.Close();

		break;
	}

RETURN_END:
	pTask->m_nHttpRetCode = nHttpCode;
	pTask->m_errResult = nRet;
	if (nRet == ERR_OK && nHttpCode == 200)
	{
		SaveDownloadedFile(pTask, buffRet.data(), buffRet.size());
	}
	else
	{
		DBGLOG3(_T("Downloaded Failed, Error: %d, httpcode: %d, %s"), nRet, nHttpCode, pTask->m_strURL.c_str());
	}

	return ERR_OK;
}


int CDownloadMgr::SaveDownloadedFile(CDownloadTask *pTask, LPVOID lpData, int nSize)
{
	if (IsFileExist(pTask->m_strSaveFile.c_str()))
	{
		SetFileNoReadOnly(pTask->m_strSaveFile.c_str());
	}

	if (!SaveDataAsFile(pTask->m_strSaveFile.c_str(), lpData, nSize))
	{
		TCHAR	szPath[MAX_PATH];

		FileGetPath(szPath, CountOf(szPath), pTask->m_strSaveFile.c_str());
		CreateDirectoryAll(szPath);

		if (!SaveDataAsFile(pTask->m_strSaveFile.c_str(), lpData, nSize))
		{
			// save in the default directory of MiniLyrics.
			pTask->m_strSaveFile = m_strDefSavePath;
			DirStringFormat(pTask->m_strSaveFile);
			pTask->m_strSaveFile += FileGetName(pTask->m_strSaveFile.c_str());

			FileGetPath(szPath, CountOf(szPath), pTask->m_strSaveFile.c_str());
			CreateDirectoryAll(szPath);
			SaveDataAsFile(pTask->m_strSaveFile.c_str(), lpData, nSize);
		}
	}

	return ERR_OK;
}
