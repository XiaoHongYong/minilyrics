/********************************************************************
	Created  :	2002/01/03	17:23:02
	FileName :	LrcParser.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "StdAfx.h"
#include "MLLib.h"
#include "LyricsParser.h"
#include "LrcParser.h"
#include "LyricsTagReader.h"
#include "LyrScrollActionRecorder.h"


#define	SZ_VERSION			"version:"
#define	SZ_AR				"ar:"
#define	SZ_TI				"ti:"
#define	SZ_AL				"al:"
#define	SZ_BY				"by:"
#define	SZ_ENCODING			"Encoding:"
#define	SZ_OFFSET			"offset:"


#define _IsCharBlank(ch) (ch == ' ' || ch == '	')

#define _IsCharReturn(ch) (ch == 0xa || ch == 0xd)


//////////////////////////////////////////////////////////////////////////
int CLrcParserObj::Compile(LPVOID szLyric, int nLen, BOOL bConvertStr)
{
	if (nLen == 0)
		return ERR_OK;

	assert(nLen > 0);

	m_pMLData->ClearLyrics();

	// word analysis
	tstringex		strValue;
	CBuffWord		buffWords;
	COMPILE_WORD	word;
	TCHAR		szEmpty[1];
	szEmpty[0] = '\0';

	m_nLatestTime = 0;
	
	// lrc word analysis
	CompileWordAnalysis(buffWords, (TCHAR *)szLyric);

// #ifdef _DEBUG
// 	// 在调试时，将词法分析的结构显示处理
// 	buffWords.DbgOut();
// #endif

	CBuffWord	buffTimeTags;
	PML_ROW		pFileRow = NULL;
	int			nTagType;

	// 词法分析完成，进行简单的语法“分析”

	if (!buffWords.pop_read_front(word))
		goto S_END;
S_BEGIN:
	if (word.nType == WORD_LRC_END)
		goto S_END;
	if (word.nType == WORD_LRC_STRING)
	{
		goto S_UNKNOW_TAG;
	}
	if (word.nType == WORD_LRC_TAG)
	{
		nTagType = GetTagType(word.szValue, word.nValLen);

		if (nTagType == TAG_TIME)
		{
			goto S_TIME1;
		}
		else if (nTagType == TAG_UNKNOWN)
		{
			// insert as a invalid tag string
			goto S_UNKNOW_TAG;
		}
		else
		{
			// other tags
			goto S_OTHER_TAGS;
		}
	}
	if (word.nType == WORD_LRC_NEWROW)
	{
		if (bConvertStr)	// Keep empty line in text lyrcs file.
			AddStrTimeRow(_T(""), 0);
		else
		{
			pFileRow = new ML_ROW;
			pFileRow->nRowType = ML_ROW_NULL;	// TAG_NEWROW;
			pFileRow->szContent = NULL;

			AddFileRowsToMLData(pFileRow);
		}

		if (!buffWords.pop_read_front(word))
			goto S_END;

		goto S_BEGIN;
	}

	//other unknow WORD_?????
	goto S_END;

S_UNKNOW_TAG:
	if (!ReadTxtTag(word.szValue, word.nValLen))
	{
		if (bConvertStr)
		{
			AddStrTimeRow(word.szValue, word.nValLen);
		}
		else
		{
			pFileRow = m_pMLData->NewMLRow(0, 0, word.szValue, word.nValLen, ML_ROW_UNKNOW);

			AddFileRowsToMLData(pFileRow);
		}
	}
	if (!buffWords.pop_read_front(word))
		goto S_END;
	if (word.nType == WORD_LRC_NEWROW)
	{
		// 因为这些属性标记后面的“新行NEWROW”标志应该去掉
		if (!buffWords.pop_read_front(word))
			goto S_END;
	}

	goto S_BEGIN;

S_OTHER_TAGS:
	strValue.clear();

	GetOtherTagValue(word.szValue, word.nValLen, strValue);

	if (nTagType == TAG_OFFSET)
	{
		// Get offset time
		if (CLyrScrollActionRecorder::IsEventTag(strValue.c_str()))
		{
			// Set event tag.
			m_pMLData->SetTxtLyrScrollEvents(strValue.c_str());
		}
		else
			m_pMLData->SetOffsetTime(_ttoi(strValue.c_str()));
	}
	else if (nTagType == TAG_ARTIST)
		m_pMLData->Properties().m_strArtist = strValue.c_str();
	else if (nTagType == TAG_TITLE)
		m_pMLData->Properties().m_strTitle = strValue.c_str();
	else if (nTagType == TAG_BY)
		m_pMLData->Properties().m_strBy = strValue.c_str();
	else if (nTagType == TAG_AL)
		m_pMLData->Properties().m_strAlbum = strValue.c_str();

	if (!buffWords.pop_read_front(word))
		goto S_END;
	if (word.nType == WORD_LRC_NEWROW)
	{
		// 因为这些属性标记后面的“新行NEWROW”标志应该去掉
		if (!buffWords.pop_read_front(word))
			goto S_END;
	}
	
	goto S_BEGIN;
	
S_TIME1:
	COMPILE_WORD wordNext;

	// read next error!
	if (!buffWords.pop_read_front(wordNext))
	{
		AddTimeRow(&word, szEmpty, 0);
		goto S_END;
	}
	
	if (wordNext.nType == WORD_LRC_END)
	{
		AddTimeRow(&word, szEmpty, 0);
		goto S_END;
	}
	
	if (wordNext.nType == WORD_LRC_NEWROW)
	{
		AddTimeRow(&word, szEmpty, 0);

		if (!buffWords.pop_read_front(word))
			goto S_END;
		goto S_BEGIN;
	}
	
	// string
	if (wordNext.nType == WORD_LRC_STRING)
	{
		AddTimeRow(&word, wordNext.szValue, wordNext.nValLen);

		if (!buffWords.pop_read_front(word))
			goto S_END;
		// 去掉行尾的回车
		if (word.nType == WORD_LRC_NEWROW)
		{
			if (!buffWords.pop_read_front(word))
				goto S_END;
		}
		goto S_BEGIN;
	}

	// tag again?
	if (wordNext.nType == WORD_LRC_TAG)
	{
		nTagType = GetTagType(wordNext.szValue, wordNext.nValLen);
		if (nTagType == TAG_TIME)
		{
			buffTimeTags.clear();
			buffTimeTags.push_back(word);
			buffTimeTags.push_back(wordNext);
			goto S_TIME2;
		}
		else if (nTagType == TAG_UNKNOWN)
		{
			// insert as a invalid tag string
			AddTimeRow(&word, szEmpty, 0);

			word = wordNext;
			goto S_UNKNOW_TAG;
		}
		else
		{
			// other tags
			AddTimeRow(&word, szEmpty, 0);

			word = wordNext;
			goto S_OTHER_TAGS;
		}
		// no more
	}
	
	// no more
	goto S_BEGIN;
S_TIME2:
	// read next error!
	if (!buffWords.pop_read_front(word))
	{
		AddTimeRow(&buffTimeTags, szEmpty, 0);
		goto S_END;
	}
	
	if (word.nType == WORD_LRC_END)
	{
		AddTimeRow(&buffTimeTags, szEmpty, 0);
		goto S_END;
	}
	
	if (word.nType == WORD_LRC_NEWROW)
	{
		AddTimeRow(&buffTimeTags, szEmpty, 0);

		if (!buffWords.pop_read_front(word))
			goto S_END;
		goto S_BEGIN;
	}

	// string
	if (word.nType == WORD_LRC_STRING)
	{
		AddTimeRow(&buffTimeTags, word.szValue, word.nValLen);
		
		if (!buffWords.pop_read_front(word))
			goto S_END;

		// 去掉行尾的回车
		if (word.nType == WORD_LRC_NEWROW)
		{
			if (!buffWords.pop_read_front(word))
				goto S_END;
		}
		goto S_BEGIN;
	}
	
	// tag again?
	if (word.nType == WORD_LRC_TAG)
	{
		nTagType = GetTagType(word.szValue, word.nValLen);
		if (nTagType == TAG_TIME)
		{
			buffTimeTags.push_back(word);
			goto S_TIME2;
		}
		else if (nTagType == TAG_UNKNOWN)
		{
			// insert as a invalid tag string
			AddTimeRow(&buffTimeTags, szEmpty, 0);

			goto S_UNKNOW_TAG;
		}
		else
		{
			// other tags
			AddTimeRow(&buffTimeTags, szEmpty, 0);

			goto S_OTHER_TAGS;
		}
		// no more
	}
	
	// no more
	goto S_BEGIN;
S_END:

	// ended ok
	return ERR_OK;
}

int CLrcParserObj::CompileWordAnalysis(CBuffWord &buffWords, TCHAR *szLyric)
{
	TCHAR		*szTagBegin = NULL;
	TCHAR		*szWordBegin, *szCur;
	COMPILE_WORD	word;
	int				n;

	szCur = szLyric;

WORD_ANALYSIS_BEGIN:
	// end 
	if (*szCur == '\0')
	{
		word.nType = WORD_LRC_END;
		word.szValue = NULL;
		buffWords.push_back(word);
		goto WORD_ANALYSIS_END;
	}

	// new row
	if ((*szCur == '\r' && *(szCur + 1) == '\n') || *szCur == '\n' || *szCur == '\r')
	{
		word.nType = WORD_LRC_NEWROW;
		word.szValue = NULL;
		buffWords.push_back(word);

		if (*szCur == '\r' && *(szCur + 1) == '\n')
			szCur += 2;
		else
			szCur++;
		goto WORD_ANALYSIS_BEGIN;
	}
	// others, continued to LRC_TAG_READ_BEGIN
	szWordBegin = szCur;

LRC_TAG_READ_BEGIN:
	// search char '['
	while (*szCur && *szCur != '[' && *szCur != '\r' && *szCur != '\n')
		szCur++;
	if (*szCur != '[')
		goto NOT_LRC_TAG;
	szTagBegin = szCur;
	szCur++;

	// read to char ':'
	while ((*szCur) && (*szCur) != ':' && (*szCur) != ']' &&
		(*szCur) != '[' && *szCur != '\r' && *szCur != '\n')
		szCur++;
	if (*szCur != ':')
		goto NOT_LRC_TAG;
	szCur++;

	// read to char ']'
	n = 0;
	while ((*szCur) && *szCur != '\r' && *szCur != '\n')
	{
		if ((*szCur) == '[')
			n++;
		if ((*szCur) == ']')
			n--;
		if (n < 0)
			break;
		szCur++;
	}
	if (*szCur != ']')
		goto NOT_LRC_TAG;

	szCur ++;	// seek beyound ']'
	// ok it is really a lrc tag

	if (szWordBegin != szTagBegin)
	{
		word.szValue = szWordBegin;
		word.nType = WORD_LRC_STRING;
		word.nValLen = szTagBegin - szWordBegin;
		buffWords.push_back(word);
	}
	word.nType = WORD_LRC_TAG;
	word.szValue = szTagBegin;
	word.nValLen = szCur - szTagBegin;
	buffWords.push_back(word);
	goto WORD_ANALYSIS_BEGIN;
	
NOT_LRC_TAG:
	if (_IsCharReturn(*szCur) || (*szCur == 0))
	{
		word.nType = WORD_LRC_STRING;
		word.szValue = szWordBegin;
		word.nValLen = szCur - szWordBegin;
		buffWords.push_back(word);
		
		goto WORD_ANALYSIS_BEGIN;
	}

	// must be other chars	'[', ':', ']'
	// so re read lrc tags again
	goto LRC_TAG_READ_BEGIN;

WORD_ANALYSIS_END:
	return ERR_OK;
}


// PURPOSE:
//		Read tag type and set data
// PARAMETER:
//	szTag - input tag string, its format is like '[xxx:ooo]'
int CLrcParserObj::GetTagType(TCHAR *szTag, int nLen)
{
	if (*szTag != '[')
		goto S_ERROR;
	szTag ++;

	// ignore black space
	while (_IsCharBlank(*szTag))
		szTag++;
	
	if (IsTagStringMatch(szTag, SZ_VERSION, nLen))
		return TAG_VERSION;
	else if (IsTagStringMatch(szTag, SZ_AR, nLen))
		return TAG_ARTIST;
	else if (IsTagStringMatch(szTag, SZ_TI, nLen))
		return TAG_TITLE;
	else if (IsTagStringMatch(szTag, SZ_AL, nLen))
		return TAG_AL;
	else if (IsTagStringMatch(szTag, SZ_ENCODING, nLen))
		return TAG_ENCODING;
	else if (IsTagStringMatch(szTag, SZ_BY, nLen))
		return TAG_BY;
	else if (IsTagStringMatch(szTag, SZ_OFFSET, nLen))
		return TAG_OFFSET;
	else
	{
		// time tag ?
		// seek to ':'
		if (*szTag == '-')
			szTag++;
		while (IsDigit(*szTag))
			szTag++;
		while (_IsCharBlank(*szTag))
			szTag++;

		// ':'
		if (*szTag != ':' && *szTag != '.')
			goto S_ERROR;
		szTag ++;

		// number
		while (_IsCharBlank(*szTag))
			szTag ++;
		if (*szTag == '-')
			szTag++;
		while (IsDigit(*szTag))
			szTag ++;
		while (_IsCharBlank(*szTag))
			szTag ++;

		if (*szTag == '.' || *szTag == ':')
		{
			szTag ++;
			// new version of lrc file
			while (_IsCharBlank(*szTag))
				szTag ++;
			if (*szTag == '-')
				szTag++;
			while (IsDigit(*szTag))
				szTag ++;
			while (_IsCharBlank(*szTag))
				szTag ++;
		}
		if (*szTag != ']')
			goto S_ERROR;

		return TAG_TIME;
	}

S_ERROR:
	return TAG_UNKNOWN;
}

bool CLrcParserObj::ReadTxtTag(TCHAR *szTag, int nLen)
{
	tstring			strValue;

	CMLData::LYRICS_PROP_NAME	*lyricsPropNames = CMLData::GetSupportedLyricsPropName();

	for (int i = 0; lyricsPropNames[i].szName != NULL; i++)
	{
		int		nNameLen = _tcslen(lyricsPropNames[i].szName);
		if (_tcsnicmp(szTag, lyricsPropNames[i].szName, nNameLen) == 0)
		{
			strValue.append(szTag + nNameLen, nLen - nNameLen);
			TrimStr(strValue);
			tstring strOrg;
			m_pMLData->GetProperty(lyricsPropNames[i].prop, strOrg);
			if (strOrg.empty())
				m_pMLData->SetProperty(lyricsPropNames[i].prop, strValue.c_str());
			return true;
		}
	}

	return false;
}

//
// Add text file lyrics row
//
void CLrcParserObj::AddStrTimeRow(TCHAR *szLyric, int nLen)
{
	PLYRIC_FRAG		pFrag;
	PML_ROW			pRow;

	{
		pFrag = m_pMLData->NewLyricFrag(m_nLatestTime, -1, szLyric, nLen, TRUE, TRUE);
	//	m_nLatestTime += 10;

		pRow = m_pMLData->NewMLRow(pFrag->nBegTime, pFrag->nEndTime, (const char *)NULL);
		pRow->vFrags.push_back(pFrag);

		AddFileRowsToMLData(pRow);
	}
}


bool ReadIntValue(TCHAR *&szBeg, TCHAR *szEnd, int nSize, int &nValue)
{
	nValue = 0;
	while (IsDigit(*szBeg) && nSize > 0 && szBeg < szEnd)
	{
		nValue *= 10;
		nValue += *szBeg - '0';
		szBeg++;
		nSize--;
	}
	return nSize == 0;
}

// [mm:ss.xx]word 1<mm:ss.xx> word 2 <mm:ss.xx> ...
TCHAR *CLrcParserObj::EatFrag(TCHAR *szLyricsBeg, TCHAR *szLyricsEnd, int &nEndTime, tstringex &buffLyrics)
{
	TCHAR	*szPos;

	buffLyrics.clear();
	szPos = szLyricsBeg;
	while (szPos < szLyricsEnd)
	{
		if (*szPos == '<')
		{
			int			n;
			TCHAR	*szTemp = szPos + 1;
			nEndTime = 0;
			if (ReadIntValue(szTemp, szLyricsEnd, 2, n) && *szTemp == ':')
			{
				szTemp++;
				nEndTime = n * 60 * 1000;
				if (ReadIntValue(szTemp, szLyricsEnd, 2, n) && *szTemp == '.')
				{
					szTemp++;
					nEndTime += n * 1000;
					if (ReadIntValue(szTemp, szLyricsEnd, 2, n) && *szTemp == '>')
					{
						szTemp++;
						nEndTime += n * 10;
						buffLyrics.append(szLyricsBeg, int(szPos - szLyricsBeg));
						m_pMLData->SetDefShowStyle(LSA_SHOW_KARAOKE);
						return szTemp;
					}
				}
			}
		}
		szPos++;
	}

	buffLyrics.append(szLyricsBeg, int(szLyricsEnd - szLyricsBeg));
	return szLyricsEnd;
}

//
// add lrc lyrics row
// [mm:ss.xx]word 1<mm:ss.xx> word 2 <mm:ss.xx> ...
void CLrcParserObj::AddTimeRow(COMPILE_WORD *pCompFrag, TCHAR *szLyric, int nLen)
{
	PLYRIC_FRAG		pFrag;
	PML_ROW			pRow;
	int				nBegTime, nEndTime;
	tstringex		buff;
	TCHAR		*szLyricsBeg, *szLyricsEnd;

	szLyricsBeg = szLyric;
	szLyricsEnd = szLyric + nLen;

	nBegTime = GetTimeTagValue(pCompFrag->szValue, pCompFrag->nValLen);
	pRow = m_pMLData->NewMLRow(nBegTime, -1, (const char *)NULL);

	do
	{
		nEndTime = -1;
		szLyricsBeg = EatFrag(szLyricsBeg, szLyricsEnd, nEndTime, buff);
		pFrag = m_pMLData->NewLyricFrag(nBegTime, nEndTime, buff.c_str(), buff.size(), FALSE, TRUE);

		pRow->vFrags.push_back(pFrag);

		if (nEndTime != -1)
			nBegTime = nEndTime;
	}
	while (szLyricsBeg < szLyricsEnd);

	m_nLatestTime = nBegTime;

	m_lyrContentType = LCT_LRC;

	AddFileRowsToMLData(pRow);
}

void CLrcParserObj::AddTimeRow(CBuffWord *pbuffCompFrags, TCHAR *szLyric, int nLen)
{
	COMPILE_WORD	CompFrag;


	while (pbuffCompFrags->pop_read_front(CompFrag))
	{
		// read ok
		AddTimeRow(&CompFrag, szLyric, nLen);
	}
}

int CLrcParserObj::GetTimeTagValue(TCHAR *szTag, int nLen)
{
	int		nTime;

	nTime = 0;

	if (*szTag != '[')
		goto S_ERROR;

	szTag++;
	// time tag ?
	// seek to ':'
	while (_IsCharBlank(*szTag))
		szTag ++;

	nTime += _xtoi(szTag) * 60 * 1000;	// nTime += atoi(szTag) * 60s * 1000ms
	if (*szTag == '-')
		szTag++;
	while (IsDigit(*szTag))
		szTag ++;
	while (_IsCharBlank(*szTag))
		szTag ++;
	
	// ':'
	if (*szTag != ':' && *szTag != '.')
		goto S_ERROR;
	szTag ++;
	
	// number
	while (_IsCharBlank(*szTag))
		szTag ++;

	nTime += _xtoi(szTag) * 1000;	// nTime += atoi(szTag) * 1000ms
	if (*szTag == '-')
		szTag++;
	while (IsDigit(*szTag))
		szTag ++;
	while (_IsCharBlank(*szTag))
		szTag ++;

	if (*szTag == '.' || *szTag == ':')
	{
		szTag ++;
		// new version of lrc file
		while (_IsCharBlank(*szTag))
			szTag ++;

		if (isdigit(szTag[1]))
		{
			// .000 or // .00
			if (isdigit(szTag[2]))
				nTime += _xtoi(szTag);
			else
				nTime += _xtoi(szTag) * 10;
		}
		else
			nTime += _xtoi(szTag) * 100;	// .0
		if (*szTag == '-')
			szTag++;

		while (IsDigit(*szTag))
			szTag ++;
		while (_IsCharBlank(*szTag))
			szTag ++;
	}
	if (*szTag != ']')
		goto S_ERROR;

S_ERROR:
	return nTime;
}

// COMMENT:
//		
// INPUT:
//		[artist:xxoo]
void CLrcParserObj::GetOtherTagValue(const TCHAR *szTag, int nLenTag, tstringex &strValue)
{
	int					i;
	const TCHAR		*szBeg;

	// EmptyStr(szValue);

	for (i = 0; i < nLenTag; i++)
	{
		if (szTag[i] == ':')
		{
			szBeg = szTag + i + 1;
			strValue.append(szBeg, nLenTag - (szBeg - szTag) - 1);
			// strncpysz_safe(szValue, nSizeValue, szBeg, nLenTag - (szBeg - szTag) - 1);
			return;
		}
	}
}

//void CLrcParserObj::GetOtherTagValue(const TCHAR *szTag, int nLenTag, TCHAR *szValue, int nSizeValue)
//{
//	int					i;
//	const TCHAR		*szBeg;
//
//	EmptyStr(szValue);
//
//	for (i = 0; i < nLenTag; i++)
//	{
//		if (szTag[i] == ':')
//		{
//			szBeg = szTag + i + 1;
//			strncpysz_safe(szValue, nSizeValue, szBeg, nLenTag - (szBeg - szTag) - 1);
//			return;
//		}
//	}
//}
//
void CLrcParserObj::AddFileRowsToMLData(PML_ROW pFileRow)
{
	m_pMLData->AddRowByTime(pFileRow);
}

bool CLrcParserObj::IsTagStringMatch(const TCHAR *szDest, const char *szTagName, int nLen1)
{
	int i, j;

	// 0? or not match break
	i = 0, j = 0;
	while (i < nLen1 && szTagName[j] != '\0' &&
		(szDest[i] - szTagName[j]) % ('a' - 'A') == 0)
		i++, j++;
	
	if (szTagName[j] == '\0')
		return true;

	return false;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLrcParser::CLrcParser(CMLData *pMLData) : CLyricsParser(pMLData)
{
	m_lyrContentType = LCT_LRC;
}

CLrcParser::~CLrcParser()
{

}

int CLrcParser::Parse(bool bForceUseEncoding)
{
	stringex 		lyric;

	if (!IsFileExist(m_pMLData->GetLyricsFileName()))
		return ERR_FILE_NOT_EXIST;

	if (!OpenFileToBuffer(m_pMLData->GetLyricsFileName(), lyric))
		return ERR_READ_FILE;

	return ParseString(bForceUseEncoding, lyric.c_str(), lyric.size(), TRUE);
}

//
// 解析字符串lrc格式的歌词
int CLrcParser::ParseString(bool bForceUseEncoding, 
	LPCVOID szLrc, int nLen, BOOL bStrict, BOOL bUnicode)
{
	int			nRet = ERR_OK;
	BYTE		*szBuffer;
	FILE_ENCODING	fileEncoding;

	szBuffer = (BYTE *)szLrc;
	fileEncoding = GetStrEncodingType(szBuffer);

	if (!bForceUseEncoding)
	{
		// 使用文件的编码
		CLyricsTagReader<TCHAR>		lrctagReader;

		lrctagReader.ReadEncodingFromStr((const char *)szLrc, nLen, bUnicode != 0);
		if (lrctagReader.m_strEncoding.size())
			m_pMLData->SetEncoding(lrctagReader.m_strEncoding.c_str());

#ifdef _WIN32_DESKTOP
		if (fileEncoding != FE_UCS2)
		{
			if (strlen((const char *)szLrc) <= 3 && nLen >= 10)
			{
				int		nFlag = IS_TEXT_UNICODE_UNICODE_MASK;
				bUnicode = IsTextUnicode((void *)szLrc, nLen, &nFlag);
			}
		}
#endif
	}

#ifdef _UNICODE
	wstringex	strLyrics;
	//
	// for windows, uncode encoding...
	//
	if (!((fileEncoding == FE_UCS2 && !bForceUseEncoding) || bUnicode ||
		(bForceUseEncoding && m_pMLData->GetEncodingIndex() == ED_UNICODE)))
	{
		// convert to unicode
		if ((fileEncoding == FE_UTF8 && !bForceUseEncoding)
			|| (bForceUseEncoding && m_pMLData->GetEncodingIndex() == ED_UTF8))
		{
			if (fileEncoding == FE_UTF8)
			{
				szBuffer += 3;
				nLen -= 3;
				m_pMLData->SetEncoding(_T(SZ_UTF8));
			}
			m_fileEncoding = FE_UTF8;
		}
		else
			m_fileEncoding = FE_ANSI;

		ConvertStr((char*)szBuffer, nLen, strLyrics, m_pMLData->GetEncodingIndex());
		if (strLyrics.empty() && nLen > 0)
			ConvertStr((char*)szBuffer, nLen, strLyrics, ED_SYSDEF);
		szBuffer = (BYTE*)strLyrics.c_str();
		nLen = strLyrics.size() * sizeof(WCHAR);
	}
	else
	{
		if (fileEncoding == FE_UCS2)
		{
			szBuffer += 2;
			nLen -= 2;
		}

		m_fileEncoding = FE_UCS2;
	}

	{
		// UNICODE
		CLrcParserObj	parser(m_pMLData);

		nRet = parser.Compile(szBuffer, nLen, !bStrict);
		if (m_pMLData->GetLyricRowCount() == 0)
		{
			//
			// 可能是解析歌词错误！重新使用不太严格的方式解析
			nRet = parser.Compile(szBuffer, nLen, TRUE);
		}

		m_lyrContentType = parser.GetLyrContentType();
	}
#else
	stringex	strLyrics;
	//
	// for linux, utf-8 encoding
	//
	if (bForceUseEncoding && bUnicode)
	{
		// internal use string buffer...
	}
	else if (fileEncoding != FE_UTF8 && m_pMLData->GetEncodingIndex() != ED_UTF8)
	{
		// convert to utf-8
		if (fileEncoding == FE_UCS2)
		{
			szBuffer += 2;
			nLen -= 2;

			UCS2ToUtf8((WCHAR*)szBuffer, (nLen + 1) / sizeof(WCHAR), strLyrics);
		}
		else
		{
			MbcsToUtf8((const char*)szBuffer, nLen, strLyrics, m_pMLData->GetEncodingIndex());
		}

		szBuffer = (BYTE*)strLyrics.c_str();
		nLen = strLyrics.size();
	}
	else
	{
		if (fileEncoding == FE_UCS2)
		{
			szBuffer += 2;
			nLen -= 2;
		}

		m_fileEncoding = FE_UCS2;
	}

	{
		// utf-8
		CLrcParserObj	parser(m_pMLData);

		nRet = parser.Compile(szBuffer, nLen, !bStrict);
		if (m_pMLData->GetLyricRowCount() == 0)
		{
			//
			// 可能是解析歌词错误！重新使用不太严格的方式解析
			nRet = parser.Compile(szBuffer, nLen, TRUE);
		}

		m_lyrContentType = parser.GetLyrContentType();
	}
#endif


	if (m_lyrContentType == LCT_TXT)
	{
		// 读取其中的标签.
		CHAR_ENCODING			nEncodingID;
		nEncodingID = m_pMLData->GetEncodingIndex();
		if (bForceUseEncoding)
			m_pMLData->SetEncoding(nEncodingID);

		RemoveExtraBlankLinesOfTextLyr();

		// 文本格式的歌词，需要重新设置歌词时间标签
		FixTextLrcTimeTag();

		return nRet;
	}

	if (bStrict)
	{
		LRCCheckRowTime();
	}
	else
	{
		//
		// 将未设置时间标签的歌词行的时间设置为－1，再调用CheckLyricsRowTime重新
		// 校正时间。注意：不能调用LRCCheckRowTime()函数
		int		nSize;
		nSize = m_pMLData->GetLyricRowCount();
		for (int	i = 0; i < nSize; i ++)
		{
			PML_ROW			pRow;

			pRow = m_pMLData->GetLyricsRow(i);
			if (pRow->vFrags.size())
			{
				PLYRIC_FRAG		pFrag = pRow->vFrags[0];
				if ((pFrag->dwAttrib & LSA_TEMP_TIME_BEG) == LSA_TEMP_TIME_BEG)
				{
					pFrag->nBegTime = -1;
					pFrag->nEndTime = -1;
					pRow->nBegTime = -1;
					pRow->nEndTime = -1;
				}
			}
		}

		m_pMLData->CheckLyricsRowTime();
	}

	m_pMLData->FixExtendedLrcTimeStamps();

	return nRet;
}

void CLrcParser::FixTextLrcTimeTag()
{
	if (m_pMLData->GetLyricRowCount() == 0)
		return;

	int		nBegTime, nEndTime, nSpan;

	nEndTime = m_pMLData->GetMediaLength();
	if (nEndTime < 0)
		nEndTime = 5 * 60 * 1000;	// 5 minutes

	nBegTime = nEndTime / 15;
	if (nBegTime > 15)
		nBegTime = 15;

	nEndTime -= nBegTime;

	if (m_pMLData->GetLyricRowCount() == 1)
		nSpan = 0;
	else
		nSpan = (nEndTime - nBegTime) / (m_pMLData->GetLyricRowCount() - 1);

	for (int i = 0; i < (int)m_pMLData->GetLyricRowCount(); i++)
	{
		PML_ROW		pRow = m_pMLData->GetLyricsRow(i);

		if (pRow->vFrags.size() > 0)
		{
			pRow->vFrags[0]->nBegTime = pRow->nBegTime = nBegTime;
			pRow->vFrags[0]->nEndTime = pRow->nEndTime = nBegTime + nSpan;
		}
		nBegTime += nSpan;
	}
}

void CLrcParser::RemoveExtraBlankLinesOfTextLyr()
{
	int			i, nBlankLines = 0;

	for (i = 0; i < (int)m_pMLData->GetLyricRowCount(); i++)
	{
		PML_ROW		pRow = m_pMLData->GetLyricsRow(i);

		if (pRow->vFrags.size() == 1)
		{
			if (IsEmptyString(pRow->vFrags[0]->szLyric))
				nBlankLines++;
			else
				nBlankLines = 0;
		}
		else if (pRow->vFrags.size() == 0)
			nBlankLines++;
		else
			nBlankLines = 0;
		if (nBlankLines >= 3)
		{
			// remove this line
			m_pMLData->RemoveLyricsRow(i);
			i--;
			nBlankLines--;
		}
	}

	//
	// if the blank lines take the half part of all lyrics, remove them all.
	//
	if (m_pMLData->GetLyricRowCount() < 10)
		return;

	// Get blank line count of lyrics.
	nBlankLines = 0;
	for (i = 0; i < (int)m_pMLData->GetLyricRowCount(); i++)
	{
		PML_ROW		pRow = m_pMLData->GetLyricsRow(i);

		if (pRow->vFrags.size() == 1)
		{
			if (IsEmptyString(pRow->vFrags[0]->szLyric))
				nBlankLines++;
		}
		else if (pRow->vFrags.size() == 0)
			nBlankLines++;
	}

	if (nBlankLines >= m_pMLData->GetLyricRowCount() / 2)
	{
		nBlankLines = 0;
		for (i = 0; i < (int)m_pMLData->GetLyricRowCount(); i++)
		{
			PML_ROW		pRow = m_pMLData->GetLyricsRow(i);

			if (pRow->vFrags.size() == 1)
			{
				if (IsEmptyString(pRow->vFrags[0]->szLyric))
					nBlankLines++;
			}
			else if (pRow->vFrags.size() == 0)
				nBlankLines++;
			if (nBlankLines > 0)
			{
				// remove this line
				m_pMLData->RemoveLyricsRow(i);
				i--;
				nBlankLines = 0;
			}
		}
	}
}

void CLrcParser::LRCCheckRowTime()
{
	int		nSize;
	int		nSizeFrag;
	int		i;

	nSize = m_pMLData->GetLyricRowCount();
	
	// 如果歌词段的开始时间 < 此行歌词的开始时间，则修改
	for (i = 0; i < nSize; i ++)
	{
		PML_ROW			pRow;
		PLYRIC_FRAG		pFrag;

		pRow = m_pMLData->GetLyricsRow(i);
		nSizeFrag = pRow->vFrags.size();

		for (int k = 0; k < nSizeFrag; k++)
		{
			pFrag = pRow->vFrags[k];

			if (pFrag->nBegTime < pRow->nBegTime)
				pFrag->nBegTime = pRow->nBegTime;
		}
	}
	
/*
	// 如果两行歌词的时间没有按先后顺序排，则修改之。
	for (i = 0; i < nSize; i ++)
	{
		PML_ROW			pRow;
		PLYRIC_FRAG		pFrag;
		int				k;
		int				nIdx;
		PLYRIC_ROW		pRow1;

		pRow = m_pMLData->m_arrLyricsRows[i];
		pRow = pRow1 = m_pMLData->m_arrLyricsRows[i];
		nIdx = i;
		for (k = i + 1; k < nSize; k ++)
		{
			PLYRIC_ROW	pRow2;
			pRow2 = (PLYRIC_ROW)m_pMLData->m_arrLyricsRows[k];
			if (pRow2->nBegTime < pRow->nBegTime )
			{
				pRow = pRow2;
				nIdx = k;
			}
		}
		// exchange prow and prow1
		if (pRow != pRow1)
		{
			m_pMLData->m_arrLyricsRows.SetAt(nIdx, pRow1);
			m_pMLData->m_arrLyricsRows.SetAt(i, pRow);
		}
	}*/
	
	// 检查每一行歌词中的所有歌词段是否正确，并且进行矫正。
	for (i = 0; i < nSize; i ++)
	{
		PML_ROW			pRow, pRowNext;
		PLYRIC_FRAG		pFrag, pFragNext;

		pRow = m_pMLData->GetLyricsRow(i);

		// 如果此行的开始时间小于结束时间
		if (pRow->nEndTime < pRow->nBegTime)
		{
			if (i + 1 >= nSize)
				pRow->nEndTime = pRow->nBegTime + 10 * 1000;	// delay ten second
			else
			{
				pRowNext = m_pMLData->GetLyricsRow(i + 1);
				pRow->nEndTime = pRowNext->nBegTime;
			}
		}
		
		// 对每一段歌词的时间正确性进行校验
		nSizeFrag = pRow->vFrags.size();
		for (int k = 0; k < nSizeFrag; k++)
		{
			pFrag = pRow->vFrags[k];

			if (pFrag->nEndTime < pFrag->nBegTime)
			{
				if (k + 1 < nSizeFrag)
				{
					pFragNext = pRow->vFrags[k + 1];
					pFrag->nEndTime = pFragNext->nBegTime;
				}
				else
					pFrag->nEndTime = pRow->nEndTime;
			}
			else
			{
				if (k + 1 < nSizeFrag)
				{
					pFragNext = pRow->vFrags[k + 1];
					if (pFrag->nEndTime > pFragNext->nBegTime)
						pFrag->nEndTime = pFragNext->nBegTime;
				}
				else
				{
					if (pFrag->nEndTime > pRow->nEndTime)
						pFrag->nEndTime = pRow->nEndTime;
				}
			}
		}
	}
}

int CLrcParser::SaveAsFile(LPCTSTR szFile, FILE_ENCODING fileEncoding)
{
	tstring		strData;

	ToString(strData, true);

	if (!SaveStringAsFile(szFile, strData.c_str(), strData.size(), fileEncoding, (CHAR_ENCODING)m_pMLData->GetEncodingIndex()))
		return ERR_WRITE_FILE;

	return ERR_OK;
}


BOOL CLrcParser::ToString(tstring &str, BOOL bIncTags)
{
	int			nCount;
	tstringex	strBuff;
	TCHAR		szBuff[1024];

	if (bIncTags)
	{
		//
		// 保存时间偏移值
		if (m_pMLData->GetLyricsSourceType() == LCT_TXT)
		{
			if (!IsEmptyString(m_pMLData->GetTxtLyrScrollEvents()))
			{
				_stprintf_s(szBuff, CountOf(szBuff), _T("[offset:%s]\r\n"), m_pMLData->GetTxtLyrScrollEvents());
				str += szBuff;
			}
		}
		else if (m_pMLData->GetOffsetTime() != 0)
		{
			_stprintf_s(szBuff, CountOf(szBuff), _T("[offset:%d]\r\n"), m_pMLData->GetOffsetTime());
			str += szBuff;
		}

		//
		// 保存其他属性: 作者、标题等……
		if (m_pMLData->Properties().m_strArtist.size())
		{
			_stprintf_s(szBuff, CountOf(szBuff), _T("[ar:%s]\r\n"), m_pMLData->Properties().m_strArtist.c_str());
			str += szBuff;
		}
		if (m_pMLData->Properties().m_strTitle.size())
		{
			_stprintf_s(szBuff, CountOf(szBuff), _T("[ti:%s]\r\n"), m_pMLData->Properties().m_strTitle.c_str());
			str += szBuff;
		}
		if (m_pMLData->Properties().m_strAlbum.size())
		{
			_stprintf_s(szBuff, CountOf(szBuff), _T("[al:%s]\r\n"), m_pMLData->Properties().m_strAlbum.c_str());
			str += szBuff;
		}
		if (m_pMLData->Properties().m_strBy.size())
		{
			_stprintf_s(szBuff, CountOf(szBuff), _T("[by:%s]\r\n"), m_pMLData->Properties().m_strBy.c_str());
			str += szBuff;
		}
		if (!IsEmptyString(m_pMLData->GetEncoding()))
		{
			_stprintf_s(szBuff, CountOf(szBuff), _T("[Encoding:%s]\r\n"), m_pMLData->GetEncoding());
			str += szBuff;
		}
	}

	//
	// 保存歌词行
	nCount = m_pMLData->m_arrFileRows.size();
	for (int i = 0; i < nCount; i++)
	{
		PML_ROW		pRow;

		pRow = m_pMLData->m_arrFileRows[i];

		if (pRow)
		{
			// save row.
			if (LyricsRowToText(pRow, strBuff))
			{
				str += strBuff;
				str += _T("\r\n");
			}
		}
	}

	return TRUE;
}

// COMMENT:
//		将输入的歌词行转换为LRC格式的字符串行
BOOL CLrcParser::LyricsRowToText(PML_ROW pRow, tstringex &strBuff)
{
	strBuff.clear();

	switch (pRow->nRowType)
	{
	case ML_ROW_UNKNOW:		// 不能识别的行
		assert(pRow->szContent);
		strBuff = pRow->szContent;
		break;
	case ML_ROW_NULL:		// 空行
		break;
	case ML_ROW_LYRICS:		// 歌词行
		{
			if (pRow->vFrags.size() && !(pRow->vFrags[0]->dwAttrib & LSA_TEMP_TIME_BEG))
			{
				int		nMin, nSec, nMs;
				int		nLen;

				nMs = (pRow->nBegTime / 10) % 100;
				nSec = (pRow->nBegTime / 1000) % 60;
				nMin = pRow->nBegTime / (1000 * 60);

				// _stprintf(szBuff, _T("[%02d:%02d.%02d]"), nMin, nSec, nMs);
				strBuff.reserve(100);
				nLen = _stprintf_s(strBuff.data(), strBuff.capacity(), _T("[%02d:%02d.%02d]"), nMin, nSec, nMs);
				strBuff.setSize(nLen);
			}

			int				nCount;
			PLYRIC_FRAG		pFrag;
			TCHAR			szTime[64];
			nCount = pRow->vFrags.size();
			for (int i = 0; i < nCount; i++)
			{
				pFrag = pRow->vFrags[i];
				// 去掉插入的注册信息，广告信息等
				if (IsTempLyricsFrag(pFrag->dwAttrib))
				{
					strBuff.clear();
					return FALSE;
				}

				strBuff += pFrag->szLyric;

				if (i + 1 < nCount)
				{
					pFrag = pRow->vFrags[i + 1];
					if (!IsTempTimeBeg(pFrag->dwAttrib))
					{
						int		nMin, nSec, nMs;

						nMs = (pFrag->nBegTime / 10) % 100;
						nSec = (pFrag->nBegTime / 1000) % 60;
						nMin = pFrag->nBegTime / (1000 * 60);
						_stprintf_s(szTime, CountOf(szTime), _T("<%02d:%02d.%02d>"), nMin, nSec, nMs);
						strBuff += szTime;
					}
				}
			}
			if (strBuff.empty())
				return false;	// do not insert temp empty lyrics 
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}
