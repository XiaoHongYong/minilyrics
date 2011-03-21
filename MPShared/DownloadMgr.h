/********************************************************************
	Created  :	2002/02/21	2:09
	FileName :	DownloadMgr.h
	Author   :	xhy
	
	Purpose  :	下载任务的关理类
*********************************************************************/

#if !defined(AFX_DOWNLOADMGR_H__4C8DE765_266C_11D6_B47A_00E04C008BA3__INCLUDED_)
#define AFX_DOWNLOADMGR_H__4C8DE765_266C_11D6_B47A_00E04C008BA3__INCLUDED_

#pragma once

enum DownloadTaskType
{
	DTT_LYRICS,
	DTT_CHECK_VERSION,
	DTT_CHECK_VERSION_NOUI,
	DTT_CHECK_AD,
};

class CDownloadTask
{
public:
	CDownloadTask(DownloadTaskType type, LPCTSTR szUrl)
	{
		taskType = type;
		m_strURL = szUrl;
		m_errResult = ERR_OK;
		m_nHttpRetCode = 0;
	}

	DownloadTaskType	taskType;
	tstring				m_strURL;
	tstring				strMediaKey;
	tstring				m_strLyrFileName;
	CCharBuffer			buffContent;

	MLRESULT			m_errResult;
	int					m_nHttpRetCode;

};

class CDownloadMgr
{
public:
	CDownloadMgr();
	virtual ~CDownloadMgr();

	typedef	list<CDownloadTask *>		LIST_TASK;

public:
	int Init();
	void Quit();

	BOOL SearchInCacheResult(bool bShowInfoText);

	void OnSongChanged();

	void DownloadLyrics(LPCTSTR szMediaKey, LPCTSTR szSongFile, LPCTSTR szUrl, LPCTSTR szFileName);
	void DownloadVersionFile(LPCTSTR szUrl, bool bNoUI);

	void SetDefSavePath(LPCTSTR szDefSavePath) { m_strDefSavePath = szDefSavePath; }
	LPCTSTR GetDefSavePath() const { return m_strDefSavePath.c_str(); }

	bool AutoDownloadEnabled() const { return m_bAutoDownload; }

	tstring GetSaveLyricsFile(LPCTSTR szSongFile, LPCTSTR szFileName);

	int SaveDownloadedLyrics(LPCTSTR szMediaFile, LPCTSTR szLyrFileName, LPCVOID lpData, int nSize, uint32 nLyrSaveFlag = LST_NONE);

protected:
	int SaveDownloadedLyrAsFile(tstring &strFile, LPCVOID lpData, int nSize);

	void AddTask(CDownloadTask *pTask);

	bool IsTaskExist(LPCTSTR szURL);

	void CreateDefaultLyricsDir(tstring &strDefSavePath);

	void OnEndDownload(CDownloadTask *pTask);

	static void DownloadThreadProc(LPVOID lpParam);

	void DownloadThread();

	int RunHttpTask(CDownloadTask *pTask);

	int TryToConnect(class CHttpClient &httpClient);

	bool IsQuiting() { return m_eventShutDown.Acquire(0); }

protected:
	CThread					m_threadDownload;
	Mutex					m_mutexAccess;
	LIST_TASK				m_listTasks, m_listRunningTasks;
	Event					m_eventShutDown;
	tstring					m_strDefSavePath;

public:
	bool					m_bAutoDownload;

};


#endif // !defined(AFX_DOWNLOADMGR_H__4C8DE765_266C_11D6_B47A_00E04C008BA3__INCLUDED_)
