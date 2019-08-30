#include "StdAfx.h"
#include "MLServer.h"
#include "MLHelperFun.h"
#include "LyricsDB.h"
#include "../../MLLib/LyricsKeywordFilter.h"


/*
 CREATE TABLE IF NOT EXISTS lyrics (
 id integer primary key AUTOINCREMENT, content_type integer, media_length integer,
 rate_total integer, rate_count integer, uploader_id integer, dl_count integer,
 upload_time integer, artist text, artistcmp text, album text, title text,
 titlecmp text, related_link text, edited_by text
*/

#define SELECT_LYR_FIELDS "SELECT artist, title, album, edited_by, related_link, media_length, rate_count, dl_count, rate_total, uploader_id, id, content_type"

#define SQL_SEARCH_LYR_BY_ARTIST_TITLE_ID    SELECT_LYR_FIELDS " FROM lyrics WHERE id = ? AND artistcmp = ? AND titlecmp = ?"
#define SQL_SEARCH_LYR_BY_ARTIST    SELECT_LYR_FIELDS " FROM lyrics WHERE artistcmp = ? LIMIT 50"
#define SQL_SEARCH_LYR_BY_TITLE    SELECT_LYR_FIELDS " FROM lyrics WHERE titlecmp = ? LIMIT 50"
#define SQL_SEARCH_LYR_BY_ARTIST_TITLE    SELECT_LYR_FIELDS " FROM lyrics WHERE artistcmp = ? and titlecmp = ? LIMIT 50"
#define SQL_SEARCH_LYR_BY_ARTIST_TITLE_CONTENT_TYPE SELECT_LYR_FIELDS " FROM lyrics WHERE artistcmp = ? and titlecmp = ? and content_type = ? LIMIT 50"

#define SQL_ADD_LYRICS      "INSERT INTO lyrics (artist, title, album, edited_by, related_link, media_length, rate_count, dl_count, rate_total, uploader_id, content_type, upload_time, artistcmp, titlecmp) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"

#define SQL_UPDATE_LYRICS_LINK_BY_ID "UPDATE lyrics SET related_link = ? WHERE id = ?"
#define SQL_UPDATE_LYRICS_PROPS_BY_ID "UPDATE lyrics SET artist = ?, title = ?, album = ?, media_length = ?, upload_time = ?  WHERE id = ?"


LyricsDB	g_LyricsDB;

//////////////////////////////////////////////////////////////////////////

CLyricsInfo::CLyricsInfo()
{
	nLyrContentType = LCT_LRC;
	nUniID = 0;
	wMediaLength = 0;
	nUploadUserID = 0;
	nRateTotal = 0;
	nRateCount = 0;
	nDlCount = 0;
	nUploadDate = 0;
}

void CLyricsInfo::SetInfo(LyricsProperties &lyrProperties, long nUploaderId, LPCTSTR szUploader)
{
	nDlCount = nRateCount = nRateTotal = 0;
	nUploadDate = time(NULL);

	stringex	strFiltered;

	// artist
	CLyricsKeywordFilter::Filter(lyrProperties.m_strArtist.c_str(), strFiltered);
	strAr = lyrProperties.m_strArtist;
	strArCmp = strFiltered.c_str();

	// album
	strAl = lyrProperties.m_strAlbum;

	// title
	CLyricsKeywordFilter::Filter(lyrProperties.m_strTitle.c_str(), strFiltered);
	strTi = lyrProperties.m_strTitle;
	strTiCmp = strFiltered.c_str();

	if (szUploader && !IsEmptyString(szUploader))
		strEditedBy = szUploader;
	else
		strEditedBy = lyrProperties.m_strBy;
	wMediaLength = lyrProperties.GetMediaLengthInt();

	nLyrContentType = lyrProperties.m_lyrContentType;
	ASSERT(nLyrContentType == LCT_LRC || nLyrContentType == LCT_TXT);

	this->nUploadUserID = nUploaderId;
}

//////////////////////////////////////////////////////////////////////////



void SqlGetLyrics(sqlite3_stmt *sqlStmt, RetLyrInfo &lyricsInfo) {
    int n = 0;
    const char *text;
    int rateTotal;

    // lyricsInfo. = sqlite3_column_int(sqlStmt, n++);
    SQLITE3_GET_TEXT_FIELD(sqlStmt, lyricsInfo.strArtist);
    SQLITE3_GET_TEXT_FIELD(sqlStmt, lyricsInfo.strTitle);
    SQLITE3_GET_TEXT_FIELD(sqlStmt, lyricsInfo.strAlbum);
    SQLITE3_GET_TEXT_FIELD(sqlStmt, lyricsInfo.strUploader);
    SQLITE3_GET_TEXT_FIELD(sqlStmt, lyricsInfo.strLink);

    SQLITE3_GET_INT_FIELD(sqlStmt, lyricsInfo.nTimeLength);
    SQLITE3_GET_INT_FIELD(sqlStmt, lyricsInfo.nRateCount);
    SQLITE3_GET_INT_FIELD(sqlStmt, lyricsInfo.nDownloads);
    SQLITE3_GET_INT_FIELD(sqlStmt, rateTotal);
    SQLITE3_GET_INT_FIELD(sqlStmt, lyricsInfo.uploaderID);
    SQLITE3_GET_INT_FIELD(sqlStmt, lyricsInfo.lyricsID);
    SQLITE3_GET_INT_FIELD(sqlStmt, lyricsInfo.contentType);

    if (lyricsInfo.nRateCount) {
        lyricsInfo.fRate = rateTotal / lyricsInfo.nRateCount;
    }
}


//////////////////////////////////////////////////////////////////////

LyricsDB::LyricsDB()
{
}

LyricsDB::~LyricsDB()
{
}

int LyricsDB::Init(const string &dataPath)
{
    string fn = dataPath + "data/dxTitleAlias.xml";
	if (LoadTitleAliasDb(fn.c_str()) != ERR_OK) {
		printf("LoadTitleAlias file: %s FAILED!", fn.c_str());
	}

	// Alias artist
	fn = dataPath + "data/dxArtistAlias.xml";
	if (LoadArtistAliasDb(fn.c_str()) != ERR_OK) {
		printf("LoadArtistAlias file: %s FAILED!", fn.c_str());
	}

    string fileName = dataPath + "/database/lyrics.db";
    int ret = sqlite3_open(fileName.c_str(), &m_db);
    if (ret != SQLITE_OK) {
        printf("Open user db FAILED: %s", fileName.c_str());
        return ERR_FALSE;
    }

    SQLIT3_STMT_PREPARE(m_db, SQL_ADD_LYRICS, m_stmtAddLyrics);
    SQLIT3_STMT_PREPARE(m_db, SQL_UPDATE_LYRICS_LINK_BY_ID, m_stmtUpdateLyrLinkById);
    SQLIT3_STMT_PREPARE(m_db, SQL_UPDATE_LYRICS_PROPS_BY_ID, m_stmtUpdateLyrPropsById);

    SQLIT3_STMT_PREPARE(m_db, SQL_SEARCH_LYR_BY_ARTIST_TITLE_ID, m_stmtSearchLyrByArtistTitleId);
    SQLIT3_STMT_PREPARE(m_db, SQL_SEARCH_LYR_BY_ARTIST, m_stmtSearchLyrByArtist);
    SQLIT3_STMT_PREPARE(m_db, SQL_SEARCH_LYR_BY_TITLE, m_stmtSearchLyrByTitle);
    SQLIT3_STMT_PREPARE(m_db, SQL_SEARCH_LYR_BY_ARTIST_TITLE, m_stmtSearchLyrByArtistTitle);
    SQLIT3_STMT_PREPARE(m_db, SQL_SEARCH_LYR_BY_ARTIST_TITLE_CONTENT_TYPE, m_stmtSearchLyrByArtistTitleContentType);

	return ERR_OK;
}

void LyricsDB::Quit()
{
}

int LyricsDB::AddLyrics(CLyricsInfo &lyrInfo) {
    int ret = ERR_OK, n = 1;

    sqlite3_reset(m_stmtAddLyrics);

    SQLITE3_BIND_TEXT(m_db, m_stmtAddLyrics, lyrInfo.strAr.c_str());
    SQLITE3_BIND_TEXT(m_db, m_stmtAddLyrics, lyrInfo.strTi.c_str());
    SQLITE3_BIND_TEXT(m_db, m_stmtAddLyrics, lyrInfo.strAl.c_str());
    SQLITE3_BIND_TEXT(m_db, m_stmtAddLyrics, lyrInfo.strEditedBy.c_str());
    SQLITE3_BIND_TEXT(m_db, m_stmtAddLyrics, lyrInfo.strRelatedHttpLink.c_str());

    SQLITE3_BIND_INT(m_db, m_stmtAddLyrics, lyrInfo.wMediaLength);
    SQLITE3_BIND_INT(m_db, m_stmtAddLyrics, (int)lyrInfo.nRateCount);
    SQLITE3_BIND_INT(m_db, m_stmtAddLyrics, (int)lyrInfo.nDlCount);
    SQLITE3_BIND_INT(m_db, m_stmtAddLyrics, (int)lyrInfo.nRateTotal);
    SQLITE3_BIND_INT(m_db, m_stmtAddLyrics, (int)lyrInfo.nUploadUserID);
    SQLITE3_BIND_INT(m_db, m_stmtAddLyrics, lyrInfo.nLyrContentType);
    SQLITE3_BIND_INT(m_db, m_stmtAddLyrics, (int)lyrInfo.nUploadDate);

    SQLITE3_BIND_TEXT(m_db, m_stmtAddLyrics, lyrInfo.strArCmp.c_str());
    SQLITE3_BIND_TEXT(m_db, m_stmtAddLyrics, lyrInfo.strTiCmp.c_str());

    ret = sqlite3_step(m_stmtAddLyrics);
    if (ret == SQLITE_ERROR) {
        LogSqlite3Error(m_db);
        ret = ERR_FALSE;
    } else {
        lyrInfo.nUniID = sqlite3_last_insert_rowid(m_db);
        ret = ERR_OK;
    }

    sqlite3_reset(m_stmtAddLyrics);

    return ret;
}

int LyricsDB::UpdateLyricsPropById(long nId, LyricsProperties &lyrInfo)
{
    int ret = ERR_OK, n = 1;
    
    sqlite3_reset(m_stmtUpdateLyrPropsById);
    
    SQLITE3_BIND_TEXT(m_db, m_stmtUpdateLyrPropsById, lyrInfo.m_strArtist.c_str());
    SQLITE3_BIND_TEXT(m_db, m_stmtUpdateLyrPropsById, lyrInfo.m_strTitle.c_str());
    SQLITE3_BIND_TEXT(m_db, m_stmtUpdateLyrPropsById, lyrInfo.m_strAlbum.c_str());

    SQLITE3_BIND_INT(m_db, m_stmtUpdateLyrPropsById, lyrInfo.GetMediaLengthInt());
    SQLITE3_BIND_INT(m_db, m_stmtUpdateLyrPropsById, (int)time(NULL));
    
    SQLITE3_BIND_INT(m_db, m_stmtUpdateLyrPropsById, (int)nId);

    ret = sqlite3_step(m_stmtUpdateLyrPropsById);
    if (ret == SQLITE_ERROR) {
        LogSqlite3Error(m_db);
        ret = ERR_FALSE;
    } else {
        ret = ERR_OK;
    }
    
    sqlite3_reset(m_stmtUpdateLyrPropsById);
    
    return ret;
}

int LyricsDB::UpdateLyricsLinkById(long nId, LPCTSTR szLink)
{
    int ret = ERR_OK, n = 1;
    
    sqlite3_reset(m_stmtUpdateLyrLinkById);
    
    SQLITE3_BIND_TEXT(m_db, m_stmtUpdateLyrLinkById, szLink);
    
    SQLITE3_BIND_INT(m_db, m_stmtUpdateLyrLinkById, (int)nId);

    ret = sqlite3_step(m_stmtUpdateLyrLinkById);
    if (ret == SQLITE_ERROR) {
        LogSqlite3Error(m_db);
        ret = ERR_FALSE;
    } else {
        ret = ERR_OK;
    }

    sqlite3_reset(m_stmtUpdateLyrLinkById);
    
    return ret;
}

// Search lyrics by id
int LyricsDB::SearchLyricsByArtistTitleID(LPCTSTR szArtist, LPCTSTR szTitle, long nID, RetLyrInfo &lyrInfo)
{
    int ret = ERR_OK, n = 1;
    
    sqlite3_reset(m_stmtSearchLyrByArtistTitleId);
    
    SQLITE3_BIND_INT(m_db, m_stmtSearchLyrByArtistTitleId, (int)nID);
    SQLITE3_BIND_TEXT(m_db, m_stmtSearchLyrByArtistTitleId, szArtist);
    SQLITE3_BIND_TEXT(m_db, m_stmtSearchLyrByArtistTitleId, szTitle);

    ret = sqlite3_step(m_stmtSearchLyrByArtistTitleId);
    if (ret == SQLITE_ERROR) {
        LogSqlite3Error(m_db);
        ret = ERR_FALSE;
    } else if (ret == SQLITE_ROW) {
        SqlGetLyrics(m_stmtSearchLyrByArtistTitleId, lyrInfo);
        ret = ERR_OK;
    } else {
        ret = ERR_NOT_FOUND;
    }

    sqlite3_reset(m_stmtSearchLyrByArtistTitleId);

    return ret;
}

// Search lyrics by artist and title
int LyricsDB::SearchLyricsByArtistTitle(LPCTSTR szArtist, LPCTSTR szTitle, RetLyrInfoList &vLyrics) {
    int ret = ERR_OK, n = 1;

    sqlite3_reset(m_stmtSearchLyrByArtistTitle);

    SQLITE3_BIND_TEXT(m_db, m_stmtSearchLyrByArtistTitle, szArtist);
    SQLITE3_BIND_TEXT(m_db, m_stmtSearchLyrByArtistTitle, szTitle);
    
    return getSearchLyricsResults(m_stmtSearchLyrByArtistTitle, vLyrics);
}

// Search lyrics by artist and title
int LyricsDB::SearchLyricsByArtistTitleContentType(LPCTSTR szArtist, LPCTSTR szTitle, RetLyrInfoList &vLyrics, uint32 contentType) {
    int ret = ERR_OK, n = 1;
    
    sqlite3_reset(m_stmtSearchLyrByArtistTitleContentType);

    SQLITE3_BIND_TEXT(m_db, m_stmtSearchLyrByArtistTitleContentType, szArtist);
    SQLITE3_BIND_TEXT(m_db, m_stmtSearchLyrByArtistTitleContentType, szTitle);
    SQLITE3_BIND_INT(m_db, m_stmtSearchLyrByArtistTitleContentType, contentType);

    return getSearchLyricsResults(m_stmtSearchLyrByArtistTitleContentType, vLyrics);
}

int LyricsDB::SearchLyricsByTitle(LPCTSTR szTitle, RetLyrInfoList &vLyrics) {
    int ret = ERR_OK, n = 1;
    
    sqlite3_reset(m_stmtSearchLyrByTitle);

    SQLITE3_BIND_TEXT(m_db, m_stmtSearchLyrByTitle, szTitle);
    
    return getSearchLyricsResults(m_stmtSearchLyrByTitle, vLyrics);
}

int LyricsDB::SearchLyricsByArtist(LPCTSTR szArtist, RetLyrInfoList &vLyrics) {
    int ret = ERR_OK, n = 1;
    
    sqlite3_reset(m_stmtSearchLyrByArtist);

    SQLITE3_BIND_TEXT(m_db, m_stmtSearchLyrByArtist, szArtist);
    
    return getSearchLyricsResults(m_stmtSearchLyrByArtist, vLyrics);
}

LPCTSTR LyricsDB::GetAliasTitleStr(LPCSTR szStrOrg)
{
	MAP_STR::iterator		it;
	it = m_mapTiAlias.find(szStrOrg);
	if (it != m_mapTiAlias.end())
		return (*it).second.c_str();
	else
		return NULL;
}

LPCTSTR LyricsDB::GetAliasArtistStr(LPCSTR szStrOrg)
{
	MAP_STR::iterator		it;
	it = m_mapArAlias.find(szStrOrg);
	if (it != m_mapArAlias.end())
		return (*it).second.c_str();
	else
		return NULL;
}

void LyricsDB::SetAliasTitle(LPCSTR szTitleOrg, LPCTSTR szTitleAlias)
{
	m_mapTiAlias[szTitleOrg] = szTitleAlias;
}

void LyricsDB::SetAliasArtist(LPCSTR szArtistOrg, LPCTSTR szArtistAlias)
{
	m_mapArAlias[szArtistOrg] = szArtistAlias;
}

int LyricsDB::LoadTitleAliasDb(LPCTSTR szFile)
{
	CSimpleXML		xml;
	stringex	strOrg, strAlias;

	if (!xml.ParseFile(szFile))
	{
		ERRLOG1("Parse xml file: %s, FAILED!", szFile);
		return ERR_PARSE_XML;
	}
	if (strcmp(xml.m_pRoot->strName.c_str(), "stralias") != 0)
		return ERR_FALSE;

	for (SXNode::iterator it = xml.m_pRoot->listChildren.begin(); it != xml.m_pRoot->listChildren.end(); it++)
	{
		SXNode	*pNode;
		pNode = *it;
		if (strcmp(pNode->strName.c_str(), "item") == 0)
		{
			const char	*szText;
			szText = pNode->GetProperty("org");
			if (szText)
			{
				CLyricsKeywordFilter::Filter(szText, strOrg);
				CLyricsKeywordFilter::Filter(pNode->GetPropertySafe("alias"), strAlias);
				SetAliasTitle(strOrg.c_str(), strAlias.c_str());
			}
		}
	}

	return ERR_OK;
}

int LyricsDB::LoadArtistAliasDb(LPCTSTR szFile)
{
	CSimpleXML		xml;
	stringex	strOrg, strAlias;

	if (!xml.ParseFile(szFile))
	{
		ERRLOG1("Parse xml file: %s, FAILED!", szFile);
		return ERR_PARSE_XML;
	}
	if (strcmp(xml.m_pRoot->strName.c_str(), "stralias") != 0)
		return ERR_FALSE;

	for (SXNode::iterator it = xml.m_pRoot->listChildren.begin(); it != xml.m_pRoot->listChildren.end(); it++)
	{
		SXNode	*pNode;
		pNode = *it;
		if (strcmp(pNode->strName.c_str(), "item") == 0)
		{
			const char	*szText;
			szText = pNode->GetProperty("org");
			if (szText)
			{
				CLyricsKeywordFilter::Filter(szText, strOrg);
				CLyricsKeywordFilter::Filter(pNode->GetPropertySafe("alias"), strAlias);
				SetAliasArtist(strOrg.c_str(), strAlias.c_str());
			}
		}
	}

	return ERR_OK;
}

int LyricsDB::getSearchLyricsResults(sqlite3_stmt *stmt, RetLyrInfoList &vLyrics) {
    int ret;

    while (true) {
        ret = sqlite3_step(stmt);
        if (ret == SQLITE_ERROR) {
            LogSqlite3Error(m_db);
            ret = ERR_FALSE;
            break;
        } else if (ret == SQLITE_ROW) {
            RetLyrInfo lyricsInfo;
            SqlGetLyrics(stmt, lyricsInfo);
            vLyrics.push_back(lyricsInfo);
        } else {
            ret = ERR_OK;
            break;
        }
    }
    
    sqlite3_reset(stmt);
    
    return ret;
}
