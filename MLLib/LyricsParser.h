/********************************************************************
	Created  :	2002/01/03	17:12
	FileName :	LyricsParser.h
	Author   :	xhy
	
	Purpose  :	对外部提供统一的接口打开各种歌词文件
*********************************************************************/

#if !defined(AFX_LYRICSPARSER_H__6B04DB02_FA47_11D5_B478_93B36F70B014__INCLUDED_)
#define AFX_LYRICSPARSER_H__6B04DB02_FA47_11D5_B478_93B36F70B014__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CLyricsParser  
{
public:
	CLyricsParser(CMLData *pMLData) : m_pMLData(pMLData)
	{
	}
	virtual ~CLyricsParser(){};

public:
	// 保存为文件
	virtual int SaveAsFile(LPCTSTR szFile, FILE_ENCODING fileEncoding) = 0;

	// 分析歌词文件，并且存储到 pMLData 中
	virtual int Parse(bool bForceUseEncoding) = 0;

	virtual LYRICS_CONTENT_TYPE GetLyrContentType() = 0;

	virtual FILE_ENCODING GetFileEncoding() { return m_fileEncoding; };

public:
    CMLData				*m_pMLData;
	FILE_ENCODING		m_fileEncoding;
	
};

#endif // !defined(AFX_LYRICSPARSER_H__6B04DB02_FA47_11D5_B478_93B36F70B014__INCLUDED_)
