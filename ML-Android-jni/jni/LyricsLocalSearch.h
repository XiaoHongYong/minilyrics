#pragma once

class CLyricsLocalSearch
{
public:
	CLyricsLocalSearch(void);
	virtual ~CLyricsLocalSearch(void);

public:
	void SetLyricsDir(LPCTSTR szDownloadDir, LPCTSTR szLyricsDir);

	// Search match lyrics
	void SearchAllMatchLyrics(LPCTSTR szSongFile, LPCTSTR szArtist, LPCTSTR szTitle, V_LRCSEARCHRESULT &vLyrics);

	// Search best match lyrics
	bool GetBestMatchLyrics(LPCTSTR szSongFile, LPCTSTR szArtist, LPCTSTR szTitle, tstring &strLyrFile);

protected:
	void SearchLyrics(CLyricsSearchParameter &searchParam, V_LRCSEARCHRESULT &vLyrics);

protected:
	tstring			m_lyricsDirOfUser, m_lyricsDirOfDownload;

};

extern CLyricsLocalSearch		g_lyricsSearch;
