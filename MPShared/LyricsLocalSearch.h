/********************************************************************
	Created  :	2002/01/04	21:41
	FileName :	LyricsLocalSearch.h
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#if !defined(AFX_LYRICSLOCALSEARCH_H__A34FB361_77F5_11D5_9E04_02608CAD9330__INCLUDED_)
#define AFX_LYRICSLOCALSEARCH_H__A34FB361_77F5_11D5_9E04_02608CAD9330__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

void AnalysisSongTitle(TCHAR szTitle[]/* out */, BOOL &bTitleIgnoreNumb/* out */, TCHAR szArtist[]/* out */, BOOL &bArtIgnoreNumb/* out */, LPCTSTR szFullTitle);

// 歌词关连定义
typedef map<tstring, tstring> SONG_LYRIC_MAP;

/*
// 歌曲相关联的歌词
typedef struct MATCHED_LYRICS
{
	tstring		strLyric;
	int			nLyricFileType;
	int			nMatchValue;
}MATCHED_LYRICS;
typedef vector<MATCHED_LYRICS>	VECTOR_MATCH_LYRICS;
*/

//
// 本地歌词搜索
//
class CLyricsLocalSearch
{
public:

	// 歌词搜索目录结构定义
	typedef struct SearchFolder
	{
		BOOL	bIncludeSub;
		tstring	strFolder;
	}SearchFolder;

	void Init();
	void Quit();

	// 搜索所有匹配的歌词，包括了歌曲名完全匹配、歌词名完全匹配
	BOOL SearchAllMatchLyrics(LPCTSTR szSongFile, LPCTSTR szSongName, V_LRCSEARCHRESULT &vLyrics);

	// 取得最匹配的歌词，首先从歌词关连列表中读取
	BOOL GetBestMatchLyrics(LPCTSTR szSongFile, LPCTSTR szArtist, LPCTSTR szTitle, TCHAR *szLyricFile);

	// 取得相关连的歌词
	// song://lyrics3v2
	// song://id3v2/sylt/xxx
	// song://id3v2/uslt/xxx
	static void GetAssociateFileKeyword(LPCTSTR szSongFile, LPCTSTR szFulltitle, LPTSTR szAssociateFileKeyword, int nMaxLen);
	BOOL GetAssociateLyrics(LPCTSTR szAssociateFileKeyword, LPTSTR szLyricFile, int nMaxBuff);
	BOOL IsAssociatedLyrics(LPCTSTR szAssociateFileKeyword);
	BOOL IsAssociatedWithNoneLyrics(LPCTSTR szAssociateKeyword);
	BOOL IsAssociatedLyrics(LPCTSTR szSongFile, LPCTSTR szFullTitle);
	bool AssociateLyrics(LPCTSTR szAssociateFileKeyword, LPCTSTR szLyricFile);
	BOOL CancelAssociate(LPCTSTR szAssociateFileKeyword);
	bool IsLyricsExist(LPCTSTR szLyricSource);

	//
	// search folder
	//
	int GetSearchFolerCount();
	BOOL IsFolderAdded(LPCTSTR szFolder);
	BOOL RemoveFolder(int nIndex);
	BOOL RemoveFolder(TCHAR *szFolder);
	SearchFolder * GetFolder(int nIndex);
	LPCTSTR  GetFolder(BOOL &bIncludeSub, int nIndex);
	BOOL SetFolder(LPCTSTR szFolder, BOOL bIncludeSub);
	void SaveLyricFolderCfg();

	static int SearchEmbeddedLyrics(LPCTSTR szSongFile, V_LRCSEARCHRESULT &vLyrics, bool bOnlyListAvailable = false);
	static int SearchEmbeddedLyrics(LPCTSTR szSongFile, uint32 &lrcSourceType);

protected:
	// 搜索所有的歌词
	BOOL SearchLyrics(LPCTSTR szSongFile, LPCTSTR szTitle, BOOL bTitleIgnoreNumb, LPCTSTR szArtist, BOOL bArtIgnoreNumb, V_LRCSEARCHRESULT *pvLyrics);

	void LoadLyricsAssociationOld();
	void LoadLyricsAssociationV2();
	void LoadLyricsAssociation();
	void SaveLyricsAssociation();

	tstring ToAssociateKeyword(LPCTSTR szKeyword)
	{
#ifdef _WIN32_DESKTOP
		// Use keyword without driver letter info "C:"
		if (szKeyword[0] && szKeyword[1] == ':')
		{
			return ToLower(szKeyword + 2);
		}
#endif
		return ToLower(szKeyword);
	}

	void LoadLyricFolderCfg();

public:
	CLyricsLocalSearch();
	virtual ~CLyricsLocalSearch();

public:
	vector<SearchFolder>	m_vLyricsFolders;

	//
	// Lyrics association map:
	//
	// song file <--->  lyrics file
	//
	// To support removable disk (mp3 file, lyrics are in removable disk):
	// 1) song file is in lower case
	// 2) if song file is in same driver with lyrics file, "x:" will be removed, but 
	//    the "x:" in lyrics file should be kept.
	// 3) if song file do not have driver info,  but lyrics file doesn't exists there, 
	//    Check whether lyrics is in the $Product$ program driver.
	// 4) "Lyrics" folder should be verified dynamically.
	//
	SONG_LYRIC_MAP			m_mapLyricsAssociate;
	Mutex					m_mutex;

	BOOL					m_bSaved;
};

#endif // !defined(AFX_LYRICSLOCALSEARCH_H__A34FB361_77F5_11D5_9E04_02608CAD9330__INCLUDED_)
