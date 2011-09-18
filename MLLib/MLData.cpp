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

LRC_SOURCE_TYPE GetLrcSourceType(LPCTSTR szLrcSource)
{
#define SONG_HDR_LEN		7
	LPCTSTR		szTemp;
	if (_tcsnicmp(szLrcSource, _T("song://"), SONG_HDR_LEN) == 0)
	{
		szTemp = SZ_SONG_LYRICS3V2 + SONG_HDR_LEN;
		if (_tcsnicmp(szLrcSource + SONG_HDR_LEN, szTemp, _tcslen(szTemp)) == 0)
			return LST_LYRICS3V2;

#ifdef _ID3V2_SUPPORT
		szTemp = SZ_SONG_ID3V2_LYRICS + SONG_HDR_LEN;
		if (_tcsnicmp(szLrcSource + SONG_HDR_LEN, szTemp, _tcslen(szTemp)) == 0)
			return LST_ID3V2_LYRICS;

		szTemp = SZ_SONG_ID3V2_SYLT + SONG_HDR_LEN;
		if (_tcsnicmp(szLrcSource + SONG_HDR_LEN, szTemp, _tcslen(szTemp)) == 0)
			return LST_ID3V2_SYLT;

		szTemp = SZ_SONG_ID3V2_USLT + SONG_HDR_LEN;
		if (_tcsnicmp(szLrcSource + SONG_HDR_LEN, szTemp, _tcslen(szTemp)) == 0)
			return LST_ID3V2_USLT;
#endif // #ifdef _ID3V2_SUPPORT

		return LST_UNKNOWN;
	}
	else if (_tcsicmp(szLrcSource, NONE_LYRCS) == 0)
		return LST_NONE;
	else
		return LST_FILE;
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

void ML_ROW::SetContent(LPCTSTR szContent, int nLen)
{
	if (this->szContent != NULL)
		delete[] this->szContent;

	this->szContent = new TCHAR[nLen + 1];
	_tcsncpysz_safe(this->szContent, nLen + 1, szContent, nLen);
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

CMLData::CMLData()
{
	EmptyStr(m_szLyrContentMd5Org);
	MD5ToString((const char *)_T(""), 0,
		m_szLyrContentMd5Org);

	m_dwDefShowStyle = LSA_SHOW_DEFAULT;

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

	lrcSourceType = GetLrcSourceType(szLrcSource);

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
		//
		// Open ID3v2 lyrics
		//

		CID3v2IF		id3v2(GetEncodingIndex());

		nRet = id3v2.Open(szSongFile, false, false);
		if (nRet != ERR_OK)
			return nRet;

		ID3v2LyricsInfo		lyricsInfo;
		tstringex			str;
		nRet = id3v2.GetLyrics(lyricsInfo);
		if (nRet == ERR_OK)
		{
			if (lrcSourceType == LST_ID3V2_SYLT)
			{
				ID3v2SynchLyrics	*plyrics;
				if (ID3v2LyricsInfoGetSynchLyrics(lyricsInfo, szLrcSource, plyrics))
				{
					nRet = OpenLyricsInSongOfID3v2SYLT(*plyrics, bUseSpecifiedEncoding, encodingSpecified);
				}
				else
					nRet = ERR_NOT_FOUND;
			}
			else if (lrcSourceType == LST_ID3V2_USLT)
			{
				ID3v2UnsynchLyrics	*plyrics;
				if (ID3v2LyricsInfoGetUnsynchLyrics(lyricsInfo, szLrcSource, plyrics))
				{
					nRet = OpenLyricsInSongOfID3v2USLT(*plyrics, bUseSpecifiedEncoding, encodingSpecified);
				}
				else
					nRet = ERR_NOT_FOUND;
			}
			else if (lrcSourceType == LST_ID3V2_LYRICS)
			{
				if (lyricsInfo.m_lyricsUserDefined.m_strValue.empty())
				{
					SetCustomErrorDesc(_T("The embedded lyrics is empty, please click the \"search lyrics\" button to get lyrics."));
					nRet = ERR_CUSTOM_ERROR;
				}
				else
				{
					nRet = FromString(lyricsInfo.m_lyricsUserDefined.m_strValue.c_str());
				}
			}
		}
		id3v2.Close();
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
int CMLData::OpenLyrics(LPCTSTR szSongFile, int nMediaLength, LPCTSTR szLyrContent)
{
	m_strSongFile = szSongFile;
	m_nMediaLength = nMediaLength;
	m_lrcSourceType = LST_ID3V2_USLT;

	int nRet = FromString(szLyrContent);
	if (nRet != ERR_OK)
		return nRet;

	OnLyricsOpened();
}
	
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
	CLyricsParser	*pParser;
	int				nRet;

	Close();

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

	nRet = pParser->ParseFile(bUseSpecifiedEncoding, encodingSpecified);
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
	PML_ROW			pRow;
	PLYRIC_FRAG		pFrag;
	CLrcTag			lrcTag;

	Close();

	m_id3v2SaveCacheInfo.m_strLanguage = lyrics.m_szLanguage;
	m_id3v2SaveCacheInfo.m_bAllSyllableIsNewLine = lyrics.m_bAllSyllableIsNewLine;
	m_lyrProperties.SetEncoding(ID3v2EncTypeToCharEncoding(lyrics.m_encodingType));

	// Read id3v2 SYLT lyrics information
	lrcTag.ReadFromText(lyrics.m_strContentDesc.c_str(), lyrics.m_strContentDesc.size());

	m_id3v2SaveCacheInfo.m_strLrcDescription = lyrics.m_strContentDesc.c_str();

	pRow = NULL;
	for (ID3v2SynchLyrics::LIST_SYLRC::iterator it = lyrics.m_vSynLyrics.begin();
		it != lyrics.m_vSynLyrics.end(); it++)
	{
		LrcSyllable		&syllable = *it;

		if (syllable.bNewLine)
		{
			if (pRow && pRow->vFrags.size() > 1)
				m_dwDefShowStyle = LSA_SHOW_KARAOKE;
			pRow = NULL;
			continue;
		}

		if (!pRow || lyrics.m_bAllSyllableIsNewLine)
		{
			pRow = NewMLRow(syllable.nTime, -1);
			AddRow(pRow);
		}

		pFrag = NewLyricFrag(syllable.nTime, -1, syllable.strText.c_str(), 
			syllable.strText.size(), false, true);
		pRow->vFrags.push_back(pFrag);
	}

	m_lyrProperties = lrcTag;
	m_lyrProperties.SetOffsetTime(0);
	SetLyrContentType(LCT_LRC);

	CheckLyricsRowTime();

	FixExtendedLrcTimeStamps();

	return ERR_OK;
}

int CMLData::OpenLyricsInSongOfLyrics3v2(const char *buffer, int len, bool bUseSpecifiedEncoding, CHAR_ENCODING encodingSpecified)
{
	int		nRet;

	Close();

	if (len == 0)
	{
		SetCustomErrorDesc(_T("The embedded lyrics is empty, please click the \"search lyrics\" button to get lyrics."));
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

	if (m_arrLyricsRows.empty())
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
			m_lyrScrollActionRecorder.InitCreateAutoEvents(m_arrLyricsRows);
			m_lyrScrollActionRecorder.ScrollToLine(m_arrLyricsRows, 0, 10 * 1000, false);
			m_lyrScrollActionRecorder.ScrollToLine(m_arrLyricsRows, m_arrLyricsRows.size(), m_nMediaLength, true);
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
	{
		return SaveLyricsInSongOfLyrics3v2(GetEncodingIndex());
	}
	else if (m_lrcSourceType == LST_ID3V2_SYLT)
	{
		return SaveLyricsInSongOfID3v2(false, true, false);
	}
	else if (m_lrcSourceType == LST_ID3V2_USLT)
	{
		return SaveLyricsInSongOfID3v2(true, false, false);
	}
	else if (m_lrcSourceType == LST_ID3V2_LYRICS)
	{
		return SaveLyricsInSongOfID3v2(false, false, true);
	}
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
int CMLData::SaveLyricsInSongOfID3v2(bool bUslt, bool bSylt, bool bUserDefinedLyr)
{
	CID3v2IF		id3v2(m_lyrProperties.GetEncodingIndex());
	int				nRet;

	if (!bUslt && !bSylt && !bUserDefinedLyr)
		return ERR_OK;

	nRet = id3v2.Open(m_strSongFile.c_str(), true, true);
	if (nRet != ERR_OK)
		return nRet;

	tstring			str;
	CHAR_ENCODING	encoding = m_lyrProperties.GetEncodingIndex();
	ToString(str, FT_LYRICS_TXT, TRUE);
	if (IsAnsiStr(str.c_str()))
		encoding = ED_SYSDEF;
	else
		encoding = ED_UNICODE;

	if (bUslt)
	{
		ID3v2UnsynchLyrics	lyrics;
		ToID3v2UnsynchLyrics(lyrics, encoding);

		nRet = id3v2.UpdateUnsynchLyricsFrame(&lyrics);
		if (nRet != ERR_OK)
			return nRet;
	}

	if (bSylt)
	{
		ID3v2SynchLyrics	lyrics;

		ToID3v2SynchLyrics(lyrics, encoding);

		nRet = id3v2.UpdateSynchLyricsFrame(&lyrics);
		if (nRet != ERR_OK)
			return nRet;
	}

	if (bUserDefinedLyr)
	{
		tstring		str;

		ToString(str, FT_LYRICS_LRC, TRUE);

		ID3v2TextUserDefined	lyrics;

		lyrics.SetValueAndOptimizeEncoding(SZ_ID3V2_LYR_USER_DEFINED, str.c_str());
		nRet = id3v2.UpdateUserDefinedTextFrameByDesc(lyrics);
		if (nRet != ERR_OK)
			return nRet;
	}

	nRet = id3v2.Save();
	if (nRet == ERR_OK)
	{
		if ((bUslt && m_lrcSourceType == LST_ID3V2_USLT
			&& m_lyrProperties.m_lyrContentType == m_lyrPropertiesOrg.m_lyrContentType) ||
			(bSylt && m_lrcSourceType == LST_ID3V2_SYLT))
		{
			ResetModificationFlag();
		}
	}

	return nRet;
}

int CMLData::SaveLyricsInSongOfLyrics3v2(CHAR_ENCODING encoding)
{
	// save into mp3 file
	int			nRet;
	tstring		str;
	CLyrics3v2	lyrics3v2;
	CCharBuffer	strData;

	ToString(str, FT_LYRICS_LRC, TRUE);

	ConvertStrByFileEncoding(strData, str.c_str(), str.size(), 
		CharEncodingToFileEncoding(encoding), encoding);

	nRet = lyrics3v2.WriteLyrcsInfo(m_strSongFile.c_str(), (const char *)strData.c_str(), strData.size() * sizeof(char), TRUE);
	if (nRet == ERR_OK && m_lrcSourceType == LST_LYRICS3V2)
		ResetModificationFlag();

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
	MLFileType		NewFileType;

	bUseNewFileName = false;

	// 根据文件的扩展名来判断文件类型，然后调用不同的歌词分析类来分析歌词。
	if (FileIsExtSame(szFile, _T(".lrc")) )
	{
		pParser = new CLrcParser(this);
		NewFileType = FT_LYRICS_LRC;
		if (GetLyrContentType() <= LCT_LRC)
			bUseNewFileName = true;
	}
	else if (FileIsExtSame(szFile, _T(".txt")) )
	{
		pParser = new CLrcParser(this);
		NewFileType = FT_LYRICS_TXT;
		if (GetLyrContentType() <= LCT_TXT)
			bUseNewFileName = true;
	}
#ifndef _IPHONE
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
	lrcTag.WriteToText(m_id3v2SaveCacheInfo.m_strLrcDescription, GetEncodingIndex(), false);
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

PML_ROW CMLData::GetLyricsRow(int nRow)
{
	int nSize = m_arrLyricsRows.size();
	if (nSize <= nRow || nRow < 0)
		return NULL;
	
	return m_arrLyricsRows[nRow];
}

// COMMENT:
//		检查歌词数据的时间顺序的正确性
//		查找所有的歌词显示时间为-1的“歌词段”，进行计算。
void CMLData::CheckLyricsRowTime()
{
	int		nRowCount;
	int		nBegRow = 0;
	int		nBegFrag = 0;
	int		nBegTime = 0;
	int		nEndRow = -1;
	int		nEndFrag = 0;
	int		nEndTime = 0;
	int		nRow = 0;
	int		nFrag = 0;
	BOOL	bFind = FALSE;

	if (GetLyricRowCount() == 0)
		return;

	if (GetLyricsRow(0)->vFrags.size() == 0)
		return;

	while (1)
	{
		// 查找开始歌词段
		bFind = FALSE;
		do
		{
			PLYRIC_FRAG	pFrag;
			PML_ROW		pRow;

			pRow = GetLyricsRow(nRow);
			assert(pRow);

			pFrag = pRow->vFrags[nFrag];
			if (pFrag->nBegTime == -1)
			{
				pFrag = PrevFrag(nRow, nFrag);
				if (pFrag)
				{
					assert(pFrag->nEndTime != -1);
					nBegTime = pFrag->nEndTime;
				}
				else
				{
					// 一定是第一行，开始时间设置为 15 s
					nBegTime = LYRICS_BEGIN_TIME;
				}
				nBegRow = nRow;
				nBegFrag = nFrag;
				bFind = TRUE; 
				break;
			}

			if (pRow->vFrags[nFrag]->nEndTime == -1)
			{
				nBegTime = pRow->vFrags[nFrag]->nBegTime;
				nBegRow = nRow;
				nBegFrag = nFrag;
				bFind = TRUE;
				break;
			}

			nRow = nBegRow;
			nFrag = nBegFrag;
		}
		while (NextFrag(nRow, nFrag, nBegRow, nBegFrag));

		if (!bFind)	// 所有的歌词段的时间都是正确的。
			break;

		// 查找结束歌词段
		while (1)
		{
			PML_ROW		pRow;

			if (!NextFrag(nRow, nFrag, nEndRow, nEndFrag))
			{
				// 所有的歌词都没有时间标记，则最后的时间定为歌曲时间 － LYRICS_END_TIME
				nEndTime = GetMediaLength() - LYRICS_END_TIME - GetOffsetTime();
				if (nBegTime >= nEndTime)
					nEndTime = GetMediaLength() - GetOffsetTime();
				if (nBegTime >= nEndTime)
					nEndTime = nBegTime + 10 * 1000;
				nEndRow = nRow;
				nEndFrag = nFrag;
				break;
			}

			pRow = GetLyricsRow(nEndRow);
			assert(pRow);

			if (pRow->vFrags[nEndFrag]->nBegTime != -1)
			{
				nEndTime = pRow->vFrags[nEndFrag]->nBegTime;
				nEndRow = nRow;
				nEndFrag = nFrag;
				break;
			}

			if (pRow->vFrags[nEndFrag]->nEndTime != -1)
			{
				nEndTime = pRow->vFrags[nEndFrag]->nEndTime;
				break;
			}

			nRow = nEndRow;
			nFrag = nEndFrag;
		}
		if (nBegTime > nEndTime)
			nBegTime = nEndTime - 2 * 1000;


		// 计算在开始段到最后段歌词之间的时间。
		CalFragTimeBetweenRows(nBegRow, nBegFrag, nEndRow, nEndFrag, nBegTime, nEndTime);
	}
	nRowCount = m_arrLyricsRows.size();
	for (int i = 0; i < nRowCount; i++)
	{
		if (m_arrLyricsRows[i]->nBegTime == -1)
		{
			assert(m_arrLyricsRows[i]->vFrags.size() != 0);
			m_arrLyricsRows[i]->nBegTime = m_arrLyricsRows[i]->vFrags[0]->nBegTime;
		}
		if (m_arrLyricsRows[i]->nEndTime == -1)
		{
			assert(m_arrLyricsRows[i]->vFrags.size() != 0);
			m_arrLyricsRows[i]->nEndTime = m_arrLyricsRows[i]->vFrags[m_arrLyricsRows[i]->vFrags.size() - 1]->nEndTime;
		}
	}
}

void CMLData::FixExtendedLrcTimeStamps()
{
	//
	// For Extended lrc lyrics, the end time of ending lyrics frag is incorrect.
	//
	if (m_dwDefShowStyle != LSA_SHOW_KARAOKE)
		return;

	for (int i = 0; i < (int)GetLyricRowCount(); i++)
	{
		PML_ROW		pRow = GetLyricsRow(i);

		if (pRow->vFrags.size() < 1)
			continue;

		int			n = pRow->vFrags.size() - 1;
		PLYRIC_FRAG	pFrag;
		pFrag = pRow->vFrags[n];
		if (IsTempTimeEnd(pFrag->dwAttrib))
		{
			PML_ROW		pNextRow;
			if (i + 1 < (int)GetLyricRowCount())
				pNextRow = GetLyricsRow(i + 1);
			else
				pNextRow = NULL;
			if (pNextRow)
			{
				if (pFrag->nBegTime + 500 < pNextRow->nBegTime)
					pFrag->nEndTime = pFrag->nBegTime + 500;
			}
			else
				pFrag->nEndTime = pFrag->nBegTime + 500;
		}
	}
}

void CMLData::ClearLyrics()
{
	m_arrLyricsRows.erase(m_arrLyricsRows.begin(), m_arrLyricsRows.end());
	m_arrFileRows.clear();
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

	m_dwDefShowStyle = LSA_SHOW_DEFAULT;

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

void CMLData::AddRow(PML_ROW pFileRow)
{
	if (pFileRow->nRowType == ML_ROW_LYRICS)
	{
		m_arrLyricsRows.push_back(pFileRow);
	}

	m_arrFileRows.push_back(pFileRow);
}

// COMMENT:
//		将歌词行按照时间排序插入
void CMLData::AddRowByTime(PML_ROW pRow)
{
	if (pRow->nRowType == ML_ROW_LYRICS)
	{
		// 将歌词行插入 m_arrLyricsRows
		CMLyrLines::iterator	itPos;

		int			i, nRowCount;

		for (i = 0; i < (int)pRow->vFrags.size(); i++)
			pRow->vFrags[i]->nDrawWidth = 0;

		nRowCount = m_arrLyricsRows.size();
		for (i = 0; i < nRowCount; i++)
		{
			if (m_arrLyricsRows[i]->nBegTime > pRow->nBegTime)
				break;
		}

		if (i != nRowCount)
		{
			// 找到了位置
			itPos = find(m_arrFileRows.begin(), m_arrFileRows.end(), m_arrLyricsRows[i]);
			
			if (itPos == m_arrFileRows.end())
			{
				// error occurs!
				assert(0);
				m_arrFileRows.push_back(pRow);
			}
			else
			{
				m_arrFileRows.insert(itPos, pRow);
			}

			// 插入歌词部分
			PML_ROW		pRowPrev;
			if (i > 0)
			{
				// reset the time of prev line.
				pRowPrev = m_arrLyricsRows[i - 1];
				if (pRowPrev->nEndTime != -1)
				{
					if (pRowPrev->vFrags.size() == 1 && IsTempTimeEnd(pRowPrev->vFrags[0]->dwAttrib))
					{
						pRowPrev->vFrags[0]->nEndTime = pRow->nBegTime;
						pRowPrev->nEndTime = pRow->nBegTime;
					}
				}
			}
			m_arrLyricsRows.insert(m_arrLyricsRows.begin() + i, pRow);
		}
		else
		{
			// 未找到位置，则插入末尾
			m_arrLyricsRows.push_back(pRow);
			m_arrFileRows.push_back(pRow);
		}
	}
	else
		m_arrFileRows.push_back(pRow);
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
int CMLData::GetCurPlayLine(const VLyrLines &lyrLines)
{
	int				nRowCount;

	//// ERROR:在多线程中使用，可能会存在问题。
	static int		nRow = 0;

	nRowCount = lyrLines.size();

	if (nRowCount == 0)
		return -1;

	if (nRow >= nRowCount)
		nRow = 0;

	// 找到前一行
	for (; nRow >= 0; nRow--)
	{
		PML_ROW pLyricRow = lyrLines[nRow];
		assert(pLyricRow);

		if (pLyricRow->nEndTime != -1 && pLyricRow->nEndTime < GetPlayElapsedTime())
		{
			nRow++;
			break; // ok.
		}
	}
	if (nRow == -1)
		nRow = 0;
	
	// 找到下一行
	for (; nRow < nRowCount; nRow ++)
	{
		PML_ROW pLyricRow = lyrLines[nRow];
		assert(pLyricRow);

		if (pLyricRow->nEndTime > GetPlayElapsedTime())
			break; // ok.
	}
	if (nRow == nRowCount)
		nRow = nRowCount - 1;

	for (int i = nRow - 1; i >= 0; i--)
	{
		if (lyrLines[nRow]->nEndTime != lyrLines[i]->nEndTime)
		{
			return i + 1;
		}
	}

	assert(nRow != -1);
	return nRow;
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
//		nRow, nBegFrag	-	开始的位置，这个位置的时间可能还没有被计算出来。
//		nRow, nEndFrag	-	结束的位置，这个位置的时间可能还没有被计算出来。
BOOL CMLData::CalFragTimeBetweenFrags(int nRow, int nBegFrag, int nEndFrag, int nBegTime, int nEndTime)
{
	int			nFragCount = 0;
	PML_ROW		pRow = NULL;

	nFragCount = nEndFrag - nBegFrag + 1;
	assert(nFragCount > 0);

	pRow = GetLyricsRow(nRow);
	assert(pRow);
	assert((unsigned int)nEndFrag < pRow->vFrags.size());

	DWORD		dwAttrBegTime, dwAttrEndTime;

	// 保存开始和结束位置的设置：开始和结束位置的时间标志可能不是TEMP类型的。
	dwAttrBegTime = pRow->vFrags[nBegFrag]->dwAttrib;
	if (pRow->vFrags[nBegFrag]->nBegTime == -1)
		dwAttrBegTime |= LSA_TEMP_TIME_BEG;
	dwAttrEndTime = pRow->vFrags[nEndFrag]->dwAttrib;
	if (pRow->vFrags[nEndFrag]->nEndTime == -1)
		dwAttrEndTime |= LSA_TEMP_TIME_END;

	for (int m = nBegFrag; m <= nEndFrag; m++)
	{
		pRow->vFrags[m]->nBegTime = nBegTime
			+ ((nEndTime - nBegTime) * (m - nBegFrag)) / nFragCount;
		pRow->vFrags[m]->nEndTime = nBegTime + 
			((nEndTime - nBegTime) * (m - nBegFrag + 1)) / nFragCount;
		pRow->vFrags[m]->dwAttrib |= LSA_TEMP_TIME_BEG | LSA_TEMP_TIME_END;
	}

	pRow->vFrags[nBegFrag]->dwAttrib = dwAttrBegTime;
	pRow->vFrags[nEndFrag]->dwAttrib = dwAttrEndTime;

	return TRUE;
}

//	COMMENT:
//		计算两个歌词行之间的所有歌词行的开始时间和结束时间的值
//	INPUT:
//		nBegRow, nBegFrag	-	开始的位置，这个位置的时间可能还没有被计算出来。
//		nEndRow, nEndFrag	-	结束的位置，这个位置的时间可能还没有被计算出来。
BOOL CMLData::CalFragTimeBetweenRows(int nBegRow, int nBegFrag, int nEndRow, int nEndFrag, int nBegTime, int nEndTime)
{
	PML_ROW		pRow = NULL;

	assert(nEndTime >= nBegTime);
	assert(nEndRow >= nBegRow);
	assert(nBegRow < GetLyricRowCount());
	assert(nEndRow < GetLyricRowCount());

	if (nBegRow >= GetLyricRowCount())
		return FALSE;
	if (nEndRow >= GetLyricRowCount())
		return FALSE;

	// 如果所有的“歌词段”在一行内，则每一段的时间采用平均值
	if (nBegRow == nEndRow)
	{
		pRow = GetLyricsRow(nBegRow);
		assert(pRow);

		CalFragTimeBetweenFrags(nBegRow, nBegFrag, nEndFrag, nBegTime, nEndTime);

//		if (pRow->nBegTime == -1)
		pRow->nBegTime = pRow->vFrags[0]->nBegTime;
//		if (pRow->nEndTime == -1)
		pRow->nEndTime = pRow->vFrags[pRow->vFrags.size() - 1]->nEndTime;
	}
	else
	{
		// 计算最开始的行
		pRow = GetLyricsRow(nBegRow);
		assert(pRow);
		
//		if (pRow->nEndTime == -1)
		pRow->nEndTime = nBegTime +
			(nEndTime - nBegTime) / (nEndRow - nBegRow + 1);

		CalFragTimeBetweenFrags(nBegRow, nBegFrag, pRow->vFrags.size() - 1, nBegTime, pRow->nEndTime);

//		if (pRow->nBegTime == -1)
		pRow->nBegTime = pRow->vFrags[0]->nBegTime;

		// “歌词段”不在一行内，每一行的时间采用平均值；在一行内再采用平均值
		for (int i = nBegRow + 1; i < nEndRow; i++)
		{
			pRow = GetLyricsRow(i);
			assert(pRow);

			pRow->nBegTime = nBegTime +
				(nEndTime - nBegTime) * (i - nBegRow) / (nEndRow - nBegRow + 1);
			pRow->nEndTime = nBegTime +
				(nEndTime - nBegTime) * (i - nBegRow + 1) / (nEndRow - nBegRow + 1);

			CalFragTimeBetweenFrags(i, 0, pRow->vFrags.size() - 1, pRow->nBegTime, pRow->nEndTime);
		}

		// 计算结束歌词行的时间
		pRow = GetLyricsRow(nEndRow);
		assert(pRow);
		pRow->nBegTime = nBegTime +
			(nEndTime - nBegTime) * (nEndRow - nBegRow) / (nEndRow - nBegRow + 1);

		CalFragTimeBetweenFrags(nEndRow, 0, nEndFrag, pRow->nBegTime, nEndTime);

//		if (pRow->nEndTime == -1)
		pRow->nEndTime = pRow->vFrags[pRow->vFrags.size() - 1]->nEndTime;
	}
	
	return TRUE;
}

//	COMMENT:
//		取得上一个“歌词段”
BOOL CMLData::PrevFrag(int nRow, int nFrag, int &nPrevRow, int &nPrevFrag, PLYRIC_FRAG *ppFrag)
{
	assert(nRow < GetLyricRowCount());
	assert(nFrag >= 0);
	assert((unsigned int)nFrag < GetLyricsRow(nRow)->vFrags.size());

	if (nFrag <= 0)
	{
		if (nRow <= 0)
		{
			nPrevRow = 0;
			nPrevFrag = 0;
			if (ppFrag)
				*ppFrag = NULL;
			return FALSE;
		}

		nPrevRow = nRow - 1;
		nPrevFrag = GetLyricsRow(nPrevRow)->vFrags.size() - 1;
	}
	else
	{
		nPrevFrag--;
		nPrevRow = nRow;
	}

	if (ppFrag)
		*ppFrag = GetLyricsRow(nPrevRow)->vFrags[nPrevFrag];

	return TRUE;
}

//	COMMENT:
//		取得上一个“歌词段”
PLYRIC_FRAG CMLData::PrevFrag(int nRow, int nFrag)
{
	assert(nRow < GetLyricRowCount());
	assert(nFrag >= 0);
	assert((unsigned int)nFrag < GetLyricsRow(nRow)->vFrags.size());

	PML_ROW		pRow;

	if (nFrag <= 0)
		nRow--;

	pRow = GetLyricsRow(nRow);
	if (!pRow || pRow->vFrags.size() == 0)
		return NULL;

	if (nFrag <= 0)
		nFrag = pRow->vFrags.size() - 1;

	return pRow->vFrags[nFrag];
}

//	COMMENT:
//		取得下一个“歌词段”
BOOL CMLData::NextFrag(int nRow, int nFrag, int &nNextRow, int &nNextFrag, PLYRIC_FRAG *ppFrag)
{
	assert(nRow >= 0 && nRow < GetLyricRowCount());
	assert(nFrag >= 0);
	assert((unsigned int)nFrag < GetLyricsRow(nRow)->vFrags.size());

	PML_ROW	pRow;
	nNextRow = nRow;
	pRow = GetLyricsRow(nNextRow);
	if (!pRow)
		return FALSE;

	nNextFrag = nFrag + 1;
	if ((unsigned int)nNextFrag >= pRow->vFrags.size())
	{
		nNextRow = nRow + 1;
		pRow = GetLyricsRow(nNextRow);
		if (!pRow || pRow->vFrags.size() <= 0)
		{
			nNextRow = 0;
			nNextFrag = 0;
			if (ppFrag)
				*ppFrag = NULL;
			return FALSE;
		}
		nNextFrag = 0;
	}

	if (ppFrag)
		*ppFrag = GetLyricsRow(nNextRow)->vFrags[nNextFrag];

	return TRUE;
}

//	COMMENT:
//		取得下一个“歌词段”
PLYRIC_FRAG CMLData::NextFrag(int nRow, int nFrag)
{
	assert(nRow < GetLyricRowCount());
	assert(nFrag >= 0);
	assert((unsigned int)nFrag < GetLyricsRow(nRow)->vFrags.size());

	nFrag ++;
	if ((unsigned int)nFrag >= GetLyricsRow(nRow)->vFrags.size())
	{
		nRow++;
		if (nRow >= GetLyricRowCount())
		{
			return NULL;
		}
		nFrag = 0;
	}

	return GetLyricsRow(nRow)->vFrags[nFrag];
}

bool CMLData::RemoveLyricsRow(int nRow)
{
	PML_ROW		pRow;

	pRow = GetLyricsRow(nRow);
	if (!pRow)
		return false;

	int			k;

	m_arrLyricsRows.erase(m_arrLyricsRows.begin() + nRow);
	for (k = 0; k < (int)m_arrFileRows.size(); k++)
	{
		if (m_arrFileRows[k] == pRow)
		{
			m_arrFileRows.erase(m_arrFileRows.begin() + k);
			break;
		}
	}

	delete pRow;

	return true;
}

void CMLData::AddTextInLyrics(V_Str &vText, DWORD dwLyrFlagAttr)
{
	int				nPrevEnd;
	PLYRIC_FRAG		pFrag;
	PML_ROW			pRow;

	if (m_arrLyricsRows.size())
		nPrevEnd = m_arrLyricsRows.back()->nEndTime;
	else
		nPrevEnd = GetPlayElapsedTime();

	for (int i = 0; i < (int)vText.size(); i++)
	{
		int		nEndTime;

		nEndTime = nPrevEnd + 8 * 1000;
		pFrag = NewLyricFrag(nPrevEnd, nEndTime, vText[i].c_str(), vText[i].size(), TRUE, TRUE, LSA_SHOW_SIMPLE | LSA_TEMP_FRAG);
		pFrag->dwAttrib |= dwLyrFlagAttr;
		nPrevEnd = nEndTime;

		pRow = NewMLRow(pFrag->nBegTime, pFrag->nEndTime);
		pRow->vFrags.push_back(pFrag);

		AddRowByTime(pRow);
	}
}

void CMLData::AddAdInLyrics(int nBegTimeOffset, LPCTSTR szText, LPCTSTR szLink)
{
	int			nBegTime;

	// if ad exists, don't add new.
	for (int i = 0; i < (int)m_arrLyricsRows.size(); i++)
	{
		PML_ROW		pRow = m_arrLyricsRows[i];
		if (pRow->IsAdRow() && pRow->szContent && _tcscmp(pRow->szContent, szLink) == 0)
			return;
	}

	nBegTime = nBegTimeOffset;

	if (nBegTime < 0)
		nBegTime = 0;

	PML_ROW		pRow;
	PLYRIC_FRAG	pFrag;

	// add ad at beginning of the lyrics
	pRow = NewMLRow(nBegTime, -1, szLink, -1);
	pFrag = NewLyricFrag(nBegTime, -1, szText, -1, TRUE, TRUE, LSA_SHOW_DEFAULT | LSA_AD_FRAG | LSA_TEMP_FRAG);
	pRow->vFrags.push_back(pFrag);
	AddRowByTime(pRow);

	// add ad at the end of the lyrics.
	pRow = GetLyricsRow(GetLyricRowCount() - 1);
	if (pRow)
	{
		nBegTime = pRow->nEndTime;
		if (nBegTime != -1)
			nBegTime = pRow->nBegTime + 5 * 1000;
		pRow = NewMLRow(nBegTime, -1, szLink, -1);
		pFrag = NewLyricFrag(nBegTime, -1, szText, -1, TRUE, TRUE, LSA_SHOW_DEFAULT | LSA_AD_FRAG | LSA_TEMP_FRAG);
		pRow->vFrags.push_back(pFrag);
		AddRow(pRow);
	}

	CheckLyricsRowTime();
}

BOOL CMLData::LyricsRowToText(PML_ROW pRow, tstring &strLyricsRow)
{
	strLyricsRow.resize(0);
	if (!pRow)
		return FALSE;

	switch (pRow->nRowType)
	{
	case ML_ROW_UNKNOW:
		if (pRow->szContent)
			strLyricsRow = pRow->szContent;
		break;
	case ML_ROW_LYRICS:
		{
			int		nCount;
			nCount = pRow->vFrags.size();
			for (int i = 0; i < nCount; i++)
			{
				strLyricsRow += pRow->vFrags[i]->szLyric;
			}
		}
		break;
	default:
		return FALSE;
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
	for (UINT i = 0; i < m_arrLyricsRows.size(); i++)
	{
		for (UINT k = 0; k < m_arrLyricsRows[i]->vFrags.size(); k++)
		{
			LPTSTR		szBeg = m_arrLyricsRows[i]->vFrags[k]->szLyric;

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
			m_arrLyricsRows[i]->vFrags[k]->nLen = pos;
		}
	}

	return 0;
}

#ifdef _ID3V2_SUPPORT
bool CMLData::ToID3v2SynchLyrics(ID3v2SynchLyrics &lyrics, CHAR_ENCODING encoding)
{
	strcpy_safe(lyrics.m_szLanguage, CountOf(lyrics.m_szLanguage), m_id3v2SaveCacheInfo.m_strLanguage.c_str());
	lyrics.m_bTimeStampMs = true;

	CLrcTag			lrcTag(m_lyrPropertiesOrg, m_lyrProperties);

	lrcTag.WriteToText(m_id3v2SaveCacheInfo.m_strLrcDescription, true);

	lyrics.m_encodingType = CharEncodingToID3v2EncType(encoding);
	lyrics.m_strContentDesc = m_id3v2SaveCacheInfo.m_strLrcDescription.c_str();

	// convert lyrics rows
	for (int i = 0; i < (int)m_arrLyricsRows.size(); i++)
	{
		PML_ROW		pRow = m_arrLyricsRows[i];

		if (pRow->nRowType == ML_ROW_LYRICS && !pRow->IsTempRow())
		{
			//
			// to compatible with windows media player, the Karaoke info has to be lost.
			//
			LrcSyllable	syllable;
			syllable.bNewLine = false;
			syllable.nTime = pRow->nBegTime - m_lyrProperties.GetOffsetTime();
			if (syllable.nTime < 0)
				syllable.nTime = 0;
			for (int k = 0; k < (int)pRow->vFrags.size(); k++)
			{
				PLYRIC_FRAG				pFrag = pRow->vFrags[k];

				syllable.strText += pFrag->szLyric;
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

	return true;
}

bool CMLData::ToID3v2UnsynchLyrics(ID3v2UnsynchLyrics &lyrics, CHAR_ENCODING encoding)
{
	tstring		str;

	ToString(str, FT_LYRICS_TXT, TRUE);

	strcpy_safe(lyrics.m_szLanguage, CountOf(lyrics.m_szLanguage), m_id3v2SaveCacheInfo.m_strLanguage.c_str());

	lyrics.m_encodingType = CharEncodingToID3v2EncType(encoding);
	lyrics.m_strContentDesc = m_id3v2SaveCacheInfo.m_strLrcDescription.c_str();
	lyrics.m_strLyrics = str.c_str();

	return true;
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

PLYRIC_FRAG CMLData::NewLyricFrag(int nBegTime, int nEndTime, 
	const char *szLyrics, int n, BOOL bTempBegTime, BOOL bTempEndTime, DWORD dwStyle)
{
	PLYRIC_FRAG	pFrag;
	tstringex	str;

	ConvertStr(szLyrics, n, str, GetEncodingIndex());

	pFrag = (PLYRIC_FRAG)new BYTE[sizeof(LYRIC_FRAG) + str.size() * sizeof(TCHAR)];

	pFrag->nBegTime = nBegTime;
	pFrag->nEndTime = nEndTime;
	pFrag->nDrawWidth = 0;
	_tcscpy_safe(pFrag->szLyric, str.size() + 1, str.c_str());
	pFrag->nLen = str.size();
	pFrag->dwAttrib = dwStyle;
	if (bTempBegTime)
		pFrag->dwAttrib |= LSA_TEMP_TIME_BEG;
	if (bTempEndTime)
		pFrag->dwAttrib |= LSA_TEMP_TIME_END;

	return pFrag;
}

PLYRIC_FRAG CMLData::NewLyricFrag(int nBegTime, int nEndTime, 
	const WCHAR *szLyrics, int n, BOOL bTempBegTime, BOOL bTempEndTime, DWORD dwStyle)
{
	PLYRIC_FRAG	pFrag;
	tstringex	str;

	ConvertStr(szLyrics, n, str, GetEncodingIndex());

	pFrag = (PLYRIC_FRAG)new BYTE[sizeof(LYRIC_FRAG) + str.size() * sizeof(TCHAR)];

	pFrag->nBegTime = nBegTime;
	pFrag->nEndTime = nEndTime;
	pFrag->nDrawWidth = 0;
	_tcscpy_safe(pFrag->szLyric, str.size() + 1, str.c_str());
	pFrag->nLen = str.size();
	pFrag->dwAttrib = dwStyle;
	if (bTempBegTime)
		pFrag->dwAttrib |= LSA_TEMP_TIME_BEG;
	if (bTempEndTime)
		pFrag->dwAttrib |= LSA_TEMP_TIME_END;

	return pFrag;
}

PML_ROW CMLData::NewMLRow(int nBegTime, int nEndTime, 
		LPCTSTR szContent, int nLen, ML_ROW_TYPE rowType)
{
	PML_ROW		pRow;

	pRow = new ML_ROW;
	pRow->nBegTime = nBegTime;
	pRow->nEndTime = nEndTime;
	pRow->nRowType = rowType;

	if (szContent)
		pRow->SetContent(szContent, nLen);

	return pRow;
}

PML_ROW CMLData::CopyMLRow(PML_ROW pRowSrc)
{
	PML_ROW		pRow;
	PLYRIC_FRAG	pFrag, pFragSrc;

	pRow = new ML_ROW;
	pRow->nBegTime = pRowSrc->nBegTime;
	pRow->nEndTime = pRowSrc->nEndTime;
	pRow->nRowType = pRowSrc->nRowType;

	if (pRowSrc->szContent)
	{
		int		n = _tcslen(pRowSrc->szContent) + 1;
		pRow->szContent = new TCHAR[n];
		_tcscpy_safe(pRow->szContent, n, pRowSrc->szContent);
	}

	for (int i = 0; i < (int)pRowSrc->vFrags.size(); i++)
	{
		pFragSrc = pRowSrc->vFrags[i];
		
		pFrag = (PLYRIC_FRAG)new BYTE[sizeof(LYRIC_FRAG) + pFragSrc->nLen * sizeof(TCHAR)];

		pFrag->nBegTime = pFragSrc->nBegTime;
		pFrag->nEndTime = pFragSrc->nEndTime;
		pFrag->nDrawWidth = pFragSrc->nDrawWidth;
		_tcscpy_safe(pFrag->szLyric, pFragSrc->nLen + 1, pFragSrc->szLyric);
		pFrag->nLen = pFragSrc->nLen;
		pFrag->dwAttrib = pFragSrc->dwAttrib;

		pRow->vFrags.push_back(pFrag);
	}

	return pRow;
}

void CMLData::CopyMLyrLines(CMLyrLines &lyrLines)
{
	int			i;
	PML_ROW		pRow;

	lyrLines.clear();

	for (i = 0; i < (int)m_arrLyricsRows.size(); i++)
	{
		PML_ROW		pRowSrc = m_arrLyricsRows[i];

		pRow = CopyMLRow(pRowSrc);
		lyrLines.push_back(pRow);
	}
}

#ifdef _CPPUNIT_TEST

//////////////////////////////////////////////////////////////////////////
// CPPUnit test

class CTestCaseCMLData : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(CTestCaseCMLData);
	CPPUNIT_TEST(Test_CMLData_OpenLrcFile);
	CPPUNIT_TEST(Test_CMLData_OpenSaveID3v2_USLT);
	CPPUNIT_TEST(Test_CMLData_OpenSaveID3v2_SYLT);
	CPPUNIT_TEST(Test_CMLData_OpenSaveLyrics3v2);
	CPPUNIT_TEST_SUITE_END();

protected:
	tstring			m_strTestDataFolder;

public:
	void setUp()
	{
		FileGetPath(_T(__FILE__), m_strTestDataFolder);
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
		};

		for (int i = 0; i < (int)CountOf(vstrSongFile); i++)
		{
			tstring	strFile = GetWorkingFolder();
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
		};

		for (int i = 0; i < (int)CountOf(vstrSongFile); i++)
		{
			tstring	strFile = GetWorkingFolder();
			strFile += FileGetName(vstrSongFile[i].c_str());
			DeleteFile(strFile.c_str());
		}
	}

	void Test_CMLData_OpenSaveID3v2_USLT()
	{
		int			nRet;
		CMLData		data;

		PrepareMediaFile();

		tstring		strWorkingFolder = GetWorkingFolder();

		//
		// Test on embedded USLT lyrics.
		//

		tstring		vstrSongFile[] = {
			strWorkingFolder + _T("lyr-usly.mp3"),
			strWorkingFolder + _T("lyr(wmp-chs).mp3"),
			strWorkingFolder + _T("lyr-usly(chs).mp3"),
			strWorkingFolder + _T("lyr(wmp-chs).mp3"),
		};
		LPCTSTR		vLyrFile[] = {
			_T("song://id3v2/uslt/eng"),
			_T("song://id3v2/uslt/eng"),
			_T("song://id3v2/uslt/eng"),
			_T("song://id3v2/uslt/chi"),
		};
		bool		vWriteTested[] = { false, false, false, false };

		for (int kk = 0; kk < 2; kk++)
		{
			// Loop twice, first for reading, second for writing.

			for (int i = 0; i < CountOf(vstrSongFile); i++)
			{
				nRet = data.OpenLyrics(vstrSongFile[i].c_str(), 60 * 1000, vLyrFile[i]);
				CPPUNIT_ASSERT(nRet == ERR_OK);

				{
					//
					// Open, Lyrics content ok?
					//
					LPCTSTR		vLyrLines[] = { _T("Line1"), _T("Line2"), _T("Line3") };
					LPCTSTR		vLyrLinesChs[] = { _T("第一行"), _T("第二行"), _T("第三行") };

					VLyrLines	&lyrLines = data.GetRawLyrics();

					for (int k = 0; k < lyrLines.size(); k++)
					{
						PML_ROW	pRow = lyrLines[k];

						CPPUNIT_ASSERT(pRow->vFrags.size() == 1);
						CPPUNIT_ASSERT(k < CountOf(vLyrLines));
						if (i < 2)
							CPPUNIT_ASSERT(_tcscmp(pRow->vFrags[0]->szLyric, vLyrLines[k]) == 0);
						else
							CPPUNIT_ASSERT(_tcscmp(pRow->vFrags[0]->szLyric, vLyrLinesChs[k]) == 0);
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

		tstring		strWorkingFolder = GetWorkingFolder();

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

					VLyrLines	&lyrLines = data.GetRawLyrics();

					for (int k = 0; k < lyrLines.size(); k++)
					{
						PML_ROW	pRow = lyrLines[k];

						CPPUNIT_ASSERT(pRow->vFrags.size() == 1);
						CPPUNIT_ASSERT(k < CountOf(vLyrLines));
						if (i < 1)
							CPPUNIT_ASSERT(_tcscmp(pRow->vFrags[0]->szLyric, vLyrLines[k]) == 0);
						else
							CPPUNIT_ASSERT(_tcscmp(pRow->vFrags[0]->szLyric, vLyrLinesChs[k]) == 0);

						CPPUNIT_ASSERT(pRow->vFrags[0]->nBegTime == vTime[k] + nOffsetTime);
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

		tstring		strWorkingFolder = GetWorkingFolder();

		tstring		vstrSongFileSylt[] = {
			strWorkingFolder + _T("lyrics3v2.mp3"),
		};
		LPCTSTR		vLyrFileSylt[] = {
			_T("song://lyrics3v2"),
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

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCMLData);

#endif // _CPPUNIT_TEST
