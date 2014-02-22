/********************************************************************
	Created  :	2002年1月3日 17:54:54
	FileName :	MLData.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "StdAfx.h"
#include "MLLib.h"
#include "MLData.h"
#include "LyricsSearch.h"
#include "HelperFun.h"
#include "LrcParser.h"
#ifndef _IPHONE
#include "SncParser.h"
#include "SrtParser.h"
#endif // #ifndef _IPHONE
#include <algorithm>
#include "LrcTag.h"
#include "../TagLib/MidiTag.h"

IMPLEMENT_CPPUNIT_TEST_REG(CMLData)

#define ROW_TIME_MIN_SPAN			300	// 300 MS

// 歌曲开始和结束的那一段空闲时间（没有歌词显示）
#define	LYRICS_BEGIN_TIME			15000	// 15 s
#define	LYRICS_END_TIME				8000	// 10 s


MLFileType LyricsConentTypeToFileType(LYRICS_CONTENT_TYPE lyrContentType)
{
	if (lyrContentType == LCT_LRC)
		return FT_LYRICS_LRC;
	else if (lyrContentType == LCT_TXT)
		return FT_LYRICS_TXT;
	else
		return FT_UNKNOWN;
}

void WrapText(LPCTSTR szText, vector<tstring> &vOutput, int nLineSize)
{
#ifdef UNICODE
	int		nCount;
	LPCWSTR		szLineBeg, szPtr, szLastSpace;
	szLineBeg = szText;
	tstringex	str;
	while (*szLineBeg)
	{
		szPtr = szLineBeg;
		nCount = 0;
		szLastSpace = NULL;
		while (*szPtr && *szPtr != '\n' && nCount <= nLineSize)
		{
			if ((*szPtr) > 255 || !(IsDigit(*szPtr) || IsDigit(*szPtr) || *szPtr == '_' || *szPtr == '-'))
				szLastSpace = szPtr;

			if (unsigned(*szPtr) >= 0xFF)
				nCount += 2;
			else
				nCount += 1;
			szPtr++;
		}
		str.clear();
		if (szLastSpace)
			szPtr = szLastSpace;
		str.append(szLineBeg, int(szPtr - szLineBeg));
		if (str.size())
		{
			if (str.data()[str.size() - 1] == '\r')
				str.resize(str.size() - 1);
		}
		vOutput.push_back(str.c_str());

		if (*szPtr == '\n')
			szPtr++;
		szLineBeg = szPtr;
	}
#else
	vOutput.push_back(szText);
#endif
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef _ID3V2_SUPPORT
CMLData::ID3v2LyricsSaveCacheInfo::ID3v2LyricsSaveCacheInfo()
{
	clear();
}

void CMLData::ID3v2LyricsSaveCacheInfo::clear()
{
	m_strLanguage = "eng";
	m_strLrcDescription = _T("Lyrics were Saved by MiniLyrics.");
	m_bAllSyllableIsNewLine = true;
}
#endif // #ifdef _ID3V2_SUPPORT

CMLData::CMLData() : m_arrLyricsLines(false)
{
	EmptyStr(m_szLyrContentMd5Org);
	MD5ToString((const char *)_T(""), 0,
		m_szLyrContentMd5Org);

	m_bKaraokeStyle = false;

	m_lrcSourceType = LST_FILE;

	m_nMediaLength = 0;
	m_nTimeElapsed = 0;
	m_nGlobalOffsetTime = 0;
}

CMLData::~CMLData()
{
	Close();
}

int CMLData::OpenLyrics(LPCTSTR szSongFile, int nMediaLength, LPCTSTR szLrcSource, bool bUseSpecifiedEncoding, CHAR_ENCODING encodingSpecified)
{
	int				nRet = ERR_OK;
	LRC_SOURCE_TYPE lrcSourceType;

	Close();

	lrcSourceType = LyrSrcTypeFromName(szLrcSource);

#ifdef _ID3V2_SUPPORT
	if (lrcSourceType == LST_LYRICS3V2)
	{
		stringex	strLyrics;
		CLyrics3v2		lyrics3v2;

		nRet = lyrics3v2.ReadLyrcsInfo(szSongFile, strLyrics);
		if (nRet == ERR_OK)
			nRet = OpenLyricsInSongOfLyrics3v2(strLyrics.c_str(), strLyrics.size(), bUseSpecifiedEncoding, encodingSpecified);
	}
	else if (lrcSourceType == LST_ID3V2_SYLT
		|| lrcSourceType == LST_ID3V2_USLT
		|| lrcSourceType == LST_ID3V2_LYRICS)
	{
		// Open ID3v2 lyrics
		CID3v2IF		id3v2(GetEncodingIndex());

		nRet = id3v2.Open(szSongFile, false, false);
		if (nRet != ERR_OK)
			return nRet;

		if (lrcSourceType == LST_ID3V2_USLT)
		{
			ID3v2UnsynchLyrics lyrics;
			nRet = id3v2.GetUnsyncLyrics(lyrics, szLrcSource);
			if (nRet != ERR_OK)
				return nRet;
			nRet = OpenLyricsInSongOfID3v2USLT(lyrics, bUseSpecifiedEncoding, encodingSpecified);
		}
		else if (lrcSourceType == LST_ID3V2_SYLT)
		{
			ID3v2SynchLyrics lyrics;
			nRet = id3v2.GetSyncLyrics(lyrics, szLrcSource);
			if (nRet != ERR_OK)
				return nRet;
			nRet = OpenLyricsInSongOfID3v2SYLT(lyrics, bUseSpecifiedEncoding, encodingSpecified);
		}
		else if (lrcSourceType == LST_ID3V2_LYRICS)
		{
			ID3v2TextUserDefined lyrics;
			nRet = id3v2.GetUserDefLyrics(lyrics);
			if (nRet != ERR_OK)
				return nRet;

			if (lyrics.m_strValue.empty())
			{
				SetCustomErrorDesc(_TLM("The embedded lyrics is empty, please click the \"search lyrics\" button to get lyrics."));
				nRet = ERR_CUSTOM_ERROR;
			}
			else
				nRet = FromString(lyrics.m_strValue.c_str());
		}
		else
			ASSERT(0);
	}
	else if (lrcSourceType == LST_M4A_LYRICS)
	{
		CM4aTag tag;
		nRet = tag.Open(szSongFile, false);
		if (nRet != ERR_OK)
			return nRet;

		tstring strLyrics;
		nRet = tag.GetLyrics(strLyrics);
		if (nRet != ERR_OK)
			return nRet;
		if (strLyrics.empty())
		{
			SetCustomErrorDesc(_TLM("The embedded lyrics is empty, please click the \"search lyrics\" button to get lyrics."));
			nRet = ERR_CUSTOM_ERROR;
		}
		else
			nRet = FromString(strLyrics.c_str());
	}
	else if (lrcSourceType == LST_KAR_LYRICS)
	{
		CMidiTag tag;
		nRet = tag.Open(szSongFile);
		if (nRet != ERR_OK)
			return nRet;

		nRet = tag.GetLyrics(m_arrFileLines);
		if (nRet != ERR_OK)
			return nRet;
		m_arrLyricsLines.insert(m_arrLyricsLines.begin(), m_arrFileLines.begin(), m_arrFileLines.end());
		SetKaraokeStyle(true);
		SetLyrContentType(LCT_KARAOKE);

		CheckLyricsLineTime();
		FixExtendedLrcTimeStamps();
	}
	else
#endif // #ifdef _ID3V2_SUPPORT
	{
		nRet = OpenLyricsFile(szLrcSource, bUseSpecifiedEncoding, encodingSpecified);
	}

	m_nMediaLength = nMediaLength;

	m_strSongFile = szSongFile;
	m_strLrcSource = szLrcSource;
	m_lrcSourceType = lrcSourceType;

	OnLyricsOpened();

	return nRet;
}

int CMLData::NewLyrics(LPCTSTR szSongFile, int nMediaLength)
{
	Close();

	m_strSongFile = szSongFile;
	m_nMediaLength = nMediaLength;

	return ERR_OK;
}

int CMLData::OpenLyrics(LPCTSTR szSongFile, int nMediaLength, LPBYTE szLrc, int nLen)
{
	CLrcParser		parser(this);

	int nRet = parser.ParseString(szLrc, nLen, false);
	if (nRet != ERR_OK)
		return nRet;

	SetLyrContentType(parser.GetLyrContentType());

	m_strSongFile = szSongFile;
	m_nMediaLength = nMediaLength;
	m_lrcSourceType = LST_ID3V2_USLT;

	OnLyricsOpened();

	return ERR_OK;
}

#ifdef _IPHONE
int CMLData::OpenLyricsContent(LPCTSTR szSongFile, int nMediaLength, LPCTSTR szLyrContent)
{
	m_strSongFile = szSongFile;
	m_nMediaLength = nMediaLength;
	m_lrcSourceType = LST_ID3V2_USLT;

	int nRet = FromString(szLyrContent);
	if (nRet != ERR_OK)
		return nRet;

	OnLyricsOpened();
	
	return ERR_OK;
}
#endif // #ifdef _IPHONE

//
// 重新打开歌词
//
int CMLData::ReopenLyrics(bool bUseSpecifiedEncoding, CHAR_ENCODING encodingSpecified)
{
	tstring		strLrcSource, strSongFile;

	strLrcSource = m_strLrcSource;
	strSongFile = m_strSongFile;

	return OpenLyrics(strSongFile.c_str(), m_nMediaLength, strLrcSource.c_str(), bUseSpecifiedEncoding, encodingSpecified);
}

int CMLData::OpenLyricsFile(LPCTSTR szFile, bool bUseSpecifiedEncoding, CHAR_ENCODING encodingSpecified)
{
	Close();

	CLyricsParser	*pParser = NULL;
	if (FileIsExtSame(szFile, _T(".txt"))
		|| FileIsExtSame(szFile, _T(".lrc")))
	{
		pParser = new CLrcParser(this);
	}
#ifndef _IPHONE
	else if (FileIsExtSame(szFile, _T(".srt")))
	{
		pParser = new CSrtParser(this);
	}
#endif // #ifndef _IPHONE
	else
		return ERR_NOT_SUPPORT_FILE_FORMAT;

	m_strLrcSource = szFile;

	if (bUseSpecifiedEncoding)
		m_lyrProperties.SetEncoding(encodingSpecified);

	int nRet = pParser->ParseFile(bUseSpecifiedEncoding, encodingSpecified);
	if (nRet == ERR_OK)
	{
		SetLyrContentType(pParser->GetLyrContentType());
	}

	delete pParser;

	FilterContents();

	return nRet;
}

#ifdef _ID3V2_SUPPORT
int CMLData::OpenLyricsInSongOfID3v2USLT(ID3v2UnsynchLyrics &lyrics, bool bUseSpecifiedEncoding, CHAR_ENCODING encodingSpecified)
{
	int		nRet = ERR_OK;

	Close();

	m_id3v2SaveCacheInfo.m_strLanguage = lyrics.m_szLanguage;
	m_lyrProperties.SetEncoding(ID3v2EncTypeToCharEncoding(lyrics.m_encodingType));

	m_id3v2SaveCacheInfo.m_strLrcDescription = lyrics.m_strContentDesc.c_str();

	nRet = FromString(lyrics.m_strLyrics.c_str());
	if (nRet != ERR_OK)
		return nRet;

	return ERR_OK;
}

int CMLData::OpenLyricsInSongOfID3v2SYLT(ID3v2SynchLyrics &lyrics, bool bUseSpecifiedEncoding, CHAR_ENCODING encodingSpecified)
{
	Close();

	m_id3v2SaveCacheInfo.m_strLanguage = lyrics.m_szLanguage;
	m_id3v2SaveCacheInfo.m_bAllSyllableIsNewLine = lyrics.m_bAllSyllableIsNewLine;
	m_lyrProperties.SetEncoding(ID3v2EncTypeToCharEncoding(lyrics.m_encodingType));

	// Read id3v2 SYLT lyrics information
	CLrcTag			lrcTag;
	lrcTag.ReadFromText(lyrics.m_strContentDesc.c_str(), lyrics.m_strContentDesc.size());

	m_id3v2SaveCacheInfo.m_strLrcDescription = lyrics.m_strContentDesc.c_str();

	LyricsLine *pLine = NULL;
	for (ID3v2SynchLyrics::LIST_SYLRC::iterator it = lyrics.m_vSynLyrics.begin();
		it != lyrics.m_vSynLyrics.end(); it++)
	{
		LrcSyllable		&syllable = *it;

		if (syllable.bNewLine)
		{
			if (pLine && pLine->vFrags.size() > 1)
				m_bKaraokeStyle = true;
			pLine = NULL;
			continue;
		}

		if (!pLine || lyrics.m_bAllSyllableIsNewLine)
		{
			pLine = NewLyricsLine(syllable.nTime, -1);
			AddLine(pLine);
		}

		pLine->AppendPiece(syllable.nTime, -1, syllable.strText.c_str(), 
			syllable.strText.size(), false, true);
	}

	m_lyrProperties = lrcTag;
	m_lyrProperties.SetOffsetTime(0);
	SetLyrContentType(LCT_LRC);

	CheckLyricsLineTime();

	FixExtendedLrcTimeStamps();

	return ERR_OK;
}

int CMLData::OpenLyricsInSongOfLyrics3v2(const char *buffer, int len, bool bUseSpecifiedEncoding, CHAR_ENCODING encodingSpecified)
{
	int		nRet;

	Close();

	if (len == 0)
	{
		SetCustomErrorDesc(_TLM("The embedded lyrics is empty, please click the \"search lyrics\" button to get lyrics."));
		return ERR_CUSTOM_ERROR;
	}

	//
	// Use CLrcParser to parse lyrics
	//
	CLrcParser		parser(this);

	if (bUseSpecifiedEncoding)
		m_lyrProperties.SetEncoding(encodingSpecified);

	nRet = parser.ParseString((LPBYTE)buffer, len, false, bUseSpecifiedEncoding, encodingSpecified);
	if (nRet == ERR_OK)
	{
		SetLyrContentType(parser.GetLyrContentType());
		m_lrcSourceType = LST_LYRICS3V2;

		FilterContents();
	}

	return nRet;
}
#endif // #ifdef _ID3V2_SUPPORT

void CMLData::OnLyricsOpened()
{
	ResetModificationFlag();

	if (m_arrLyricsLines.empty())
		return;

	ASSERT(GetLyrContentType() != LCT_UNKNOWN);

	FilterContents();

	if (GetLyrContentType() == LCT_TXT)
	{
		m_lyrScrollActionRecorder.EventsDataFromString(
			GetTxtLyrScrollEvents(), 
			GetRawLyrics());

		if (!m_lyrScrollActionRecorder.HasActions() && m_nMediaLength > 0)
		{
			// We need to the simple actions to make it scroll.
			m_lyrScrollActionRecorder.InitCreateAutoEvents(m_arrLyricsLines);
			m_lyrScrollActionRecorder.ScrollToLine(m_arrLyricsLines, 0, 10 * 1000, false);
			m_lyrScrollActionRecorder.ScrollToLine(m_arrLyricsLines, m_arrLyricsLines.size(), m_nMediaLength, true);
		}
	}
}

int CMLData::Save()
{
	if (!HasLyricsOpened())
		return ERR_NOLYRICS_OPENED;

	if (!IsModified())
		return ERR_OK;

#ifdef _ID3V2_SUPPORT
	if (m_lrcSourceType == LST_LYRICS3V2)
		return SaveLyricsInSongOfLyrics3v2();
	else if ((m_lrcSourceType & LST_ID3V2) != 0)
	{
		VecString vLyrNames;
		vLyrNames.push_back(m_strLrcSource);
		return SaveLyricsInSongOfID3v2(vLyrNames);
	}
	else if (m_lrcSourceType == LST_M4A_LYRICS)
		return SaveLyricsInSongOfM4a();
	else
#endif // #ifdef _ID3V2_SUPPORT
	{
		if (IsContentModified()
			|| FileIsExtSame(m_strLrcSource.c_str(), _T(".srt")))
		{
			tstring		strFile = m_strLrcSource;
			bool		bUseNewFileName;

			return SaveAsFile(strFile.c_str(), m_lyrProperties.GetEncodingIndex(), bUseNewFileName);
		}
		else
		{
			// Only .lrc or .txt can call SaveTag()
			if (SaveTag())
				return ERR_OK;
			else
				return ERR_FALSE;
		}
	}
}

#ifdef _ID3V2_SUPPORT
int CMLData::SaveLyricsInSongOfID3v2(VecString &vLyrNames)
{
	if (vLyrNames.empty())
		return ERR_OK;

	CID3v2IF	id3v2(m_lyrProperties.GetEncodingIndex());

	int nRet = id3v2.Open(m_strSongFile.c_str(), true, true);
	if (nRet != ERR_OK)
		return nRet;

	tstring			strLyrics;
	CHAR_ENCODING	encoding;
	ToString(strLyrics, FT_LYRICS_TXT, TRUE);
	if (IsAnsiStr(strLyrics.c_str()))
		encoding = ED_SYSDEF;
	else
		encoding = ED_UNICODE;

	for (size_t i = 0; i < vLyrNames.size(); ++i)
	{
		LPCTSTR szLyrName = vLyrNames[i].c_str();
		LRC_SOURCE_TYPE lst = LyrSrcTypeFromName(szLyrName);

		if (lst == LST_ID3V2_USLT)
		{
			nRet = id3v2.SetUnsynchLyrics(szLyrName, _T(""), strLyrics.c_str());
			if (nRet != ERR_OK)
				return nRet;
		}
		else if (lst == LST_ID3V2_SYLT && GetLyrContentType() != LCT_TXT)
		{
			ID3v2SynchLyrics	lyrics;

			ToID3v2SynchLyrics(lyrics, encoding);

			nRet = id3v2.SetSynchLyrics(szLyrName, lyrics);
			if (nRet != ERR_OK)
				return nRet;
		}
		else if (lst == LST_ID3V2_LYRICS)
		{
			nRet = id3v2.SetUserDefLyrics(strLyrics.c_str());
			if (nRet != ERR_OK)
				return nRet;
		}
	}

	nRet = id3v2.Save();
	if (nRet == ERR_OK)
		ResetModificationFlag();

	return nRet;
}

int CMLData::SaveLyricsInSongOfLyrics3v2()
{
	// Save lyrics3v2 lyrics in mp3 file
	tstring		str;
	ToString(str, FT_LYRICS_LRC, TRUE);

	CCharBuffer	strData;
	ConvertStrByFileEncoding(strData, str.c_str(), str.size(), 
		CharEncodingToFileEncoding(GetEncodingIndex()), GetEncodingIndex());

	CLyrics3v2	lyrics3v2;
	int nRet = lyrics3v2.WriteLyrcsInfo(m_strSongFile.c_str(), (const char *)strData.c_str(), strData.size() * sizeof(char), TRUE);
	if (nRet == ERR_OK && m_lrcSourceType == LST_LYRICS3V2)
		ResetModificationFlag();

	return nRet;
}

int CMLData::SaveLyricsInSongOfM4a()
{
	// Save lyrics in m4a files
	tstring		str;
	ToString(str, FT_LYRICS_TXT, TRUE);

	CM4aTag tag;
	int nRet = tag.Open(m_strSongFile.c_str(), true, false);
	if (nRet == ERR_OK)
	{
		int nRet = tag.SetLyrics(str.c_str());
		if (nRet == ERR_OK)
		{
			nRet = tag.SaveClose();
			if (nRet == ERR_OK && m_lrcSourceType == LST_M4A_LYRICS)
				ResetModificationFlag();
		}
	}

	return nRet;
}
#endif // #ifdef _ID3V2_SUPPORT

// COMMENT:
//		将CMLData中的数据保存为文件szFile，文件的类型，由szFile的扩展名决定。
int CMLData::SaveAsFile(LPCTSTR szFile, CHAR_ENCODING newEncoding, bool &bUseNewFileName)
{
	if (!HasLyricsOpened())
		return ERR_NOLYRICS_OPENED;

	// 基类接口
	CLyricsParser	*pParser;

	bUseNewFileName = false;

	// 根据文件的扩展名来判断文件类型，然后调用不同的歌词分析类来分析歌词。
	if (FileIsExtSame(szFile, _T(".lrc")) )
	{
		pParser = new CLrcParser(this);
		if (GetLyrContentType() <= LCT_LRC)
			bUseNewFileName = true;
	}
	else if (FileIsExtSame(szFile, _T(".txt")) )
	{
		pParser = new CLrcParser(this);
		if (GetLyrContentType() <= LCT_TXT)
			bUseNewFileName = true;
	}
#ifndef _IPHONE
	else if (FileIsExtSame(szFile, _T(".srt")) )
	{
		pParser = new CSrtParser(this);
		if (GetLyrContentType() <= LCT_TXT)
			bUseNewFileName = true;
	}
	else if (FileIsExtSame(szFile, _T(".snc")) )
	{
		pParser = new CSncParser(this);
	}
#endif // #ifndef _IPHONE
	else
	{
		return ERR_NOT_SUPPORT_FILE_FORMAT;
	}

	int		nRet;

	// 分析歌词
	nRet = pParser->SaveAsFile(szFile, newEncoding);
	if (nRet == ERR_OK && bUseNewFileName)
	{
		m_lyrProperties.SetEncoding(newEncoding);
		ResetModificationFlag();

		// 保存歌词文件名
		m_strLrcSource = szFile;
	}

	delete pParser;

	return nRet;
}

#ifdef _ID3V2_SUPPORT
int CMLData::UpdateTagToID3v2DescTxt()
{
	CLrcTag			lrcTag(m_lyrPropertiesOrg, m_lyrProperties);

#ifdef UNICODE
	lrcTag.WriteToText(m_id3v2SaveCacheInfo.m_strLrcDescription, false);
#else
	// UTF8
	lrcTag.WriteToText(m_id3v2SaveCacheInfo.m_strLrcDescription, false);
#endif

	return ERR_OK;
}
#endif // #ifdef _ID3V2_SUPPORT

BOOL CMLData::SaveTag()
{
	ASSERT(!IsContentModified());
	ASSERT(FileIsExtSame(m_strLrcSource.c_str(), _T(".txt"))
		|| FileIsExtSame(m_strLrcSource.c_str(), _T(".lrc")));

	CLrcTag		lrcTag(m_lyrPropertiesOrg, m_lyrProperties);

	if (lrcTag.WriteToFile(m_strLrcSource.c_str(), GetLyrContentType() != LCT_TXT) != ERR_OK)
		return false;

	m_lyrPropertiesOrg = m_lyrProperties;

	return true;
}

void CMLData::ResetModificationFlag()
{
	tstring		strContent;

	m_lyrPropertiesOrg = m_lyrProperties;
	ToString(strContent, FT_LYRICS_LRC, false);
	MD5ToString((const char *)strContent.c_str(), strContent.size() * sizeof(TCHAR),
		m_szLyrContentMd5Org);
}

LyricsLine *CMLData::GetLyricsLine(int nLine)
{
	int nSize = m_arrLyricsLines.size();
	if (nSize <= nLine || nLine < 0)
		return NULL;
	
	return m_arrLyricsLines[nLine];
}

// COMMENT:
//		检查歌词数据的时间顺序的正确性
//		查找所有的歌词显示时间为-1的“歌词段”，进行计算。
void CMLData::CheckLyricsLineTime()
{
	int		nLineCount;
	int		nBegLine = 0;
	int		nBegFrag = 0;
	int		nBegTime = 0;
	int		nEndLine = -1;
	int		nEndFrag = 0;
	int		nEndTime = 0;
	int		nLine = 0;
	int		nFrag = 0;
	BOOL	bFind = FALSE;

	if (GetLyricsLinesCount() == 0)
		return;

	if (GetLyricsLine(0)->vFrags.size() == 0)
		return;

	while (1)
	{
		// 查找开始歌词段
		bFind = FALSE;
		do
		{
			LyricsPiece *pPiece;
			LyricsLine *pLine;

			pLine = GetLyricsLine(nLine);
			assert(pLine);

			pPiece = pLine->vFrags[nFrag];
			if (pPiece->nBegTime == -1)
			{
				pPiece = PrevFrag(nLine, nFrag);
				if (pPiece)
				{
					assert(pPiece->nEndTime != -1);
					nBegTime = pPiece->nEndTime;
				}
				else
				{
					// 一定是第一行，开始时间设置为 15 s
					nBegTime = LYRICS_BEGIN_TIME;
				}
				nBegLine = nLine;
				nBegFrag = nFrag;
				bFind = TRUE; 
				break;
			}

			if (pLine->vFrags[nFrag]->nEndTime == -1)
			{
				nBegTime = pLine->vFrags[nFrag]->nBegTime;
				nBegLine = nLine;
				nBegFrag = nFrag;
				bFind = TRUE;
				break;
			}

			nLine = nBegLine;
			nFrag = nBegFrag;
		}
		while (NextFrag(nLine, nFrag, nBegLine, nBegFrag));

		if (!bFind)	// 所有的歌词段的时间都是正确的。
			break;

		// 查找结束歌词段
		while (1)
		{
			LyricsLine *pLine;

			if (!NextFrag(nLine, nFrag, nEndLine, nEndFrag))
			{
				// 所有的歌词都没有时间标记，则最后的时间定为歌曲时间 － LYRICS_END_TIME
				nEndTime = GetMediaLength() - LYRICS_END_TIME - GetOffsetTime();
				if (nBegTime >= nEndTime)
					nEndTime = GetMediaLength() - GetOffsetTime();
				if (nBegTime >= nEndTime)
					nEndTime = nBegTime + 10 * 1000;
				nEndLine = nLine;
				nEndFrag = nFrag;
				break;
			}

			pLine = GetLyricsLine(nEndLine);
			assert(pLine);

			if (pLine->vFrags[nEndFrag]->nBegTime != -1)
			{
				nEndTime = pLine->vFrags[nEndFrag]->nBegTime;
				nEndLine = nLine;
				nEndFrag = nFrag;
				break;
			}

			if (pLine->vFrags[nEndFrag]->nEndTime != -1)
			{
				nEndTime = pLine->vFrags[nEndFrag]->nEndTime;
				break;
			}

			nLine = nEndLine;
			nFrag = nEndFrag;
		}
		if (nBegTime > nEndTime)
			nBegTime = nEndTime - 2 * 1000;


		// 计算在开始段到最后段歌词之间的时间。
		CalFragTimeBetweenLines(nBegLine, nBegFrag, nEndLine, nEndFrag, nBegTime, nEndTime);
	}

	// Set time stamp of lyrics line.
	nLineCount = m_arrLyricsLines.size();
	for (int i = 0; i < nLineCount; i++)
	{
		LyricsLine *pLine = m_arrLyricsLines[i];
		assert(pLine->vFrags.size() != 0);
		if (pLine->nBegTime == -1)
			pLine->nBegTime = pLine->vFrags[0]->nBegTime;
		if (pLine->nEndTime == -1)
			pLine->nEndTime = pLine->vFrags[pLine->vFrags.size() - 1]->nEndTime;
	}

	// If a lyrics line is 0 length, recover to display multiple line at same time.
	for (int i = 0; i < m_arrLyricsLines.size(); i++)
	{
		LyricsLine *pLine = m_arrLyricsLines[i];
		if (pLine->nBegTime != pLine->nEndTime || pLine->vFrags.size() != 1)
			continue;

		int nextLine = i;
		for (; nextLine < m_arrLyricsLines.size(); nextLine++)
		{
			LyricsLine *pNext = m_arrLyricsLines[nextLine];
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

void CMLData::FixExtendedLrcTimeStamps()
{
	//
	// For Extended lrc lyrics, the end time of ending lyrics frag is incorrect.
	//
	if (!m_bKaraokeStyle)
		return;

	for (int i = 0; i < (int)GetLyricsLinesCount(); i++)
	{
		LyricsLine *pLine = GetLyricsLine(i);

		if (pLine->vFrags.size() < 1)
			continue;

		int			n = pLine->vFrags.size() - 1;
		LyricsPiece *pPiece;
		pPiece = pLine->vFrags[n];
		if (pPiece->bTempEndTime)
		{
			LyricsLine *pNextLine;
			if (i + 1 < (int)GetLyricsLinesCount())
				pNextLine = GetLyricsLine(i + 1);
			else
				pNextLine = NULL;
			if (pNextLine)
			{
				if (pPiece->nBegTime + 500 < pNextLine->nBegTime)
					pPiece->nEndTime = pPiece->nBegTime + 500;
			}
			else
				pPiece->nEndTime = pPiece->nBegTime + 500;
		}
	}
}

void CMLData::ClearLyrics()
{
	m_arrLyricsLines.erase(m_arrLyricsLines.begin(), m_arrLyricsLines.end());
	m_arrFileLines.clear();
}

void CMLData::Close()
{
	m_lyrPropertiesOrg.clear();
	m_lyrProperties.clear();
	ASSERT(GetLyrContentType() == LCT_UNKNOWN);
	EmptyStr(m_szLyrContentMd5Org);
	MD5ToString((const char *)_T(""), 0,
		m_szLyrContentMd5Org);
	m_nTimeElapsed = 0;

	m_bKaraokeStyle = false;

	m_lrcSourceType = LST_FILE;
	m_strSongFile.resize(0);;
	m_strLrcSource.resize(0);;

#ifdef _ID3V2_SUPPORT
	m_id3v2SaveCacheInfo.clear();
#endif // #ifdef _ID3V2_SUPPORT
	ClearLyrics();
}

BOOL CMLData::HasLyricsOpened()
{
	return GetLyrContentType() != LCT_UNKNOWN;
}

void CMLData::AddLine(LyricsLine *pFileLine)
{
	if (pFileLine->bLyricsLine)
	{
		m_arrLyricsLines.push_back(pFileLine);
	}

	m_arrFileLines.push_back(pFileLine);
}

// COMMENT:
//		将歌词行按照时间排序插入
void CMLData::AddLineByTime(LyricsLine *pLine)
{
	if (pLine->bLyricsLine)
	{
		// 将歌词行插入 m_arrLyricsLines
		CLyricsLines::iterator	itPos;

		int			i, nLineCount;

		for (i = 0; i < (int)pLine->vFrags.size(); i++)
			pLine->vFrags[i]->nDrawWidth = 0;

		nLineCount = m_arrLyricsLines.size();
		for (i = 0; i < nLineCount; i++)
		{
			if (m_arrLyricsLines[i]->nBegTime > pLine->nBegTime)
				break;
		}

		if (i != nLineCount)
		{
			// 找到了位置
			itPos = find(m_arrFileLines.begin(), m_arrFileLines.end(), m_arrLyricsLines[i]);
			
			if (itPos == m_arrFileLines.end())
			{
				// error occurs!
				assert(0);
				m_arrFileLines.push_back(pLine);
			}
			else
			{
				m_arrFileLines.insert(itPos, pLine);
			}

			// 插入歌词部分
			LyricsLine *pLinePrev;
			if (i > 0)
			{
				// reset the time of prev line.
				pLinePrev = m_arrLyricsLines[i - 1];
				if (pLinePrev->nEndTime != -1)
				{
					if (pLinePrev->vFrags.size() == 1 && pLinePrev->vFrags[0]->bTempEndTime)
					{
						pLinePrev->vFrags[0]->nEndTime = pLine->nBegTime;
						pLinePrev->nEndTime = pLine->nBegTime;
					}
				}
			}
			m_arrLyricsLines.insert(m_arrLyricsLines.begin() + i, pLine);
		}
		else
		{
			// 未找到位置，则插入末尾
			m_arrLyricsLines.push_back(pLine);
			m_arrFileLines.push_back(pLine);
		}
	}
	else
		m_arrFileLines.push_back(pLine);
}

tstring CMLData::GetSuggestedLyricsFileName()
{
	// Combine file name
	tstring	strFileName;

	strFileName = FormatMediaTitle(m_lyrProperties.m_strArtist.c_str(), m_lyrProperties.m_strTitle.c_str());
	strFileName = FileNameFilterInvalidChars(strFileName.c_str());

	if (strFileName.empty())
		strFileName = FileGetTitle(m_strSongFile.c_str());

	if (m_lyrProperties.m_lyrContentType == LCT_LRC)
		strFileName += _T(".lrc");
	else
		strFileName += _T(".txt");

	return strFileName;
}

LPCTSTR CMLData::GetLyricsFileName()
{
	return m_strLrcSource.c_str();
}

// COMMENT:
//		设置时间偏移
void CMLData::SetPlayElapsedTime(int nTimeElapsed)
{
	m_nTimeElapsed = nTimeElapsed + m_lyrProperties.GetOffsetTime() - m_nGlobalOffsetTime;
}

// COMMENT:
//		取得
int CMLData::GetPlayElapsedTime()
{
	return m_nTimeElapsed;
}

// COMMENT:
//		取得当前正在演唱的歌词行
// RETURN:
//		歌词行的行号
int CMLData::GetCurPlayLine(const CLyricsLines &lyrLines)
{
	int				nLineCount;

	//// ERROR:在多线程中使用，可能会存在问题。
	static int		nLine = 0;

	nLineCount = lyrLines.size();

	if (nLineCount == 0)
		return -1;

	if (nLine >= nLineCount)
		nLine = 0;

	// 找到前一行
	for (; nLine >= 0; nLine--)
	{
		LyricsLine *pLyricLine = lyrLines[nLine];
		assert(pLyricLine);

		if (pLyricLine->nEndTime != -1 && pLyricLine->nEndTime < GetPlayElapsedTime())
		{
			nLine++;
			break; // ok.
		}
	}
	if (nLine == -1)
		nLine = 0;
	
	// 找到下一行
	for (; nLine < nLineCount; nLine++)
	{
		LyricsLine *pLyricLine = lyrLines[nLine];
		if (pLyricLine->nEndTime > GetPlayElapsedTime())
			break; // ok.
	}
//	if (nLine > 0)
//		nLine--;
	if (nLine >= nLineCount)
		nLine = nLineCount - 1;

/*	for (int i = nLine - 1; i >= 0; i--)
	{
		if (lyrLines[nLine]->nEndTime != lyrLines[i]->nEndTime)
		{
			return i + 1;
		}
	}*/

	assert(nLine != -1);
	return nLine;
}

bool CMLData::DoesChooseNewFileName()
{
	return (m_strLrcSource.empty() || GetLyrContentType() != m_lyrPropertiesOrg.m_lyrContentType);
}

BOOL CMLData::IsModified()
{
	// No lyrics opened.
	if (!HasLyricsOpened())
		return false;

	return !m_lyrProperties.Equal(m_lyrPropertiesOrg) || IsContentModified();
}

bool CMLData::IsOnlyAddLyrScrollActions()
{
	if (GetLyrContentType() != LCT_TXT)
		return false;
	if (IsContentModified())
		return false;

	return (IsEmptyString(m_lyrPropertiesOrg.GetTxtLyrScrollEvents())
		&& !IsEmptyString(m_lyrProperties.GetTxtLyrScrollEvents()));
}

// 
// bool CMLData::IsEncodingModified()
// {
// 	return m_lyrProperties.GetEncodingIndex() != m_lyrPropertiesOrg.GetEncodingIndex();
// }

BOOL CMLData::IsContentModified()
{
	tstring		strContent;
	TCHAR		szLyrContentMd5[33];

	ToString(strContent, FT_LYRICS_LRC, false);
	MD5ToString((const char *)strContent.c_str(), strContent.size() * sizeof(TCHAR),
		szLyrContentMd5);

	return _tcscmp(szLyrContentMd5, m_szLyrContentMd5Org) != 0;
}

//	COMMENT:
//		计算两个歌词段之间的所有歌词段开始时间和结束时间的值
//	INPUT:
//		nLine, nBegFrag	-	开始的位置，这个位置的时间可能还没有被计算出来。
//		nLine, nEndFrag	-	结束的位置，这个位置的时间可能还没有被计算出来。
BOOL CMLData::CalFragTimeBetweenFrags(int nLine, int nBegFrag, int nEndFrag, int nBegTime, int nEndTime)
{
	int nFragCount = nEndFrag - nBegFrag + 1;
	assert(nFragCount > 0);

	LyricsLine *pLine = GetLyricsLine(nLine);
	assert(pLine);
	assert((unsigned int)nEndFrag < pLine->vFrags.size());

	for (int m = nBegFrag; m <= nEndFrag; m++)
	{
		pLine->vFrags[m]->nBegTime = nBegTime
			+ ((nEndTime - nBegTime) * (m - nBegFrag)) / nFragCount;
		pLine->vFrags[m]->nEndTime = nBegTime + 
			((nEndTime - nBegTime) * (m - nBegFrag + 1)) / nFragCount;
	}

	return TRUE;
}

//	COMMENT:
//		计算两个歌词行之间的所有歌词行的开始时间和结束时间的值
//	INPUT:
//		nBegLine, nBegFrag	-	开始的位置，这个位置的时间可能还没有被计算出来。
//		nEndLine, nEndFrag	-	结束的位置，这个位置的时间可能还没有被计算出来。
BOOL CMLData::CalFragTimeBetweenLines(int nBegLine, int nBegFrag, int nEndLine, int nEndFrag, int nBegTime, int nEndTime)
{
	LyricsLine *pLine = NULL;

	assert(nEndTime >= nBegTime);
	assert(nEndLine >= nBegLine);
	assert(nBegLine < GetLyricsLinesCount());
	assert(nEndLine < GetLyricsLinesCount());

	if (nBegLine >= GetLyricsLinesCount())
		return FALSE;
	if (nEndLine >= GetLyricsLinesCount())
		return FALSE;

	// 如果所有的“歌词段”在一行内，则每一段的时间采用平均值
	if (nBegLine == nEndLine)
	{
		pLine = GetLyricsLine(nBegLine);
		assert(pLine);

		CalFragTimeBetweenFrags(nBegLine, nBegFrag, nEndFrag, nBegTime, nEndTime);

//		if (pLine->nBegTime == -1)
		pLine->nBegTime = pLine->vFrags[0]->nBegTime;
//		if (pLine->nEndTime == -1)
		pLine->nEndTime = pLine->vFrags[pLine->vFrags.size() - 1]->nEndTime;
	}
	else
	{
		// 计算最开始的行
		pLine = GetLyricsLine(nBegLine);
		assert(pLine);
		
//		if (pLine->nEndTime == -1)
		pLine->nEndTime = nBegTime +
			(nEndTime - nBegTime) / (nEndLine - nBegLine + 1);

		CalFragTimeBetweenFrags(nBegLine, nBegFrag, pLine->vFrags.size() - 1, nBegTime, pLine->nEndTime);

//		if (pLine->nBegTime == -1)
		pLine->nBegTime = pLine->vFrags[0]->nBegTime;

		// “歌词段”不在一行内，每一行的时间采用平均值；在一行内再采用平均值
		for (int i = nBegLine + 1; i < nEndLine; i++)
		{
			pLine = GetLyricsLine(i);
			assert(pLine);

			pLine->nBegTime = nBegTime +
				(nEndTime - nBegTime) * (i - nBegLine) / (nEndLine - nBegLine + 1);
			pLine->nEndTime = nBegTime +
				(nEndTime - nBegTime) * (i - nBegLine + 1) / (nEndLine - nBegLine + 1);

			CalFragTimeBetweenFrags(i, 0, pLine->vFrags.size() - 1, pLine->nBegTime, pLine->nEndTime);
		}

		// 计算结束歌词行的时间
		pLine = GetLyricsLine(nEndLine);
		assert(pLine);
		pLine->nBegTime = nBegTime +
			(nEndTime - nBegTime) * (nEndLine - nBegLine) / (nEndLine - nBegLine + 1);

		CalFragTimeBetweenFrags(nEndLine, 0, nEndFrag, pLine->nBegTime, nEndTime);

//		if (pLine->nEndTime == -1)
		pLine->nEndTime = pLine->vFrags[pLine->vFrags.size() - 1]->nEndTime;
	}
	
	return TRUE;
}

//	COMMENT:
//		取得上一个“歌词段”
BOOL CMLData::PrevFrag(int nLine, int nFrag, int &nPrevLine, int &nPrevFrag, LyricsPiece **ppPiece)
{
	assert(nLine < GetLyricsLinesCount());
	assert(nFrag >= 0);
	assert((unsigned int)nFrag < GetLyricsLine(nLine)->vFrags.size());

	if (nFrag <= 0)
	{
		if (nLine <= 0)
		{
			nPrevLine = 0;
			nPrevFrag = 0;
			if (ppPiece)
				*ppPiece = NULL;
			return FALSE;
		}

		nPrevLine = nLine - 1;
		nPrevFrag = GetLyricsLine(nPrevLine)->vFrags.size() - 1;
	}
	else
	{
		nPrevFrag--;
		nPrevLine = nLine;
	}

	if (ppPiece)
		*ppPiece = GetLyricsLine(nPrevLine)->vFrags[nPrevFrag];

	return TRUE;
}

//	COMMENT:
//		取得上一个“歌词段”
LyricsPiece *CMLData::PrevFrag(int nLine, int nFrag)
{
	assert(nLine < GetLyricsLinesCount());
	assert(nFrag >= 0);
	assert((unsigned int)nFrag < GetLyricsLine(nLine)->vFrags.size());

	LyricsLine *pLine;

	if (nFrag <= 0)
		nLine--;

	pLine = GetLyricsLine(nLine);
	if (!pLine || pLine->vFrags.size() == 0)
		return NULL;

	if (nFrag <= 0)
		nFrag = pLine->vFrags.size() - 1;

	return pLine->vFrags[nFrag];
}

//	COMMENT:
//		取得下一个“歌词段”
BOOL CMLData::NextFrag(int nLine, int nFrag, int &nNextLine, int &nNextFrag, LyricsPiece **ppPiece)
{
	assert(nLine >= 0 && nLine < GetLyricsLinesCount());
	assert(nFrag >= 0);
	assert((unsigned int)nFrag < GetLyricsLine(nLine)->vFrags.size());

	LyricsLine *pLine;
	nNextLine = nLine;
	pLine = GetLyricsLine(nNextLine);
	if (!pLine)
		return FALSE;

	nNextFrag = nFrag + 1;
	if ((unsigned int)nNextFrag >= pLine->vFrags.size())
	{
		nNextLine = nLine + 1;
		pLine = GetLyricsLine(nNextLine);
		if (!pLine || pLine->vFrags.size() <= 0)
		{
			nNextLine = 0;
			nNextFrag = 0;
			if (ppPiece)
				*ppPiece = NULL;
			return FALSE;
		}
		nNextFrag = 0;
	}

	if (ppPiece)
		*ppPiece = GetLyricsLine(nNextLine)->vFrags[nNextFrag];

	return TRUE;
}

//	COMMENT:
//		取得下一个“歌词段”
LyricsPiece *CMLData::NextFrag(int nLine, int nFrag)
{
	assert(nLine < GetLyricsLinesCount());
	assert(nFrag >= 0);
	assert((unsigned int)nFrag < GetLyricsLine(nLine)->vFrags.size());

	nFrag ++;
	if ((unsigned int)nFrag >= GetLyricsLine(nLine)->vFrags.size())
	{
		nLine++;
		if (nLine >= GetLyricsLinesCount())
		{
			return NULL;
		}
		nFrag = 0;
	}

	return GetLyricsLine(nLine)->vFrags[nFrag];
}

bool CMLData::RemoveLyricsLine(int nLine)
{
	LyricsLine *pLine;

	pLine = GetLyricsLine(nLine);
	if (!pLine)
		return false;

	int			k;

	m_arrLyricsLines.erase(m_arrLyricsLines.begin() + nLine);
	for (k = 0; k < (int)m_arrFileLines.size(); k++)
	{
		if (m_arrFileLines[k] == pLine)
		{
			m_arrFileLines.erase(m_arrFileLines.begin() + k);
			break;
		}
	}

	delete pLine;

	return true;
}

void CMLData::AddTextInLyrics(V_Str &vText)
{
	int		nPrevEnd;
	if (m_arrLyricsLines.size())
		nPrevEnd = m_arrLyricsLines.back()->nEndTime;
	else
		nPrevEnd = GetPlayElapsedTime();

	for (int i = 0; i < (int)vText.size(); i++)
	{
		int nEndTime = nPrevEnd + 8 * 1000;

		LyricsLine *pLine = NewLyricsLine(nPrevEnd, nEndTime);
		pLine->bTempLine = true;

		pLine->AppendPiece(pLine->nBegTime, pLine->nEndTime, vText[i].c_str(), vText[i].size(), TRUE, TRUE);
		nPrevEnd = nEndTime;

		AddLineByTime(pLine);
	}
}

void CMLData::AddAdInLyrics(int nBegTimeOffset, LPCTSTR szText, LPCTSTR szLink)
{
	int			nBegTime;

	// if ad exists, don't add new.
	for (int i = 0; i < (int)m_arrLyricsLines.size(); i++)
	{
		LyricsLine *pLine = m_arrLyricsLines[i];
		if (pLine->IsAdvertise() && pLine->szContent && _tcscmp(pLine->szContent, szLink) == 0)
			return;
	}

	nBegTime = nBegTimeOffset;

	if (nBegTime < 0)
		nBegTime = 0;

	// add ad at beginning of the lyrics
	LyricsLine *pLine = NewLyricsLine(nBegTime, -1, szLink, -1);
	pLine->bAdvertise = true;
	pLine->bTempLine = true;
	pLine->AppendPiece(nBegTime, -1, szText, -1, TRUE, TRUE);
	AddLineByTime(pLine);

	// add ad at the end of the lyrics.
	pLine = GetLyricsLine(GetLyricsLinesCount() - 1);
	if (pLine)
	{
		nBegTime = pLine->nEndTime;
		if (nBegTime != -1)
			nBegTime = pLine->nBegTime + 5 * 1000;
		pLine = NewLyricsLine(nBegTime, -1, szLink, -1);
		pLine->bAdvertise = true;
		pLine->bTempLine = true;
		pLine->AppendPiece(nBegTime, -1, szText, -1, TRUE, TRUE);
		AddLine(pLine);
	}

	CheckLyricsLineTime();
}

BOOL CMLData::LyricsLineToText(LyricsLine *pLine, tstring &strLyricsLine)
{
	strLyricsLine.resize(0);
	if (!pLine)
		return FALSE;

	if (pLine->bLyricsLine)
	{
		int		nCount;
		nCount = pLine->vFrags.size();
		for (int i = 0; i < nCount; i++)
		{
			strLyricsLine += pLine->vFrags[i]->szLyric;
		}
	}
	else
	{
		if (pLine->szContent)
			strLyricsLine = pLine->szContent;
	}

	return TRUE;
}

//
// 1、过滤歌词中的一些无法显示的字符：Tab,回车符号
// ?2、增加选项是否过滤行末、行首的空格
// 3、对HTML中的一些符号进行转义
int CMLData::FilterContents(void)
{
	static LPCTSTR		szEscape[] = 
	{
		_T("&lt;"), _T("<"),
		_T("&gt;"), _T(">"),
		_T("&amp;"), _T("&"),
		_T("&quot;"), _T("\""),
		_T("&apos;"), _T("\'"),
		_T("&#58;"), _T(":"),
		NULL, NULL
	};

	//
	// 进行替换，TAB，
	// 对HTML进行转义
	for (UINT i = 0; i < m_arrLyricsLines.size(); i++)
	{
		for (UINT k = 0; k < m_arrLyricsLines[i]->vFrags.size(); k++)
		{
			LPTSTR		szBeg = m_arrLyricsLines[i]->vFrags[k]->szLyric;

			int		pos = 0;
			for (int m = 0; szBeg[m] != '\0'; m++)
			{
				if (szBeg[m] == '\t')
				{
					szBeg[pos] = ' ';
					pos++;
				}
				else if (szBeg[m] == '&')
				{
					LPCTSTR	*pszEsc = szEscape;
					while (*pszEsc)
					{
						int		nLen;
						nLen = _tcslen(*pszEsc);
						if (_tcsnicmp(*pszEsc, szBeg, nLen) == 0)
						{
							szBeg[pos] = (*(pszEsc + 1))[0];
							pos++;
							m += nLen - 1;
							break;
						}
						pszEsc+=2;
					}
					if (*pszEsc == NULL)
					{
						// "&#58;", ":",
						BOOL		bNumb = FALSE;
						LPCTSTR szNumb = szBeg + m + 1;
						if (*szNumb == '#')
						{
							int		nNumb = 0;
							int		x;
							for (x = 0; x < 3 && IsDigit(szNumb[x + 1]); x++)
								nNumb = nNumb * 10 + (szNumb[x + 1] - '0');
							if (nNumb > 0 && nNumb <= 0xFFF && szNumb[x + 1] == ';')
							{
								bNumb = TRUE;
								m += 2 + x;
								szBeg[pos] = (TCHAR)nNumb;
								pos++;
							}
						}

						if (!bNumb)
						{
							szBeg[pos] = '&';
							pos++;
						}
					}
				}
				else
				{
					szBeg[pos] = szBeg[m];
					pos++;
				}
			}
			szBeg[pos] = '\0';
			m_arrLyricsLines[i]->vFrags[k]->nLen = pos;
		}
	}

	return 0;
}

#ifdef _ID3V2_SUPPORT
void CMLData::ToID3v2SynchLyrics(ID3v2SynchLyrics &lyrics, CHAR_ENCODING encoding)
{
	strcpy_safe(lyrics.m_szLanguage, CountOf(lyrics.m_szLanguage), m_id3v2SaveCacheInfo.m_strLanguage.c_str());
	lyrics.m_bTimeStampMs = true;

	CLrcTag			lrcTag(m_lyrPropertiesOrg, m_lyrProperties);

	lrcTag.WriteToText(m_id3v2SaveCacheInfo.m_strLrcDescription, true);

	lyrics.m_encodingType = CharEncodingToID3v2EncType(encoding);
	lyrics.m_strContentDesc = m_id3v2SaveCacheInfo.m_strLrcDescription.c_str();

	// convert lyrics rows
	for (int i = 0; i < (int)m_arrLyricsLines.size(); i++)
	{
		LyricsLine *pLine = m_arrLyricsLines[i];

		if (pLine->bLyricsLine && !pLine->IsTempLine())
		{
			//
			// to compatible with windows media player, the Karaoke info has to be lost.
			//
			LrcSyllable	syllable;
			syllable.bNewLine = false;
			syllable.nTime = pLine->nBegTime - m_lyrProperties.GetOffsetTime();
			if (syllable.nTime < 0)
				syllable.nTime = 0;
			for (int k = 0; k < (int)pLine->vFrags.size(); k++)
			{
				LyricsPiece *pPiece = pLine->vFrags[k];

				syllable.strText += pPiece->szLyric;
			}
			lyrics.m_vSynLyrics.push_back(syllable);

			if (!m_id3v2SaveCacheInfo.m_bAllSyllableIsNewLine)
			{
				LrcSyllable	syllable;
				syllable.bNewLine = true;
				lyrics.m_vSynLyrics.push_back(syllable);
			}
		}
	}
}

void CMLData::ToID3v2UnsynchLyrics(ID3v2UnsynchLyrics &lyrics, CHAR_ENCODING encoding)
{
	tstring		str;

	ToString(str, FT_LYRICS_TXT, TRUE);

	strcpy_safe(lyrics.m_szLanguage, CountOf(lyrics.m_szLanguage), m_id3v2SaveCacheInfo.m_strLanguage.c_str());

	lyrics.m_encodingType = CharEncodingToID3v2EncType(encoding);
	lyrics.m_strContentDesc = m_id3v2SaveCacheInfo.m_strLrcDescription.c_str();
	lyrics.m_strLyrics = str.c_str();
}
#endif // #ifdef _ID3V2_SUPPORT

void CMLData::ToString(tstring &str, MLFileType lrcFormat, bool bIncTags)
{
	bool	bToLrcFormat = (lrcFormat == FT_LYRICS_LRC && GetLyrContentType() != LCT_TXT);

	CLrcParser		parser(this);

	parser.ToString(str, bIncTags, !bToLrcFormat);
}

/*int CMLData::FromString(LPCVOID lpLrc, int nLen, bool bAddTimeStampForTextLine, bool bUseSpecifiedEncoding, CHAR_ENCODING encodingSpecified)
{
	if (nLen == 0)
		return ERR_OK;

	ClearLyrics();

	CLrcParser		parser(this);
	int				nRet;

	if (bUseSpecifiedEncoding)
		m_lyrProperties.SetEncoding(encodingSpecified);

	nRet = parser.ParseString((LPBYTE)lpLrc, nLen, bAddTimeStampForTextLine, bUseSpecifiedEncoding, encodingSpecified);
	if (nRet == ERR_OK)
	{
		SetLyrContentType(parser.GetLyrContentType());

		FilterContents();
	}

	return nRet;
}*/

int CMLData::FromString(LPCTSTR szLyrics)
{
	ClearLyrics();

	CLrcParser		parser(this);
	int				nRet;

	nRet = parser.ParseString(szLyrics, false);
	if (nRet == ERR_OK)
	{
		SetLyrContentType(parser.GetLyrContentType());

		FilterContents();
	}

	return nRet;
}

void CMLData::CopyLyricsLines(CLyricsLines &lyrLines)
{
	int			i;
	LyricsLine *pLine;

	lyrLines.clear();

	for (i = 0; i < (int)m_arrLyricsLines.size(); i++)
	{
		LyricsLine *pLineSrc = m_arrLyricsLines[i];

		pLine = DuplicateLyricsLine(pLineSrc);
		lyrLines.push_back(pLine);
	}
}

#ifdef _CPPUNIT_TEST

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

class CTestCaseCMLData : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseCMLData);
#ifndef _IPHONE
	CPPUNIT_TEST(Test_CMLData_OpenLrcFile);
	CPPUNIT_TEST(Test_CMLData_OpenSaveID3v2_USLT);
	CPPUNIT_TEST(Test_CMLData_OpenSaveID3v2_SYLT);
	CPPUNIT_TEST(Test_CMLData_OpenSaveLyrics3v2);
#endif // #ifndef _IPHONE
	CPPUNIT_TEST_SUITE_END();

protected:
	tstring			m_strTestDataFolder;

public:
#ifndef _IPHONE
	void setUp()
	{
		m_strTestDataFolder = FileGetPath(_T(__FILE__));
		DirStringAddSlash(m_strTestDataFolder);
		m_strTestDataFolder += _T("TestData\\");
	}
	void tearDown() 
	{
		RemoveTempMediaFile();
	}

protected:
	void Test_CMLData_OpenLrcFile()
	{
		LPCTSTR		szSongFile = _T("artist - title.mp3");
		tstring		vstrLyrFile[] = {
			m_strTestDataFolder + _T("test(gb).lrc"),
			m_strTestDataFolder + _T("test(ucs2).lrc"),
			m_strTestDataFolder + _T("test(ucs2-be).lrc"),
			m_strTestDataFolder + _T("test(utf8).lrc")
		};
		int			nRet;
		CMLData		data;

		for (int i = 0; i < CountOf(vstrLyrFile); i++)
		{
			nRet = data.OpenLyrics(szSongFile, 60 * 1000, vstrLyrFile[i].c_str());
			CPPUNIT_ASSERT(nRet == ERR_OK);

			{
				// Test with CLrcTag, tag same?
				CLrcTag		tag;

				nRet = tag.ReadFromFile(vstrLyrFile[i].c_str());
				CPPUNIT_ASSERT(nRet == ERR_OK);

				CLrcTag		tagCompare(tag, data.Properties());
				CPPUNIT_ASSERT(!tagCompare.IsTagChanged());
			}

			{
				// Lyrics content ok?
				// [00:01.00]L1
				// [01:01.01][01:00.01]L2
				// [02:01.10]L3
				LPCTSTR		vLyrLines[] = { _T("L1"), _T("L2"), _T("L2"), _T("L3") };
				int			vTime[] = { 1*1000, 1*60*1000+10, (1*60+1)*1000+10, (2*60+1)*1000+100};

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
			}
		}
	}

	void PrepareMediaFile()
	{
		RemoveTempMediaFile();

		tstring		vstrSongFile[] = {
			m_strTestDataFolder + _T("lyr-usly.mp3"),
			m_strTestDataFolder + _T("lyr(wmp-chs).mp3"),
			m_strTestDataFolder + _T("lyr-usly(chs).mp3"),
			m_strTestDataFolder + _T("lyrics3v2.mp3"),
			m_strTestDataFolder + _T("lyr-chs.m4a"),
		};

		for (int i = 0; i < (int)CountOf(vstrSongFile); i++)
		{
			tstring	strFile = GetAppDataDir();
			strFile += FileGetName(vstrSongFile[i].c_str());
			CPPUNIT_ASSERT(CopyFile(vstrSongFile[i].c_str(), strFile.c_str(), false));
		}
	}

	void RemoveTempMediaFile()
	{
		tstring		vstrSongFile[] = {
			m_strTestDataFolder + _T("lyr-usly.mp3"),
			m_strTestDataFolder + _T("lyr(wmp-chs).mp3"),
			m_strTestDataFolder + _T("lyr-usly(chs).mp3"),
			m_strTestDataFolder + _T("lyrics3v2.mp3"),
			m_strTestDataFolder + _T("lyr-chs.m4a"),
		};

		for (int i = 0; i < (int)CountOf(vstrSongFile); i++)
		{
			tstring	strFile = GetAppDataDir();
			strFile += FileGetName(vstrSongFile[i].c_str());
			DeleteFile(strFile.c_str());
		}
	}

	void Test_CMLData_OpenSaveID3v2_USLT()
	{
		int			nRet;

		PrepareMediaFile();

		tstring		strWorkingFolder = GetAppDataDir();

		//
		// Test on embedded USLT lyrics.
		//

		tstring		vstrSongFile[] = {
			strWorkingFolder + _T("lyr-usly.mp3"),
			strWorkingFolder + _T("lyr(wmp-chs).mp3"),
			strWorkingFolder + _T("lyr-usly(chs).mp3"),
			strWorkingFolder + _T("lyr(wmp-chs).mp3"),
			strWorkingFolder + _T("lyr-chs.m4a"),
		};
		LPCTSTR		vLyrFile[] = {
			_T("song://id3v2/uslt/eng"),
			_T("song://id3v2/uslt/eng"),
			_T("song://id3v2/uslt/eng"),
			_T("song://id3v2/uslt/chi"),
			SZ_SONG_M4A_LYRICS,
		};
		bool		vWriteTested[] = { false, false, false, false, false };
		ASSERT(CountOf(vWriteTested) == CountOf(vLyrFile));
		ASSERT(CountOf(vWriteTested) == CountOf(vstrSongFile));

		for (int i = 0; i < CountOf(vstrSongFile); i++)
		{
			for (int kk = 0; kk < 2; kk++)
			{
				// Loop twice, first for reading, second for writing.
				CMLData		data;
				nRet = data.OpenLyrics(vstrSongFile[i].c_str(), 60 * 1000, vLyrFile[i]);
				CPPUNIT_ASSERT(nRet == ERR_OK);

				{
					//
					// Open, Lyrics content ok?
					//
					LPCTSTR		vLyrLines[] = { _T("Line1"), _T("Line2"), _T("Line3") };
					LPCTSTR		vLyrLinesChs[] = { _T("第一行"), _T("第二行"), _T("第三行") };

					CLyricsLines	&lyrLines = data.GetRawLyrics();

					for (int k = 0; k < lyrLines.size(); k++)
					{
						LyricsLine *pLine = lyrLines[k];

						CPPUNIT_ASSERT(pLine->vFrags.size() == 1);
						CPPUNIT_ASSERT(k < CountOf(vLyrLines));
						if (i < 2)
							CPPUNIT_ASSERT(_tcscmp(pLine->vFrags[0]->szLyric, vLyrLines[k]) == 0);
						else
							CPPUNIT_ASSERT(_tcscmp(pLine->vFrags[0]->szLyric, vLyrLinesChs[k]) == 0);
					}
					CPPUNIT_ASSERT(lyrLines.size() == CountOf(vLyrLines));
				}

				// Test on save uslt
				if (!vWriteTested[i])
				{
					data.Properties().m_strArtist = _T("artist");
					data.Properties().m_strTitle = _T("title");
					CPPUNIT_ASSERT(data.Save() == ERR_OK);
					vWriteTested[i] = true;
				}
				else
				{
					CPPUNIT_ASSERT(_tcscmp(data.Properties().m_strArtist.c_str(), _T("artist")) == 0);
					CPPUNIT_ASSERT(_tcscmp(data.Properties().m_strTitle.c_str(), _T("title")) == 0);
				}
			}
		}
	}

	void Test_CMLData_OpenSaveID3v2_SYLT()
	{
		int			nRet;
		CMLData		data;

		PrepareMediaFile();

		tstring		strWorkingFolder = GetAppDataDir();

		//
		// Test on embedded SYLT lyrics.
		//
		tstring		vstrSongFileSylt[] = {
			strWorkingFolder + _T("lyr(wmp-chs).mp3"),
			strWorkingFolder + _T("lyr(wmp-chs).mp3"),
		};
		LPCTSTR		vLyrFileSylt[] = {
			_T("song://id3v2/sylt/eng"),
			_T("song://id3v2/sylt/chi"),
		};

		bool		vWriteTested[] = { false, false, false, false };

		for (int kk = 0; kk < 2; kk++)
		{
			// Loop twice, first for reading, second for writing.
			int		nOffsetTime = 0;
			if (kk == 1)
				nOffsetTime = 500;

			for (int i = 0; i < CountOf(vstrSongFileSylt); i++)
			{
				nRet = data.OpenLyrics(vstrSongFileSylt[i].c_str(), 60 * 1000, vLyrFileSylt[i]);
				CPPUNIT_ASSERT(nRet == ERR_OK);

				{
					// Lyrics content ok?
					LPCTSTR		vLyrLines[] = { _T("Line1"), _T("Line2"), _T("Line3") };
					LPCTSTR		vLyrLinesChs[] = { _T("第一行"), _T("第二行"), _T("第三行") };
					int			vTime[] = { 1, 1*1000+9, 1*1000+18 };

					CLyricsLines	&lyrLines = data.GetRawLyrics();

					for (int k = 0; k < lyrLines.size(); k++)
					{
						LyricsLine *pLine = lyrLines[k];

						CPPUNIT_ASSERT(pLine->vFrags.size() == 1);
						CPPUNIT_ASSERT(k < CountOf(vLyrLines));
						if (i < 1)
							CPPUNIT_ASSERT(_tcscmp(pLine->vFrags[0]->szLyric, vLyrLines[k]) == 0);
						else
							CPPUNIT_ASSERT(_tcscmp(pLine->vFrags[0]->szLyric, vLyrLinesChs[k]) == 0);

						CPPUNIT_ASSERT(pLine->vFrags[0]->nBegTime == vTime[k] + nOffsetTime);
					}
					CPPUNIT_ASSERT(lyrLines.size() == CountOf(vLyrLines));
				}

				// Test on save SYLT
				if (!vWriteTested[i])
				{
					data.Properties().m_strArtist = _T("artist");
					data.Properties().m_strTitle = _T("title");
					data.Properties().SetOffsetTime(-500);
					CPPUNIT_ASSERT(data.Save() == ERR_OK);
					vWriteTested[i] = true;
				}
				else
				{
					CPPUNIT_ASSERT(_tcscmp(data.Properties().m_strArtist.c_str(), _T("artist")) == 0);
					CPPUNIT_ASSERT(_tcscmp(data.Properties().m_strTitle.c_str(), _T("title")) == 0);
				}
			}
		}
	}

	void Test_CMLData_OpenSaveLyrics3v2()
	{
		int			nRet;
		CMLData		data;

		PrepareMediaFile();

		tstring		strWorkingFolder = GetAppDataDir();

		tstring		vstrSongFileSylt[] = {
			strWorkingFolder + _T("lyrics3v2.mp3"),
		};
		LPCTSTR		vLyrFileSylt[] = {
			SZ_SONG_LYRICS3V2,
		};
		bool		vWriteTested[] = { false };

		for (int kk = 0; kk < 2; kk++)
		{
			// Loop twice, first for reading, second for writing.
			int		nOffsetTime = 0;
			if (kk == 1)
				nOffsetTime = 500;

			for (int i = 0; i < CountOf(vstrSongFileSylt); i++)
			{
				nRet = data.OpenLyrics(vstrSongFileSylt[i].c_str(), 60 * 1000, vLyrFileSylt[i]);
				CPPUNIT_ASSERT(nRet == ERR_OK);

				{
					// Lyrics content ok?
					LPCTSTR		vLyrLines[] = { _T("第一行"), _T("第二行"), _T("第三行") };
					int			vTime[] = { 0, 1*1000+10, 1*1000+20 };

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
				}

				// Test on saving
				if (!vWriteTested[i])
				{
					data.Properties().m_strArtist = _T("artist");
					data.Properties().m_strTitle = _T("title");
					CPPUNIT_ASSERT(data.Save() == ERR_OK);
					vWriteTested[i] = true;
				}
				else
				{
					CPPUNIT_ASSERT(_tcscmp(data.Properties().m_strArtist.c_str(), _T("artist")) == 0);
					CPPUNIT_ASSERT(_tcscmp(data.Properties().m_strTitle.c_str(), _T("title")) == 0);
				}
			}
		}
	}
#endif // #ifndef _IPHONE

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCMLData);

#endif // _CPPUNIT_TEST
