#pragma once

#include "../../MLLib/LrcTag.h"
#include "Sqlite3Utils.h"


class CLyricsInfo
{
public:
	WORD			nLyrContentType;	// LYRICS_CONTENT_TYPE, LCT_TXT, LCT_LRC...
	WORD			wMediaLength;		// In seconds.
	tstring			strAr, strTi, strAl;
	tstring			strEditedBy;

	tstring			strArCmp, strTiCmp;
	tstring			strRelatedHttpLink;

	long			nUploadUserID;

	long			nUniID;
	long			nRateTotal;
	long			nRateCount;
	long			nDlCount;
	time_t	        nUploadDate;

	const char	*szArtist() { return strAr.c_str(); }
	const char	*ArtistCmp() { return strArCmp.c_str(); }
	const char	*szAlbum() { return strAl.c_str(); }
	const char	*szTitle() { return strTi.c_str(); }
	const char	*TitleCmp() { return strTiCmp.c_str(); }
	const char	*szEditedBy() { return strEditedBy.c_str(); }

	const char  *GetRelatedHttpLink() { return strRelatedHttpLink.c_str(); }

	void AnalysisInfo(LPCTSTR szFile, CCharBuffer &content);

	CLyricsInfo();

	void SetInfo(LyricsProperties &lyrProperties, long nUploaderId, LPCTSTR szUploader);

};

class CVectorLrcInfo : public vector<CLyricsInfo *>
{
public:
	~CVectorLrcInfo()
	{
		iterator	it, itEnd;
		itEnd = end();
		for (it = begin(); it != itEnd; ++it)
			delete *it;
	}
	void AddLrcInfo(CLyricsInfo *pInfo)
	{
		push_back(pInfo);
	}

};

class LyricsDB
{
public:
	LyricsDB();
	virtual ~LyricsDB();

public:
	int Init(const string &dataPath);
	void Quit();

	int AddLyrics(CLyricsInfo &lyrInfo);
	int UpdateLyricsPropById(long nId, LyricsProperties &lyrProperties);
	int UpdateLyricsLinkById(long nId, LPCTSTR szLink);

	int SearchLyricsByArtistTitleID(LPCTSTR szArtist, LPCTSTR szTitle, long nID, RetLyrInfo &lyrInfo);
	int SearchLyricsByArtist(LPCTSTR szArtist, RetLyrInfoList &vLyrics);
	int SearchLyricsByTitle(LPCTSTR szTitle, RetLyrInfoList &vLyrics);
	int SearchLyricsByArtistTitle(LPCTSTR szArtist, LPCTSTR szTitle, RetLyrInfoList &vLyrics);
    int SearchLyricsByArtistTitleContentType(LPCTSTR szArtist, LPCTSTR szTitle, RetLyrInfoList &vLyrics, uint32 contentType);

public:
	//
	// string Alias
	//
	int LoadArtistAliasDb(LPCTSTR szFile);
	int LoadTitleAliasDb(LPCTSTR szFile);

	LPCTSTR GetAliasTitleStr(LPCSTR szStrOrg);
	LPCTSTR GetAliasArtistStr(LPCSTR szStrOrg);

	void SetAliasTitle(LPCSTR szTitleOrg, LPCTSTR szTitleAlias);
	void SetAliasArtist(LPCSTR szArtistOrg, LPCTSTR szArtistAlias);

protected:
    int getSearchLyricsResults(sqlite3_stmt *stmt, RetLyrInfoList &vLyrics);

protected:
	typedef map<tstring, tstring>	MAP_STR;
	typedef std::set<tstring>		SET_STR;

	//
	// Alias artist or artist
	//
	MAP_STR			m_mapArAlias, m_mapTiAlias;

    sqlite3*        m_db;

    sqlite3_stmt    *m_stmtAddLyrics;
    sqlite3_stmt    *m_stmtUpdateLyrLinkById;
    sqlite3_stmt    *m_stmtUpdateLyrPropsById;

    sqlite3_stmt    *m_stmtSearchLyrByArtistTitleId;
    sqlite3_stmt    *m_stmtSearchLyrByArtist;
    sqlite3_stmt    *m_stmtSearchLyrByTitle;
    sqlite3_stmt    *m_stmtSearchLyrByArtistTitle;
    sqlite3_stmt    *m_stmtSearchLyrByArtistTitleContentType;

};

extern LyricsDB	g_LyricsDB;
