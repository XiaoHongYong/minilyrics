// OnlineSearch.h: interface for the COnlineSearch class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "../sqlite_cpp/Sqlite3.h"

DECLAR_CPPUNIT_TEST_REG(COnlineSearch)

class ILyrSearchResultCacheDB
{
public:
	virtual bool Open() = 0;

	virtual void Close() = 0;

	virtual bool IsOpened() = 0;

	virtual bool SearchCache(LPCTSTR szFileAssociateKeyword, LPCTSTR szArtist, LPCTSTR szTitle, V_LRCSEARCHRESULT *pvResult, DWORD *pdwSearchTime) = 0;
	virtual bool UpdateSearchLrcInfo(LPCTSTR szFileAssociateKeyword, MLMsgRetSearch &retSearch) = 0;

protected:
	void ResultToXML(MLMsgRetSearch &retSearch, CXMLWriter &xmlWriter);
	void ResultFromXML(MLMsgRetSearch &retSearch, LPCSTR szXML);

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
	virtual bool UpdateSearchLrcInfo(LPCTSTR szFileAssociateKeyword, MLMsgRetSearch &retSearch);

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
	virtual bool UpdateSearchLrcInfo(LPCTSTR szFileAssociateKeyword, MLMsgRetSearch &retSearch);

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
	class INotifyEvent
	{
	public:
		virtual void OnErrorSaveLyrics(int nIndex, LPCTSTR szMessage) = 0;
		virtual void OnLyricsReturned(int nIndex, string &bufLyrics) = 0;
		virtual void OnNoLyricsReturned(int nIndex) = 0;

	};

	struct COnlineSearchThreadParam
	{
		tstring		strTitle;
		tstring		strArtist;
		tstring		strAssociateMediaKey;
		bool		bOnlyMatched;

		COnlineSearch	*pOnlineSearch;
		COnlineSearchThreadParam()
		{
			pOnlineSearch = NULL;
			bOnlyMatched = false;
		}
	};

	COnlineSearch();
	virtual ~COnlineSearch();

public:
	BOOL Init();
	void Quit();

	BOOL SearchCacheForCur(V_LRCSEARCHRESULT *pvResult = NULL, DWORD *pdwSearchTime = NULL);

	bool AutoSearch(LPCTSTR szArtist, LPCTSTR szTitle);

	int SearchOnline(CMLClientSession &session, LPCTSTR szArtist, LPCTSTR szTitle, LPCTSTR szMediaKey, V_LRCSEARCHRESULT &vResult, tstring &strMsg, int &nCurPage, int &nPageCount);
	int BatchSearch(CMLClientSession &session, MLListSearchItems &listSearch, uint32 nLyrSaveFlag, tstring &strMsg, INotifyEvent *pNotify);

	int BatchSearch(CMLClientSession &session, MLListSearchItems &listSearch, MLMsgRetBatchSearch &retLyrics);

protected:
	bool CheckLicenseLimitation();

	int UpdateSearchLrcInfo(LPCTSTR szFileAssociateKeyword, MLMsgRetSearch &retSearch);
	bool AddSearchJob(bool bOnlyMatched, LPCTSTR szArtist, LPCTSTR szTitle);

	static void ThreadSearchOnline(LPVOID lpParam);

	Mutex			m_mutexDB;
	bool			m_bRunning;
	CThread			m_threadSearch;

	CLyrSearchResultCacheDB		m_dbSearchResult;
	CLyrSearchResultCacheMemDB	m_dbSearchResultMem;
	//CLyricsInetSearch			m_lyricsInetSearch;

};

bool FilterSpamInfo(tstring &artist, tstring &title);

extern COnlineSearch		g_OnlineSearch;
