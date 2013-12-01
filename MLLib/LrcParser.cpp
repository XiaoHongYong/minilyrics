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
#include "LrcTag.h"
#include "LyrScrollActionRecorder.h"
#include "LrcParserHelper.h"
#include "LyrTimeStamp.h"

IMPLEMENT_CPPUNIT_TEST_REG(LrcParser)

void RemoveExtraBlankLine(tstring &str)
{
	bool bAppendNewLine = (str.size() > 0 && str[str.size() - 1] == '\n');
	TrimStr(str, _T("\r\n"));

	int oldSize = str.size();

	while (true) 
	{
		// Replace 2 blank lines to 1 blank line
		strrep(str, _T("\r\n\r\n\r\n"), _T("\r\n\r\n"));

		LPCTSTR		p = str.c_str();
		int			nLineCount = 0, nBlankLineCount = 0;
		bool		bPrevEndLine = false;

		while (*p)
		{
			while (*p && *p != '\n')
			{
				if (*p != '\r')
					bPrevEndLine = false;
				p++;
			}
			if (*p == '\0')
				break;
			p++;

			if (bPrevEndLine)
				nBlankLineCount++;
			nLineCount++;
			bPrevEndLine = true;
		}

		if (nBlankLineCount >= nLineCount * 2 / 5)
			strrep(str, _T("\r\n\r\n"), _T("\r\n"));

		if (oldSize == str.size())
			break;
		oldSize = str.size();
	}

	if (bAppendNewLine)
		str += _T("\r\n");
}

bool ReadIntValue(LPCTSTR &szBeg, LPCTSTR szEnd, int nSize, int &nValue)
{
	LPCTSTR p = szBeg;
	nValue = 0;
	while (IsDigit(*p) && nSize > 0 && p < szEnd)
	{
		nValue *= 10;
		nValue += *p - '0';
		p++;
		nSize--;
	}

	if (nSize == 0)
	{
		szBeg = p;
		return true;
	}

	return false;
}

// [mm:ss.xx]word 1<mm:ss.xx> word 2 <mm:ss.xx> ...
LPCTSTR EatFrag(LPCTSTR szLyricsBeg, int &nEndTime, tstringex &buffLyrics)
{
	LPCTSTR		szPos;
	LPCTSTR		szLyricsEnd = szLyricsBeg + _tcslen(szLyricsBeg);

	buffLyrics.clear();
	szPos = szLyricsBeg;
	while (szPos < szLyricsEnd)
	{
		if (*szPos == '<')
		{
			int			n;
			LPCTSTR		szTemp = szPos + 1;
			nEndTime = 0;
			if (ReadIntValue(szTemp, szLyricsEnd, 2, n) && *szTemp == ':')
			{
				szTemp++;
				nEndTime = n * 60 * 1000;
				if (ReadIntValue(szTemp, szLyricsEnd, 2, n) && *szTemp == '.')
				{
					szTemp++;
					nEndTime += n * 1000;
					if (ReadIntValue(szTemp, szLyricsEnd, 3, n) && *szTemp == '>')
					{
						szTemp++;
						nEndTime += n;
						buffLyrics.append(szLyricsBeg, int(szPos - szLyricsBeg));
						return szTemp;
					}
					else if (ReadIntValue(szTemp, szLyricsEnd, 2, n) && *szTemp == '>')
					{
						szTemp++;
						nEndTime += n * 10;
						buffLyrics.append(szLyricsBeg, int(szPos - szLyricsBeg));
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

//////////////////////////////////////////////////////////////////////////

CLrcParser::CLrcParser(CMLData *pMLData, bool bTurnOffPref) : CLyricsParser(pMLData)
{
	m_lyrContentType = LCT_LRC;
	m_bTurnOffPref = bTurnOffPref;
}

CLrcParser::~CLrcParser()
{
}

int CLrcParser::ParseFile(bool bUseSpecifiedEncoding, CHAR_ENCODING encoding)
{
	stringex 		lyrics;

	if (!IsFileExist(m_pMLData->GetLyricsFileName()))
		return ERR_FILE_NOT_EXIST;

	if (!OpenFileToBuffer(m_pMLData->GetLyricsFileName(), lyrics))
		return ERR_READ_FILE;

	return ParseString((LPBYTE)lyrics.data(), lyrics.size(), false, bUseSpecifiedEncoding, encoding);
}

int CLrcParser::SaveAsFile(LPCTSTR szFile, CHAR_ENCODING newEncoding)
{
	tstring		strData;

	bool bToTxtFormat = Tobool(FileIsExtSame(szFile, _T(".txt")));
	ToString(strData, true, bToTxtFormat);

	if (!SaveStringAsFile(szFile, strData.c_str(), strData.size(), newEncoding))
		return ERR_WRITE_FILE;

	return ERR_OK;
}

int CLrcParser::ParseString(LPCTSTR szText, bool bAddTimeStampForTextLine)
{
	tstringex		strLine;
	CLyrTagNameValueMap	propNameValueMap(m_pMLData->Properties());

	if (IsEmptyString(szText))
		return ERR_EMPTY_LYRICS;

	m_pMLData->Properties().SetEncoding(DEFAULT_ENCODING);

	//
	// Parse .lrc line by line
	//
	LPCTSTR szNextLine = szText;
	while (szNextLine)
	{
		szNextLine = ReadLine(szNextLine, strLine);

		int	nNextPos, nTime;
		vector<int>	vTimes;
		LPCTSTR		szLine = strLine.c_str();

		szLine = IgnoreSpaces(szLine);

		if (*szLine != '[')
		{
			// NOT lrc tag or time stamps.
			if (bAddTimeStampForTextLine)
				AddStrAsTimedLine(szLine);
			else
				AddStrAsUnknowLine(szLine);
			continue;
		}

		while (ParseLrcTimeTag(szLine, nNextPos, nTime))
		{
			szLine += nNextPos;
			vTimes.push_back(nTime);
		}
		if (vTimes.size())
		{
			// Time tag.
			AddLyrLines(vTimes, szLine);
			continue;
		}

		// Parse lrc tag.
		int propIndex = propNameValueMap.ParseLrcTag(szLine, _tcslen(szLine));
		if (propIndex == -1)
			AddStrAsUnknowLine(szLine);
	}

	if (m_pMLData->GetLyricsLinesCount() == 0 && !bAddTimeStampForTextLine)
	{
		// .txt lyrics
		m_pMLData->ClearLyrics();

		// Parse as text lyrics
		ParseString(szText, true);

		// Parse text tags from lyrics text.
		ParseTxtTagInLyrics();

		m_lyrContentType = LCT_TXT;

		// Does text tag including time stamps.
		tstring strTimeStamps = m_pMLData->GetTxtLyrScrollEvents();
		if (CLyrTimeStamp::IsTimeStamps(strTimeStamps.c_str()))
		{
			if (CLyrTimeStamp::Parse(strTimeStamps.c_str(), m_pMLData))
				m_lyrContentType = LCT_LRC;
		}

		FinalFixOnLyrics(bAddTimeStampForTextLine);
	}
	else
	{
		m_lyrContentType = LCT_LRC;

		FinalFixOnLyrics(bAddTimeStampForTextLine);
	}

	return ERR_OK;
}

void CLrcParser::FinalFixOnLyrics(bool bAddTimeStampForTextLine)
{
	//
	// Extra fix or check on lyrics time tags.
	//
	if (m_lyrContentType == LCT_TXT)
	{
		RemoveExtraBlankLinesOfTextLyr();

		FixTextLrcTimeTag();
		return;
	}

	if (!bAddTimeStampForTextLine)
	{
		LRCCheckRowTime();
	}
	else
	{
		//
		// 将未设置时间标签的歌词行的时间设置为－1，再调用CheckLyricsLineTime重新
		// 校正时间。注意：不能调用LRCCheckRowTime()函数
		CLyricsLines	&lyrLines = m_pMLData->GetRawLyrics();
		for (int i = 0; i < (int)lyrLines.size(); i++)
		{
			LyricsLine *pLine = lyrLines[i];
			if (pLine->vFrags.size())
			{
				LyricsPiece *pPiece = pLine->vFrags[0];
				if (pPiece->bTempBegTime)
				{
					pPiece->nBegTime = -1;
					pPiece->nEndTime = -1;
					pLine->nBegTime = -1;
					pLine->nEndTime = -1;
				}
			}
		}

		m_pMLData->CheckLyricsLineTime();
	}

	m_pMLData->FixExtendedLrcTimeStamps();
}

//
// Parse .lrc or .txt lyrics text
//
int CLrcParser::ParseString(LPBYTE szLrc, int nLen, bool bAddTimeStampForTextLine, bool bUseSpecifiedEncoding, CHAR_ENCODING encoding)
{
	if (!bUseSpecifiedEncoding)
		encoding = GetLyricsTextEncoding(szLrc, nLen);

	// Save original position after removed encoding.
	LPBYTE szLrcOrg = szLrc;

	//
	// Convert lyrics to the current encoding
	//
	
	wstringex	wstrBE;
	if (encoding == ED_UNICODE_BIG_ENDIAN)
	{
		wstrBE.reserve(nLen / sizeof(WCHAR));
		wstrBE.append((WCHAR *)szLrc, nLen / sizeof(WCHAR));
		UCS2BEToLE(wstrBE.data(), wstrBE.size());
		szLrc = (LPBYTE)wstrBE.data();
	}

	tstringex		strSysEncoding;
	if (encoding == ED_UNICODE || encoding == ED_UNICODE_BIG_ENDIAN)
	{
#ifdef UTF8
		UCS2ToUtf8((LPCWSTR)szLrc, nLen / sizeof(WCHAR), strSysEncoding);
		szLrc = (LPBYTE)strSysEncoding.data();
		nLen = strSysEncoding.size();
#else
		// Unicode
		nLen = nLen / sizeof(WCHAR);
#endif
	}
	else
	{
#ifdef UNICODE
		ConvertStr((const char *)szLrc, nLen, strSysEncoding, encoding);
		szLrc = (LPBYTE)strSysEncoding.data();
		nLen = strSysEncoding.size();
#endif
#ifdef UTF8
		if (encoding != ED_UTF8)
		{
			MbcsToUtf8((LPCSTR)szLrc, nLen, strSysEncoding, encoding);
			szLrc = (LPBYTE)strSysEncoding.data();
			nLen = strSysEncoding.size();
		}
#endif
	}

	if (szLrc == szLrcOrg)
	{
		// Need to append extra '\0' at the end of string, to make sure
		// the ending of string is OK.
		strSysEncoding.clear();
		strSysEncoding.append((LPCTSTR)szLrcOrg, nLen);
		strSysEncoding.append((TCHAR)'\0');
		strSysEncoding.append((TCHAR)'\0');
		szLrc = (LPBYTE)strSysEncoding.c_str();
	}

	// Parse lyrics with current encoding.
	ParseString((LPCTSTR)szLrc, bAddTimeStampForTextLine);

	// Save the encoding information in lyrics data.
	m_pMLData->Properties().SetEncoding(encoding);

	return ERR_OK;
}

void CLrcParser::ParseTxtTagInLyrics()
{
	CLyrTagNameValueMap	propNameValueMap(m_pMLData->Properties());
	CLyricsLines	&lyrLines = m_pMLData->GetRawLyrics();

	// For some duplicated tags (lyrics file are not well written, 
	// we need to filter those tags).

	//
	// Parse text tags in the file header.
	//
	for (int i = 0; i < (int)lyrLines.size(); i++)
	{
		LyricsLine *pLine = lyrLines[i];
		ASSERT(pLine->vFrags.size() == 1);
		LyricsPiece *pPiece = pLine->vFrags[0];
		ASSERT(pPiece->IsTempBegTime());

		// Parse Txt tag.
		int propIndex = propNameValueMap.ParseTxtTag(pPiece->szLyric, _tcslen(pPiece->szLyric));
		if (propIndex != -1)
		{
			if (IsTxtTagAtTailOfFile(propNameValueMap.GetTagType(propIndex)))
			{
				// Encoding tag is very special, it may be defined by String BOM.
				if (propNameValueMap.GetTagType(propIndex) != LTT_ENCODING)
					propNameValueMap.ClearValue(propIndex);
			}
			else
			{
				propNameValueMap.Remove(propIndex);
				m_pMLData->RemoveLyricsLine(i);
				i--;
			}
		}
	}

	//
	// Parse text tags at the end of file.
	//
	for (int i = (int)lyrLines.size() - 1; i >= 0; i--)
	{
		LyricsLine *pLine = lyrLines[i];
		ASSERT(pLine->vFrags.size() == 1);
		LyricsPiece *pPiece = pLine->vFrags[0];
		ASSERT(pPiece->IsTempBegTime());

		// Parse Txt tag.
		int propIndex = propNameValueMap.ParseTxtTag(pPiece->szLyric, _tcslen(pPiece->szLyric));
		if (propIndex != -1)
		{
			propNameValueMap.Remove(propIndex);
			m_pMLData->RemoveLyricsLine(i);
		}
	}
}

void CLrcParser::ToString(tstring &str, bool bIncTags, bool bToTxtFormat)
{
	if (bToTxtFormat)
		ToTxtString(str, bIncTags);
	else
		ToLrcString(str, bIncTags, true);

	if (!m_bTurnOffPref)
	{
		// Remove extra blank line only work under !turn of preferences.
		if (!bToTxtFormat
			|| !g_Profile.GetBool(_T("AppendTimeStampsInTxtLyr"), true)
			|| m_lyrContentType == LCT_TXT)
		{
			if (g_Profile.GetBool(_T("RemoveExtraBlankLines"), true))
				RemoveExtraBlankLine(str);
		}
	}
}

void CLrcParser::ToLrcString(tstring &str, bool bIncTags, bool bLongTimeFmt)
{
	// reserver 4k spaces for lyrics data.
	str.reserve(1024 * 4);

	if (bIncTags)
	{
		CLyrTagNameValueMap	propNameValueMap(m_pMLData->Properties());

		for (int i = 0; i < propNameValueMap.GetCount(); i++)
		{
			tstring strValue = propNameValueMap.GetValue(i);
			if (strValue.size())
				str += propNameValueMap.GetLrcTagString(i);
		}
	}

	if (!bLongTimeFmt)
	{
		// Test whether is long time format
		for (int i = 0; i < (int)m_pMLData->m_arrFileLines.size(); i++)
		{
			LyricsLine *pLine = m_pMLData->m_arrFileLines[i];
			if (pLine->vFrags.empty())
				continue;

			if (!pLine->vFrags[0]->bTempBegTime)
			{
				if (pLine->nBegTime % 1000 != 0)
				{
					bLongTimeFmt = true;
					break;
				}
			}
		}
	}

	//
	// Save every lyrics line
	tstringex		strBuff;
	for (int i = 0; i < (int)m_pMLData->m_arrFileLines.size(); i++)
	{
		LyricsLine *pLine = m_pMLData->m_arrFileLines[i];
		ASSERT(pLine);

		if (LyricsLineToLrcString(pLine, bLongTimeFmt, strBuff))
		{
			str += strBuff;
			str += _T("\r\n");
		}
	}

	if (str.size() > 2 && str[str.size() - 1] == '\n')
		str.resize(str.size() - 2);
}

void CLrcParser::ToTxtString(tstring &str, bool bIncTags)
{
	// reserver 4k spaces for lyrics data.
	str.reserve(1024 * 4);

	//
	// Convert and remove lyrics line which is text lyrics tag.
	//
	tstring			strBuff;
	CLyricsLines	lyrLines(false);
	LyricsProperties properties;
	CLyrTagNameValueMap	propNameValueMap(properties);

	bool bRemoveExtraBlankLines = false;
	if (!m_bTurnOffPref)
		bRemoveExtraBlankLines = g_Profile.GetBool(_T("RemoveExtraBlankLines"), true);
	bool bLastLineBlank = false;

	for (int i = 0; i < (int)m_pMLData->m_arrFileLines.size(); i++)
	{
		LyricsLine *pLine = m_pMLData->m_arrFileLines[i];
		if (pLine->bLyricsLine)
		{
			bLastLineBlank = false;
			LyricsLineToTxtString(pLine, strBuff);
			if (propNameValueMap.ParseTxtTag(strBuff.c_str(), strBuff.size()) == -1)
			{
				lyrLines.push_back(pLine);
				str += strBuff;
				str += _T("\r\n");
			}
		}
		else
		{
			// Don't write extra blank lines.
			if (bRemoveExtraBlankLines && bLastLineBlank)
				continue;

			lyrLines.push_back(pLine);
			// Only append blank spaces, not "pLine->szContent";
			str += _T("\r\n");
			bLastLineBlank = true;
		}
	}

	if (bIncTags)
	{
		// Make a new copy of lyrics properties, we may modify it and do not want to save it.
		LyricsProperties properties = m_pMLData->Properties();
		if (m_pMLData->GetLyrContentType() == LCT_LRC)
		{
			// Convert to timestamps.
			tstring		strTimeStamps;
			CLyrTimeStamp::ToString(strTimeStamps, m_pMLData->GetOffsetTime(), lyrLines);
			properties.SetTxtLyrScrollEvents(strTimeStamps.c_str());
		}

		CLyrTagNameValueMap	propNameValueMap(properties);

		tstring		strTagsAtEndOfFile, strTagsAtBeginOfFile;
		for (int i = 0; i < propNameValueMap.GetCount(); i++)
		{
			tstring strValue = propNameValueMap.GetValue(i);
			if (strValue.size() && propNameValueMap.GetTxtName(i).size())
			{
				tstring	strTagLine = propNameValueMap.GetTxtTagString(i);
				if (IsTxtTagAtTailOfFile(propNameValueMap.GetTagType(i)))
					strTagsAtEndOfFile += strTagLine;
				else
					strTagsAtBeginOfFile += strTagLine;
			}
		}

		if (strTagsAtBeginOfFile.size())
			str.insert(0, strTagsAtBeginOfFile.c_str());

		if ((m_bTurnOffPref || g_Profile.GetBool(_T("AppendTimeStampsInTxtLyr"), true))
			&& strTagsAtEndOfFile.size())
		{
			if (str.size() && str[str.size() - 1] != '\n')
				str += _T("\r\n");
			str += strTagsAtEndOfFile;
		}
	}

	if (str.size() > 2 && str[str.size() - 2] == '\r' && str[str.size() - 1] == '\n')
		str.resize(str.size() - 2);
}

//
bool CLrcParser::LyricsLineToLrcString(LyricsLine *pLine, bool bLongTimeFmt, tstringex &strBuff)
{
	ASSERT(pLine);
	strBuff.clear();

	if (pLine->IsTempLine())
		return FALSE;

	if (!pLine->bLyricsLine)
	{
		assert(pLine->szContent);
		strBuff = pLine->szContent;
		return true;
	}

	if (pLine->vFrags.size() && !(pLine->vFrags[0]->bTempBegTime))
	{
		strBuff.reserve(100);
		int nLen = FormtLrcTimeTag(pLine->nBegTime, bLongTimeFmt, strBuff.data());
		strBuff.resize(nLen);
	}

	TCHAR			szTime[64];
	for (int i = 0; i < (int)pLine->vFrags.size(); i++)
	{
		LyricsPiece *pPiece = pLine->vFrags[i];

		strBuff += pPiece->szLyric;

		if (i + 1 < (int)pLine->vFrags.size())
		{
			pPiece = pLine->vFrags[i + 1];
			if (!pPiece->bTempBegTime)
			{
				int		nMin, nSec, nMs;
				int		nTime = pPiece->nBegTime + 10 / 2;

				nMs = (nTime / 10) % 100;
				nSec = (nTime / 1000) % 60;
				nMin = nTime / (1000 * 60);
				_stprintf_s(szTime, CountOf(szTime), _T("<%02d:%02d.%02d>"), nMin, nSec, nMs);
				strBuff += szTime;
			}
		}
	}
	if (strBuff.empty())
		return false;	// do not insert temp empty lyrics 

	return TRUE;
}

//
void CLrcParser::LyricsLineToTxtString(LyricsLine *pLine, tstring &strBuff)
{
	ASSERT(pLine->bLyricsLine);
	strBuff.clear();

	if (pLine->IsTempLine())
		return;

	for (int i = 0; i < (int)pLine->vFrags.size(); i++)
	{
		LyricsPiece *pPiece = pLine->vFrags[i];
		strBuff += pPiece->szLyric;
	}
}

//
// Add lyrics lyrics line
// [mm:ss.xx]word 1<mm:ss.xx> word 2 <mm:ss.xx> ...
void CLrcParser::AddLyrLines(vector<int> &vTimes, LPCTSTR szLyrLine)
{
	int				nBegTime, nEndTime;
	tstringex		buff;

	for (int i = 0; i < (int)vTimes.size(); i++)
	{
		nBegTime = vTimes[i];
		LyricsLine *pLine = NewLyricsLine(nBegTime, -1);

		LPCTSTR	szLyricsBeg = szLyrLine;
		do
		{
			nEndTime = -1;
			szLyricsBeg = EatFrag(szLyricsBeg, nEndTime, buff);
			
			pLine->AppendPiece(nBegTime, nEndTime, buff.c_str(), buff.size(), FALSE, TRUE);

			if (nEndTime != -1)
				nBegTime = nEndTime;
		}
		while (*szLyricsBeg);

		m_nLatestTime = nBegTime;

		if (pLine->vFrags.size() > 1)
			m_pMLData->SetKaraokeStyle(true);

		m_pMLData->AddLineByTime(pLine);
	}

	m_lyrContentType = LCT_LRC;
}


//
// Add text file lyrics row
//
void CLrcParser::AddStrAsTimedLine(const TCHAR *szLine)
{
	LyricsLine *pLine = NewLyricsLine(m_nLatestTime, -1);
	pLine->AppendPiece(m_nLatestTime, -1, szLine, _tcslen(szLine), TRUE, TRUE);

	m_pMLData->AddLineByTime(pLine);
}

void CLrcParser::AddStrAsUnknowLine(const TCHAR *szLine)
{
	// Unknown tag, just add it in the file.
	LyricsLine *pLine = NewLyricsLine(0, 0, szLine, _tcslen(szLine), false);
	m_pMLData->AddLine(pLine);
}

void CLrcParser::FixTextLrcTimeTag()
{
	if (m_pMLData->GetLyricsLinesCount() == 0)
		return;

	int		nBegTime, nEndTime, nSpan;

	nEndTime = m_pMLData->GetMediaLength();
	if (nEndTime < 0)
		nEndTime = 5 * 60 * 1000;	// 5 minutes

	nBegTime = nEndTime / 15;
	if (nBegTime > 15)
		nBegTime = 15;

	nEndTime -= nBegTime;

	if (m_pMLData->GetLyricsLinesCount() == 1)
		nSpan = 0;
	else
		nSpan = (nEndTime - nBegTime) / (m_pMLData->GetLyricsLinesCount() - 1);

	for (int i = 0; i < (int)m_pMLData->GetLyricsLinesCount(); i++)
	{
		LyricsLine *pLine = m_pMLData->GetLyricsLine(i);

		if (pLine->vFrags.size() > 0)
		{
			pLine->vFrags[0]->nBegTime = pLine->nBegTime = nBegTime;
			pLine->vFrags[0]->nEndTime = pLine->nEndTime = nBegTime + nSpan;
		}
		nBegTime += nSpan;
	}
}

void CLrcParser::RemoveExtraBlankLinesOfTextLyr()
{
	int			i, nBlankLines = 0;

	for (i = 0; i < (int)m_pMLData->GetLyricsLinesCount(); i++)
	{
		LyricsLine *pLine = m_pMLData->GetLyricsLine(i);

		if (pLine->vFrags.size() == 1)
		{
			if (IsEmptyString(pLine->vFrags[0]->szLyric))
				nBlankLines++;
			else
				nBlankLines = 0;
		}
		else if (pLine->vFrags.size() == 0)
			nBlankLines++;
		else
			nBlankLines = 0;
		if (nBlankLines >= 3)
		{
			// remove this line
			m_pMLData->RemoveLyricsLine(i);
			i--;
			nBlankLines--;
		}
	}

	//
	// if the blank lines take the half part of all lyrics, remove them all.
	//
	if (m_pMLData->GetLyricsLinesCount() < 10)
		return;

	// Get blank line count of lyrics.
	nBlankLines = 0;
	for (i = 0; i < (int)m_pMLData->GetLyricsLinesCount(); i++)
	{
		LyricsLine *pLine = m_pMLData->GetLyricsLine(i);

		if (pLine->vFrags.size() == 1)
		{
			if (IsEmptyString(pLine->vFrags[0]->szLyric))
				nBlankLines++;
		}
		else if (pLine->vFrags.size() == 0)
			nBlankLines++;
	}

	if (nBlankLines >= m_pMLData->GetLyricsLinesCount() / 2)
	{
		nBlankLines = 0;
		for (i = 0; i < (int)m_pMLData->GetLyricsLinesCount(); i++)
		{
			LyricsLine *pLine = m_pMLData->GetLyricsLine(i);

			if (pLine->vFrags.size() == 1)
			{
				if (IsEmptyString(pLine->vFrags[0]->szLyric))
					nBlankLines++;
			}
			else if (pLine->vFrags.size() == 0)
				nBlankLines++;
			if (nBlankLines > 0)
			{
				// remove this line
				m_pMLData->RemoveLyricsLine(i);
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

	nSize = m_pMLData->GetLyricsLinesCount();
	
	// 如果歌词段的开始时间 < 此行歌词的开始时间，则修改
	for (i = 0; i < nSize; i ++)
	{
		LyricsLine *pLine = m_pMLData->GetLyricsLine(i);
		nSizeFrag = pLine->vFrags.size();

		for (int k = 0; k < nSizeFrag; k++)
		{
			LyricsPiece *pPiece = pLine->vFrags[k];

			if (pPiece->nBegTime < pLine->nBegTime)
				pPiece->nBegTime = pLine->nBegTime;
		}
	}
	
/*
	// 如果两行歌词的时间没有按先后顺序排，则修改之。
	for (i = 0; i < nSize; i ++)
	{
		LyricsLine *pLine;
		LyricsPiece *pPiece;
		int				k;
		int				nIdx;
		PLYRIC_ROW		pLine1;

		pLine = m_pMLData->m_arrLyricsLines[i];
		pLine = pLine1 = m_pMLData->m_arrLyricsLines[i];
		nIdx = i;
		for (k = i + 1; k < nSize; k ++)
		{
			PLYRIC_ROW	pLine2;
			pLine2 = (PLYRIC_ROW)m_pMLData->m_arrLyricsLines[k];
			if (pLine2->nBegTime < pLine->nBegTime )
			{
				pLine = pLine2;
				nIdx = k;
			}
		}
		// exchange prow and prow1
		if (pLine != pLine1)
		{
			m_pMLData->m_arrLyricsLines.SetAt(nIdx, pLine1);
			m_pMLData->m_arrLyricsLines.SetAt(i, pLine);
		}
	}*/
	
	// 检查每一行歌词中的所有歌词段是否正确，并且进行矫正。
	for (i = 0; i < nSize; i ++)
	{
		LyricsLine *pLine, *pLineNext;
		LyricsPiece *pPiece, *pPieceNext;

		pLine = m_pMLData->GetLyricsLine(i);

		// 如果此行的开始时间小于结束时间
		if (pLine->nEndTime < pLine->nBegTime)
		{
			if (i + 1 >= nSize)
				pLine->nEndTime = pLine->nBegTime + 10 * 1000;	// delay ten second
			else
			{
				pLineNext = m_pMLData->GetLyricsLine(i + 1);
				pLine->nEndTime = pLineNext->nBegTime;
			}
		}
		
		// 对每一段歌词的时间正确性进行校验
		nSizeFrag = pLine->vFrags.size();
		for (int k = 0; k < nSizeFrag; k++)
		{
			pPiece = pLine->vFrags[k];

			if (pPiece->nEndTime < pPiece->nBegTime)
			{
				if (k + 1 < nSizeFrag)
				{
					pPieceNext = pLine->vFrags[k + 1];
					pPiece->nEndTime = pPieceNext->nBegTime;
				}
				else
					pPiece->nEndTime = pLine->nEndTime;
			}
			else
			{
				if (k + 1 < nSizeFrag)
				{
					pPieceNext = pLine->vFrags[k + 1];
					if (pPiece->nEndTime > pPieceNext->nBegTime)
						pPiece->nEndTime = pPieceNext->nBegTime;
				}
				else
				{
					if (pPiece->nEndTime > pLine->nEndTime)
						pPiece->nEndTime = pLine->nEndTime;
				}
			}
		}
	}

	CLyricsLines &lyrLines = m_pMLData->GetRawLyrics();

	// If a lyrics line is 0 length, recover to display multiple line at same time.
	for (int i = 0; i < lyrLines.size(); i++)
	{
		LyricsLine *pLine = lyrLines[i];
		if (pLine->nBegTime != pLine->nEndTime || pLine->vFrags.size() != 1)
			continue;

		int nextLine = i;
		for (; nextLine < lyrLines.size(); nextLine++)
		{
			LyricsLine *pNext = lyrLines[nextLine];
			if (pNext->nBegTime == pLine->nBegTime && pNext->nBegTime != pNext->nEndTime)
			{
				// Next line is not 0 length.
				pLine->nEndTime = pNext->nEndTime;
				pLine->vFrags[0]->nEndTime = pNext->nEndTime;
				break;
			}

			if (pNext->nBegTime != pLine->nBegTime)
			{
				// Find till next line has different begin time.
				pLine->nEndTime = pNext->nBegTime;
				pLine->vFrags[0]->nEndTime = pNext->nBegTime;
				break;
			}
		}
	}
}


#ifdef _CPPUNIT_TEST

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

void EncodeTimeStamps(int duration, tstring &timeStamps);

LPCTSTR DecodeTimeStamps(LPCTSTR szTimeStamps, int &duration);

class CTestCaseLrcParser : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseLrcParser);
	CPPUNIT_TEST(Test_EncodeTimeStamps);
	CPPUNIT_TEST(Test_CLrcParser_Lrc);
	CPPUNIT_TEST(Test_CLrcParser_Txt);
	CPPUNIT_TEST_SUITE_END();

protected:
	void Test_EncodeTimeStamps()
	{
		for (int nDuration = -22 * 1000; nDuration < 1000 * 30; nDuration += 100)
		{
			tstring timeStamps;
			int nDuration2 = 0;

			EncodeTimeStamps(nDuration, timeStamps);

			LPCTSTR szRet = DecodeTimeStamps(timeStamps.c_str(), nDuration2);
			CPPUNIT_ASSERT(IsEmptyString(szRet));
			CPPUNIT_ASSERT(nDuration2 == nDuration);
		}
	}

	void Test_CLrcParser_Lrc()
	{
		LPCSTR	SZ_LYRICS_TEST = "[ar:artist]\n[ti:title]\n[encoding:big5]\n[offset:-500]\n[00:01.12]L1\n[00:02.01]\nDummy Line\n[00:03:01]L3\nL4\n\n";
		LPCTSTR		valArtist = _T("artist");
		LPCTSTR		valTitle = _T("title");
		LPCTSTR		vLyrLines[] = { _T("L1"), _T(""), _T("L3") };
		int			vTime[] = { 1 * 1000 + 120, 2 * 1000 + 10, 3 *1000 + 10 };

		wstringex	wStr;
		ConvertStr(SZ_LYRICS_TEST, -1, wStr);

		wstringex	wLyrics;
		wLyrics.append((const WCHAR *)SZ_FE_UCS2, 1);
		wLyrics.append(wStr.c_str(), wStr.size());

		wstringex	wLyricsBe;
		wLyricsBe.append(wLyrics.c_str(), wLyrics.size());
		UCS2LEToBE(wLyricsBe.data(), wLyricsBe.size());

		stringex	utf8Lyrics;
		utf8Lyrics.append(SZ_FE_UTF8);
		utf8Lyrics.append(SZ_LYRICS_TEST);

		LPBYTE		vLyrics[] = { (LPBYTE)SZ_LYRICS_TEST, (LPBYTE)wLyrics.c_str(),		(LPBYTE)wLyricsBe.c_str(),			(LPBYTE)utf8Lyrics.c_str(), };
		int			vLen[] = { strlen(SZ_LYRICS_TEST),	wLyrics.size() * sizeof(WCHAR), wLyricsBe.size() * sizeof(WCHAR),	utf8Lyrics.size(), };
		CHAR_ENCODING	vEncoding[] = { ED_BIG5,	ED_UNICODE,						ED_UNICODE_BIG_ENDIAN,				ED_UTF8, };


		for (int i = 0; i < CountOf(vLyrics); i++)
		{
			CMLData			data;
			CLrcParser		parser(&data, true);

			int nRet = parser.ParseString(vLyrics[i], vLen[i], false, false);
			CPPUNIT_ASSERT(nRet == ERR_OK);
			data.Properties().m_lyrContentType = parser.GetLyrContentType();

			CPPUNIT_ASSERT(_tcscmp(data.Properties().m_strArtist.c_str(), valArtist) == 0);
			CPPUNIT_ASSERT(_tcscmp(data.Properties().m_strTitle.c_str(), valTitle) == 0);
			CPPUNIT_ASSERT(data.Properties().GetEncodingIndex() == vEncoding[i]);

			CLyricsLines	&lyrLines = data.GetRawLyrics();

			for (int k = 0; k < lyrLines.size(); k++)
			{
				LyricsLine *pLine = lyrLines[k];

				CPPUNIT_ASSERT(pLine->vFrags.size() == 1);
				CPPUNIT_ASSERT(k < CountOf(vLyrLines));
				CPPUNIT_ASSERT(_tcscmp(pLine->vFrags[0]->szLyric, vLyrLines[k]) == 0);
				CPPUNIT_ASSERT(pLine->vFrags[0]->nBegTime == vTime[k]);
			}
			CPPUNIT_ASSERT(lyrLines.size() == CountOf(vLyrLines));

			//
			// Cases on CLrcParser::ToString
			//
			tstring		strNewLyr;
			CLrcParser		parser2(&data, true);
			parser2.ToString(strNewLyr, true, false);

			CMLData			data3;
			CLrcParser		parser3(&data3, true);
			parser3.ParseString(strNewLyr.c_str(), false);
			data3.Properties().SetEncoding(vEncoding[i]);

			{
				// Compare ToString data.
				CLrcTag		tag(data.Properties(), data3.Properties());
				CPPUNIT_ASSERT(!tag.IsTagChanged()); // data and data3 should be same.

				CPPUNIT_ASSERT(_tcscmp(data3.Properties().m_strArtist.c_str(), valArtist) == 0);
				CPPUNIT_ASSERT(_tcscmp(data3.Properties().m_strTitle.c_str(), valTitle) == 0);
				CPPUNIT_ASSERT(data3.Properties().GetEncodingIndex() == vEncoding[i]);

				CLyricsLines	&lyrLines = data3.GetRawLyrics();

				for (int k = 0; k < lyrLines.size(); k++)
				{
					LyricsLine *pLine = lyrLines[k];

					CPPUNIT_ASSERT(pLine->vFrags.size() == 1);
					CPPUNIT_ASSERT(k < CountOf(vLyrLines));
					CPPUNIT_ASSERT(_tcscmp(pLine->vFrags[0]->szLyric, vLyrLines[k]) == 0);
					CPPUNIT_ASSERT(pLine->vFrags[0]->nBegTime == vTime[k]);
				}
				CPPUNIT_ASSERT(lyrLines.size() == CountOf(vLyrLines));
			}

			//
			// Case: Convert to text including timestamps
			//
			tstring			strTxtLyr;
			CLrcParser		parserTxt(&data, true);
			parserTxt.ToString(strTxtLyr, true, true);

			CMLData			data5;
			CLrcParser		parser5(&data5, true);
			parser5.ParseString(strTxtLyr.c_str(), false);
			data5.Properties().SetEncoding(vEncoding[i]);

			// The new parsed data5 shall be same with previous data.
			CPPUNIT_ASSERT(data5.Properties().GetOffsetTime() == data.GetOffsetTime());

			// Compare line by line
			CLyricsLines	&lyrLines2 = data5.GetRawLyrics();
			for (int k = 0; k < lyrLines2.size(); k++)
			{
				LyricsLine *pLine = lyrLines2[k];

				CPPUNIT_ASSERT(pLine->vFrags.size() == 1);
				CPPUNIT_ASSERT(_tcscmp(pLine->vFrags[0]->szLyric, vLyrLines[k]) == 0);
				// 100 = TIME UNIT (TIME_STAMP_UNIT) in timestamps.
				CPPUNIT_ASSERT(pLine->vFrags[0]->nBegTime == (vTime[k] + 50) / 100 * 100);
			}

		}
	}

	void Test_CLrcParser_Txt()
	{
		LPCSTR		SZ_LYRICS_TEST = "Artist:artist\nTitle: title\nEncoding:big5\nL1\n\n\nL3\nL4\n\nL6";
		LPCTSTR		valArtist = _T("artist");
		LPCTSTR		valTitle = _T("title");
		LPCTSTR		vLyrLines[] = { _T("L1"), _T(""), _T(""), _T("L3"), _T("L4"), _T(""), _T("L6") };

		wstringex	wStr;
		ConvertStr(SZ_LYRICS_TEST, -1, wStr);

		wstringex	wLyrics;
		wLyrics.append((const WCHAR *)SZ_FE_UCS2, 1);
		wLyrics.append(wStr.c_str(), wStr.size());

		wstringex	wLyricsBe;
		wLyricsBe.append(wLyrics.c_str(), wLyrics.size());
		UCS2LEToBE(wLyricsBe.data(), wLyricsBe.size());

		stringex	utf8Lyrics;
		utf8Lyrics.append(SZ_FE_UTF8);
		utf8Lyrics.append(SZ_LYRICS_TEST);

		LPBYTE		vLyrics[] = { (LPBYTE)SZ_LYRICS_TEST, (LPBYTE)wLyrics.c_str(),		(LPBYTE)wLyricsBe.c_str(),			(LPBYTE)utf8Lyrics.c_str(), };
		int			vLen[] = { strlen(SZ_LYRICS_TEST),	wLyrics.size() * sizeof(WCHAR), wLyricsBe.size() * sizeof(WCHAR),	utf8Lyrics.size(), };
		CHAR_ENCODING	vEncoding[] = { ED_BIG5,	ED_UNICODE,						ED_UNICODE_BIG_ENDIAN,				ED_UTF8, };


		for (int i = 0; i < CountOf(vLyrics); i++)
		{
			CMLData			data;
			CLrcParser		parser(&data, true);

			int nRet = parser.ParseString(vLyrics[i], vLen[i], false, false);
			CPPUNIT_ASSERT(nRet == ERR_OK);

			CPPUNIT_ASSERT(_tcscmp(data.Properties().m_strArtist.c_str(), valArtist) == 0);
			CPPUNIT_ASSERT(_tcscmp(data.Properties().m_strTitle.c_str(), valTitle) == 0);
			CPPUNIT_ASSERT(data.Properties().GetEncodingIndex() == vEncoding[i]);

			CLyricsLines	&lyrLines = data.GetRawLyrics();

			for (int k = 0; k < lyrLines.size(); k++)
			{
				LyricsLine *pLine = lyrLines[k];

				CPPUNIT_ASSERT(pLine->vFrags.size() == 1);
				CPPUNIT_ASSERT(k < CountOf(vLyrLines));
				CPPUNIT_ASSERT(_tcscmp(pLine->vFrags[0]->szLyric, vLyrLines[k]) == 0);
			}
			CPPUNIT_ASSERT(lyrLines.size() == CountOf(vLyrLines));

			//
			// Cases on CLrcParser::ToString
			//
			tstring		strNewLyr;
			CLrcParser		parser2(&data, true);
			parser2.ToString(strNewLyr, true, true);

			CMLData			data3;
			CLrcParser		parser3(&data3, true);
			parser3.ParseString(strNewLyr.c_str(), false);
			data3.Properties().SetEncoding(vEncoding[i]);

			{
				// Compare ToString data.
				CLrcTag		tag(data.Properties(), data3.Properties());
				CPPUNIT_ASSERT(!tag.IsTagChanged()); // data and data3 should be same.

				CPPUNIT_ASSERT(_tcscmp(data3.Properties().m_strArtist.c_str(), valArtist) == 0);
				CPPUNIT_ASSERT(_tcscmp(data3.Properties().m_strTitle.c_str(), valTitle) == 0);
				CPPUNIT_ASSERT(data3.Properties().GetEncodingIndex() == vEncoding[i]);

				CLyricsLines	&lyrLines = data3.GetRawLyrics();

				for (int k = 0; k < CountOf(vLyrLines); k++)
				{
					LyricsLine *pLine = lyrLines[k];

					CPPUNIT_ASSERT(pLine->vFrags.size() == 1);
					CPPUNIT_ASSERT(k < lyrLines.size());
					CPPUNIT_ASSERT(_tcscmp(pLine->vFrags[0]->szLyric, vLyrLines[k]) == 0);
				}
				// New generated lyrics may increase 1 line by adding encoding.
				// CPPUNIT_ASSERT(lyrLines.size() == CountOf(vLyrLines));
			}
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseLrcParser);

#endif // _CPPUNIT_TEST
