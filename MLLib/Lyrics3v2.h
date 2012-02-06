#pragma once

class CLyrics3v2  
{
public:
	CLyrics3v2();
	virtual ~CLyrics3v2();

public:
	int ReadLyrcsInfo(LPCTSTR szMp3File, stringex &strLyrics);
	int WriteLyrcsInfo(LPCTSTR szMp3File, const char *szLyrics, int nLen, BOOL bTimeStamp = TRUE);

	int ClearLyricsInfo(LPCTSTR szMp3File);

};

void SetId3v1(char *szId3v1, const char *szTitle, const char *szArtist, const char * szAlbum, const char * szYear = NULL, const char *szComment = NULL, BYTE byTrack = 0, BYTE byGenre = 0xFF);
