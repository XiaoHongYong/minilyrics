// MediaLibrary.cpp: implementation of the CMediaLibrary class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayer.h"
#include "MediaLibrary.h"
#include "SqliteHelper.h"

#define MEDIA_LIB_VER_CUR		1


/*

CREATE TABLE lib_ver
(
  version integer Primary Key,
);


CREATE TABLE IF NOT EXISTS medialib
(
  id integer Primary Key,
  url text COLLATE NOCASE,
  file_deleted integer,
  info_updated integer,
  is_user_rating integer,
  artist text COLLATE NOCASE,
  album text COLLATE NOCASE,
  title text COLLATE NOCASE,
  track integer,
  year integer,
  genre integer COLLATE NOCASE,
  comment integer COLLATE NOCASE,
  length integer,
  filesize integer,
  time_added integer,
  time_played integer,
  rating integer,
  times_played integer,
  times_play_skipped integer,
  lyrics_file text COLLATE NOCASE
);

CREATE INDEX IF NOT EXISTS mli_url on medialib (url);
CREATE INDEX IF NOT EXISTS mli_artist on medialib (artist);
CREATE INDEX IF NOT EXISTS mli_album on medialib (album);
CREATE INDEX IF NOT EXISTS mli_title on medialib (title);
CREATE INDEX IF NOT EXISTS mli_genre on medialib (genre);
CREATE INDEX IF NOT EXISTS mli_comment on medialib (comment);
CREATE INDEX IF NOT EXISTS mli_time_added on medialib (time_added);
CREATE INDEX IF NOT EXISTS mli_time_played on medialib (time_played);
CREATE INDEX IF NOT EXISTS mli_rating on medialib (rating);
CREATE INDEX IF NOT EXISTS mli_times_played on medialib (times_played);
// CREATE INDEX IF NOT EXISTS ml_ on medialib ();

  update medialib set url=?, info_updated=1, artist=?, album=?, title=?, track=?,
    year=?, genre=?, comment=?, length=?, filesize=?, lyrics_file=?

Add Media
	insert into medialib values (NULL, ?, ?, ?, ?,  ?, ?, ?, ?,  ?, ?, ?, ?,  ?, 0, 0, 0, ?);
								 id				 title        comment      time_played
*/

#define SQL_CREATE_LIB_VER		"CREATE TABLE lib_ver\
(\
  version integer Primary Key\
);"

#define SQL_INSERT_VER_NUMB		"insert into lib_ver values (%d)"

#define SQL_CREATE_MEDIALIB		"CREATE TABLE IF NOT EXISTS medialib	\
(\
  id integer Primary Key,\
  url text COLLATE NOCASE,\
  file_deleted integer,\
  info_updated integer,\
  is_user_rating integer,\
  artist text COLLATE NOCASE,\
  album text COLLATE NOCASE,\
  title text COLLATE NOCASE,\
  track integer,\
  year integer,\
  genre text COLLATE NOCASE,\
  comment text COLLATE NOCASE,\
  length integer,\
  filesize integer,\
  time_added integer,\
  time_played integer,\
  rating integer,\
  times_played integer,\
  times_play_skipped integer,\
  lyrics_file text COLLATE NOCASE\
);\
\
CREATE INDEX IF NOT EXISTS mli_url on medialib (url);\
CREATE INDEX IF NOT EXISTS mli_artist on medialib (artist);\
CREATE INDEX IF NOT EXISTS mli_album on medialib (album);\
CREATE INDEX IF NOT EXISTS mli_title on medialib (title);\
CREATE INDEX IF NOT EXISTS mli_genre on medialib (genre);\
CREATE INDEX IF NOT EXISTS mli_comment on medialib (comment);\
CREATE INDEX IF NOT EXISTS mli_time_added on medialib (time_added);\
CREATE INDEX IF NOT EXISTS mli_time_played on medialib (time_played);\
CREATE INDEX IF NOT EXISTS mli_rating on medialib (rating);\
CREATE INDEX IF NOT EXISTS mli_times_played on medialib (times_played);"

#define SQL_MAX_MEDIALIB_ID		"select MAX(id) from medialib"

#define SQL_COUNT_OF_MEDIA	"select count(*) from medialib"


#define SQL_ADD_MEDIA	"insert into medialib values (NULL, ?, 0, 1, 0, ?, ?,  ?, ?, ?, ?,  ?, ?, ?, ?,  -1, 300, 0, 0, ?);"

#define SQL_ADD_MEDIA_FAST	"insert into medialib (id, url, file_deleted, info_updated, is_user_rating, artist, title, time_added, time_played, rating, times_played, times_play_skipped) "\
								"values (NULL, ?, 0, 0, 1, ?, ?,   ?,  -1, 300, 0, 0);"

#define SQL_QUERY_MEDIA_BY_URL	"select * from medialib where url=? "


#define SQLITE3_BIND_UCS2(sqlstmt, strData)			\
	UCS2ToUtf8(strData.c_str(), strData.size(), strUtf8);\
	nRet = sqlstmt.BindText(n, strUtf8.c_str(), strUtf8.size());\
	if (nRet != ERR_OK)\
		goto RET_FAILED;\
	n++;

CMedia *NewMedia()
{
	CMedia		*pMedia;

	pMedia = new CMedia;
	pMedia->AddRef();

	return pMedia;
}

void Utf8ToUCS2(const char *str, int nLen, CXStr &strUCS2)
{
	int			nLenWC;

	if (nLen == -1)
		nLen = strlen(str);

	strUCS2.reserve(nLen + 1);

	nLenWC = MultiByteToWideChar(CP_UTF8, 0,
		str, nLen, strUCS2.data(), strUCS2.capacity());

	strUCS2.resize(nLenWC);
}

/*
How to calculate auto rating value:

default auto rating: 300
max auto rating: 590


*/
int GetAutoRating(IMedia *pMedia)
{
	int			length, rating = 300;
	int			times_play_skipped, times_played;

	pMedia->GetAttribute(MA_DURATION, &length);
	pMedia->GetAttribute(MA_TIMES_PLAY_SKIPPED, &times_play_skipped);
	pMedia->GetAttribute(MA_TIMES_PLAYED, &times_played);

	if (length > 30 * 1000)	// x seconds
	{
		times_play_skipped -= 1;

		if (times_play_skipped < 0)
			times_play_skipped = 0;

		int		nValue = times_played - times_play_skipped * 5;
		if (nValue > 0)
			rating = 400 + nValue;
		else if (nValue < 0)
			rating = 300 + nValue;

		if (rating > 590)
			rating = 589;
		else if (rating <= 100)
			rating = 101;
	}

	return rating;
}

void AppendQeuryStatment(MediaLibOrderBy orderBy, int nTopN, string &strSql)
{
	if (orderBy == MLOB_NONE)
	{
	}
	else if (orderBy == MLOB_ARTIST)
		strSql += " order by artist";
	else if (orderBy == MLOB_ALBUM)
		strSql += " order by album";
	else if (orderBy == MLOB_GENRE)
		strSql += " order by genre";
	else if (orderBy == MLOB_ARTIST_ALBUM)
		strSql += " order by artist, album";
	else if (orderBy == MLOB_RATING)
		strSql += " order by rating desc";

	if (nTopN != -1)
	{
		char	szBuff[64];
		_itoa_s(nTopN, szBuff, CountOf(szBuff), 10);
		strSql += " limit";
		strSql += szBuff;
	}
}

//////////////////////////////////////////////////////////////////////////

class CMLQueryPlaylist
{
public:
	CMLQueryPlaylist()
	{
		m_pPlaylist = NULL;
		m_pMediaLib = NULL;
	}

	IPlaylist				*m_pPlaylist;
	CMediaLibrary			*m_pMediaLib;

	MLRESULT Query(CMediaLibrary *pMediaLib, LPCSTR szSQL)
	{
		if (!pMediaLib->IsOK())
			return pMediaLib->m_nInitResult;

		int		nRet;

		m_pMediaLib = pMediaLib;

		m_pPlaylist = m_pMediaLib->NewPlaylist();
		if (!m_pPlaylist)
			return ERR_NO_MEM;

		nRet = sqlite3_exec(
            m_pMediaLib->m_db.m_db,
            szSQL, 
            &QueryCallback, 
            this, 
            NULL);
		if (nRet != SQLITE_OK)
		{
			ERRLOG2(_T("Exe SQL:%S, : %S"), szSQL, m_pMediaLib->m_db.ErrorMsg());
			m_pPlaylist->Release();
			return ERR_SL_EXE_SQL;
		}

		return ERR_OK;
	}

	static int QueryCallback(void *args, int numCols, char **results, char ** columnNames)
	{
		CMLQueryPlaylist	*pThis = (CMLQueryPlaylist*)args;

		CMedia		*pMedia = NewMedia();

		pThis->m_pMediaLib->GetMediaCallback(pMedia, numCols, results);

		pThis->m_pPlaylist->InsertItem(-1, pMedia);

		pMedia->Release();

		return 0;
	}

};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMediaLibrary::CMediaLibrary()
{
	OBJ_REFERENCE_INIT;
}

CMediaLibrary::~CMediaLibrary()
{
	Close();
}

OBJ_REFERENCE_IMP(CMediaLibrary)

MLRESULT CMediaLibrary::GetAllArtist(IVXStr **ppvArtist)
{
	return QueryVStr("select artist from medialib group by artist", ppvArtist);
}

MLRESULT CMediaLibrary::GetAllAlbum(IVXStr **ppvAlbum)
{
	return QueryVStr("select album from medialib group by album", ppvAlbum);
}

MLRESULT CMediaLibrary::GetAllGenre(IVXStr **ppvAlbum)
{
	return QueryVStr("select genre from medialib group by genre", ppvAlbum);
}

MLRESULT CMediaLibrary::GetAllYear(IVInt **ppvYear)
{
	if (!IsOK())
		return m_nInitResult;

	*ppvYear = NULL;

	CMutexAutolock	autolock(m_mutexDataAccess);
	CSqlite3Stmt	sqlQuery;
	int				nRet;
	CVInt			*pvInt;

	sqlQuery.Prepare(&m_db, "select year from medialib group by year");

	pvInt = new CVInt();
	pvInt->AddRef();

	nRet = sqlQuery.Step();
	while (nRet == ERR_SL_OK_ROW)
	{
		pvInt->push_back(sqlQuery.ColumnInt(0));

		nRet = sqlQuery.Step();
	}
	if (pvInt->size() == 0)
	{
		pvInt->Release();
		pvInt = NULL;
		if (nRet == ERR_OK)
			nRet = ERR_NOT_FOUND;
	}
	else
		nRet = ERR_OK;

	*ppvYear = pvInt;

	return nRet;
}

MLRESULT CMediaLibrary::GetAlbumOfArtist(LPCXSTR szArtist, IVXStr **ppvAlbum)
{
	if (!IsOK())
		return m_nInitResult;

	stringex	strUtf8;
	UCS2ToUtf8(szArtist, -1, strUtf8);
	CMutexAutolock	autolock(m_mutexDataAccess);

	CSqlite3Stmt	sqlQuery;
	int				nRet;
	tstringex		strUCS2;
	CVXStr			*pvStr;
	LPCSTR			text;

	*ppvAlbum = NULL;

	nRet = sqlQuery.Prepare(&m_db, "select album from medialib where artist=? group by album");
	if (nRet != ERR_OK)
		return nRet;

	pvStr = new CVXStr();
	pvStr->AddRef();

	sqlQuery.BindStaticText(1, strUtf8.c_str());

	nRet = sqlQuery.Step();
	while (nRet == ERR_SL_OK_ROW)
	{
		text = sqlQuery.ColumnText(0);
		if (text)
		{
			Utf8ToUCS2(text, -1, strUCS2);
			pvStr->push_back(strUCS2.c_str());
		}

		nRet = sqlQuery.Step();
	}
	if (pvStr->size() == 0)
	{
		pvStr->Release();
		pvStr = NULL;
		if (nRet == ERR_OK)
			nRet = ERR_NOT_FOUND;
	}
	else
		nRet = ERR_OK;

	*ppvAlbum = pvStr;

	return nRet;
}

uint32 CMediaLibrary::GetMediaCount()
{
	return sqlite_query_int_value(m_db.m_db, SQL_COUNT_OF_MEDIA);
}

MLRESULT CMediaLibrary::GetMediaByUrl(LPCXSTR szUrl, IMedia **ppMedia)
{
	ASSERT(ppMedia);

	if (!IsOK())
		return m_nInitResult;

	*ppMedia = NULL;

	CMutexAutolock	autolock(m_mutexDataAccess);

	int			nRet;
	stringex	strUtf8;

	UCS2ToUtf8(szUrl, -1, strUtf8);

	m_sqlQueryByUrl.BindStaticText(1, strUtf8.c_str());

	nRet = m_sqlQueryByUrl.Step();
	if (nRet == ERR_SL_OK_ROW)
	{
		CMedia		*media;

		media = NewMedia();
		SqliteQueryMedia(&m_sqlQueryByUrl, media);
		*ppMedia = media;

		if (media->m_bIsFileDeleted)
		{
			// recover to undeleted.
			if (UndeleteMedia(media->GetID()) == ERR_OK)
				media->m_bIsFileDeleted = false;
		}
		nRet = ERR_OK;
	}
	else
		nRet = ERR_NOT_FOUND;

	m_sqlQueryByUrl.Reset();

	return nRet;
}

MLRESULT CMediaLibrary::Add(LPCXSTR szMediaUrl, IMedia **ppMedia)
{
	if (!IsOK())
		return m_nInitResult;

	*ppMedia = NULL;

	int				nRet;
	CMedia			*pMedia;

	nRet = GetMediaByUrl(szMediaUrl, ppMedia);
	if (nRet != ERR_NOT_FOUND)
		return nRet;

	CMutexAutolock	autolock(m_mutexDataAccess);

	pMedia = NewMedia();

	pMedia->SetSourceUrl(szMediaUrl);

	m_player->LoadMediaTagInfo(pMedia, true);

	pMedia->m_nTimePlayed.GetCurrentTime();
	pMedia->m_nTimeAdded.GetCurrentTime();

	nRet = DoAddMedia(pMedia);
	if (nRet == ERR_OK)
		*ppMedia = pMedia;
	else
		pMedia->Release();

	return ERR_OK;
}

// Add media to media library fast, didn't update media info.
MLRESULT CMediaLibrary::AddFast(LPCXSTR szMediaUrl, LPCXSTR szArtist, LPCXSTR szTitle, IMedia **ppMedia)
{
	if (!IsOK())
		return m_nInitResult;

	int			nRet;
	int			n = 1;
	stringex	strUtf8;
	CMedia		*pMedia = NULL;

	*ppMedia = NULL;

	nRet = GetMediaByUrl(szMediaUrl, ppMedia);
	if (nRet != ERR_NOT_FOUND)
		return nRet;

	CMutexAutolock	autolock(m_mutexDataAccess);

	pMedia = NewMedia();

	pMedia->SetSourceUrl(szMediaUrl);

	pMedia->m_nTimePlayed.GetCurrentTime();
	pMedia->m_nTimeAdded.GetCurrentTime();

	UCS2ToUtf8(szMediaUrl, -1, strUtf8);
	nRet = m_sqlAddFast.BindText(n++, strUtf8.c_str());
	if (nRet != ERR_OK)
		goto RET_FAILED;

	SQLITE3_BIND_UCS2(m_sqlAddFast, pMedia->m_strArtist);

	SQLITE3_BIND_UCS2(m_sqlAddFast, pMedia->m_strTitle);

	m_sqlAddFast.BindInt(n++, pMedia->m_nTimeAdded.m_time);

	nRet = m_sqlAddFast.Step();
	if (nRet != ERR_OK)
		goto RET_FAILED;

	pMedia->m_nID =  m_db.LastInsertRowId();

	m_sqlAddFast.Reset();

	*ppMedia = pMedia;

	return nRet;

RET_FAILED:
	if (pMedia)
		delete pMedia;

	return nRet;
}

// Update Media Info to DB
MLRESULT CMediaLibrary::UpdateMediaInfo(IMedia *pMedia)
{
	if (!IsOK())
		return m_nInitResult;

	ASSERT(pMedia->GetID() != MEDIA_ID_INVALID);
	if (pMedia->GetID() == MEDIA_ID_INVALID)
		return ERR_ML_NOT_FOUND;

	CMutexAutolock	autolock(m_mutexDataAccess);
	CSqlite3Stmt	sqlQuery;
	int				nRet;
	int				n = 1;
	int				nValue;
	CXStr			str;
	stringex	strUtf8;

	sqlQuery.Prepare(&m_db, "update medialib set url=?, info_updated=1, artist=?, album=?, title=?, track=?,"\
		"year=?, genre=?, comment=?, length=?, filesize=?, lyrics_file=? where id=?");

	pMedia->GetSourceUrl(&str);
	SQLITE3_BIND_UCS2(sqlQuery, str);

	pMedia->GetAttribute(MA_ARTIST, &str);
	SQLITE3_BIND_UCS2(sqlQuery, str);

	pMedia->GetAttribute(MA_ALBUM, &str);
	SQLITE3_BIND_UCS2(sqlQuery, str);

	pMedia->GetAttribute(MA_TITLE, &str);
	SQLITE3_BIND_UCS2(sqlQuery, str);

	pMedia->GetAttribute(MA_TRACK_NUMB, &nValue);
	sqlQuery.BindInt(n++, nValue);

	pMedia->GetAttribute(MA_YEAR, &nValue);
	sqlQuery.BindInt(n++, nValue);

	pMedia->GetAttribute(MA_GENRE, &str);
	SQLITE3_BIND_UCS2(sqlQuery, str);

	pMedia->GetAttribute(MA_COMMENT, &str);
	SQLITE3_BIND_UCS2(sqlQuery, str);

	pMedia->GetAttribute(MA_DURATION, &nValue);
	sqlQuery.BindInt(n++, nValue);

	pMedia->GetAttribute(MA_FILESIZE, &nValue);
	sqlQuery.BindInt(n++, nValue);

	pMedia->GetAttribute(MA_LYRICS_FILE, &str);
	SQLITE3_BIND_UCS2(sqlQuery, str);

	sqlQuery.BindInt(n++, pMedia->GetID());
	nRet = sqlQuery.Step();
	if (nRet == ERR_OK)
		pMedia->SetInfoUpdatedToMediaLib(true);

RET_FAILED:
	return nRet;
}

// removes the specified item from the media library
MLRESULT CMediaLibrary::Remove(IMedia **pMedia, bool bDeleteFile)
{
	if (!IsOK())
		return m_nInitResult;

	CMutexAutolock	autolock(m_mutexDataAccess);
	CSqlite3Stmt	sqlQuery;
	int				nRet;

	sqlQuery.Prepare(&m_db, "delete from medialib where id=?");

	sqlQuery.BindInt(1, (*pMedia)->GetID());
	nRet = sqlQuery.Step();
	if (nRet == ERR_OK)
	{
		if (bDeleteFile)
		{
			CXStr		str;
			(*pMedia)->GetSourceUrl(&str);
			DeleteFile(str.c_str());
		}

		(*pMedia)->Release();
		*pMedia = NULL;
	}

	return ERR_OK;
}

// If the media file was removed temporarily, set this flag on.
MLRESULT CMediaLibrary::SetDeleted(IMedia **pMedia)
{
	char		szSql[256];
	int			nRet;

	sprintf_s(szSql, CountOf(szSql), "update medialib set file_deleted=1 where id=%d", (*pMedia)->GetID());

	nRet = ExecuteSQL(szSql);
	if (nRet == ERR_OK)
	{
		(*pMedia)->Release();
		*pMedia = NULL;
	}

	return nRet;
}

MLRESULT CMediaLibrary::GetAll(IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN)
{
	CMLQueryPlaylist	query;
	int					nRet;
	string				strSql;

	strSql = "select * from medialib where file_deleted=0";

	AppendQeuryStatment(orderBy, nTopN, strSql);

	nRet = query.Query(this, strSql.c_str());
	*ppPlaylist = query.m_pPlaylist;

	return nRet;
}

MLRESULT CMediaLibrary::GetByArtist(LPCXSTR szArtist, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN)
{
	string				strSql;

	strSql = "select * from medialib where artist=? and file_deleted=0";
	AppendQeuryStatment(orderBy, nTopN, strSql);

	return QueryPlaylist(strSql.c_str(), szArtist, ppPlaylist);
}

MLRESULT CMediaLibrary::GetByAlbum(LPCXSTR szAlbum, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN)
{
	string				strSql;

	strSql = "select * from medialib where album=? and file_deleted=0";
	AppendQeuryStatment(orderBy, nTopN, strSql);

	return QueryPlaylist(strSql.c_str(), szAlbum, ppPlaylist);
}

MLRESULT CMediaLibrary::GetByAlbum(LPCXSTR szArtist, LPCXSTR szAlbum, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN)
{
	CMLQueryPlaylist	query;
	int					nRet;
	stringex			strArUtf8, strAlUtf8;
	string				strSql;

	UCS2ToUtf8(szArtist, -1, strArUtf8);
	UCS2ToUtf8(szAlbum, -1, strAlUtf8);

	strrep(strArUtf8, "'", "''");
	strrep(strAlUtf8, "'", "''");

	// char				szQuery[512];
	// sprintf(szQuery, "select * from medialib where artist='%s' and album='%s' and file_deleted=0", strArUtf8.c_str(), strAlUtf8.c_str());
	strSql = "select * from medialib where artist='";
	strSql += strArUtf8.c_str();
	strSql += "' and album='";
	strSql += strAlUtf8.c_str();
	strSql += "' and file_deleted=0";

	AppendQeuryStatment(orderBy, nTopN, strSql);

	nRet = query.Query(this, strSql.c_str());
	*ppPlaylist = query.m_pPlaylist;

	return nRet;
}

MLRESULT CMediaLibrary::GetByTitle(LPCXSTR szTitle, IPlaylist **ppPlaylist)
{
	return QueryPlaylist("select * from medialib where title=? and file_deleted=0", szTitle, ppPlaylist);
}

MLRESULT CMediaLibrary::GetByGenre(LPCXSTR szGenre, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN)
{
	string				strSql;

	strSql = "select * from medialib where genre=? and file_deleted=0";
	AppendQeuryStatment(orderBy, nTopN, strSql);

	return QueryPlaylist(strSql.c_str(), szGenre, ppPlaylist);
}

MLRESULT CMediaLibrary::GetByYear(int nYear, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN)
{
	if (!IsOK())
		return m_nInitResult;

	string			strSql;
	CMutexAutolock	autolock(m_mutexDataAccess);
	CSqlite3Stmt	sqlQuery;
	int				nRet;
	CPlaylist		*playlist = NULL;

	strSql = "select * from medialib where year=? and file_deleted=0";
	AppendQeuryStatment(orderBy, nTopN, strSql);

	nRet = sqlQuery.Prepare(&m_db, strSql.c_str());
	if (nRet != ERR_OK)
		return nRet;

	playlist = NewPlaylist();

	sqlQuery.BindInt(1, nYear);
	nRet = sqlQuery.Step();
	while (nRet == ERR_SL_OK_ROW)
	{
		CMedia		*media;

		media = NewMedia();
		SqliteQueryMedia(&sqlQuery, media);
		playlist->InsertItem(-1, media);
		media->Release();

		nRet = sqlQuery.Step();
	}
	if (playlist->GetCount() == 0)
	{
		playlist->Release();
		playlist = NULL;
		if (nRet == ERR_OK)
			nRet = ERR_NOT_FOUND;
	}
	else
		nRet = ERR_OK;

	*ppPlaylist = playlist;

	return nRet;
}

MLRESULT CMediaLibrary::GetByRating(int nRating, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN)
{
	if (!IsOK())
		return m_nInitResult;

	string			strSql;
	CMutexAutolock	autolock(m_mutexDataAccess);
	CSqlite3Stmt	sqlQuery;
	int				nRet;
	CPlaylist		*playlist = NULL;
	int				nRatingStart, nRatingEnd;

	strSql = "select * from medialib where rating>=? and rating<? and file_deleted=0";
	AppendQeuryStatment(orderBy, nTopN, strSql);

	nRatingStart = nRating * 100;
	nRatingEnd = (nRating + 1) * 100 - 1;

	nRet = sqlQuery.Prepare(&m_db, strSql.c_str());
	if (nRet != ERR_OK)
		return nRet;

	playlist = NewPlaylist();

	sqlQuery.BindInt(1, nRatingStart);
	sqlQuery.BindInt(2, nRatingEnd);
	nRet = sqlQuery.Step();
	while (nRet == ERR_SL_OK_ROW)
	{
		CMedia		*media;

		media = NewMedia();
		SqliteQueryMedia(&sqlQuery, media);
		playlist->InsertItem(-1, media);
		media->Release();

		nRet = sqlQuery.Step();
	}
	if (playlist->GetCount() == 0)
	{
		playlist->Release();
		playlist = NULL;
		if (nRet == ERR_OK)
			nRet = ERR_NOT_FOUND;
	}
	else
		nRet = ERR_OK;

	*ppPlaylist = playlist;

	return nRet;
}

MLRESULT CMediaLibrary::GetRecentPlayed(uint32 nCount, IPlaylist **ppPlaylist)
{
	CMLQueryPlaylist	query;
	char				szQuery[256];
	int					nRet;

	sprintf_s(szQuery, CountOf(szQuery), "select * from medialib where file_deleted=0 order by time_played desc limit %d", nCount);

	nRet = query.Query(this, szQuery);
	*ppPlaylist = query.m_pPlaylist;

	return nRet;
}

MLRESULT CMediaLibrary::GetTopPlayed(uint32 nCount, IPlaylist **ppPlaylist)
{
	CMLQueryPlaylist	query;
	char				szQuery[256];
	int					nRet;

	sprintf_s(szQuery, CountOf(szQuery), "select * from medialib where file_deleted=0 order by times_played desc limit %d", nCount);

	nRet = query.Query(this, szQuery);
	*ppPlaylist = query.m_pPlaylist;

	return nRet;
}

MLRESULT CMediaLibrary::GetTopRating(uint32 nCount, IPlaylist **ppPlaylist)
{
	CMLQueryPlaylist	query;
	char				szQuery[256];
	int					nRet;

	sprintf_s(szQuery, CountOf(szQuery), "select * from medialib where file_deleted=0 order by rating desc limit %d", nCount);

	nRet = query.Query(this, szQuery);
	*ppPlaylist = query.m_pPlaylist;

	return nRet;
}

MLRESULT CMediaLibrary::GetRecentAdded(uint32 nCount, IPlaylist **ppPlaylist)
{
	CMLQueryPlaylist	query;
	char				szQuery[256];
	int					nRet;

	sprintf_s(szQuery, CountOf(szQuery), "select * from medialib where file_deleted=0 order by time_added desc limit %d", nCount);

	nRet = query.Query(this, szQuery);
	*ppPlaylist = query.m_pPlaylist;

	return nRet;
}

MLRESULT CMediaLibrary::GetRecentPlayed(int nDayAgoBegin, int nDayAgoEnd, IPlaylist **ppPlaylist)
{
	CMLQueryPlaylist	query;
	char				szQuery[256];
	int					nRet;
	CMPTime			dateBegin, dateEnd;
	CMPTime			dateCur;
	MPTM			tm;

	dateCur.GetCurrentTime();
	dateCur.GetMPTM(&tm);

	dateBegin.Set(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, 0, 0, 0);
	dateEnd = dateBegin;
	dateBegin -= CMPTimeSpan(nDayAgoBegin - 1, 0, 0, 0);
	dateEnd -= CMPTimeSpan(nDayAgoEnd - 1, 0, 0, 0);

	sprintf_s(szQuery, CountOf(szQuery), "select * from medialib where time_played between %d and %d and file_deleted=0 order by time_played", dateBegin.m_time, dateEnd.m_time);

	nRet = query.Query(this, szQuery);
	*ppPlaylist = query.m_pPlaylist;

	return nRet;
}

MLRESULT CMediaLibrary::GetRandom(uint32 nCount, IPlaylist **ppPlaylist)
{
	if (!IsOK())
		return m_nInitResult;
	return ERR_OK;
}

MLRESULT CMediaLibrary::GetRandomByTime(uint32 nDurationInMin, IPlaylist **ppPlaylist)
{
	if (!IsOK())
		return m_nInitResult;
	return ERR_OK;
}

// 0-5, 0 for unrate.
MLRESULT CMediaLibrary::Rate(IMedia *pMedia, uint32 nRating)
{
	if (!IsOK())
		return m_nInitResult;

	if (!pMedia)
		return ERR_OK;

	bool			bIsUserRating = (nRating != 0);
	char			szSql[256];

	if (bIsUserRating)
		nRating = nRating * 100 + 99;
	else
		nRating = GetAutoRating(pMedia);

	CMutexAutolock	autolock(m_mutexDataAccess);

	sprintf_s(szSql, CountOf(szSql), "update medialib set is_user_rating=%d, rating=%d where id=%d", 
		bIsUserRating, nRating, pMedia->GetID());

	pMedia->SetAttribute(MA_RATING, nRating / 100);
	pMedia->SetAttribute(MA_IS_USER_RATING, bIsUserRating);

	return ExecuteSQL(szSql);
}

MLRESULT CMediaLibrary::UpdatePlayedTime(IMedia *pMedia)
{
	if (!IsOK())
		return m_nInitResult;

	if (!pMedia || pMedia->GetID() == MEDIA_ID_INVALID)
		return ERR_OK;

	int				nValue;
	char			szSql[256];

	CMutexAutolock	autolock(m_mutexDataAccess);

	pMedia->GetAttribute(MA_TIME_PLAYED, &nValue);
	sprintf_s(szSql, CountOf(szSql), "update medialib set time_played=%d where id=%d", nValue, pMedia->GetID());

	return ExecuteSQL(szSql);
}

MLRESULT CMediaLibrary::MarkPlayFinished(IMedia *pMedia)
{
	if (!IsOK())
		return m_nInitResult;

	if (!pMedia || pMedia->GetID() == MEDIA_ID_INVALID)
		return ERR_OK;

	char			szSql[256];
	int				nRet;
	int				nTimesPlayed;
	int				bIsUserRating;

	CMutexAutolock	autolock(m_mutexDataAccess);

	pMedia->GetAttribute(MA_TIMES_PLAYED, &nTimesPlayed);
	pMedia->SetAttribute(MA_TIMES_PLAYED, nTimesPlayed + 1);
	pMedia->GetAttribute(MA_IS_USER_RATING, &bIsUserRating);
	if (bIsUserRating)
		sprintf_s(szSql, CountOf(szSql), "update medialib set times_played=times_played+1 where id=%d", pMedia->GetID());
	else
	{
		int		nRating = GetAutoRating(pMedia);
		sprintf_s(szSql, CountOf(szSql), "update medialib set times_played=times_played+1, rating=%d where id=%d",
			nRating, pMedia->GetID());
		pMedia->SetAttribute(MA_RATING, nRating / 100);
	}

	nRet = ExecuteSQL(szSql);

	UpdatePlayedTime(pMedia);

	return nRet;
}

MLRESULT CMediaLibrary::MarkPlaySkipped(IMedia *pMedia)
{
	if (!IsOK())
		return m_nInitResult;

	if (!pMedia || pMedia->GetID() == MEDIA_ID_INVALID)
		return ERR_OK;

	char			szSql[256];
	int				nRet;
	int				nTimesPlaySkipped;
	int				bIsUserRating;

	CMutexAutolock	autolock(m_mutexDataAccess);

	pMedia->GetAttribute(MA_TIMES_PLAY_SKIPPED, &nTimesPlaySkipped);
	pMedia->SetAttribute(MA_TIMES_PLAY_SKIPPED, nTimesPlaySkipped + 1);
	pMedia->GetAttribute(MA_IS_USER_RATING, &bIsUserRating);
	if (bIsUserRating)
		sprintf_s(szSql, CountOf(szSql), "update medialib set times_play_skipped=times_play_skipped+1 where id=%d", pMedia->GetID());
	else
	{
		int		nRating = GetAutoRating(pMedia);
		sprintf_s(szSql, CountOf(szSql), "update medialib set times_play_skipped=times_play_skipped+1, rating=%d where id=%d", 
			nRating, pMedia->GetID());
		pMedia->SetAttribute(MA_RATING, nRating / 100);
	}

	nRet = ExecuteSQL(szSql);

	return nRet;
}

int CMediaLibrary::Init(CMPlayer *pPlayer)
{
	TCHAR			szFile[MAX_PATH];
	tstringex		strFile;

	GetWorkingFolder(szFile);
	_tcscat_safe(szFile, CountOf(szFile), _T("medialib.db"));
	ConvertStr(szFile, -1, strFile);

	m_nInitResult = m_db.Open(strFile.c_str());
	if (m_nInitResult != ERR_OK)
		goto INIT_FAILED;

	// create version table.
	m_nInitResult = m_db.Exec(SQL_CREATE_LIB_VER);
	if (m_nInitResult != ERR_OK)
	{
		// Do convert old lib to current.
		m_nInitResult = ConvertLib();
	}
	else
	{
		// new version table created, insert version numb.
		char		szSql[MAX_PATH];

		sprintf_s(szSql, CountOf(szSql), SQL_INSERT_VER_NUMB, MEDIA_LIB_VER_CUR);
		m_nInitResult = m_db.Exec(szSql);
		if (m_nInitResult != ERR_OK)
			return m_nInitResult;
	}

	// Create medialib talbe
	m_nInitResult = m_db.Exec(SQL_CREATE_MEDIALIB);
	if (m_nInitResult != ERR_OK)
		return m_nInitResult;

	m_nInitResult = m_sqlAdd.Prepare(&m_db, SQL_ADD_MEDIA);
	if (m_nInitResult != ERR_OK)
		goto INIT_FAILED;

	m_nInitResult = m_sqlAddFast.Prepare(&m_db, SQL_ADD_MEDIA_FAST);
	if (m_nInitResult != ERR_OK)
		goto INIT_FAILED;

	m_nInitResult = m_sqlQueryByUrl.Prepare(&m_db, SQL_QUERY_MEDIA_BY_URL);
	if (m_nInitResult != ERR_OK)
		goto INIT_FAILED;

	m_player = pPlayer;

	return m_nInitResult;

INIT_FAILED:
	Close();
	return m_nInitResult;
}

void CMediaLibrary::Close()
{
	m_sqlAdd.Finalize();
	m_sqlAddFast.Finalize();
	m_sqlQueryByUrl.Finalize();
	m_db.Close();
	m_player.Release();
}

MLRESULT CMediaLibrary::QueryPlaylist(LPCSTR szSQL, LPCXSTR szClause, IPlaylist **ppPlaylist)
{
	if (!IsOK())
		return m_nInitResult;

	CMutexAutolock	autolock(m_mutexDataAccess);
	CSqlite3Stmt	sqlQuery;
	int				nRet;
	CPlaylist		*playlist = NULL;
	stringex	strUtf8;

	nRet = sqlQuery.Prepare(&m_db, szSQL);
	if (nRet != ERR_OK)
		return nRet;

	UCS2ToUtf8(szClause, -1, strUtf8);
	nRet = sqlQuery.BindText(1, strUtf8.c_str());
	if (nRet != ERR_OK)
		return nRet;

	playlist = NewPlaylist();

	nRet = sqlQuery.Step();
	while (nRet == ERR_SL_OK_ROW)
	{
		CMedia		*media;

		media = NewMedia();
		SqliteQueryMedia(&sqlQuery, media);
		playlist->InsertItem(-1, media);
		media->Release();

		nRet = sqlQuery.Step();
	}
	if (playlist->GetCount() == 0)
	{
		playlist->Release();
		playlist = NULL;
		if (nRet == ERR_OK)
			nRet = ERR_NOT_FOUND;
	}
	else
		nRet = ERR_OK;

	*ppPlaylist = playlist;

	return nRet;
}

// 
MLRESULT CMediaLibrary::QueryVStr(LPCSTR szSql, IVXStr **ppvStr)
{
	if (!IsOK())
		return m_nInitResult;

	*ppvStr = NULL;

	CMutexAutolock	autolock(m_mutexDataAccess);
	CSqlite3Stmt	sqlQuery;
	int				nRet;
	tstringex		strUCS2;
	CVXStr			*pvStr;
	LPCSTR			text;

	sqlQuery.Prepare(&m_db, szSql);

	pvStr = new CVXStr();
	pvStr->AddRef();

	nRet = sqlQuery.Step();
	while (nRet == ERR_SL_OK_ROW)
	{
		text = (const char *)sqlQuery.ColumnText(0);
		if (text)
		{
			Utf8ToUCS2(text, -1, strUCS2);
			pvStr->push_back(strUCS2.c_str());
		}

		nRet = sqlQuery.Step();
	}
	if (pvStr->size() == 0)
	{
		pvStr->Release();
		pvStr = NULL;
		if (nRet == ERR_OK)
			nRet = ERR_NOT_FOUND;
	}
	else
		nRet = ERR_OK;

	*ppvStr = pvStr;

	return nRet;
}

MLRESULT CMediaLibrary::ExecuteSQL(LPCSTR szSql)
{
	return m_db.Exec(szSql);
}

#define  MediaGetSqlite3ColumText(mediaAttri)	\
	text = (const char *)sqlStmt->ColumnText(n++);	\
	if (text)\
		Utf8ToUCS2(text, -1, mediaAttri);\
	else\
		mediaAttri.resize(0);

void CMediaLibrary::SqliteQueryMedia(CSqlite3Stmt *sqlStmt, CMedia *pMedia)
{
	int			n = 0;
	const char	*text;
	tstringex	str;

	pMedia->m_nID = sqlStmt->ColumnInt(n++);

	MediaGetSqlite3ColumText(pMedia->m_strUrl);
	pMedia->m_bIsFileDeleted = Tobool(sqlStmt->ColumnInt(n++));
	pMedia->m_bInfoUpdated = Tobool(sqlStmt->ColumnInt(n++));
	pMedia->m_bIsUserRating = Tobool(sqlStmt->ColumnInt(n++));
	MediaGetSqlite3ColumText(pMedia->m_strArtist);
	MediaGetSqlite3ColumText(pMedia->m_strAlbum);
	MediaGetSqlite3ColumText(pMedia->m_strTitle);
	pMedia->m_nTrackNumb = sqlStmt->ColumnInt(n++);
	pMedia->m_nYear = sqlStmt->ColumnInt(n++);
	MediaGetSqlite3ColumText(pMedia->m_strGenre);
	MediaGetSqlite3ColumText(pMedia->m_strComment);
	pMedia->m_nLength = sqlStmt->ColumnInt(n++);
	pMedia->m_nFileSize = sqlStmt->ColumnInt(n++);
	pMedia->m_nTimeAdded.m_time = sqlStmt->ColumnInt(n++);
	pMedia->m_nTimePlayed.m_time = sqlStmt->ColumnInt(n++);
	pMedia->m_nRating = sqlStmt->ColumnInt(n++) / 100;
	pMedia->m_nPlayed = sqlStmt->ColumnInt(n++);
	pMedia->m_nPlaySkipped = sqlStmt->ColumnInt(n++);
	MediaGetSqlite3ColumText(pMedia->m_strLyricsFile);
}

void CMediaLibrary::GetMediaCallback(CMedia *pMedia, int numCols, char **results)
{
	int			n = 0;

#define FILED_GET_INT(feild)	\
	if (results[n])\
		feild = atoi(results[n]);\
	else\
		feild = false;\
	n++;

#define FILED_GET_BOOL(feild)	\
	if (results[n])\
		feild = Tobool(atoi(results[n]));\
	else\
		feild = false;\
	n++;

#define FILED_GET_TEXT(feild)	\
	if (results[n])\
		Utf8ToUCS2(results[n], -1, feild);\
	else\
		feild.resize(0);\
	n++;

	pMedia->m_nID = atoi(results[n++]);

	FILED_GET_TEXT(pMedia->m_strUrl);

	FILED_GET_BOOL(pMedia->m_bIsFileDeleted);
	FILED_GET_BOOL(pMedia->m_bInfoUpdated);
	FILED_GET_BOOL(pMedia->m_bIsUserRating);

	FILED_GET_TEXT(pMedia->m_strArtist);
	FILED_GET_TEXT(pMedia->m_strAlbum);
	FILED_GET_TEXT(pMedia->m_strTitle);
	FILED_GET_INT(pMedia->m_nTrackNumb);
	FILED_GET_INT(pMedia->m_nYear);

	FILED_GET_TEXT(pMedia->m_strGenre);
	FILED_GET_TEXT(pMedia->m_strComment);
	FILED_GET_INT(pMedia->m_nLength);
	FILED_GET_INT(pMedia->m_nFileSize);
	FILED_GET_INT(pMedia->m_nTimeAdded.m_time);
	FILED_GET_INT(pMedia->m_nTimePlayed.m_time);
	FILED_GET_INT(pMedia->m_nRating);
	pMedia->m_nRating /= 100;
	FILED_GET_INT(pMedia->m_nPlayed);
	FILED_GET_INT(pMedia->m_nPlaySkipped);
	FILED_GET_TEXT(pMedia->m_strLyricsFile);

	ASSERT(n <= numCols);
}

MLRESULT CMediaLibrary::UndeleteMedia(long nMediaID)
{
	char		szSql[256];

	sprintf_s(szSql, CountOf(szSql), "update medialib set file_deleted=0 where id=%d", nMediaID);

	return ExecuteSQL(szSql);
}

MLRESULT CMediaLibrary::Add(IMedia *pMedia)
{
	if (!IsOK())
		return m_nInitResult;

	CMPAutoPtr<IMedia>	pMediaExist;
	int				nRet;
	CXStr			str;

	pMedia->GetSourceUrl(&str);
	nRet = GetMediaByUrl(str.c_str(), &pMediaExist);
	if (nRet != ERR_NOT_FOUND)
		return nRet;

	CMutexAutolock	autolock(m_mutexDataAccess);

	return DoAddMedia((CMedia*)pMedia);
}

MLRESULT CMediaLibrary::DoAddMedia(CMedia *pMedia)
{
	int				nRet;
	int				n = 1;
	stringex	strUtf8;

	SQLITE3_BIND_UCS2(m_sqlAdd, pMedia->m_strUrl);
	SQLITE3_BIND_UCS2(m_sqlAdd, pMedia->m_strArtist);
	SQLITE3_BIND_UCS2(m_sqlAdd, pMedia->m_strAlbum);
	SQLITE3_BIND_UCS2(m_sqlAdd, pMedia->m_strTitle);
	m_sqlAdd.BindInt(n++, pMedia->m_nTrackNumb);
	m_sqlAdd.BindInt(n++, pMedia->m_nYear);
	SQLITE3_BIND_UCS2(m_sqlAdd, pMedia->m_strGenre);
	SQLITE3_BIND_UCS2(m_sqlAdd, pMedia->m_strComment);
	m_sqlAdd.BindInt(n++, pMedia->m_nLength);
	m_sqlAdd.BindInt(n++, pMedia->m_nFileSize);
	m_sqlAdd.BindInt(n++, pMedia->m_nTimeAdded.m_time);
	SQLITE3_BIND_UCS2(m_sqlAdd, pMedia->m_strLyricsFile);

	nRet = m_sqlAdd.Step();
	if (nRet != ERR_OK)
		pMedia->Release();
	else
	{
		pMedia->m_nID =  m_db.LastInsertRowId();
		pMedia->SetInfoUpdatedToMediaLib(true);
	}

	m_sqlAdd.Reset();

RET_FAILED:

	return nRet;
}

CPlaylist *CMediaLibrary::NewPlaylist()
{
	CPlaylist		*pPlaylist;

	pPlaylist = new CPlaylist(m_player);
	pPlaylist->AddRef();

	return pPlaylist;
}

int CMediaLibrary::ConvertLib()
{
	return ERR_OK;
}
