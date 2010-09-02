/********************************************************************
	Created  :	2002/01/04	21:40
	FileName :	LrcParser.h
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#if !defined(AFX_LRCPARSER_H__01A9EAFE_777D_4209_A312_DA2DE993DE78__INCLUDED_)
#define AFX_LRCPARSER_H__01A9EAFE_777D_4209_A312_DA2DE993DE78__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LyricsParser.h"

enum LRC_TAG_TYPE
{
	TAG_VERSION,
	TAG_ARTIST,
	TAG_TITLE,
	TAG_AL,
	TAG_BY,
	TAG_OFFSET,
	TAG_ENCODING,
	TAG_COMMENT,
	TAG_LENGTH,
	TAG_NEWROW,
	TAG_UNKNOWN,
	TAG_TIME
};

#define	TAG_ENDOFDEFINE			100
//#define TAG_ERROR			10

#define	TAG_MAX_LEN			256


//
// COMPILE_WORD::nType 单词类别定义
//
enum LRC_WORD_TYPE
{
	WORD_LRC_TAG,
	WORD_LRC_STRING,
	WORD_LRC_END,
	WORD_LRC_NEWROW
};

class CLrcParserObj
{
public:
	CLrcParserObj(CMLData *pMLdata):m_pMLData(pMLdata)
	{
		m_nLatestTime = 0;
		m_lyrContentType = LCT_TXT;
	}
	// local struct definition
	struct COMPILE_WORD
	{
		LRC_WORD_TYPE	nType;			// 单词类别
		int				nValLen;		// 字符串长度
		TCHAR		*szValue;		// 单词的值
	};
	class CBuffWord
	{
	public:
		bool pop_read_front(COMPILE_WORD &word)
		{
			if (listWords.empty())
				return false;
			word = listWords.front();
			listWords.pop_front();
			return true;
		}
		void push_back(const COMPILE_WORD &word)
		{
			listWords.push_back(word);
		}
		void clear()
		{
			listWords.clear();
		}
// #ifdef _DEBUG
// 		void DbgOut()
// 		{
// 			for (list<COMPILE_WORD>::iterator it = listWords.begin(); it != listWords.end(); it++)
// 			{
// 				COMPILE_WORD	&w = *it;
// 				tstringex		str;
// 				static LPCTSTR		szWordType[] = { _T("LRC_TAG"), _T("LRC_STRING"), _T("LRC_END"), _T("LRC_NEWLINE") };
// 				if (w.szValue)
// 					ConvertStr(w.szValue, w.nValLen, str);
// 				DBGLOG2(_T("Word: %s, Value: %s"), szWordType[w.nType], str.c_str());
// 			}
// 		}
// #endif
	protected:
		list<COMPILE_WORD>		listWords;
	};

	typedef COMPILE_WORD*	PCOMPILE_WORD;
	typedef stringex_t<TCHAR>	tstringexT;

public:
	int Compile(LPVOID szLyric, int nLen, BOOL bConvertStr = FALSE);

	virtual LYRICS_CONTENT_TYPE GetLyrContentType() { return m_lyrContentType; }

protected:
	// new 
	int CompileWordAnalysis(CBuffWord &buffWords, TCHAR *szLyric);

	int GetTagType(TCHAR *szTag, int nLen);

	bool ReadTxtTag(TCHAR *szTag, int nLen);

	void AddTimeRow(CBuffWord *pbuffCompWords, TCHAR *szLyric, int nLen);

	void AddTimeRow(COMPILE_WORD *pCompWord, TCHAR *szLyric, int nLen);
	void AddStrTimeRow(const TCHAR *szLyric, int nLen);

	void AddFileRowsToMLData(PML_ROW pFileRow);

	int GetTimeTagValue(TCHAR *szTag, int nLen);
	int GetOffsetTagValue(TCHAR *szTag, int nLen);
	// void GetOtherTagValue(const TCHAR *szTag, int nLenTag, TCHAR *szValue, int nSizeValue);
	void GetOtherTagValue(const TCHAR *szTag, int nLenTag, tstringex &strValue);

	bool IsTagStringMatch(const TCHAR *szDest, const char *szTagName, int nLen1);

	TCHAR *EatFrag(TCHAR *szLyricsBeg, TCHAR *szLyricsEnd, int &nEndTime, tstringex &buffLyrics);

protected:
	// 上一次添加的歌词行的时间，用于添加没有时间标签的文本歌词行
	int					m_nLatestTime;
	CMLData				*m_pMLData;
	LYRICS_CONTENT_TYPE 	m_lyrContentType;

};

class CLrcParser : public CLyricsParser
{
public:
	CLrcParser(CMLData *pMLData);
	virtual ~CLrcParser();

public:
	BOOL LyricsRowToText(PML_ROW pRow, tstringex &strBuff);
	int SaveAsFile(LPCTSTR szFile, FILE_ENCODING fileEncoding);
	int Parse(bool bForceUseEncoding);
	BOOL ToString(tstring &str, BOOL bIncTags = FALSE);
	int ParseString(bool bForceUseEncoding,
		LPCVOID szLrc, int nLen = -1, BOOL bStrict = TRUE, BOOL bUnicode = FALSE);

	virtual LYRICS_CONTENT_TYPE GetLyrContentType() { return m_lyrContentType; }

protected:
	void AddFileRowsToMLData(PML_ROW pFileRow);

	void ReadTxtTags();

	void FixTextLrcTimeTag();
	void RemoveExtraBlankLinesOfTextLyr();

	// old
	void LRCCheckRowTime();

protected:
	LYRICS_CONTENT_TYPE		m_lyrContentType;

};

#endif // !defined(AFX_LRCPARSER_H__01A9EAFE_777D_4209_A312_DA2DE993DE78__INCLUDED_)
