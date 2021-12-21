/********************************************************************
	Created  :	2002/01/04	21:40
	FileName :	LrcParser.h
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#pragma once

#include "LyricsParser.h"

DECLAR_CPPUNIT_TEST_REG(LrcParser)

class CLrcParser : public CLyricsParser
{
public:
	CLrcParser(CMLData *pMLData, bool bTurnOffPref = false);
	virtual ~CLrcParser();

public:
	int ParseFile(bool bUseSpecifiedEncoding, CHAR_ENCODING encoding);
	int SaveAsFile(LPCTSTR szFile, CHAR_ENCODING newEncoding);

	virtual LYRICS_CONTENT_TYPE GetLyrContentType() { return m_lyrContentType; }

	int ParseString(LPCTSTR szText, bool bAddTimeStampForTextLine = false);
	int ParseString(LPBYTE szLrc, int nLen, bool bAddTimeStampForTextLine = false, bool bUseSpecifiedEncoding = false, CHAR_ENCODING encoding = ED_SYSDEF);

	void ToString(tstring &str, bool bIncTags, bool bToTxtFormat);

protected:
	void ToLrcString(tstring &str, bool bIncTags, bool bLongTimeFmt);
	void ToTxtString(tstring &str, bool bIncTags);

	bool LyricsLineToLrcString(LyricsLine *pLine, bool bLongTimeFmt, tstringex &strBuff);
	void LyricsLineToTxtString(LyricsLine *pLine, tstring &strBuff);

	void AddLyrLines(vector<int> &vTimes, LPCTSTR szLyrLine);
	void AddStrAsTimedLine(const TCHAR *szLine);
	void AddStrAsUnknowLine(const TCHAR *szLine);

	void ParseTxtTagInLyrics();

	void FinalFixOnLyrics(bool bAddTimeStampForTextLine);

	void FixTextLrcTimeTag();
	void RemoveExtraBlankLinesOfTextLyr();

	// old
	void LRCCheckRowTime();

protected:
	LYRICS_CONTENT_TYPE		m_lyrContentType;

	// 上一次添加的歌词行的时间，用于添加没有时间标签的文本歌词行
	int						m_nLatestTime;
	bool					m_bTurnOffPref;

};
