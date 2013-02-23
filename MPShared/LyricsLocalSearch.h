/********************************************************************
	Created  :	2002/01/04	21:41
	FileName :	LyricsLocalSearch.h
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#pragma once


//
// Search local lyrics
//
class CLyricsLocalSearch
{
public:
	void Init();
	void Quit();

	// Search match lyrics
	void SearchAllMatchLyrics(LPCTSTR szSongFile, LPCTSTR szArtist, LPCTSTR szTitle, V_LRCSEARCHRESULT &vLyrics);

	// Search best match lyrics
	bool GetBestMatchLyrics(LPCTSTR szSongFile, LPCTSTR szArtist, LPCTSTR szTitle, tstring &strLyrFile);

	// song://lyrics3v2
	// song://id3v2/sylt/xxx
	// song://id3v2/uslt/xxx
	static tstring GetAssociateFileKeyword(LPCTSTR szSongFile, LPCTSTR szFulltitle);
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
	BOOL RemoveFolder(int nIndex);
	tstring GetFolder(int nIndex);
	BOOL SetFolder(LPCTSTR szFolder);
	void SaveLyricFolderCfg();

	static int SearchEmbeddedLyrics(LPCTSTR szSongFile, V_LRCSEARCHRESULT &vLyrics, bool bOnlyListAvailable = false);
	static int SearchEmbeddedLyrics(LPCTSTR szSongFile, uint32 &lrcSourceType);

protected:
	void SearchLyrics(CLyricsSearchParameter &searchParam, V_LRCSEARCHRESULT &vLyrics);

#ifdef _WIN32_DESKTOP
	void LoadLyricsAssociationOld();
#endif
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
	V_Str	m_vLyricsFolders;

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
	typedef map<tstring, tstring> SONG_LYRIC_MAP;

	SONG_LYRIC_MAP			m_mapLyricsAssociate;
	Mutex					m_mutex;

	BOOL					m_bSaved;
};
