// OnlineSearch.h: interface for the COnlineSearch class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ONLINESEARCH_H__F7E2D78C_B3D3_42B9_BFE8_6DD7F2AEFDA2__INCLUDED_)
#define AFX_ONLINESEARCH_H__F7E2D78C_B3D3_42B9_BFE8_6DD7F2AEFDA2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../sqlite_cpp/Sqlite3.h"

#define MSG_ONLINE_SEARCH_RESULT		(WM_USER + 6)

class ILyrSearchResultCacheDB
{
public:
	virtual bool Open() = 0;

	virtual void Close() = 0;

	virtual bool IsOpened() = 0;

	virtual bool SearchCache(LPCTSTR szFileAssociateKeyword, LPCTSTR szArtist, LPCTSTR szTitle, V_LRCSEARCHRESULT *pvResult, DWORD *pdwSearchTime) = 0;
	virtual bool UpdateSearchLrcInfo(LPCTSTR szFileAssociateKeyword, MLFileInfoList &fileList) = 0;

protected:
	void ResultToXML(MLFileInfoList &fileList, CXMLWriter &xmlWriter);
	void ResultFromXML(MLFileInfoList &fileList, LPCSTR szXML);

};

class CLyrSearchResultCacheDB : public ILyrSearchResultCacheDB
{
public:
	CLyrSearchResultCacheDB();
	~CLyrSearchResultCacheDB();

	virtual bool Open();

	virtual void Close();

	virtual bool IsOpened() { return m_db.m_db != NULL; }

	virtual bool SearchCache(LPCTSTR szFileAssociateKeyword, LPCTSTR szArtist, LPCTSTR szTitle, V_LRCSEARCHRESULT *pvResult, DWORD *pdwSearchTime);
	virtual bool UpdateSearchLrcInfo(LPCTSTR szFileAssociateKeyword, MLFileInfoList &fileList);

protected:
	CSqlite3		m_db;

};

class CLyrSearchResultCacheMemDB : public ILyrSearchResultCacheDB
{
public:
	virtual bool Open() { return true; }

	virtual void Close() { m_listResults.clear(); }

	virtual bool IsOpened() { return true; }

	virtual bool SearchCache(LPCTSTR szFileAssociateKeyword, LPCTSTR szArtist, LPCTSTR szTitle, V_LRCSEARCHRESULT *pvResult, DWORD *pdwSearchTime);
	virtual bool UpdateSearchLrcInfo(LPCTSTR szFileAssociateKeyword, MLFileInfoList &fileList);

	static void Free();

protected:
	struct ITEM
	{
		tstring		strKeyword;
		string		strXml;
	};
	typedef list<ITEM>		LIST_ITEMS;
	static LIST_ITEMS		m_listResults;

};

class COnlineSearch  
{
public:
	struct COnlineSearchThreadParam
	{
		tstring		strTitle;
		tstring		strArtist;
		TCHAR		szAssociateKeyword[MAX_PATH];
		COnlineSearch	*pOnlineSearch;
		COnlineSearchThreadParam()
		{
			EmptyStr(szAssociateKeyword);
			pOnlineSearch = NULL;
		}
	};

	COnlineSearch();
	virtual ~COnlineSearch();

public:
	BOOL Init();
	void Quit();

	BOOL SearchCacheForCur(V_LRCSEARCHRESULT *pvResult = NULL, DWORD *pdwSearchTime = NULL);

	BOOL AutoSearch(LPCTSTR szArtist, LPCTSTR szTitle);

	int SearchOnline(CMLClientSession &session, LPCTSTR szArtist, LPCTSTR szTitle, V_LRCSEARCHRESULT &vResult, tstring &strMsg, int &nCurPage, int &nPageCount);

protected:
	int UpdateSearchLrcInfo(LPCTSTR szFileAssociateKeyword, MLFileInfoList &fileList);
	BOOL AddSearchJob(LPCTSTR szArtist, LPCTSTR szTitle);
	bool IsTitleExcludedFromAutoSearch(LPCTSTR szTitle);

	static void ThreadSearchOnline(LPVOID lpParam);

	Mutex			m_mutexDB;
	bool			m_bRunning;
	CThread			m_threadSearch;

	CLyrSearchResultCacheDB		m_dbSearchResult;
	CLyrSearchResultCacheMemDB	m_dbSearchResultMem;

};

extern COnlineSearch		g_OnlineSearch;

#endif // !defined(AFX_ONLINESEARCH_H__F7E2D78C_B3D3_42B9_BFE8_6DD7F2AEFDA2__INCLUDED_)
