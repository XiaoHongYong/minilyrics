/********************************************************************
	Created  :	2002/01/03	17:12
	FileName :	LyricsParser.h
	Author   :	xhy
*********************************************************************/

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
