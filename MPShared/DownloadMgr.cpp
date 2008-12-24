 /********************************************************************
	Created  :	2002/02/21	2:11
	FileName :	DownloadMgr.cpp
	Author   :	xhy
	
	Purpose  :	下载任务的管理类
*********************************************************************/

#include "stdafx.h"
#include "MPlayerApp.h"
#include "DownloadMgr.h"
#include "HttpDownImp2.h"


#define SZ_USER_AGENT_IE5 _T("Mozilla/4.0 (compatible; MSIE 5.00; Windows 98)")

// COMMENT:
//		取得不存在的的文件名，如果存在则增加序号
//		EXP:
//			Test.txt	->     Test(1).txt     -> Test(2).txt
//			Test		->     Test(1)         -> Test(2)
//			C:\a.dir\b  ->     C:\a.dir\b(1)   -> C:\a.dir\b(2)
void GetNotExistFileName(const TCHAR *szFileName, TCHAR *szFileNew, int nMaxLen)
{
	TCHAR	szBeforeExt[MAX_PATH];
	LPCTSTR	szExt;
	int		i = 1;

	if (!IsFileExist(szFileName))
	{
		_tcscpy(szFileNew, szFileName);
		return;
	}

	szExt = FileGetExt(szFileName);

	_tcsncpysz_safe(szBeforeExt, MAX_PATH, szFileName, _tcslen(szFileName) - _tcslen(szExt));

	while (1)
	{
		_stprintf(szFileNew, _T("%s(%d)%s"), szBeforeExt, i, szExt);
		i++;
		
		if (!IsFileExist(szFileNew))
			break;
	}
}

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


	// 取得端口号
	//	LPSERVENT lpServEnt;
	//	lpServEnt = getservbyname("http", "tcp");
	//	if (lpServEnt == NULL)
	nOutPort = __HTTP_PORT;
	//	else
	//		nOutPort = ntohs(lpServEnt->s_port);

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

CDownloadMgr::CDownloadMgr()
{
	m_dwTaskIDNext = DOWN_TASK_BASE_ID;
	m_nRetryCount = 2;
}


CDownloadMgr::~CDownloadMgr()
{

}


int CDownloadMgr::AddTask(LPCTSTR szURL, LPCTSTR szRefUrl, LPCTSTR szFile, DWORD dwDownOkID, bool bReportErr, bool bHideTask)
{
	Task		task;

	if (IsTaskExist(szURL))
		return ERR_OK;

	task.m_strURL = szURL;
	task.m_strReferUrl = szRefUrl;
	task.m_strSaveFile = szFile;
	task.m_dwDownOkID = dwDownOkID;
	task.m_bReportErr = bReportErr;
	task.m_bHideTask = bHideTask;

	m_mutexAccess.Acquire();
	m_listTasks.push_back(task);
	m_mutexAccess.Release();

	return ERR_OK;
}


bool CDownloadMgr::IsTaskExist(LPCTSTR szURL)
{
	CMutexAutolock	lock(m_mutexAccess);

	LIST_TASK::iterator	it;

	for (it = m_listTasks.begin(); it != m_listTasks.end(); ++it)
	{
		Task	&t = *it;
		if (_tcsicmp(t.m_strURL.c_str(), szURL) == 0)
			return true;
	}

	return false;
}


void CDownloadMgr::OnEndDownload(Task *pTask)
{
	if (pTask->m_dwState == TASK_STATE_RUNNING_REDIRECT)
	{
		RunTask(pTask);
	}
	else if (pTask->IsStopped())
	{
		if (pTask->m_bReportErr)
		{
			CEventDownloadEnd	*pEvent = new CEventDownloadEnd;
			pEvent->eventType = ET_DOWNLOAD_END;
			pEvent->pTask = pTask;
			pEvent->downloadType = CEventDownloadEnd::DT_DL_LRC_FAILED;
			g_eventDispather.DispatchSyncEventByNoUIThread(pEvent);
		}
		RemoveTask(&pTask);
		OnTaskQueueChanged();
	}
}


int CDownloadMgr::RunHttpTask(Task *pTask)
{
	CHttpClient			httpClient;
	BOOL				bUseProxy = false;
	int					nProxyPort = 80;
	TCHAR				szProxyServer[256];
	LPCTSTR				szProxyPwd;

	int					nHttpCode = 0;
	CCharBuffer			buffRet;
	int					nRet;
	CDownloadTask		&Task = *pTask;
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

	httpClient.SetUserAgent(SZ_USER_AGENT_IE5);
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

	for (iRetry = 1; iRetry <= m_nRetryCount; iRetry++)
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
				// 休息10秒钟吧
				if (!pTask->m_bHideTask)
					g_eventDispather.DispatchLongErrorText(TOLOCAL(_T("Wait 10 seconds and retry.")));
				if (pHttpDownImp->m_evtShutDown.Acquire(10 * 1000))
					break;
			}

			continue;
		}

		if (nHttpCode != 200 && !Task.m_bHideTask)
		{
			tstringex	strBuff;

			ConvertStr(buffRet.c_str(), buffRet.size(), strBuff);
			FilterAllHTMLTags(strBuff.data());
			strBuff.resize(_tcslen(strBuff.c_str()));

			CStrPrintf	str(_T("Url: %s\r\nHttp %d error!\r\n%s"), Task.m_strURL.c_str(), nHttpCode, strBuff.c_str());
			g_eventDispather.DispatchLongErrorText(str.c_str());
			ERRLOG1(_T("%s"), str.c_str());
		}

		httpClient.Close();

		break;
	}

RETURN_END:

	// LOG2(LOG_LVL_EVENT_IMP, _T("End of downloading, http Code: %d, error: %d"), nHttpCode, nRet);

	Task.m_nHttpRetCode = nHttpCode;
	Task.m_errResult = nRet;
	if (nRet == ERR_OK && nHttpCode == 200)
	{
		Task.m_dwState = TASK_STATE_DOWNLOAD_OK;
		Task.OnDownOKSaveFile(buffRet.data(), buffRet.size());
	}
	else
	{
		Task.m_dwState = TASK_STATE_STOP_ERROR;
		DBGLOG3(_T("Downloaded Failed, Error: %d, httpcode: %d, %s"), nRet, nHttpCode, Task.m_strURL.c_str());
	}

	/////////////
	// Debug code here, 因为在检查该任务是否退出时，这时该线程还没有退出，
	// 所以只能通过这个方式来释放内存了。
	//	if (Task.m_dwState != TASK_STATE_RUNNING_REDIRECT)
	{
		if (pTask->m_pDownloadImp)
			delete pTask->m_pDownloadImp;
		pTask->m_pDownloadImp = NULL;
	}
	// Debug code here
	/////////////

	pTask->m_pDownMgr->OnEndDownload(pTask);
}


//////////////////////////////////////////////////////////////////////////

int CDownloadMgr::RemoveTask(CDownloadTask **ppTask)
{
	if ((*ppTask)->IsRunning())
	{
		// must force to stop it!
		return ERR_FALSE;
	}

	EnterWriteSection();
	for (UINT i = 0; i < m_listTasks.size(); i++)
	{
		if (m_listTasks[i]->m_dwID == (*ppTask)->m_dwID)
		{
			assert(m_listTasks[i] == *ppTask);

			m_listTasks.erase(m_listTasks.begin() + i);
			LeaveWriteSection();

			if ((*ppTask)->Shutdown() == ERR_OK)
			{
				delete *ppTask;
				*ppTask = NULL;
			}


			return ERR_OK;
		}
	}
	LeaveWriteSection();

	return ERR_FALSE;
}

void CDownloadMgr::SetDefSavePath(const TCHAR *szSavePath)
{
	assert(szSavePath);
	m_strDefSavePath = szSavePath;
}

int CDownloadMgr::AddTask(const CDownloadTask &Task, DWORD *pdwTaskID)
{
	CDownloadTask	*pTask;

	if (pdwTaskID)
		*pdwTaskID = 0;
	
	if (IsTaskExist(Task) == ERR_OK)
		return ERR_URL_EXIST_IN_QUEUE;

	pTask = new CDownloadTask;
	assert(pTask);

	*pTask = Task;

	pTask->m_pDownMgr = this;

	pTask->m_dwState = TASK_STATE_READY;
	pTask->m_errResult = ERR_OK;

	// 保存路径
	if ((pTask->m_dwMask & DTASK_SAVEPATH) != DTASK_SAVEPATH)
	{
		pTask->SetSavePath(m_strDefSavePath.c_str());
	}

	// 保存文件
	if ((pTask->m_dwMask & DTASK_FILENAME) != DTASK_FILENAME)
	{
		TCHAR	szFileName[MAX_PATH];

		GetFileNameFromURL(pTask->m_strURL.c_str(), szFileName, CountOf(szFileName));
		pTask->SetSaveFileName(szFileName);
	}

	pTask->m_dwID = m_dwTaskIDMax;
	m_dwTaskIDMax++;

	if (pdwTaskID)
	*pdwTaskID = pTask->m_dwID;

	// 添加到队列中
	EnterWriteSection();
	m_listTasks.push_back(pTask);
	LeaveWriteSection();

	if (GetRunningTaskCount() < m_nMaxRunningTasks)
	{
		RunTask(pTask);
	}

	return ERR_OK;
}

int CDownloadMgr::IsTaskExist(const CDownloadTask &Task)
{
	EnterReadSection();
	for (UINT i = 0; i < m_listTasks.size(); i++)
	{
		// 对于某些HTTP服务器, 是要区分大小写的 
		if (_tcscmp(m_listTasks[i]->m_strURL.c_str(), Task.m_strURL.c_str()) == 0)
		{
			// 针对HTTP协议，需要检查POST 是否一致
			if (Task.m_bHttpPost)
			{
				if (_tcscmp(m_listTasks[i]->m_strHttpPost.c_str(), Task.m_strHttpPost.c_str()) == 0)
				{
					LeaveReadSection();
					return ERR_OK;
				}
			}
			else
			{
				if (!m_listTasks[i]->m_bHttpPost)
				{
					LeaveReadSection();
					return ERR_OK;
				}
			}
		}
	}
	LeaveReadSection();

	return ERR_FALSE;
}

int CDownloadMgr::Init()
{
//	CDownTasksFile		file;

//	file.ReadTasks(m_listTasks);

	// 设置缺省的UserAgent
	// SetUserAgent(USER_AGENT_CUSTOM);

	return ERR_OK;
}

void CDownloadMgr::Quit()
{
}

int CDownloadMgr::GetMaxRetryCount()
{
	return m_nMaxRetryCount;
}

void CDownloadMgr::SetMaxRetryCount(int nCount)
{
	m_nMaxRetryCount = nCount;
}

///////////////////////////////////////////////////////////////////////////////
// CDownloadTask
CDownloadTask::CDownloadTask()
{
	m_bHideTask = FALSE;

	m_bHttpPost = FALSE;
	m_dwMask = 0;

	m_dwState = TASK_STATE_STOP;

	m_dwFileSize = 0;

	m_dwDownOkID = 0;
	m_dwParam2 = 0;

	m_dwRetryCount = 0;

	m_pDownMgr = NULL;
	m_pDownloadImp = NULL;
	m_errResult = ERR_OK;
	m_nHttpRetCode = 0;
	m_bReportErr = false;

	m_strReferUrl = _T("http://www.ml-search.com/");
}

CDownloadTask::~CDownloadTask()
{
	if (m_pDownloadImp)
	{
//		if (m_pDownloadImp->Shutdown() == ERR_OK)
			delete m_pDownloadImp;
	}
}

void CDownloadTask::Redirect(LPCTSTR szURL)
{
	m_dwRetryCount = 0;
	m_dwState = TASK_STATE_READY;

	// 将szURL的路径补充完整
	if (_tcsstr(szURL, _T("://")) != 0)
		m_strURL = szURL;
	else
	{
		LPCTSTR		szBeg, szEnd;

		szBeg = _tcsstr(m_strURL.c_str(), _T("://"));
		assert(szBeg);
		szBeg += 3;		// 3= length(_T("://"))
		szEnd = _tcschr(szBeg, '/');
		if (szEnd == NULL)
		{
			if (szURL[0] == '/')
			{
				m_strURL += szURL;
			}
			else
			{
				m_strURL += _T("/");
				m_strURL += szURL;
			}
		}
		else
		{
			m_strURL[szEnd - m_strURL.c_str()] = '\0';
			m_strURL = m_strURL.c_str();
			if (szURL[0] == '/')
			{
				m_strURL = szURL;
			}
			else
			{
				m_strURL += _T("/");
				m_strURL += szURL;
			}
		}
	}
}

const CDownloadTask & CDownloadTask::operator =(const CDownloadTask &Task)
{
	m_strURL = Task.m_strURL;		// 任务链接
	m_strSaveFile = Task.m_strSaveFile;		// 下载后保存的文件名, 包括全路径

	m_dwMask = Task.m_dwMask;

	m_bHideTask = Task.m_bHideTask;

	m_pDownMgr = Task.m_pDownMgr;
	m_dwState = Task.m_dwState;

	m_dwFileSize = Task.m_dwFileSize;

	m_dwDownOkID = Task.m_dwDownOkID;
	m_dwParam2 = Task.m_dwParam2;

	m_dwRetryCount = Task.m_dwRetryCount;

	m_dwID = Task.m_dwID;

	m_errResult = Task.m_errResult;			// 下载任务运行的结果码
	m_nHttpRetCode = Task.m_nHttpRetCode;
	m_bReportErr = Task.m_bReportErr;

	m_bHttpPost = Task.m_bHttpPost;				// 使用POST 还是GET ？
	m_strHttpPost = Task.m_strHttpPost;	// POST 的数据
	m_strReferUrl = Task.m_strReferUrl;

	return *this;
}

BOOL CDownloadTask::IsRunning() const
{
//	if (m_dwState == TASK_STATE_RUNNING || m_dwState == TASK_STATE_RUNNING_REDIRECT)
	if (m_pDownloadImp)
		return TRUE;
	else
		return FALSE;
}

BOOL CDownloadTask::IsStopped() const
{
	switch (m_dwState)
	{
	case TASK_STATE_DOWNLOAD_OK:
//	case TASK_STATE_STOP_ERR_MAX_RETRY:
	case TASK_STATE_STOP_ERROR:
	case TASK_STATE_STOP:
//	case TASK_STATE_STOP_EXIST_FILE:
		return TRUE;
	default:
		return FALSE;
	}
}


// COMMENT:
//		设置下载后的保存路径
void CDownloadTask::SetSavePath(LPCTSTR szSavePath)
{
	assert(szSavePath);
	assert(_tcslen(szSavePath));

	m_dwMask |= DTASK_SAVEPATH;

	m_strSaveFile = szSavePath;
	m_strSaveFile += FileGetName(m_strSaveFile.c_str());
}

// COMMENT:
//		取得设置的保存路径
void CDownloadTask::GetSavePath(LPTSTR szSavePath, int nLenMax) const
{
	assert(szSavePath);

	FileGetPath(szSavePath, nLenMax, m_strSaveFile.c_str());
}

// COMMENT:
//		设置下载后保存的文件名, 不包含路径
void CDownloadTask::SetSaveFileName(LPCTSTR szFileName)
{
	TCHAR	szTemp[MAX_PATH];
//	stringex	str;
//	tstringex		str2;
//	char	szTemp2[MAX_PATH];

	FileGetPath(szTemp, CountOf(szTemp), m_strSaveFile.c_str());

	m_strSaveFile = szTemp;

//	ConvertStr(szFileName, -1, str);
//
//	// 转换文件名称, 保存到szTemp
//	UriUnescape(str.c_str(), szTemp2, MAX_PATH);
//
//	ConvertStr(szTemp2, -1, str2);
//	_xcscpy(szTemp, str2.c_str());
	UriUnescape(szFileName, szTemp, CountOf(szTemp));

	FileNameFilterInvalidChars(szTemp);

	m_strSaveFile += szTemp;
}

// COMMENT:
//		取得要保存的文件名, 不包含路径
void CDownloadTask::GetSaveFileName(LPTSTR szFileName, int nLenMax) const
{
	_tcscpy_safe(szFileName, nLenMax, FileGetName(m_strSaveFile.c_str()));
}

// COMMENT:
//		取得要保存的文件, 包含路径
LPCTSTR CDownloadTask::GetSaveFile() const
{
	return m_strSaveFile.c_str();
}

// COMMENT:
//		设置下载后保存的文件, 包含路径
void CDownloadTask::SetSaveFile(LPCTSTR szFile)
{
	m_strSaveFile = szFile;
}

int CDownloadTask::Shutdown()
{
	int		nRet = ERR_OK;

	if (m_pDownloadImp)
	{
		nRet = m_pDownloadImp->Shutdown();
		
		if (nRet == ERR_OK)
		{
			delete m_pDownloadImp;
			m_pDownloadImp = NULL;
		}
	}

	return nRet;
}

int CDownloadTask::Run()
{
	assert(m_pDownloadImp == NULL);

	ASSERT(_tcsnicmp(m_strURL.c_str(), _T("HTTP://"), 7) == 0);
	m_pDownloadImp = new CHttpDownImp2(this);

	m_pDownloadImp->Run();

	return ERR_OK;
}

void CDownloadTask::OnDownOKSaveFile(LPCVOID lpData, int nLen)
{
	if (IsFileExist(m_strSaveFile.c_str()))
	{
		SetFileNoReadOnly(m_strSaveFile.c_str());
	}

	if (!SaveDataAsFile(m_strSaveFile.c_str(), lpData, nLen))
	{
		TCHAR	szPath[MAX_PATH];
		FileGetPath(szPath, CountOf(szPath), m_strSaveFile.c_str());
		CreateDirectoryAll(szPath);
		if (!SaveDataAsFile(m_strSaveFile.c_str(), lpData, nLen))
		{
			// save in the default directory of MiniLyrics.
			m_strSaveFile = m_pDownMgr->GetDefSavePath();
			DirStringFormat(m_strSaveFile);
			m_strSaveFile += FileGetName(m_strSaveFile.c_str());

			FileGetPath(szPath, CountOf(szPath), m_strSaveFile.c_str());
			CreateDirectoryAll(szPath);
			SaveDataAsFile(m_strSaveFile.c_str(), lpData, nLen);
		}
	}
//	if (pFileTime)
//		SetFileTime(m_strSaveFile.c_str(), pFileTime, pFileTime, pFileTime);
}

// END OF CDownloadTask
/////////////////////////////////////////////////////////////////////////////////

vector<CDownloadTask *> &CDownloadMgr::GetDownTaskQueue()
{
	return m_listTasks;
}

// 
// 根据每一个任务的ID来取得对应的任务
//
// !!!!调用该函数存在潜在的问题：下载线程删除了任务，但是另外的地方却在使用该任务
CDownloadTask * CDownloadMgr::GetDownloadTaskByID(DWORD dwID)
{
	EnterReadSection();
	for (UINT i = 0; i < m_listTasks.size(); i++)
	{
		if (m_listTasks[i]->m_dwID == dwID)
		{
			LeaveReadSection();
			return m_listTasks[i];
		}
	}
	LeaveReadSection();

	return NULL;
}

// 
// 根据每一个任务的ID来取得对应的任务
//
BOOL CDownloadMgr::GetDownloadTaskByID(DWORD dwID, CDownloadTask &task)
{
	EnterReadSection();
	for (UINT i = 0; i < m_listTasks.size(); i++)
	{
		if (m_listTasks[i]->m_dwID == dwID)
		{
			task = *m_listTasks[i];
			LeaveReadSection();
			return TRUE;
		}
	}
	LeaveReadSection();

	return FALSE;
}

int CDownloadMgr::RemoveDownloadTaskByID(DWORD dwID)
{
	EnterWriteSection();
	for (UINT i = 0; i < m_listTasks.size(); i++)
	{
		if (m_listTasks[i]->m_dwID == dwID)
		{
			if (m_listTasks[i]->Shutdown() == ERR_OK)
			{
				CDownloadTask	*pTask;

				pTask = m_listTasks[i]; 

				m_listTasks.erase(m_listTasks.begin() + i);
				LeaveWriteSection();

				delete pTask;
			}
			else
				LeaveWriteSection();

			return ERR_OK;
		}
	}
	LeaveWriteSection();

	return ERR_FALSE;
}

int CDownloadMgr::StartTask(CDownloadTask *pTask)
{
	if (!IsTaskStarted(pTask->m_dwState))
	{
		pTask->m_dwState = TASK_STATE_READY;

		if (GetRunningTaskCount() <= m_nMaxRunningTasks)
			return RunTask(pTask);
	}

	return ERR_FALSE;
}

void CDownloadMgr::OnTaskQueueChanged()
{
	int		nRunCount = 0;

	nRunCount = GetRunningTaskCount();
	if (nRunCount < m_nMaxRunningTasks)
	{
		// runTask

		EnterReadSection();
		for (UINT i = 0; i < m_listTasks.size(); i++)
		{
			if (m_listTasks[i]->m_dwState == TASK_STATE_READY)
			{
				if (RunTask(m_listTasks[i]) == ERR_OK)
					nRunCount++;

				if (nRunCount >= m_nMaxRunningTasks)
				{
					LeaveReadSection();
					return;
				}
			}
		}
		LeaveReadSection();
	}
}

LPCTSTR CDownloadMgr::GetUserAgent()
{
	return m_strUserAgent.c_str();
}

void CDownloadMgr::SetUserAgent(DL_USER_AGENT dwUserAgent, LPCTSTR szAgentCustom)
{
	if (dwUserAgent == USER_AGENT_IE5X)
		m_strUserAgent = _T("Mozilla/4.0 (compatible; MSIE 5.00; Windows 98)");
	else if (dwUserAgent == USER_AGENT_NETSCAPE4X)
		m_strUserAgent = _T("Mozilla/4.05 [en] (Win98; I))");
	else if (dwUserAgent == USER_AGENT_CUSTOM)
	{
		assert(szAgentCustom);
		m_strUserAgent = szAgentCustom;
	}
}

LPCTSTR CDownloadTask::GetReferUrl()
{
	return m_strReferUrl.c_str();
}

void CDownloadTask::SetReferUrl(LPCTSTR szReferUrl)
{
	if (!IsEmptyString(szReferUrl))
	{
		int		nLen;

		m_strReferUrl = szReferUrl;
		nLen = _tcslen(szReferUrl);

		if (szReferUrl[nLen - 2] != '\r' || szReferUrl[nLen - 1] != '\n')
		{
			m_strReferUrl += _T("\r\n");
		}
	}
}
