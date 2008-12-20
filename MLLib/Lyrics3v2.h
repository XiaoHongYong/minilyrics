// Lyrics3v2.h: interface for the CLyrics3v2 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LYRICS3V2_H__1BFC214C_9838_434B_BB56_F608E290D130__INCLUDED_)
#define AFX_LYRICS3V2_H__1BFC214C_9838_434B_BB56_F608E290D130__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CLyrics3v2  
{
public:
	CLyrics3v2();
	virtual ~CLyrics3v2();

public:
	int ReadLyrcsInfo(LPCTSTR szMp3File, stringex &strLyrics);
	int WriteLyrcsInfo(LPCTSTR szMp3File, const char *szLyrics, int nLen, BOOL bTimeStamp = TRUE);

	int ClearLyricsInfo(LPCTSTR szMp3File);

protected:
	// tstring		m_strInd;

};

void SetId3v1(char *szId3v1, const char *szTitle, const char *szArtist, const char * szAlbum, const char * szYear = NULL, const char *szComment = NULL, BYTE byTrack = 0, BYTE byGenre = 0xFF);

#endif // !defined(AFX_LYRICS3V2_H__1BFC214C_9838_434B_BB56_F608E290D130__INCLUDED_)
