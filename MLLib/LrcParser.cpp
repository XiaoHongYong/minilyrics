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


bool ReadIntValue(LPCTSTR &szBeg, LPCTSTR szEnd, int nSize, int &nValue)
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
					if (ReadIntValue(szTemp, szLyricsEnd, 2, n) && *szTemp == '>')
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

CLrcParser::CLrcParser(CMLData *pMLData) : CLyricsParser(pMLData)
{
	m_lyrContentType = LCT_LRC;
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
		return ERR_NOT_SUPPORT_FILE_FORMAT;

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
		int		nValBeg, nValEnd;
		int		i;
		for (i = 0; i < propNameValueMap.GetCount(); i++)
		{
			if (ParseLrcPropTag(szLine, _tcslen(szLine), 
				propNameValueMap.GetLrcName(i), nValBeg, nValEnd))
			{
				propNameValueMap.SetValue(i, szLine + nValBeg, nValEnd - nValBeg);
				break;
			}
		}
		if (i == propNameValueMap.GetCount())
			AddStrAsUnknowLine(szLine);
	}

	if (m_pMLData->GetLyricRowCount() == 0 && !bAddTimeStampForTextLine)
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
		// 将未设置时间标签的歌词行的时间设置为－1，再调用CheckLyricsRowTime重新
		// 校正时间。注意：不能调用LRCCheckRowTime()函数
		VLyrLines	&lyrLines = m_pMLData->GetRawLyrics();
		for (int i = 0; i < (int)lyrLines.size(); i++)
		{
			PML_ROW			pRow = lyrLines[i];
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
}

//
// Parse .lrc or .txt lyrics text
//
int CLrcParser::ParseString(LPBYTE szLrc, int nLen, bool bAddTimeStampForTextLine, bool bUseSpecifiedEncoding, CHAR_ENCODING encoding)
{
	if (!bUseSpecifiedEncoding)
		encoding = GetLyricsTextEncoding(szLrc, nLen);

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
			MbcsToUtf8(szLrc, nLen, strSysEncoding, encoding);
			szLrc = (LPBYTE)strSysEncoding.data();
			nLen = strSysEncoding.size();
		}
#endif
	}

	m_pMLData->Properties().SetEncoding(encoding);
	ParseString((LPCTSTR)szLrc, bAddTimeStampForTextLine);

	return ERR_OK;
}

void CLrcParser::ParseTxtTagInLyrics()
{
	CLyrTagNameValueMap	propNameValueMap(m_pMLData->Properties());
	VLyrLines	&lyrLines = m_pMLData->GetRawLyrics();
	for (int i = 0; i < (int)lyrLines.size(); i++)
	{
		PML_ROW		pRow = lyrLines[i];
		ASSERT(pRow->vFrags.size() == 1);
		PLYRIC_FRAG	pFrag = pRow->vFrags[0];
		ASSERT(pFrag->IsTempBegTime());

		// Parse Txt tag.
		int		nValBeg, nValEnd;
		for (int k = 0; k < propNameValueMap.GetCount(); k++)
		{
			if (ParseTxtPropTag(pFrag->szLyric, _tcslen(pFrag->szLyric), 
				propNameValueMap.GetTxtName(k), nValBeg, nValEnd))
			{
				propNameValueMap.SetValue(k, pFrag->szLyric + nValBeg, nValEnd - nValBeg);
				m_pMLData->RemoveLyricsRow(i);
				i--;
				break;
			}
		}
	}
}

bool CLrcParser::ToString(tstring &str, bool bIncTags, bool bToTxtFormat)
{
	if (bToTxtFormat)
		return ToTxtString(str, bIncTags);
	else
		return ToLrcString(str, bIncTags, true);
}

bool CLrcParser::ToLrcString(tstring &str, bool bIncTags, bool bLongTimeFmt)
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
		for (int i = 0; i < (int)m_pMLData->m_arrFileRows.size(); i++)
		{
			PML_ROW		pRow = m_pMLData->m_arrFileRows[i];
			if (pRow->nRowType != ML_ROW_LYRICS
				|| pRow->vFrags.empty())
				continue;

			if (!IsFlagSet(pRow->vFrags[0]->dwAttrib, LSA_TEMP_TIME_BEG))
			{
				if (pRow->nBegTime % 1000 != 0)
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
	for (int i = 0; i < (int)m_pMLData->m_arrFileRows.size(); i++)
	{
		PML_ROW		pRow = m_pMLData->m_arrFileRows[i];
		ASSERT(pRow);

		if (LyricsRowToLrcString(pRow, bLongTimeFmt, strBuff))
		{
			str += strBuff;
			str += _T("\r\n");
		}
	}

	if (str.size() > 2 && str[str.size() - 1] == '\n')
		str.resize(str.size() - 2);

	return true;
}

bool CLrcParser::ToTxtString(tstring &str, bool bIncTags)
{
	// reserver 4k spaces for lyrics data.
	str.reserve(1024 * 4);
	tstring		strTagsAtEndOfFile;

	if (bIncTags)
	{
		int		nLrcOffsetTime = 0;
		if (m_pMLData->GetLyrContentType() == LCT_LRC)
		{
			// Convert to timestamps.
			tstring		strTimeStamps;
			CLyrTimeStamp::ToString(strTimeStamps, m_pMLData);
			nLrcOffsetTime = m_pMLData->Properties().GetOffsetTime();
			m_pMLData->Properties().SetTxtLyrScrollEvents(strTimeStamps.c_str());
		}

		CLyrTagNameValueMap	propNameValueMap(m_pMLData->Properties());

		for (int i = 0; i < propNameValueMap.GetCount(); i++)
		{
			tstring strValue = propNameValueMap.GetValue(i);
			if (strValue.size() && propNameValueMap.GetTxtName(i).size())
			{
				tstring	strTagLine = propNameValueMap.GetTxtTagString(i);
				if (IsTxtTagAtTailOfFile(propNameValueMap.GetTagType(i)))
					strTagsAtEndOfFile += strTagLine;
				else
					str += strTagLine;
			}
		}

		// Recover offset time.
		if (m_pMLData->GetLyrContentType() == LCT_LRC)
		{
			m_pMLData->Properties().SetTxtLyrScrollEvents(_T(""));
			m_pMLData->Properties().SetOffsetTime(nLrcOffsetTime);
		}
	}

	//
	// Save every lyrics line
	tstringex		strBuff;
	for (int i = 0; i < (int)m_pMLData->m_arrFileRows.size(); i++)
	{
		PML_ROW		pRow = m_pMLData->m_arrFileRows[i];
		ASSERT(pRow);

		if (pRow->nRowType == ML_ROW_LYRICS)
			LyricsRowToTxtString(pRow, strBuff);
		else
		{
			ASSERT(pRow->nRowType == ML_ROW_UNKNOW);
			strBuff = pRow->szContent;
		}

		str += strBuff;
		str += _T("\r\n");
	}

	if (strTagsAtEndOfFile.size())
	{
		if (str.size() && str[str.size() - 1] != '\n')
			str += _T("\r\n");
		str += strTagsAtEndOfFile;
	}

	if (str.size() > 2 && str[str.size() - 1] == '\n')
		str.resize(str.size() - 2);

	return true;
}

//
bool CLrcParser::LyricsRowToLrcString(PML_ROW pRow, bool bLongTimeFmt, tstringex &strBuff)
{
	ASSERT(pRow);
	strBuff.clear();

	switch (pRow->nRowType)
	{
	case ML_ROW_UNKNOW:
		assert(pRow->szContent);
		strBuff = pRow->szContent;
		break;
	case ML_ROW_LYRICS:
		{
			if (pRow->vFrags.size() && !(pRow->vFrags[0]->dwAttrib & LSA_TEMP_TIME_BEG))
			{
				strBuff.reserve(100);
				int nLen = FormtLrcTimeTag(pRow->nBegTime, bLongTimeFmt, strBuff.data());
				strBuff.resize(nLen);
			}

			TCHAR			szTime[64];
			for (int i = 0; i < (int)pRow->vFrags.size(); i++)
			{
				PLYRIC_FRAG	pFrag = pRow->vFrags[i];
				// Remove ad strings
				if (IsTempLyricsFrag(pFrag->dwAttrib))
				{
					strBuff.clear();
					return FALSE;
				}

				strBuff += pFrag->szLyric;

				if (i + 1 < (int)pRow->vFrags.size())
				{
					pFrag = pRow->vFrags[i + 1];
					if (!IsTempTimeBeg(pFrag->dwAttrib))
					{
						int		nMin, nSec, nMs;
						int		nTime = pFrag->nBegTime + 10 / 2;

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
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

//
void CLrcParser::LyricsRowToTxtString(PML_ROW pRow, tstringex &strBuff)
{
	ASSERT(pRow->nRowType == ML_ROW_LYRICS);
	strBuff.clear();

	for (int i = 0; i < (int)pRow->vFrags.size(); i++)
	{
		PLYRIC_FRAG	pFrag = pRow->vFrags[i];
		// Remove ad strings
		if (IsTempLyricsFrag(pFrag->dwAttrib))
		{
			strBuff.clear();
			return;
		}

		strBuff += pFrag->szLyric;
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
		PML_ROW			pRow;

		nBegTime = vTimes[i];
		pRow = m_pMLData->NewMLRow(nBegTime, -1);

		LPCTSTR			szLyricsBeg = szLyrLine;
		do
		{
			nEndTime = -1;
			szLyricsBeg = EatFrag(szLyricsBeg, nEndTime, buff);
			PLYRIC_FRAG	pFrag = m_pMLData->NewLyricFrag(nBegTime, nEndTime, buff.c_str(), buff.size(), FALSE, TRUE);

			pRow->vFrags.push_back(pFrag);

			if (nEndTime != -1)
				nBegTime = nEndTime;
		}
		while (*szLyricsBeg);

		m_nLatestTime = nBegTime;

		if (pRow->vFrags.size() > 1)
			m_pMLData->SetDefShowStyle(LSA_SHOW_KARAOKE);

		m_pMLData->AddRowByTime(pRow);
	}

	m_lyrContentType = LCT_LRC;
}


//
// Add text file lyrics row
//
void CLrcParser::AddStrAsTimedLine(const TCHAR *szLine)
{
	PLYRIC_FRAG		pFrag;
	PML_ROW			pRow;

	pFrag = m_pMLData->NewLyricFrag(m_nLatestTime, -1, szLine, _tcslen(szLine), TRUE, TRUE);
	//	m_nLatestTime += 10;

	pRow = m_pMLData->NewMLRow(pFrag->nBegTime, pFrag->nEndTime);
	pRow->vFrags.push_back(pFrag);

	m_pMLData->AddRowByTime(pRow);
}

void CLrcParser::AddStrAsUnknowLine(const TCHAR *szLine)
{
	// Unknown tag, just add it in the file.
	PML_ROW pRow = m_pMLData->NewMLRow(0, 0, szLine, _tcslen(szLine), ML_ROW_UNKNOW);
	m_pMLData->AddRow(pRow);
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
			CLrcParser		parser(&data);

			int nRet = parser.ParseString(vLyrics[i], vLen[i], false, false);
			CPPUNIT_ASSERT(nRet == ERR_OK);
			data.Properties().m_lyrContentType = parser.GetLyrContentType();

			CPPUNIT_ASSERT(_tcscmp(data.Properties().m_strArtist.c_str(), valArtist) == 0);
			CPPUNIT_ASSERT(_tcscmp(data.Properties().m_strTitle.c_str(), valTitle) == 0);
			CPPUNIT_ASSERT(data.Properties().GetEncodingIndex() == vEncoding[i]);

			VLyrLines	&lyrLines = data.GetRawLyrics();

			for (int k = 0; k < lyrLines.size(); k++)
			{
				PML_ROW	pRow = lyrLines[k];

				CPPUNIT_ASSERT(pRow->vFrags.size() == 1);
				CPPUNIT_ASSERT(k < CountOf(vLyrLines));
				CPPUNIT_ASSERT(_tcscmp(pRow->vFrags[0]->szLyric, vLyrLines[k]) == 0);
				CPPUNIT_ASSERT(pRow->vFrags[0]->nBegTime == vTime[k]);
			}
			CPPUNIT_ASSERT(lyrLines.size() == CountOf(vLyrLines));

			//
			// Cases on CLrcParser::ToString
			//
			tstring		strNewLyr;
			CLrcParser		parser2(&data);
			parser2.ToString(strNewLyr, true, false);

			CMLData			data3;
			CLrcParser		parser3(&data3);
			data3.Properties().SetEncoding(vEncoding[i]);
			parser3.ParseString(strNewLyr.c_str(), false);

			{
				// Compare ToString data.
				CLrcTag		tag(data.Properties(), data3.Properties());
				CPPUNIT_ASSERT(!tag.IsTagChanged()); // data and data3 should be same.

				CPPUNIT_ASSERT(_tcscmp(data3.Properties().m_strArtist.c_str(), valArtist) == 0);
				CPPUNIT_ASSERT(_tcscmp(data3.Properties().m_strTitle.c_str(), valTitle) == 0);
				CPPUNIT_ASSERT(data3.Properties().GetEncodingIndex() == vEncoding[i]);

				VLyrLines	&lyrLines = data3.GetRawLyrics();

				for (int k = 0; k < lyrLines.size(); k++)
				{
					PML_ROW	pRow = lyrLines[k];

					CPPUNIT_ASSERT(pRow->vFrags.size() == 1);
					CPPUNIT_ASSERT(k < CountOf(vLyrLines));
					CPPUNIT_ASSERT(_tcscmp(pRow->vFrags[0]->szLyric, vLyrLines[k]) == 0);
					CPPUNIT_ASSERT(pRow->vFrags[0]->nBegTime == vTime[k]);
				}
				CPPUNIT_ASSERT(lyrLines.size() == CountOf(vLyrLines));
			}

			//
			// Case: Convert to text including timestamps
			//
			tstring			strTxtLyr;
			CLrcParser		parserTxt(&data);
			parserTxt.ToString(strTxtLyr, true, true);

			CMLData			data5;
			CLrcParser		parser5(&data5);
			data5.Properties().SetEncoding(vEncoding[i]);
			parser5.ParseString(strTxtLyr.c_str(), false);

			// The new parsed data5 shall be same with previous data.
			CPPUNIT_ASSERT(data5.Properties().GetOffsetTime() == data.GetOffsetTime());

			// Compare line by line
			VLyrLines	&lyrLines2 = data5.GetRawLyrics();
			for (int k = 0; k < lyrLines2.size(); k++)
			{
				PML_ROW	pRow = lyrLines2[k];

				CPPUNIT_ASSERT(pRow->vFrags.size() == 1);
				CPPUNIT_ASSERT(_tcscmp(pRow->vFrags[0]->szLyric, vLyrLines[k]) == 0);
				// 100 = TIME UNIT (TIME_STAMP_UNIT) in timestamps.
				CPPUNIT_ASSERT(pRow->vFrags[0]->nBegTime == (vTime[k] + 50) / 100 * 100);
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
			CLrcParser		parser(&data);

			int nRet = parser.ParseString(vLyrics[i], vLen[i], false, false);
			CPPUNIT_ASSERT(nRet == ERR_OK);

			CPPUNIT_ASSERT(_tcscmp(data.Properties().m_strArtist.c_str(), valArtist) == 0);
			CPPUNIT_ASSERT(_tcscmp(data.Properties().m_strTitle.c_str(), valTitle) == 0);
			CPPUNIT_ASSERT(data.Properties().GetEncodingIndex() == vEncoding[i]);

			VLyrLines	&lyrLines = data.GetRawLyrics();

			for (int k = 0; k < lyrLines.size(); k++)
			{
				PML_ROW	pRow = lyrLines[k];

				CPPUNIT_ASSERT(pRow->vFrags.size() == 1);
				CPPUNIT_ASSERT(k < CountOf(vLyrLines));
				CPPUNIT_ASSERT(_tcscmp(pRow->vFrags[0]->szLyric, vLyrLines[k]) == 0);
			}
			CPPUNIT_ASSERT(lyrLines.size() == CountOf(vLyrLines));

			//
			// Cases on CLrcParser::ToString
			//
			tstring		strNewLyr;
			CLrcParser		parser2(&data);
			parser2.ToString(strNewLyr, true, true);

			CMLData			data3;
			CLrcParser		parser3(&data3);
			parser3.ParseString(strNewLyr.c_str(), false);

			{
				// Compare ToString data.
				CLrcTag		tag(data.Properties(), data3.Properties());
				CPPUNIT_ASSERT(!tag.IsTagChanged()); // data and data3 should be same.

				CPPUNIT_ASSERT(_tcscmp(data3.Properties().m_strArtist.c_str(), valArtist) == 0);
				CPPUNIT_ASSERT(_tcscmp(data3.Properties().m_strTitle.c_str(), valTitle) == 0);
				CPPUNIT_ASSERT(data3.Properties().GetEncodingIndex() == vEncoding[i]);

				VLyrLines	&lyrLines = data3.GetRawLyrics();

				for (int k = 0; k < CountOf(vLyrLines); k++)
				{
					PML_ROW	pRow = lyrLines[k];

					CPPUNIT_ASSERT(pRow->vFrags.size() == 1);
					CPPUNIT_ASSERT(k < lyrLines.size());
					CPPUNIT_ASSERT(_tcscmp(pRow->vFrags[0]->szLyric, vLyrLines[k]) == 0);
				}
				// New generated lyrics may increase 1 line by adding encoding.
				// CPPUNIT_ASSERT(lyrLines.size() == CountOf(vLyrLines));
			}
		}
	}

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseLrcParser);

#endif // _CPPUNIT_TEST
