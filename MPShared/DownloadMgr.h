/********************************************************************
	Created  :	2002/02/21	2:09
	FileName :	DownloadMgr.h
	Author   :	xhy
	
	Purpose  :	下载任务的关理类
*********************************************************************/

#if !defined(AFX_DOWNLOADMGR_H__4C8DE765_266C_11D6_B47A_00E04C008BA3__INCLUDED_)
#define AFX_DOWNLOADMGR_H__4C8DE765_266C_11D6_B47A_00E04C008BA3__INCLUDED_

#pragma once

class CDownloadTask
{
public:
	CDownloadTask()
	{
		m_bReportErr = true;
		m_bHideTask = false;
		m_dwID = 0;
		m_dwDownOkID = 0;
		m_errResult = 0;
		m_nHttpRetCode = 0;
	}

	tstring			m_strSaveFile;

	tstring			m_strURL;
	tstring			m_strReferUrl;		// Referer: http://
	uint32			m_dwID;
	MLRESULT		m_errResult;
	int				m_nHttpRetCode;
	bool			m_bReportErr;
	bool			m_bHideTask;
	DWORD			m_dwDownOkID;

};

class CDownloadMgr
{
public:
	CDownloadMgr();
	virtual ~CDownloadMgr();

public:

	typedef	list<CDownloadTask *>		LIST_TASK;

public:
	int Init();
	void Quit();

	int AddTask(LPCTSTR szURL, LPCTSTR szRefUrl, LPCTSTR szFile, DWORD dwDownOkID, DWORD *pdwTaskID, bool bReportErr = true, bool bHideTask = false);

	bool IsTaskExist(LPCTSTR szURL);

	void SetDefSavePath(LPCTSTR szDefSavePath) { m_strDefSavePath = szDefSavePath; }
	LPCTSTR GetDefSavePath() const { return m_strDefSavePath.c_str(); }

protected:
	virtual void OnEndDownload(CDownloadTask *pTask);

	static void DownloadThreadProc(LPVOID lpParam);

	void DownloadThread();

	int RunHttpTask(CDownloadTask *pTask);

	virtual int SaveDownloadedFile(CDownloadTask *pTask, LPVOID lpData, int nSize);

protected:
	CThread					m_threadDownload;
	Mutex					m_mutexAccess;
	LIST_TASK				m_listTasks, m_listRunningTasks;
	uint32					m_dwTaskIDNext;
	int						m_nRetryCount;
	Event					m_eventShutDown;
	bool					m_bQuit;
	tstring					m_strDefSavePath;
	tstring					m_strUserAgent;

};


#endif // !defined(AFX_DOWNLOADMGR_H__4C8DE765_266C_11D6_B47A_00E04C008BA3__INCLUDED_)
