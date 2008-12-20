/********************************************************************
	Created  :	2002/02/21	2:09
	FileName :	DownloadMgr.h
	Author   :	xhy
	
	Purpose  :	下载任务的关理类
*********************************************************************/

#if !defined(AFX_DOWNLOADMGR_H__4C8DE765_266C_11D6_B47A_00E04C008BA3__INCLUDED_)
#define AFX_DOWNLOADMGR_H__4C8DE765_266C_11D6_B47A_00E04C008BA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDownloadMgr;


//
// CDownloadTask::m_dwMask 字段的值定义
//
#define DTASK_FILENAME				0x1		// 是否使用strFile字段,否则保存的文件名有strURL中获得
#define DTASK_SAVEPATH				0x2		// 是否使用strSavePath字段, 否则使用缺省的保存路径
#define DTASK_SAVE_TOBUFFER			0x4		// 保存到内存中,而不是保存到文件中

//
// CDownloadTask::m_dwState 任务状态
//
#define TASK_STATE_DELETED				0	// 该任务已经被删除, 在保存的文件中作为一个标记
#define TASK_STATE_STOP					1	// 停止状态
#define TASK_STATE_RUNNING				2	// 正在下载状态
#define TASK_STATE_READY				3	// 准备下载状态
//#define TASK_STATE_STOP_ERR_MAX_RETRY	4	// 停止: 最大重试数已经达到
#define TASK_STATE_DOWNLOAD_OK			5	// 停止: 成功下载完毕
#define TASK_STATE_STOP_ERROR			6	// 停止: 出现错误
#define TASK_STATE_RUNNING_REDIRECT		7	// 停止: 重定向
//#define TASK_STATE_STOP_EXIST_FILE		8	// 停止: 文件已经存在

#define IsTaskStarted(dwState)	(dwState == TASK_STATE_RUNNING || \
								dwState == TASK_STATE_RUNNING_REDIRECT || \
								dwState == TASK_STATE_READY)


class CDownloadMgr;

// 下载任务的m_dwID的最初值
#define DOWN_TASK_BASE_ID		0x80000000


//
// UserAgent, CDownloadTask::m_dwUserAgent
//
typedef enum DL_USER_AGENT
{
	USER_AGENT_CUSTOM		= 0,
	USER_AGENT_IE5X			= 1,
	USER_AGENT_NETSCAPE4X	= 2
}DL_USER_AGENT;


class CDownloadTask;

//
// 下载任务的实现的虚基类
//
class CDownloadImp
{
public:
	CDownloadImp(CDownloadTask *pTask)
	{
		m_pTask = pTask;
	}
	virtual ~CDownloadImp()
	{
	}
	virtual int Run() = 0;
	virtual int Shutdown() = 0;
	
protected:
	CDownloadTask		*m_pTask;
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
	void SetUserAgent(DL_USER_AGENT dwUserAgent = USER_AGENT_CUSTOM, LPCTSTR szAgentCustom = NULL);
	LPCTSTR GetUserAgent();

	int StartTask(CDownloadTask *pTask);

	int RemoveDownloadTaskByID(DWORD dwID);

	BOOL GetDownloadTaskByID(DWORD dwID, CDownloadTask &task);
	CDownloadTask * GetDownloadTaskByID(DWORD dwID);
	vector<CDownloadTask *> &GetDownTaskQueue();

	void SetMaxRetryCount(int nCount);
	int GetMaxRetryCount();

	void SetDefSavePath(const TCHAR *szSavePath);
	LPCTSTR GetDefSavePath() { return m_strDefSavePath.c_str(); }

	virtual int Init();
	virtual void Quit();

	int IsTaskExist(const CDownloadTask &Task);

	int IsURLExist(const TCHAR *szURL);
	int GetRunningTaskCount();

	CDownloadMgr();
	virtual ~CDownloadMgr();

	void EnterReadSection()
	{
		m_mutexAccess.Acquire();
	}
	void LeaveReadSection()
	{
		m_mutexAccess.Release();
	}

protected:
	int RunTask(CDownloadTask *pTask);

	void OnTaskQueueChanged();
	int RemoveTask(CDownloadTask **ppTask);

	void EnterWriteSection()
	{
		m_mutexAccess.Acquire();
	}
	void LeaveWriteSection()
	{
		m_mutexAccess.Release();
	}

protected:
	//
	// 下载通知回调函数
	LPVOID					m_lpUserParam;

	Mutex					m_mutexAccess;

	tstring					m_strUserAgent;

	vector<CDownloadTask *>	m_vDownloadTasks;	// 管理的所有任务

	int				m_nMaxRunningTasks;		// 最大同时运行的任务数目

	DWORD			m_dwTaskIDMax;			// 下载任务ID的最大值

	int				m_nMaxRetryCount;		// 每一个任务的最大重试次数

	tstring			m_strDefSavePath;


};

#endif // !defined(AFX_DOWNLOADMGR_H__4C8DE765_266C_11D6_B47A_00E04C008BA3__INCLUDED_)
