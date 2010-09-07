/********************************************************************
	Created  :	2002/01/03	17:12
	FileName :	LyricsParser.h
	Author   :	xhy
	
	Purpose  :	对外部提供统一的接口打开各种歌词文件
*********************************************************************/

#if !defined(AFX_LYRICSPARSER_H__6B04DB02_FA47_11D5_B478_93B36F70B014__INCLUDED_)
#define AFX_LYRICSPARSER_H__6B04DB02_FA47_11D5_B478_93B36F70B014__INCLUDED_

#pragma once

class CLyricsParser  
{
public:
	CLyricsParser(CMLData *pMLData) : m_pMLData(pMLData)
	{
	}
	virtual ~CLyricsParser() { };

public:
	virtual int ParseFile(bool bUseSpecifiedEncoding, CHAR_ENCODING encoding) = 0;
	virtual int SaveAsFile(LPCTSTR szFile, CHAR_ENCODING newEncoding) = 0;

	virtual LYRICS_CONTENT_TYPE GetLyrContentType() = 0;

public:
    CMLData				*m_pMLData;
	
};

#endif // !defined(AFX_LYRICSPARSER_H__6B04DB02_FA47_11D5_B478_93B36F70B014__INCLUDED_)
