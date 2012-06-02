// OnlineSearch.cpp: implementation of the COnlineSearch class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "../sqlite_cpp/Sqlite3.h"
#include "OnlineSearch.h"
#include "DownloadMgr.h"
#include "Helper.h"

COnlineSearch		g_OnlineSearch;

/*
CREATE TABLE IF NOT EXISTS SearchResults
(
  song_file text,
  modify_time integer,
  xml_data text
);

CREATE INDEX IF NOT EXISTS SR_keyword on SearchResults (song_file);
CREATE INDEX IF NOT EXISTS SR_modify_time on SearchResults (modify_time);
*/

#define SQL_CREATE_SEARCHRESULT		"CREATE TABLE IF NOT EXISTS SearchResults"\
	"("\
	"song_file text,"\
	"modify_time integer,"\
	"xml_data text"\
	");"\
	"CREATE INDEX IF NOT EXISTS SR_keyword on SearchResults (song_file);"\
	"CREATE INDEX IF NOT EXISTS SR_modify_time on SearchResults (modify_time);"

#define SQL_DROP_SEARCHRESULT		"DROP TABLE SearchResults;"\
	"DROP INDEX SR_keyword;"\
	"DROP INDEX SR_modify_time;"

#define SQL_ADD_SEARCHRESULT	"insert into SearchResults (song_file, modify_time, xml_data) VALUES ("\
						"?, ?, ?)"

#define SQL_QUERY_SEARCHRESULT	"select * from SearchResults where song_file=?"
//#define SQL_QUERY_SEARCHRESULT	"select * from SearchResults"

#define SQL_DEL_SEARCHRESULT	"delete from SearchResults where song_file=?"

void RetLyrInfoListToVResult(LPCTSTR szArtist, LPCTSTR szTitle, RetLyrInfoList &filelist, V_LRCSEARCHRESULT &vResult)
{
	BOOL		bIgnoreTitleNumb, bIgnoreArtistNumb;
	bIgnoreTitleNumb = IsIgnoreNumber(szTitle);
	bIgnoreArtistNumb = IsIgnoreNumber(szArtist);

	for (RetLyrInfoList::iterator it = filelist.begin(); it != filelist.end(); it++)
	{
		RetLyrInfo		&file = *it;
		LrcSearchResult		item;
		item.strUrl = file.strLink;
		item.strSaveFileName = file.GetFileName();
		item.strArtist = file.strArtist;
		item.strTitle = file.strTitle;
		item.strAlbum = file.strAlbum;
		item.strUploader = file.strUploader;
		item.fRate = file.fRate;
		item.nRateCount = file.nRateCount;
		item.nDownloads = file.nDownloads;
		if (IsEmptyString(szTitle))
			item.nMatchValue = 2;
		else
			item.nMatchValue = (float)CalLyricsMatchValue(MLGetFileType(item.strSaveFileName.c_str()), item.strTitle.c_str(), item.strArtist.c_str(), szTitle, bIgnoreTitleNumb, szArtist, bIgnoreArtistNumb);
		// DBGLOG2(_T("file: %s, value: %d"), item.strSaveFileName.c_str(), item.nMatchValue);

		// Sort English result before other language.
		if (IsAnsiStr(item.strSaveFileName.c_str()))
			item.nMatchValue += 0.5;

		// increase rating
		if (item.nRateCount > 0)
		{
			item.nMatchValue += item.fRate - 3;
			if (item.nRateCount > 5)
			{
				if (item.fRate >= 4.8)
					item.nMatchValue++;
				else if (item.fRate <= 3.0)
					item.nMatchValue--;
			}
		}

		vResult.AddResult(item);
	}

	vResult.Sort();
}

int GetCurDaysFrom2000()
{
	CBaseDate dateNow = CBaseDate::GetCurrentDate();
	CBaseDate date2000(2000, 1, 1);

	return (dateNow.GetTime() - date2000.GetTime()) / CBaseDate::MILLIS_IN_ONE_DAY;
}

void ILyrSearchResultCacheDB::ResultToXML(MLMsgRetSearch &retSearch, CXMLWriter &xmlWriter)
{
	xmlWriter.WriteStartElement(_T("SearchResults"));

	tstring		strEmptyServerUrl;
	for (list<RetLyrInfo>::iterator it = retSearch.listResultFiles.begin(); 
		it != retSearch.listResultFiles.end(); it++)
	{
		ASSERT(StrIBeginWith((*it).strLink.c_str(), _T("http://")));
		(*it).ToXML(xmlWriter, strEmptyServerUrl, true);
	}

	xmlWriter.WriteEndElement();
}

void ILyrSearchResultCacheDB::ResultFromXML(MLMsgRetSearch &retSearch, LPCSTR szXML)
{
	CSimpleXML	xml;
	SXNode		*pNode;

	if (!xml.ParseData((void*)szXML, strlen(szXML)))
		return;

	pNode = xml.m_pRoot;
	if (_tcscmp(pNode->strName.c_str(), _T("SearchResults")) != 0)
		return;

	SXNode::iterator	it;
	tstring		strEmptyServerUrl;
	for (it = pNode->listChildren.begin(); it != pNode->listChildren.end(); it++)
	{
		RetLyrInfo	Info;
		Info.FromXML(*it, strEmptyServerUrl.c_str());
		ASSERT(StrIBeginWith(Info.strLink.c_str(), _T("http://")));
		retSearch.listResultFiles.push_back(Info);
	}
}

CLyrSearchResultCacheDB::CLyrSearchResultCacheDB()
{
}

CLyrSearchResultCacheDB::~CLyrSearchResultCacheDB()
{
	Close();
}

bool CLyrSearchResultCacheDB::Open()
{
	int			nRet;
	tstring		strFileT;

	strFileT = GetAppDataDir();
	strFileT += _T("dbSearchCache.db");

	if (GetFileLength(strFileT.c_str()) >= 1024 * 1024 * 2)
		DeleteFile(strFileT.c_str());

	nRet = m_db.Open(strFileT.c_str());
	if (nRet != ERR_OK)
		return false;

	/*nRet = sqlite3_exec(m_db, SQL_DROP_SEARCHRESULT, NULL, NULL, NULL);
	if (nRet != SQLITE_OK)
	{
	    LogSqlite3Error(m_db);
	}*/

	nRet = m_db.Exec(SQL_CREATE_SEARCHRESULT);
	if (nRet != ERR_OK)
		return false;

	return true;
}

void CLyrSearchResultCacheDB::Close()
{
	m_db.Close();
}

bool CLyrSearchResultCacheDB::SearchCache(LPCTSTR szFileAssociateKeyword, LPCTSTR szArtist, LPCTSTR szTitle, V_LRCSEARCHRESULT *pvResult, DWORD *pdwSearchTime)
{
	int			nRet;
	stringex	strUtf8;
	CSqlite3Stmt	sqlQuery;

#ifdef _UNICODE
	UCS2ToUtf8(szFileAssociateKeyword, -1, strUtf8);
#else
	strUtf8 = szFileAssociateKeyword;
#endif

	nRet = sqlQuery.Prepare(&m_db, SQL_QUERY_SEARCHRESULT);
	if (nRet != ERR_OK)
		return FALSE;

	sqlQuery.BindStaticText(1, strUtf8.c_str(), strUtf8.size());

	nRet = sqlQuery.Step();
	if (nRet != ERR_SL_OK_ROW)
		return FALSE;

	if (pdwSearchTime)
		*pdwSearchTime = sqlQuery.ColumnInt(1);

	if (pvResult)
	{
		LPCSTR	str = sqlQuery.ColumnText(2);
		if (str)
		{
			MLMsgRetSearch retSearch;
			ResultFromXML(retSearch, str);

			RetLyrInfoListToVResult(szArtist, szTitle, retSearch.listResultFiles, *pvResult);
		}
	}

	return TRUE;
}

bool CLyrSearchResultCacheDB::UpdateSearchLrcInfo(LPCTSTR szFileAssociateKeyword, MLMsgRetSearch &retSearch)
{
	int			nRet;
	int			n = 1;
	stringex	strUtf8;
	CXMLWriter		xml;
	CSqlite3Stmt	sqlDelete;
	CSqlite3Stmt	sqlAdd;

#ifdef _WIN32
	UCS2ToUtf8(szFileAssociateKeyword, -1, strUtf8);
#else
	strUtf8 = szFileAssociateKeyword;
#endif
	ResultToXML(retSearch, xml);

	// delete old one
	nRet = sqlDelete.Prepare(&m_db, SQL_DEL_SEARCHRESULT);
	if (nRet != ERR_OK)
		return false;

	sqlDelete.BindStaticText(1, strUtf8.c_str(), strUtf8.size());

	nRet = sqlDelete.Step();
	if (nRet != ERR_OK)
		return false;
	sqlDelete.Finalize();

	// Add new
	nRet = sqlAdd.Prepare(&m_db, SQL_ADD_SEARCHRESULT);
	if (nRet != ERR_OK)
		return false;

	n = 1;
	sqlAdd.BindStaticText(n++, strUtf8.c_str(), strUtf8.size());
	sqlAdd.BindInt(n++, GetCurDaysFrom2000());
	CCharBuffer &buf = xml.GetBuffer();
	sqlAdd.BindStaticText(n++, buf.c_str(), buf.size());

	nRet = sqlAdd.Step();
	if (nRet != ERR_OK)
		return false;

	return TRUE;
}

CLyrSearchResultCacheMemDB::LIST_ITEMS CLyrSearchResultCacheMemDB::m_listResults;

bool CLyrSearchResultCacheMemDB::SearchCache(LPCTSTR szFileAssociateKeyword, LPCTSTR szArtist, LPCTSTR szTitle, V_LRCSEARCHRESULT *pvResult, DWORD *pdwSearchTime)
{
	LIST_ITEMS::iterator	it, itEnd;

	itEnd = m_listResults.end();
	for (it = m_listResults.begin(); it != itEnd; ++it)
	{
		ITEM	&item = *it;
		if (_tcscmp(item.strKeyword.c_str(), szFileAssociateKeyword) == 0)
		{
			if (pdwSearchTime)
				*pdwSearchTime = GetCurDaysFrom2000();

			if (pvResult)
			{
				LPCSTR	str = item.strXml.c_str();
				if (str)
				{
					MLMsgRetSearch retSearch;
					ResultFromXML(retSearch, str);

					RetLyrInfoListToVResult(szArtist, szTitle, retSearch.listResultFiles, *pvResult);
				}
			}

			return true;
		}
	}

	return false;
}

bool CLyrSearchResultCacheMemDB::UpdateSearchLrcInfo(LPCTSTR szFileAssociateKeyword, MLMsgRetSearch &retSearch)
{
	LIST_ITEMS::iterator	it, itEnd;
	CXMLWriter				xml;

	ResultToXML(retSearch, xml);

	if (m_listResults.size() >= 5)
		m_listResults.pop_front();

	itEnd = m_listResults.end();
	for (it = m_listResults.begin(); it != itEnd; ++it)
	{
		ITEM	&item = *it;
		if (_tcscmp(item.strKeyword.c_str(), szFileAssociateKeyword) == 0)
		{
			item.strXml = xml.GetBuffer().c_str();
			return true;
		}
	}

	ITEM	item;

	item.strKeyword = szFileAssociateKeyword;
	item.strXml = xml.GetBuffer().c_str();

	m_listResults.push_back(item);

	return true;
}

void CLyrSearchResultCacheMemDB::Free()
{
	m_listResults.clear();
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COnlineSearch::COnlineSearch()
{
	m_bRunning = false;
}

COnlineSearch::~COnlineSearch()
{

}

BOOL COnlineSearch::Init()
{
	return TRUE;
}

void COnlineSearch::Quit()
{
	m_threadSearch.Destroy();
	m_threadSearch.Join();
	m_dbSearchResult.Close();
	m_dbSearchResultMem.Close();
}

BOOL COnlineSearch::SearchCacheForCur(V_LRCSEARCHRESULT *pvResult, DWORD *pdwSearchTime)
{
	tstring strAssociateMediaKey = CMPlayerAppBase::GetAssociateFileKeyword();

	CMutexAutolock	autolock(m_mutexDB);

	if (m_dbSearchResultMem.SearchCache(strAssociateMediaKey.c_str(),
		g_Player.GetArtist(), g_Player.GetTitle(), pvResult, pdwSearchTime))
		return true;

	if (!m_dbSearchResult.IsOpened())
		m_dbSearchResult.Open();

	if (m_dbSearchResult.IsOpened())
		return m_dbSearchResult.SearchCache(strAssociateMediaKey.c_str(),
		g_Player.GetArtist(), g_Player.GetTitle(), pvResult, pdwSearchTime);

	return FALSE;
}

void COnlineSearch::ThreadSearchOnline(LPVOID lpParam)
{
	int						nRet;
	CMLClientSession		session;
	MLMsgRetSearch			retSearch;

	COnlineSearchThreadParam	*pSearchParam = (COnlineSearchThreadParam *)lpParam;
	ASSERT(pSearchParam);

	pSearchParam->pOnlineSearch->m_bRunning = true;

	session.Init(GetAppNameLong().c_str());

	nRet = session.Connect();
	if (nRet != ERR_OK)
	{
		tstring		str;
		str += _TLT("Failed to search lyrics with error:");
		str += _T(" ");
		str += LPCTSTR(ERROR2STR_LOCAL(nRet));
		CMPlayerAppBase::GetInstance()->DispatchLongErrorText(str.c_str());

		pSearchParam->pOnlineSearch->m_bRunning = false;
		delete pSearchParam;
		return;
	}

	nRet = session.Search(pSearchParam->bOnlyMatched, pSearchParam->strArtist.c_str(), pSearchParam->strTitle.c_str(), 0, retSearch);
	session.Close();

	if (retSearch.strMessage.size() && CMPlayerAppBase::GetMainWnd())
		CMPlayerAppBase::GetInstance()->MessageOut(retSearch.strMessage.c_str());

	if (retSearch.listResultFiles.size() > 0 || nRet == ERR_NOT_FOUND)
	{
		pSearchParam->pOnlineSearch->UpdateSearchLrcInfo(pSearchParam->strAssociateMediaKey.c_str(), retSearch);

		//
		// 通知给调用者
		IEvent		*pEvent = new IEvent;
		pEvent->eventType = ET_LYRICS_SEARCH_END;
		CMPlayerAppBase::GetEventsDispatcher()->DispatchUnsyncEvent(pEvent);
	}
	else
	{
		tstring			str;
		str += _TLT("Failed to search lyrics with error:");
		str += _T(" ");
		str += (LPCTSTR)ERROR2STR_LOCAL(nRet);
		CMPlayerAppBase::GetInstance()->DispatchLongErrorText(str.c_str());
	}

	pSearchParam->pOnlineSearch->m_bRunning = false;
	delete pSearchParam;
}

// 创建一个线程搜索歌词，线程执行完毕之后会通知目标
BOOL COnlineSearch::AddSearchJob(bool bOnlyMatched, LPCTSTR szArtist, LPCTSTR szTitle)
{
	// LOG0(LOG_LVL_INFO, _T("Enter Add search job proc"));

	if (m_bRunning)
	{
		// LOG0(LOG_LVL_INFO, _T("Another search job is running, quit."));
		return FALSE;
	}

	COnlineSearchThreadParam *pParam = new COnlineSearchThreadParam;
	pParam->strArtist = szArtist;
	pParam->strTitle = szTitle;
	pParam->strAssociateMediaKey = CMPlayerAppBase::GetAssociateFileKeyword();
	pParam->pOnlineSearch = this;
	pParam->bOnlyMatched = bOnlyMatched;

	m_threadSearch.Create(ThreadSearchOnline, pParam);

	return true;
}

bool IsTextWidthExceed(LPCTSTR sztext, int nWidth)
{
	while (*sztext && nWidth >= 0)
	{
#ifdef UNICODE
		if (unsigned(*sztext) >= 0xFF)
			nWidth -= 2;
		else
			nWidth -= 1;
		sztext++;
#else
#pragma warning("IsTextWidthExceed Not implemented!");
#endif
	}
	return nWidth < 0;
}

BOOL COnlineSearch::AutoSearch(LPCTSTR szArtist, LPCTSTR szTitle)
{
	// LOG0(LOG_LVL_INFO, _T("Enter auto search proc"));

	// 歌词为空, 小于3个字符
	if (_tcslen(szTitle) < 3)
	{
		// LOG1(LOG_LVL_INFO, _T("Title is : %s, quit."), sztitleNew);
		return FALSE;
	}

	if (IsTitleExcludedFromAutoSearch(szTitle))
	{
		// LOG1(LOG_LVL_INFO, _T("Title is : %s, quit."), sztitleNew);
		return FALSE;
	}

	//
	// 检查上次自动搜索相隔时间小于1个月才搜索
	DWORD		dwLatestSearchTime;
	bool		bAdvVersion = g_licenseManager.IsLicenseValid();
	if (SearchCacheForCur(NULL, &dwLatestSearchTime))
	{
		int			nNextSearch = 30;
		if (bAdvVersion && !g_LyricData.HasLyricsOpened())
			nNextSearch = 10;

		if (abs((int)dwLatestSearchTime - GetCurDaysFrom2000()) < nNextSearch)
		{
			// ....todo 添加上次搜索时间的介绍, 优化离线歌词搜索库和在下歌词搜索缓存
// 			tstring			str;
// 			CStrPrintf		str2(_TLT("Searched lyrics %d days ago, no results returned."), abs((int)dwLatestSearchTime - GetCurDaysFrom2000()));
// 			str = str2.c_str();
// 			str += _T(" ");
// 			str += _TLT("Auto search canceled.");
// 			CMPlayerAppBase::GetInstance()->DispatchLongErrorText(str.c_str());
			return FALSE;
		}
	}

	//
	// 两次调用自动搜索的时间不能够超过10秒
	static DWORD		dwLastSearch = GetSecCount() - 120;
	if (int(GetSecCount() - dwLastSearch) < 
		(bAdvVersion ? 2 : 10))
	{
		// LOG1(LOG_LVL_INFO, _T("Lastest auto search is about :%d s ago, quit."), GetSecCount() - dwLastSearch);
		return FALSE;
	}

	if (!(bAdvVersion))
	{
		//
		// 平均在一个小时内调用自动搜索的次数不能超过 40 次
		static DWORD		dwStartCall = GetSecCount();
		static DWORD		dwSearchCount = 0;
		if (GetSecCount() - dwStartCall >= 10 * 60 && dwSearchCount / (((double)(GetSecCount() - dwStartCall)) / (60 * 60)) > 40)
		{
			// LOG1(LOG_LVL_INFO, _T("Auto search is too frequent: %d"), dwSearchCount);
			return FALSE;
		}
		// 增加搜索次数
		dwSearchCount++;
	}

	// 更新上次调用自动搜索的时间
	dwLastSearch = GetSecCount();

	tstring			str;
	str = _TLT("Searching lyrics");
	str += _T(": ");
	str += g_Player.GetFullTitle();
	CMPlayerAppBase::GetInstance()->DispatchLongErrorText(str.c_str());

	return AddSearchJob(true, szArtist, szTitle);
}

int COnlineSearch::SearchOnline(CMLClientSession &session, LPCTSTR szArtist, LPCTSTR szTitle, LPCTSTR szMediaKey, V_LRCSEARCHRESULT &vResult, tstring &strMsg, int &nCurPage, int &nPageCount)
{
	int					nRet;

	tstring strAssociateMediaKey = szMediaKey;

	session.Init(GetAppNameLong().c_str());

	nRet = session.Connect();
	if (nRet != ERR_OK)
	{
		CMPlayerApp::GetAppAnalytics().IncreaseNumberOfResult(SZ_UD_SEARCH_LYRICS_DATA, nRet);
		return nRet;
	}

	MLMsgRetSearch	retSearch;

	nRet = session.Search(false, szArtist, szTitle, nCurPage, retSearch);
	session.Close();

	if (retSearch.strMessage.size() && CMPlayerAppBase::GetMainWnd())
		CMPlayerAppBase::GetInstance()->MessageOut(retSearch.strMessage.c_str());

	nCurPage = retSearch.nCurPage;
	nPageCount = retSearch.nPageCount;

	CMPlayerApp::GetAppAnalytics().IncreaseNumberOfResult(SZ_UD_SEARCH_LYRICS_DATA, nRet);

	if (retSearch.listResultFiles.size() > 0)
	{
		UpdateSearchLrcInfo(strAssociateMediaKey.c_str(), retSearch);

		RetLyrInfoListToVResult(szArtist, szTitle, retSearch.listResultFiles, vResult);

		return ERR_OK;
	}

	return nRet;
}

int COnlineSearch::BatchSearch(CMLClientSession &session, MLListSearchItems &listSearch, MLMsgRetBatchSearch &retLyrics)
{
	session.Init(GetAppNameLong().c_str());

	int nRet = session.Connect();
	if (nRet != ERR_OK)
	{
		CMPlayerApp::GetAppAnalytics().IncreaseNumberOfResult(SZ_UD_BATCH_SEARCH_DATA, nRet);
		return nRet;
	}

	nRet = session.BatchSearch(listSearch, retLyrics);
	session.Close();

	// Save License status result
	if (nRet == ERR_OK)
		g_licenseManager.SaveActivateResult(retLyrics);

	CMPlayerApp::GetAppAnalytics().IncreaseNumberOfResult(SZ_UD_BATCH_SEARCH_DATA, nRet);
	return nRet;
}

int COnlineSearch::BatchSearch(CMLClientSession &session, MLListSearchItems &listSearch, uint32 nLyrSaveFlag, tstring &strMsg, INotifyEvent *pNotify)
{
	MLMsgRetBatchSearch	retLyrics;
	int	nRet = BatchSearch(session, listSearch, retLyrics);
	if (nRet != ERR_OK)
		return nRet;

	if (retLyrics.strMessage.size() && CMPlayerAppBase::GetMainWnd())
		CMPlayerAppBase::GetInstance()->MessageOut(retLyrics.strMessage.c_str());

	if (listSearch.size() != retLyrics.listLyricsInfo.size())
	{
		ASSERT(0 && "Invalid protocol.");
		return ERR_BAD_MSG;
	}

	int i = 0;
	for (ListLyricsInfoLite::iterator it = retLyrics.listLyricsInfo.begin();
		it != retLyrics.listLyricsInfo.end() && i < (int)listSearch.size(); ++it, i++)
	{
		MLSearchItem	&search = listSearch[i];
		MLLyricsInfoLite &lyr = *it;
		if (lyr.bufLyrContent.empty())
		{
			if (pNotify)
				pNotify->OnNoLyricsReturned(i);

			g_LyricSearch.AssociateLyrics(search.strMediaFile.c_str(), NONE_LYRCS);
		}
		else
		{
			int nRet = g_LyricsDownloader.SaveDownloadedLyrics(search.strMediaFile.c_str(), lyr.strFile.c_str(), lyr.bufLyrContent.c_str(), lyr.bufLyrContent.size());
			if (pNotify)
			{
				pNotify->OnLyricsReturned(i, lyr.bufLyrContent);
				if (nRet != ERR_OK)
					pNotify->OnErrorSaveLyrics(i, ERROR2STR_LOCAL(nRet));
			}
		}
	}

	return nRet;
}

int COnlineSearch::UpdateSearchLrcInfo(LPCTSTR szFileAssociateKeyword, MLMsgRetSearch &retSearch)
{
	if (IsEmptyString(szFileAssociateKeyword))
		return ERR_NOT_FOUND;

	CMutexAutolock	autolock(m_mutexDB);

	m_dbSearchResultMem.UpdateSearchLrcInfo(szFileAssociateKeyword, retSearch);

	if (!m_dbSearchResult.IsOpened())
		m_dbSearchResult.Open();

	if (m_dbSearchResult.IsOpened())
		m_dbSearchResult.UpdateSearchLrcInfo(szFileAssociateKeyword, retSearch);

	return ERR_OK;
}

bool COnlineSearch::IsTitleExcludedFromAutoSearch(LPCTSTR szTitle)
{
	TCHAR		szTitleFiltered[MAX_PATH];
	int			i;
	static LPCTSTR szWithoutWords[] = { _T("www."), _T(".net"), _T(".com"), _T("http://") };

	// 过滤掉包含网址的：www. .net .com http://
	for (i = 0; i < CountOf(szWithoutWords); i++)
	{
		if (stristr(szTitle, szWithoutWords[i]))
			return true;
	}

	FilterChars(szTitleFiltered, MAX_PATH, szTitle, true);

	if (IsEmptyString(szTitleFiltered))
		return true;

	static LPCTSTR szExcludedTitle[] = { _T("track"), _T("music"), _T("title"), _T("cd"), _T("record"), _T("movie"), _T("wav") };

	for (i = 0; i < CountOf(szExcludedTitle); i++)
	{
		if (_tcscmp(szExcludedTitle[i], szTitleFiltered) == 0)
			return true;
		if (_tcscmp(_TL(szExcludedTitle[i]), szTitleFiltered) == 0)
			return true;
	}

	return false;
}
