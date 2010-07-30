// OnlineSearch.cpp: implementation of the COnlineSearch class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "../sqlite_cpp/Sqlite3.h"
#include "OnlineSearch.h"
#include "DownloadMgr.h"
#include "AdManage.h"
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

void MLFileInfoListToVResult(LPCTSTR szArtist, LPCTSTR szTitle, MLFileInfoList &filelist, V_LRCSEARCHRESULT &vResult)
{
	BOOL		bIgnoreTitleNumb, bIgnoreArtistNumb;
	bIgnoreTitleNumb = IsIgnoreNumber(szTitle);
	bIgnoreArtistNumb = IsIgnoreNumber(szArtist);

	for (MLFileInfoList::iterator it = filelist.begin(); it != filelist.end(); it++)
	{
		MLFileInfo		&file = *it;
		LrcSearchResult		item;
		TCHAR	szLyricsName[MAX_PATH];

		_stprintf_s(szLyricsName, CountOf(szLyricsName), _T("%s - %s"), file.strArtist.c_str(), file.strTitle.c_str());
		item.strSaveFileName = szLyricsName;
		item.strUrl = file.strLink;
		item.strSaveFileName = file.strFileName;
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
		if (g_bEngEdition)
		{
			// Sort English result before other language.
			if (IsAnsiStr(item.strSaveFileName.c_str()))
			{
				item.nMatchValue++;
			}
		}

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

void ILyrSearchResultCacheDB::ResultToXML(MLFileInfoList &fileList, CXMLWriter &xmlWriter)
{
	xmlWriter.WriteStartElement(_T("SearchResults"));

	for (list<MLFileInfo>::iterator it = fileList.begin(); it != fileList.end(); it++)
	{
		(*it).ToXML(xmlWriter);
	}

	xmlWriter.WriteEndElement();
}

void ILyrSearchResultCacheDB::ResultFromXML(MLFileInfoList &fileList, LPCSTR szXML)
{
	CSimpleXML	xml;
	SXNode		*pNode;

	if (!xml.ParseData((void*)szXML, strlen(szXML)))
		return;

	pNode = xml.m_pRoot;
	if (_tcscmp(pNode->strName.c_str(), _T("SearchResults")) != 0)
		return;

	SXNode::iterator	it;
	for (it = pNode->listChildren.begin(); it != pNode->listChildren.end(); it++)
	{
		MLFileInfo	Info;
		Info.FromXML(*it);
		fileList.push_back(Info);
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

	CSimpleXML		xml;
	MLFileInfoList	result;

	if (pdwSearchTime)
		*pdwSearchTime = sqlQuery.ColumnInt(1);

	if (pvResult)
	{
		LPCSTR	str = sqlQuery.ColumnText(2);
		if (str)
		{
			ResultFromXML(result, str);

			MLFileInfoListToVResult(szArtist, szTitle, result, *pvResult);
		}
	}

	return TRUE;
}

bool CLyrSearchResultCacheDB::UpdateSearchLrcInfo(LPCTSTR szFileAssociateKeyword, MLFileInfoList &fileList)
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
	ResultToXML(fileList, xml);

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
	sqlAdd.BindStaticText(n++, xml.GetData(), xml.GetLength());

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
			CSimpleXML		xml;
			MLFileInfoList	result;

			if (pdwSearchTime)
				*pdwSearchTime = GetCurDaysFrom2000();

			if (pvResult)
			{
				LPCSTR	str = item.strXml.c_str();
				if (str)
				{
					ResultFromXML(result, str);

					MLFileInfoListToVResult(szArtist, szTitle, result, *pvResult);
				}
			}

			return true;
		}
	}

	return false;
}

bool CLyrSearchResultCacheMemDB::UpdateSearchLrcInfo(LPCTSTR szFileAssociateKeyword, MLFileInfoList &fileList)
{
	LIST_ITEMS::iterator	it, itEnd;
	CXMLWriter				xml;

	ResultToXML(fileList, xml);

	if (m_listResults.size() >= 5)
		m_listResults.pop_front();

	itEnd = m_listResults.end();
	for (it = m_listResults.begin(); it != itEnd; ++it)
	{
		ITEM	&item = *it;
		if (_tcscmp(item.strKeyword.c_str(), szFileAssociateKeyword) == 0)
		{
			item.strXml = xml.GetData();
			return true;
		}
	}

	ITEM	item;

	item.strKeyword = szFileAssociateKeyword;
	item.strXml = xml.GetData();

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
	m_dbSearchResult.Close();
	m_dbSearchResultMem.Close();
}

BOOL COnlineSearch::SearchCacheForCur(V_LRCSEARCHRESULT *pvResult, DWORD *pdwSearchTime)
{
	TCHAR		szKeyword[MAX_PATH];

	g_mpApp.GetAssociateFileKeyword(szKeyword, CountOf(szKeyword));

	CMutexAutolock	autolock(m_mutexDB);

	if (m_dbSearchResultMem.SearchCache(szKeyword, g_Player.GetArtist(), g_Player.GetTitle(), pvResult, pdwSearchTime))
		return true;

	if (!m_dbSearchResult.IsOpened())
		m_dbSearchResult.Open();

	if (m_dbSearchResult.IsOpened())
		return m_dbSearchResult.SearchCache(szKeyword, g_Player.GetArtist(), g_Player.GetTitle(), pvResult, pdwSearchTime);

	return FALSE;
}

void COnlineSearch::ThreadSearchOnline(LPVOID lpParam)
{
	int							nRet;
	CMLClientSession		session;
	MLMsgRetSearch			retSearch;

	COnlineSearchThreadParam	*pSearchParam = (COnlineSearchThreadParam *)lpParam;
	ASSERT(pSearchParam);

	pSearchParam->pOnlineSearch->m_bRunning = true;

	session.Init(SZ_ML_VERSION);

	nRet = session.Connect();
	if (nRet != ERR_OK)
	{
		tstring		str;
		str += TOLOCAL(_T("Failed to search lyrics with error:"));
		str += _T(" ");
		str += LPCTSTR(ERROR2STR_LOCAL(nRet));
		g_eventDispather.DispatchLongErrorText(str.c_str());

		pSearchParam->pOnlineSearch->m_bRunning = false;
		delete pSearchParam;
		return;
	}

	nRet = session.Search(pSearchParam->strArtist.c_str(), pSearchParam->strTitle.c_str(), 0, retSearch);//listResult, bInvalidRegCode, strMsg);
	session.Close();

	if (retSearch.RegVerifyValue != RVV_OK)
	{
		// 注册码无效
		if (retSearch.RegVerifyValue == RVV_TRIAL)
			g_LicenseType = LT_TRIAL;
		else
			g_LicenseType = LT_CRACK;
		TCHAR		szRegCode[MAX_PATH];
		g_Profile.EncryptGetProfileString(SZ_REGCODE, _T(""), szRegCode, CountOf(szRegCode));
		g_Profile.EncryptWriteProfileString(SZ_INVALID_RC, szRegCode);
		g_Profile.EncryptWriteProfileString(SZ_REGCODE, _T(""));
		g_AdManage.CheckNewAd();
	}

	if (retSearch.strMessage.size() && g_mpApp.GetMainWnd())
		g_mpApp.MessageOut(retSearch.strMessage.c_str());

	if (retSearch.listResultFiles.size() > 0 || nRet == ERR_NOT_FOUND)
	{
		pSearchParam->pOnlineSearch->UpdateSearchLrcInfo(pSearchParam->szAssociateKeyword, retSearch.listResultFiles);

		//
		// 通知给调用者
		IEvent		*pEvent = new IEvent;
		pEvent->eventType = ET_LYRICS_SEARCH_END;
		g_eventDispather.DispatchUnsyncEvent(pEvent);
	}
	else
	{
		tstring			str;
		str += TOLOCAL(_T("Failed to search lyrics with error:"));
		str += _T(" ");
		str += (LPCTSTR)ERROR2STR_LOCAL(nRet);
		g_eventDispather.DispatchLongErrorText(str.c_str());
	}

	pSearchParam->pOnlineSearch->m_bRunning = false;
	delete pSearchParam;
}

// 创建一个线程搜索歌词，线程执行完毕之后会通知目标
BOOL COnlineSearch::AddSearchJob(LPCTSTR szArtist, LPCTSTR szTitle)
{
	COnlineSearchThreadParam		*pParam;
	DWORD							dwThreadId = 0;

	// LOG0(LOG_LVL_EVENT_IMP, _T("Enter Add search job proc"));

	if (m_bRunning)
	{
		// LOG0(LOG_LVL_EVENT_IMP, _T("Another search job is running, quit."));
		return FALSE;
	}

	pParam = new COnlineSearchThreadParam;
	pParam->strArtist = szArtist;
	pParam->strTitle = szTitle;
	g_mpApp.GetAssociateFileKeyword(pParam->szAssociateKeyword, CountOf(pParam->szAssociateKeyword));
	pParam->pOnlineSearch = this;

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
	// LOG0(LOG_LVL_EVENT_IMP, _T("Enter auto search proc"));

	// 歌词为空, 小于3个字符
	if (_tcslen(szTitle) < 3)
	{
		// LOG1(LOG_LVL_EVENT_IMP, _T("Title is : %s, quit."), sztitleNew);
		return FALSE;
	}

	if (IsTitleExcludedFromAutoSearch(szTitle))
	{
		// LOG1(LOG_LVL_EVENT_IMP, _T("Title is : %s, quit."), sztitleNew);
		return FALSE;
	}

	//
	// 检查上次自动搜索相隔时间小于1个月才搜索
	DWORD		dwLatestSearchTime;
	bool		bAdvVersion = false;
	if (g_bEngEdition || IsRegistered())
		bAdvVersion = true;
	if (SearchCacheForCur(NULL, &dwLatestSearchTime))
	{
		int			nNextSearch = 30;
		if (bAdvVersion && !g_LyricData.HasLyricsOpened())
			nNextSearch = 10;

		if (abs((int)dwLatestSearchTime - GetCurDaysFrom2000()) < nNextSearch)
		{
			// ....todo 添加上次搜索时间的介绍, 优化离线歌词搜索库和在下歌词搜索缓存
// 			tstring			str;
// 			CStrPrintf		str2(TOLOCAL(_T("Searched lyrics %d days ago, no results returned.")), abs((int)dwLatestSearchTime - GetCurDaysFrom2000()));
// 			str = str2.c_str();
// 			str += _T(" ");
// 			str += TOLOCAL(_T("Auto search canceled."));
// 			g_eventDispather.DispatchLongErrorText(str.c_str());
			return FALSE;
		}
	}

	//
	// 两次调用自动搜索的时间不能够超过10秒
	static DWORD		dwLastSearch = GetSecCount() - 120;
	if (int(GetSecCount() - dwLastSearch) < 
		(bAdvVersion ? 2 : 10))
	{
		// LOG1(LOG_LVL_EVENT_IMP, _T("Lastest auto search is about :%d s ago, quit."), GetSecCount() - dwLastSearch);
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
			// LOG1(LOG_LVL_EVENT_IMP, _T("Auto search is too frequent: %d"), dwSearchCount);
			return FALSE;
		}
		// 增加搜索次数
		dwSearchCount++;
	}

	// 更新上次调用自动搜索的时间
	dwLastSearch = GetSecCount();

	tstring			str;
	str = TOLOCAL(_T("Searching lyrics:"));
	str += _T(" ");
	str += g_Player.GetFullTitle();
	g_eventDispather.DispatchLongErrorText(str.c_str());

	return AddSearchJob(szArtist, szTitle);
}

int COnlineSearch::SearchOnline(CMLClientSession &session, LPCTSTR szArtist, LPCTSTR szTitle, V_LRCSEARCHRESULT &vResult, tstring &strMsg, int &nCurPage, int &nPageCount)
{
	int					nRet;
	TCHAR				szKeyword[MAX_PATH];

	session.Init(SZ_ML_VERSION);

	nRet = session.Connect();
	if (nRet != ERR_OK)
		return nRet;

	MLMsgRetSearch	retSearch;

	nRet = session.Search(szArtist, szTitle, nCurPage, retSearch);
	session.Close();

	if (retSearch.RegVerifyValue)
	{
		// 注册码无效
		if (retSearch.RegVerifyValue == RVV_TRIAL)
			g_LicenseType = LT_TRIAL;
		else
			g_LicenseType = LT_CRACK;
		TCHAR		szRegCode[MAX_PATH];
		g_Profile.EncryptGetProfileString(SZ_REGCODE, _T(""), szRegCode, CountOf(szRegCode));
		g_Profile.EncryptWriteProfileString(SZ_INVALID_RC, szRegCode);
		g_Profile.EncryptWriteProfileString(SZ_REGCODE, _T(""));
		g_AdManage.CheckNewAd();
	}

	if (retSearch.strMessage.size() && g_mpApp.GetMainWnd())
		g_mpApp.MessageOut(retSearch.strMessage.c_str());

	nCurPage = retSearch.nCurPage;
	nPageCount = retSearch.nPageCount;

	if (retSearch.listResultFiles.size() > 0)
	{
		g_mpApp.GetAssociateFileKeyword(szKeyword, CountOf(szKeyword));
		UpdateSearchLrcInfo(szKeyword, retSearch.listResultFiles);

		MLFileInfoListToVResult(szArtist, szTitle, retSearch.listResultFiles, vResult);

		return ERR_OK;
	}

	return nRet;
}

int COnlineSearch::UpdateSearchLrcInfo(LPCTSTR szFileAssociateKeyword, MLFileInfoList &fileList)
{
	if (IsEmptyString(szFileAssociateKeyword))
		return ERR_NOT_FOUND;

	CMutexAutolock	autolock(m_mutexDB);

	m_dbSearchResultMem.UpdateSearchLrcInfo(szFileAssociateKeyword, fileList);

	if (!m_dbSearchResult.IsOpened())
		m_dbSearchResult.Open();

	if (m_dbSearchResult.IsOpened())
		m_dbSearchResult.UpdateSearchLrcInfo(szFileAssociateKeyword, fileList);

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
		if (_tcscmp(TOLOCAL(szExcludedTitle[i]), szTitleFiltered) == 0)
			return true;
	}

	return false;
}
