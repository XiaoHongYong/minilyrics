// MediaLibrary.h: interface for the CMediaLibrary class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEDIALIBRARY_H__12D1F10C_DB4B_4E0A_AE95_30251D39B91B__INCLUDED_)
#define AFX_MEDIALIBRARY_H__12D1F10C_DB4B_4E0A_AE95_30251D39B91B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../sqlite_cpp/Sqlite3.h"

#include "IMPlayer.h"
#include "Playlist.h"

class CMPlayer;

class CMediaLibrary : public IMediaLibrary  
{
	OBJ_REFERENCE_DECL
public:
	CMediaLibrary();
	virtual ~CMediaLibrary();

	virtual MLRESULT GetAllArtist(IVXStr **ppvArtist);

	virtual MLRESULT GetAllAlbum(IVXStr **ppvAlbum);

	virtual MLRESULT GetAllGenre(IVXStr **ppvAlbum);

	virtual MLRESULT GetAllYear(IVInt **ppvYear);

	virtual MLRESULT GetAlbumOfArtist(LPCXSTR szArtist, IVXStr **ppvAlbum);

	virtual uint32 GetMediaCount();

	virtual MLRESULT GetMediaByUrl(LPCXSTR szUrl, IMedia **pMedia);

	virtual MLRESULT Add(LPCXSTR szMediaUrl, IMedia **ppMedia);

	// Add media to media library fast, didn't update media info.
	virtual MLRESULT AddFast(LPCXSTR szMediaUrl, LPCXSTR szArtist, LPCXSTR szTitle, IMedia **ppMedia);

	virtual MLRESULT UpdateMediaInfo(IMedia *pMedia);

	// removes the specified item from the media library
	virtual MLRESULT Remove(IMedia **pMedia, bool bDeleteFile);

	// If the media file was removed temporarily, set this flag on.
	virtual MLRESULT SetDeleted(IMedia **pMedia);

	virtual MLRESULT GetAll(IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN);

	virtual MLRESULT GetByArtist(LPCXSTR szArtist, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN);

	virtual MLRESULT GetByAlbum(LPCXSTR szAlbum, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN);

	virtual MLRESULT GetByAlbum(LPCXSTR szArtist, LPCXSTR szAlbum, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN);

	virtual MLRESULT GetByTitle(LPCXSTR szTitle, IPlaylist **ppPlaylist);

	virtual MLRESULT GetByGenre(LPCXSTR szGenre, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN);

	virtual MLRESULT GetByYear(int nYear, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN);

	virtual MLRESULT GetByRating(int nRating, IPlaylist **ppPlaylist, MediaLibOrderBy orderBy, int nTopN);

	virtual MLRESULT GetRecentPlayed(uint32 nCount, IPlaylist **ppPlaylist);

	virtual MLRESULT GetRecentPlayed(int nDayAgoBegin, int nDayAgoEnd, IPlaylist **ppPlaylist);

	virtual MLRESULT GetTopPlayed(uint32 nCount, IPlaylist **ppPlaylist);

	virtual MLRESULT GetTopRating(uint32 nCount, IPlaylist **ppPlaylist);

	virtual MLRESULT GetRecentAdded(uint32 nCount, IPlaylist **ppPlaylist);

	virtual MLRESULT GetRandom(uint32 nCount, IPlaylist **ppPlaylist);

	virtual MLRESULT GetRandomByTime(uint32 nDurationInMin, IPlaylist **ppPlaylist);

	// 0-5, 0 for unrate.
	virtual MLRESULT Rate(IMedia *pMedia, uint32 nRating);

	virtual MLRESULT UpdatePlayedTime(IMedia *pMedia);
	virtual MLRESULT MarkPlayFinished(IMedia *pMedia);
	virtual MLRESULT MarkPlaySkipped(IMedia *pMedia);

public:
	int Init(CMPlayer *pPlayer);
	void Close();

	bool IsOK() { return m_nInitResult == ERR_OK; }

	MLRESULT QueryPlaylist(LPCSTR szSQL, LPCXSTR szClause, IPlaylist **ppPlaylist);

	MLRESULT QueryVStr(LPCSTR szSql, IVXStr **ppvStr);

	MLRESULT ExecuteSQL(LPCSTR szSql);

	void SqliteQueryMedia(CSqlite3Stmt *sqlStmt, CMedia *pMedia);

	void GetMediaCallback(CMedia *pMedia, int numCols, char **results);

	MLRESULT UndeleteMedia(long nMediaID);

	virtual MLRESULT Add(IMedia *pMedia);

protected:
	MLRESULT DoAddMedia(CMedia *pMedia);

	CPlaylist *NewPlaylist();

	int ConvertLib();

protected:
	friend class CMLQueryPlaylist;

	CMPAutoPtr<CMPlayer>	m_player;

	CSqlite3			m_db;
	Mutex				m_mutexDataAccess;

	CSqlite3Stmt		m_sqlAdd, m_sqlAddFast, m_sqlQueryByUrl;
	int					m_nInitResult;

};

#endif // !defined(AFX_MEDIALIBRARY_H__12D1F10C_DB4B_4E0A_AE95_30251D39B91B__INCLUDED_)
