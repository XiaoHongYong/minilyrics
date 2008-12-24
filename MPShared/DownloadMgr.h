/********************************************************************
	Created  :	2002/02/21	2:09
	FileName :	DownloadMgr.h
	Author   :	xhy
	
	Purpose  :	下载任务的关理类
*********************************************************************/

#if !defined(AFX_DOWNLOADMGR_H__4C8DE765_266C_11D6_B47A_00E04C008BA3__INCLUDED_)
#define AFX_DOWNLOADMGR_H__4C8DE765_266C_11D6_B47A_00E04C008BA3__INCLUDED_

#pragma once

// 下载任务的m_dwID的最初值
#define DOWN_TASK_BASE_ID		0x80000000


class CDownloadMgr
{
public:
	CDownloadMgr();
	virtual ~CDownloadMgr();

public:
	class Task
	{
	public:
		Task()
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

	typedef	list<Task>		LIST_TASK;

public:
	int AddTask(LPCTSTR szURL, LPCTSTR szFile, DWORD dwDownOkID, bool bReportErr = true, bool bHideTask = false);

	bool IsTaskExist(LPCTSTR szURL);

	virtual void OnEndDownload(Task *pTask);

protected:
	int RunHttpTask(Task *pTask);

protected:
	Mutex					m_mutexAccess;
	LIST_TASK				m_listTasks;
	uint32					m_dwTaskIDNext;
	int						m_nRetryCount;
	Event					m_eventShutDown;

};



//
// 下载任务的基本信息
//
class CDownloadTask
{
public:
	CDownloadTask();
	~CDownloadTask();

	void Redirect(LPCTSTR szURL);

public:
	const CDownloadTask & operator =(const CDownloadTask &Task);

	void SetDownOkID(DWORD dwDownOkID) { m_dwDownOkID = dwDownOkID; }
	DWORD GetDownOkID()	{ return m_dwDownOkID; }

	void  SetParameter2(DWORD dwParam) { m_dwParam2 = dwParam; }
	DWORD  GetParameter2() { return m_dwParam2; }

	void SetReferUrl(LPCTSTR szReferUrl);
	LPCTSTR GetReferUrl();

	void GetSaveFileName(LPTSTR szFileName, int nLenMax) const;
	void SetSaveFileName(LPCTSTR szFileName);
	
	void GetSavePath(LPTSTR szSavePath, int nLenMax) const;
	void SetSavePath(LPCTSTR szSavePath);

	void SetSaveFile(LPCTSTR szFile);
	LPCTSTR GetSaveFile() const;

	BOOL IsStopped() const;
	BOOL IsRunning() const;

	int Shutdown();
	int Run();

	void OnDownOKSaveFile(LPCVOID lpData, int nLen);

protected:
	tstring			m_strSaveFile;		// 下载后保存的文件, 是文件路径和文件名的组合

public:
	// 下面的字段由用户设置
	tstring			m_strURL;			// 任务链接
	DWORD			m_dwMask;			// 标志

	BOOL			m_bHideTask;		// 是否隐藏？

	//
	// 下面的字段由程序设置
	CDownloadImp	*m_pDownloadImp;	// 下载的实现类
	CDownloadMgr	*m_pDownMgr;		// 管理此下载任务的 CDownloadMgr
	DWORD			m_dwState;			// 任务状态,参见前面定义

	DWORD			m_dwFileSize;		// 文件大小

	DWORD			m_dwDownOkID;			// 存储用户自定义的数据1
	DWORD			m_dwParam2;			// 存储用户自定义的数据2

	DWORD			m_dwRetryCount;		// 下载重试的次数

	DWORD			m_dwID;				// 下载任务的ID，唯一

	int				m_errResult;		// 下载任务运行的结果码
	int				m_nHttpRetCode;		// 
	bool			m_bReportErr;

	// HTTP 协议使用到的数据
	BOOL			m_bHttpPost;			// 使用POST 还是GET ？
	tstring			m_strHttpPost;			// POST 的数据

	tstring			m_strReferUrl;		// Referer: http://202.115.32.32/...........

};

//
// 下载管理
//
class CDownloadMgr  
{
public:
	virtual void OnEndDownload(CDownloadTask *pTask);

	int AddTask(const CDownloadTask &Task, DWORD *pdwTaskID);

public:
	void SetMaxRetryCount(int nCount);
	int GetMaxRetryCount();

	CDownloadMgr();
	virtual ~CDownloadMgr();

};

#endif // !defined(AFX_DOWNLOADMGR_H__4C8DE765_266C_11D6_B47A_00E04C008BA3__INCLUDED_)
