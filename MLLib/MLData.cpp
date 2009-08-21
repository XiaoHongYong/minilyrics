/********************************************************************
	Created  :	2002年1月3日 17:54:54
	FileName :	MLData.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "StdAfx.h"
#include "MLLib.h"
#include "MLData.h"
#include "LyricsParser.h"
#include "LyricsSearch.h"
#include "HelperFun.h"
#include "LrcParser.h"
#include "TextFileParser.h"
#include "SncParser.h"
#include "SrtParser.h"
#include <algorithm>
#include "LyricsTagReader.h"

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

		szTemp = SZ_SONG_ID3V2_LYRICS + SONG_HDR_LEN;
		if (_tcsnicmp(szLrcSource + SONG_HDR_LEN, szTemp, _tcslen(szTemp)) == 0)
			return LST_ID3V2_LYRICS;

		szTemp = SZ_SONG_ID3V2_SYLT + SONG_HDR_LEN;
		if (_tcsnicmp(szLrcSource + SONG_HDR_LEN, szTemp, _tcslen(szTemp)) == 0)
			return LST_ID3V2_SYLT;

		szTemp = SZ_SONG_ID3V2_USLT + SONG_HDR_LEN;
		if (_tcsnicmp(szLrcSource + SONG_HDR_LEN, szTemp, _tcslen(szTemp)) == 0)
			return LST_ID3V2_USLT;

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
			if ((*szPtr) > 255 || !(isalpha(*szPtr) || isdigit(*szPtr) || *szPtr == '_' || *szPtr == '-'))
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

void LyricsProperties::SetEncoding(CHAR_ENCODING nEncodingID)
{
	tstringex		str;

	ConvertStr(GetCharEncodingByID(nEncodingID).szEncoding, -1, str);

	m_strEncoding = str.c_str();
	m_nEncodingID = nEncodingID;
}

void LyricsProperties::SetEncoding(LPCTSTR szEncoding)
{
	m_strEncoding = szEncoding;

	m_nEncodingID = GetCharEncodingID(szEncoding);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMLData::ID3v2LyricsSaveCacheInfo::ID3v2LyricsSaveCacheInfo()
{
	clear();
}

void CMLData::ID3v2LyricsSaveCacheInfo::clear()
{
	m_strLanguage = "eng";
	m_strLrcDescription = _T("Lyrics was Saved by MiniLyrics.");
	m_bAllSyllableIsNewLine = true;
}

CMLData::CMLData()
{
	EmptyStr(m_szLyrContentMd5Org);
	MD5Str((const char *)_T(""), 0,
		m_szLyrContentMd5Org);

	m_fileEncoding = FE_ANSI;

	m_dwDefShowStyle = LSA_SHOW_DEFAULT;
	m_lyrConentTypeOrg = m_lyrConentType = LCT_UNKNOWN;
	m_LyricFileType = FT_UNKNOWN;

	m_bHasLyrics = FALSE;

	m_lrcSourceType = LST_FILE;

	m_nMediaLength = 0;
	m_nTimeElapsed = 0;
}

CMLData::~CMLData()
{
	Close();
}

int CMLData::OpenLyrics(LPCTSTR szSongFile, int nMediaLength, LPCTSTR szLrcSource, bool bForceUseEncoding, CHAR_ENCODING nForceEncodingID)
{
	int				nRet = ERR_OK;
	LRC_SOURCE_TYPE lrcSourceType;

	m_nMediaLength = nMediaLength;

	lrcSourceType = GetLrcSourceType(szLrcSource);

	if (lrcSourceType == LST_LYRICS3V2)
	{
		stringex	strLyrics;
		CLyrics3v2		lyrics3v2;

		nRet = lyrics3v2.ReadLyrcsInfo(szSongFile, strLyrics);
		if (nRet == ERR_OK)
			nRet = OpenLyricsInSongOfLyrics3v2(strLyrics.c_str(), strLyrics.size(), bForceUseEncoding, nForceEncodingID);

		for (int i = 0; i < (int)m_arrLyricsRows.size(); i++)
		{
			tstring		str;
			LyricsRowToText(m_arrLyricsRows[i], str);
		}
	}
	else if (lrcSourceType == LST_ID3V2_SYLT || lrcSourceType == LST_ID3V2_USLT || lrcSourceType == LST_ID3V2_LYRICS)
	{
		CID3v2IF		id3v2;

		nRet = id3v2.Open(szSongFile, false, false);
		if (nRet == ERR_OK)
		{
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
						nRet = OpenLyricsInSongOfID3v2SYLT(*plyrics, bForceUseEncoding, nForceEncodingID);
					}
					else
						nRet = ERR_NOT_FOUND;
				}
				else if (lrcSourceType == LST_ID3V2_USLT)
				{
					ID3v2UnsynchLyrics	*plyrics;
					if (ID3v2LyricsInfoGetUnsynchLyrics(lyricsInfo, szLrcSource, plyrics))
					{
						nRet = OpenLyricsInSongOfID3v2USLT(*plyrics, bForceUseEncoding, nForceEncodingID);
					}
					else
						nRet = ERR_NOT_FOUND;
				}
				else if (lrcSourceType == LST_ID3V2_LYRICS)
				{
					Close();

					if (lyricsInfo.m_lyricsUserDefined.m_strValueAnsi.empty()
						&& lyricsInfo.m_lyricsUserDefined.m_strValueUCS2.empty())
					{
						SetCustomErrorDesc(_T("The embedded lyrics is empty, please click the \"search lyrics\" button to get lyrics."));
						nRet = ERR_CUSTOM_ERROR;
					}
					else
					{
						m_lrcSourceType = LST_ID3V2_LYRICS;
						m_LyricFileType = FT_LYRICS_LRC;

						if (lyricsInfo.m_lyricsUserDefined.m_EncodingType == IET_ANSI)
						{
							nRet = FromString(lyricsInfo.m_lyricsUserDefined.m_strValueAnsi.c_str(),
								lyricsInfo.m_lyricsUserDefined.m_strValueAnsi.size() + 1,
								&m_fileEncoding, FT_LYRICS_LRC, TRUE, FALSE,
								bForceUseEncoding, nForceEncodingID);
						}
						else
						{
							nRet = FromString(lyricsInfo.m_lyricsUserDefined.m_strValueUCS2.c_str(),
								(lyricsInfo.m_lyricsUserDefined.m_strValueUCS2.size() + 1) * sizeof(wchar_t),
								&m_fileEncoding, FT_LYRICS_LRC, TRUE, TRUE,
								bForceUseEncoding, nForceEncodingID);
						}
					}
				}
			}
			id3v2.Close();
		}
	}
	else
		nRet = OpenLyricsFile(szLrcSource, bForceUseEncoding, nForceEncodingID);

	if (nRet == ERR_OK)
		m_bHasLyrics = TRUE;

	m_strSongFile = szSongFile;
	m_strLrcSource = szLrcSource;
	m_lrcSourceType = lrcSourceType;

	FilterContents();

	ResetModificationFlag();

	return nRet;
}

//
// 重新打开歌词
//
int CMLData::ReopenLyrics(bool bForceUseEncoding, CHAR_ENCODING nForceEncodingID)
{
	tstring		strLrcSource, strSongFile;

	strLrcSource = m_strLrcSource;
	strSongFile = m_strSongFile;

	return OpenLyrics(strSongFile.c_str(), m_nMediaLength, strLrcSource.c_str(), bForceUseEncoding, nForceEncodingID);
}

// COMMENT:
//		调用此函数打开各种歌词文件.
//		实现采用对应的歌词文件类来打开不同类型的歌词文件
// INPUT:
//		歌词文件名, 支持各种歌词文件格式: ML/FL/LRC/TXT/HTML
int CMLData::OpenLyricsFile(LPCTSTR szFile, bool bForceUseEncoding, CHAR_ENCODING nForceEncodingID)
{
	// 基类接口
	CLyricsParser	*pParser;
	int				nRet;

	// 清除前一首歌词
	Close();

	// 根据文件的扩展名来判断文件类型，然后调用不同的歌词分析类来分析歌词。
	if (FileIsExtSame(szFile, _T(".txt")))
	{
		pParser = new CLrcParser(this);
		m_LyricFileType = FT_LYRICS_TXT;
	}
	else if (FileIsExtSame(szFile, _T(".lrc")))
	{
		pParser = new CLrcParser(this);
		m_LyricFileType = FT_LYRICS_LRC;
	}
	else if (FileIsExtSame(szFile, _T(".srt")))
	{
		pParser = new CSrtParser(this);
		m_LyricFileType = FT_SUBTITLE_SRT;
	}
	else
	{
		return ERR_NOT_SUPPORT_FILE_FORMAT;
	}

	// 保存歌词文件名
	// strcpy(m_szFile, szFile);
	m_strLrcSource = szFile;

	if (bForceUseEncoding)
	{
		// 设置缺省的编码
		SetEncoding(nForceEncodingID);
	}

	// 分析歌词
	nRet = pParser->Parse(bForceUseEncoding);

	m_fileEncoding = pParser->GetFileEncoding();

	// 解析歌词错误
	if (nRet == ERR_OK)
		m_lyrConentType = pParser->GetLyrContentType();
	else
	{
		if (bForceUseEncoding)
		{
			// 设置缺省的编码
			SetEncoding(nForceEncodingID);
		}

		CLrcParser				parser(this);
		nRet = parser.Parse(bForceUseEncoding);
		m_lyrConentType = parser.GetLyrContentType();

		m_fileEncoding = parser.GetFileEncoding();
	}

	delete pParser;

	FilterContents();

	return nRet;
}

int CMLData::OpenLyricsInSongOfID3v2USLT(ID3v2UnsynchLyrics &lyrics, bool bForceUseEncoding, CHAR_ENCODING nForceEncodingID)
{
	int		nRet = ERR_OK;

	Close();

	m_id3v2SaveCacheInfo.m_strLanguage = lyrics.m_szLanguage;
	if (lyrics.m_encodingType != IET_ANSI)
	{
		m_fileEncoding = FE_UCS2;

//		ConvertStr(lyrics.m_strUcs2ContentDesc.c_str(), lyrics.m_strUcs2ContentDesc.size(),
//			m_id3v2SaveCacheInfo.m_strLrcDescription, nForceEncodingID);
#ifdef UNICODE
		m_id3v2SaveCacheInfo.m_strLrcDescription = lyrics.m_strUcs2ContentDesc.c_str();
#else
		ConvertStr(lyrics.m_strUcs2ContentDesc.c_str(), lyrics.m_strUcs2ContentDesc.size(),
			m_id3v2SaveCacheInfo.m_strLrcDescription, GetEncodingIndex());
#endif

		nRet = FromString(lyrics.m_strUcs2Lyrics.c_str(), lyrics.m_strUcs2Lyrics.size(),
			NULL, FT_LYRICS_LRC, TRUE, TRUE, FALSE, ED_SYSDEF);
		if (nRet != ERR_OK)
			return nRet;
	}
	else
	{
		if (bForceUseEncoding)
			SetEncoding(nForceEncodingID);
		m_fileEncoding = FE_ANSI;

		nRet = FromString(lyrics.m_strAnsiLyrics.c_str(), lyrics.m_strAnsiLyrics.size(),
			NULL, FT_LYRICS_LRC, TRUE, FALSE, bForceUseEncoding, nForceEncodingID);
		if (nRet != ERR_OK)
			return nRet;

#ifdef UNICODE
		ConvertStr(lyrics.m_strAnsiContentDesc.c_str(), lyrics.m_strAnsiContentDesc.size(),
			m_id3v2SaveCacheInfo.m_strLrcDescription, GetEncodingIndex());
#else
		m_id3v2SaveCacheInfo.m_strLrcDescription = lyrics.m_strAnsiContentDesc.c_str();
#endif
	}

	m_LyricFileType = FT_LYRICS_TXT;

	return ERR_OK;
}

int CMLData::OpenLyricsInSongOfID3v2SYLT(ID3v2SynchLyrics &lyrics, bool bForceUseEncoding, CHAR_ENCODING nForceEncodingID)
{
	PML_ROW			pRow;
	PLYRIC_FRAG		pFrag;
	CLyricsTagReader<TCHAR>		reader;
	tstringex		strTitle, strArtist, strAlbum, strBy;

	Close();

	reader.Set(&strTitle, &strArtist, &strAlbum, &strBy);

	m_id3v2SaveCacheInfo.m_strLanguage = lyrics.m_szLanguage;
	m_id3v2SaveCacheInfo.m_bAllSyllableIsNewLine = lyrics.m_bAllSyllableIsNewLine;
	if (lyrics.m_encodingType != IET_ANSI)
	{
		m_fileEncoding = FE_UCS2;

		// Read id3v2 uslt lyrics information
		reader.ReadFromStr((const char *)lyrics.m_strUcs2ContentDesc.c_str(), 
			lyrics.m_strUcs2ContentDesc.size() * sizeof(WCHAR),
			true, bForceUseEncoding, nForceEncodingID);

		if (reader.m_strEncoding.size())
			SetEncoding(reader.m_strEncoding.c_str());

#ifdef UNICODE
		m_id3v2SaveCacheInfo.m_strLrcDescription = lyrics.m_strUcs2ContentDesc.c_str();
#else
		ConvertStr(lyrics.m_strUcs2ContentDesc.c_str(), lyrics.m_strUcs2ContentDesc.size(),
			m_id3v2SaveCacheInfo.m_strLrcDescription, GetEncodingIndex());
#endif

		pRow = NULL;
		for (ID3v2SynchLyrics::LIST_SYLRC_W::iterator it = lyrics.m_vUcs2SynLyrics.begin();
			it != lyrics.m_vUcs2SynLyrics.end(); it++)
		{
			LrcSyllable<wchar_t>		&syllable = *it;

			if (syllable.bNewLine)
			{
				pRow = NULL;
				continue;
			}

			if (!pRow || lyrics.m_bAllSyllableIsNewLine)
			{
				pRow = NewMLRow(syllable.nTime, -1, (TCHAR *)NULL);
				AddRow(pRow);
			}

			pFrag = NewLyricFrag(syllable.nTime, -1, syllable.strText.c_str(), 
				syllable.strText.size(), false, true);
			pRow->vFrags.push_back(pFrag);
		}
	}
	else
	{
		m_fileEncoding = FE_ANSI;

		if (bForceUseEncoding)
			SetEncoding(nForceEncodingID);

		// Read id3v2 uslt lyrics information
		reader.ReadFromStr((const char *)lyrics.m_strAnsiContentDesc.c_str(), 
			lyrics.m_strAnsiContentDesc.size() * sizeof(char),
			false, bForceUseEncoding, nForceEncodingID);

		if (reader.m_strEncoding.size())
			SetEncoding(reader.m_strEncoding.c_str());

#ifdef UNICODE
		ConvertStr(lyrics.m_strAnsiContentDesc.c_str(), lyrics.m_strAnsiContentDesc.size(),
			m_id3v2SaveCacheInfo.m_strLrcDescription, GetEncodingIndex());
#else
		m_id3v2SaveCacheInfo.m_strLrcDescription = lyrics.m_strAnsiContentDesc.c_str();
#endif

		pRow = NULL;
		for (ID3v2SynchLyrics::LIST_SYLRC::iterator it = lyrics.m_vAnsiSynLyrics.begin();
			it != lyrics.m_vAnsiSynLyrics.end(); it++)
		{
			LrcSyllable<char>		&syllable = *it;

			if (syllable.bNewLine)
			{
				pRow = NULL;
				continue;
			}

			if (!pRow || lyrics.m_bAllSyllableIsNewLine)
			{
				pRow = NewMLRow(syllable.nTime, -1, (TCHAR *)NULL);
				AddRow(pRow);
			}

			pFrag = NewLyricFrag(syllable.nTime, -1, syllable.strText.c_str(), 
				syllable.strText.size(), false, true);
			pRow->vFrags.push_back(pFrag);
		}
	}

	m_lyrProperties.m_strTitle = strTitle.c_str();
	m_lyrProperties.m_strArtist = strArtist.c_str();
	m_lyrProperties.m_strAlbum = strAlbum.c_str();
	m_lyrProperties.m_strBy = strBy.c_str();
	m_lyrProperties.m_nTimeOffset = 0;
	m_lyrConentType = LCT_LRC;

	CheckLyricsRowTime();

	return ERR_OK;
}

int CMLData::OpenLyricsInSongOfLyrics3v2(const char *buffer, int len, bool bForceUseEncoding, CHAR_ENCODING nForceEncodingID)
{
	int		nRet;

	Close();

	if (len == 0)
	{
		SetCustomErrorDesc(_T("The embedded lyrics is empty, please click the \"search lyrics\" button to get lyrics."));
		return ERR_CUSTOM_ERROR;
	}

	m_lrcSourceType = LST_LYRICS3V2;
	m_LyricFileType = FT_LYRICS_LRC;

	nRet = FromString(buffer, len, &m_fileEncoding, FT_LYRICS_LRC, TRUE, FALSE, bForceUseEncoding, nForceEncodingID);

	return nRet;
}

int CMLData::Save()
{
	if (!IsOK())
	{
		return ERR_FALSE;
	}

	if (!IsModified())
		return ERR_OK;

	if (m_lrcSourceType == LST_LYRICS3V2)
	{
		return SaveLyricsInSongOfLyrics3v2(m_fileEncoding, GetEncodingIndex());
	}
	else if (m_lrcSourceType == LST_ID3V2_SYLT)
	{
		return SaveLyricsInSongOfID3v2(m_fileEncoding, false, true, false, GetEncodingIndex());
	}
	else if (m_lrcSourceType == LST_ID3V2_USLT)
	{
		return SaveLyricsInSongOfID3v2(m_fileEncoding, true, false, false, GetEncodingIndex());
	}
	else if (m_lrcSourceType == LST_ID3V2_LYRICS)
	{
		return SaveLyricsInSongOfID3v2(m_fileEncoding, false, false, true, GetEncodingIndex());
	}
	else
	{
		if (IsContentModified())
		{
			tstring		strFile = m_strLrcSource;
			bool		bUseNewFileName;

			return SaveAsFile(strFile.c_str(), m_fileEncoding, bUseNewFileName);
		}
		else
		{
			if (SaveTag())
				return ERR_OK;
			else
				return ERR_FALSE;
		}
	}
}
// 
// int CMLData::SaveLyricsInSongOfID3v2USLT(FILE_ENCODING fileEncoding, CHAR_ENCODING nEncodingID)
// {
// 	CID3v2IF		id3v2;
// 	int				nRet;
// 
// 	nRet = id3v2.Open(m_strSongFile.c_str(), true, true);
// 	if (nRet == ERR_OK)
// 	{
// 		ID3v2UnsynchLyrics	lyrics;
// 		ToID3v2UnsynchLyrics(lyrics, fileEncoding, nEncodingID);
// 
// 		nRet = id3v2.UpdateUnsynchLyrics(&lyrics);
// 		if (nRet != ERR_OK)
// 			return nRet;
// 
// 		nRet = id3v2.Save();
// 		if (nRet == ERR_OK)
// 		{
// 			m_modifyFlag = MF_NONE;
// 			m_nTimeOffsetOld = m_nTimeOffset;
// 		}
// 	}
// 
// 	return nRet;
// }
// 
// int CMLData::SaveLyricsInSongOfID3v2SYLT(FILE_ENCODING fileEncoding, CHAR_ENCODING nEncodingID)
// {
// 	CID3v2IF		id3v2;
// 	int				nRet;
// 
// 	nRet = id3v2.Open(m_strSongFile.c_str(), true, true);
// 	if (nRet == ERR_OK)
// 	{
// 		ID3v2SynchLyrics	lyrics;
// 
// 		ToID3v2SynchLyrics(lyrics, fileEncoding, nEncodingID);
// 
// 		nRet = id3v2.UpdateSynchLyrics(&lyrics);
// 		if (nRet != ERR_OK)
// 			return nRet;
// 
// 		nRet = id3v2.Save();
// 		if (nRet == ERR_OK)
// 		{
// 			m_modifyFlag = MF_NONE;
// 			m_nTimeOffsetOld = m_nTimeOffset;
// 		}
// 	}
// 
// 	return nRet;
// }

int CMLData::SaveLyricsInSongOfID3v2(FILE_ENCODING fileEncoding, bool bUslt, bool bSylt, bool bUserDefinedLyr, CHAR_ENCODING nEncodingID)
{
	CID3v2IF		id3v2;
	int				nRet;

	if (!bUslt && !bSylt && !bUserDefinedLyr)
		return ERR_OK;

	nRet = id3v2.Open(m_strSongFile.c_str(), true, true);
	if (nRet != ERR_OK)
		return nRet;

	if (bUslt)
	{
		ID3v2UnsynchLyrics	lyrics;
		ToID3v2UnsynchLyrics(lyrics, fileEncoding, nEncodingID);

		nRet = id3v2.UpdateUnsynchLyricsFrame(&lyrics);
		if (nRet != ERR_OK)
			return nRet;
	}

	if (bSylt)
	{
		ID3v2SynchLyrics	lyrics;

		ToID3v2SynchLyrics(lyrics, fileEncoding, nEncodingID);

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
			&& m_lyrConentType == m_lyrConentTypeOrg) ||
			(bSylt && m_lrcSourceType == LST_ID3V2_SYLT))
		{
			ResetModificationFlag();
		}
	}

	return nRet;
}

int CMLData::SaveLyricsInSongOfLyrics3v2(FILE_ENCODING fileEncoding, CHAR_ENCODING nEncodingID)
{
	// save into mp3 file
	int			nRet;
	tstring		str;
	CLyrics3v2	lyrics3v2;
	CCharBuffer	strData;

	ToString(str, FT_LYRICS_LRC, TRUE);

	ConvertStrByFileEncoding(strData, str.c_str(), str.size(), 
		fileEncoding, nEncodingID);

	nRet = lyrics3v2.WriteLyrcsInfo(m_strSongFile.c_str(), (const char *)strData.c_str(), strData.size() * sizeof(char), TRUE);
	if (nRet == ERR_OK && m_lrcSourceType == LST_LYRICS3V2)
	{
		ResetModificationFlag();
	}

	return nRet;
}

// COMMENT:
//		将CMLData中的数据保存为文件szFile，文件的类型，由szFile的扩展名决定。
int CMLData::SaveAsFile(LPCTSTR szFile, FILE_ENCODING fileEncoding, bool &bUseNewFileName)
{
	if (!m_bHasLyrics)
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
		if (m_lyrConentType <= LCT_LRC)
			bUseNewFileName = true;
	}
	else if (FileIsExtSame(szFile, _T(".txt")) )
	{
		pParser = new CTextFileParser(this);
		NewFileType = FT_LYRICS_TXT;
		if (m_lyrConentType <= LCT_TXT)
			bUseNewFileName = true;
	}
	else if (FileIsExtSame(szFile, _T(".snc")) )
	{
		pParser = new CSncParser(this);
	}
	else
	{
		return ERR_UNKNOWN_LRC_FILE_TYPE;
	}

	int		nRet;

	// 分析歌词
	nRet = pParser->SaveAsFile(szFile, fileEncoding);

	delete pParser;

	if (nRet == ERR_OK && bUseNewFileName)
	{
		m_LyricFileType = NewFileType;
		m_fileEncoding = fileEncoding;
		ResetModificationFlag();

		// 保存歌词文件名
		m_strLrcSource = szFile;
	}

	return nRet;
}

// COMMENT:
//		将CMLData中的数据保存为文件szFile，文件的类型，由szFile的扩展名决定。
int CMLData::SaveToTextFile(LPCTSTR szFile, bool bSaveErrorInfo)
{
	if (!m_bHasLyrics)
	{
		if (!bSaveErrorInfo)
			return ERR_NOLYRICS_OPENED;
		tstring			str;
		tstring			strLyricsRow;
		for (int i = 0; i < (int)m_arrLyricsRows.size(); i++)
		{
			LyricsRowToText(m_arrLyricsRows[i], strLyricsRow);
			str += strLyricsRow;
			str += _T("\r\n");
		}
		SaveStringAsFile(szFile, str.c_str(), str.size(), FE_ANSI);
		return ERR_OK;
	}

	// 基类接口
	CTextFileParser	Parser(this);

	return Parser.SaveAsFile(szFile, m_fileEncoding);
}

int CMLData::UpdateTagToID3v2DescTxt()
{
	CLyricsTagReader<TCHAR>		lrctagWriter;

	LPCTSTR		szTitle = NULL, szArtist = NULL, szAlbum = NULL;
	LPCTSTR		szEncoding = NULL, szBy = NULL, szOffsetTime = NULL;
	TCHAR		szOffsetTimeBuff[100];

	if (m_lyrProperties.m_strArtist != m_lyrPropertiesOrg.m_strArtist)
		szArtist = m_lyrProperties.m_strArtist.c_str();
	if (m_lyrProperties.m_strTitle != m_lyrPropertiesOrg.m_strTitle)
		szTitle = m_lyrProperties.m_strTitle.c_str();
	if (m_lyrProperties.m_strAlbum != m_lyrPropertiesOrg.m_strAlbum)
		szAlbum = m_lyrProperties.m_strAlbum.c_str();
	if (m_lyrProperties.m_nTimeOffset != m_lyrPropertiesOrg.m_nTimeOffset)
	{
		_itot_s(m_lyrProperties.m_nTimeOffset, szOffsetTimeBuff, CountOf(szOffsetTimeBuff), 10);
		szOffsetTime = szOffsetTimeBuff;
	}
	if (_tcscmp(m_lyrProperties.GetEncoding(), m_lyrPropertiesOrg.GetEncoding()) != 0)
		szEncoding = m_lyrProperties.GetEncoding();

#ifdef UNICODE
	stringex		str;
	str.append((const char *)m_id3v2SaveCacheInfo.m_strLrcDescription.c_str(),
		m_id3v2SaveCacheInfo.m_strLrcDescription.size() * sizeof(TCHAR));
	lrctagWriter.TxtWriteToStr(str, true, szTitle, szArtist, szAlbum, szEncoding);
	m_id3v2SaveCacheInfo.m_strLrcDescription.clear();
	m_id3v2SaveCacheInfo.m_strLrcDescription.append((const wchar_t*)str.c_str(), str.size() / sizeof(TCHAR));
#else
	lrctagWriter.TxtWriteToStr(m_id3v2SaveCacheInfo.m_strLrcDescription, false, szTitle, szArtist, szAlbum, szEncoding);
#endif

	return ERR_OK;
}

BOOL CMLData::SaveTag()
{
	ASSERT(!IsContentModified());
	ASSERT(m_LyricFileType == FT_LYRICS_TXT || m_LyricFileType == FT_LYRICS_LRC);

	BOOL		bRet;

	LPCTSTR		szTitle = NULL, szArtist = NULL, szAlbum = NULL;
	LPCTSTR		szEncoding = NULL, szBy = NULL, szOffsetTime = NULL;
	TCHAR		szOffsetTimeBuff[100];

	if (m_lyrProperties.m_strArtist != m_lyrPropertiesOrg.m_strArtist)
		szArtist = m_lyrProperties.m_strArtist.c_str();
	if (m_lyrProperties.m_strTitle != m_lyrPropertiesOrg.m_strTitle)
		szTitle = m_lyrProperties.m_strTitle.c_str();
	if (m_lyrProperties.m_strAlbum != m_lyrPropertiesOrg.m_strAlbum)
		szAlbum = m_lyrProperties.m_strAlbum.c_str();
	if (m_lyrProperties.m_strBy != m_lyrPropertiesOrg.m_strBy)
		szBy = m_lyrProperties.m_strBy.c_str();
	if (m_lyrProperties.m_nTimeOffset != m_lyrPropertiesOrg.m_nTimeOffset)
	{
		_itot_s(m_lyrProperties.m_nTimeOffset, szOffsetTimeBuff, CountOf(szOffsetTimeBuff), 10);
		szOffsetTime = szOffsetTimeBuff;
	}
	if (_tcscmp(m_lyrProperties.GetEncoding(), m_lyrPropertiesOrg.GetEncoding()) != 0)
		szEncoding = m_lyrProperties.GetEncoding();

	CLyricsTagReader<TCHAR>		lrctagReader;

	if (m_lyrConentType == LCT_TXT)
	{
		bRet = lrctagReader.TxtWriteToFile(m_strLrcSource.c_str(), szTitle, szArtist, szAlbum, szEncoding);
	}
	else if (m_lyrConentType == LCT_LRC)
	{
		bRet = lrctagReader.LrcWriteToFile(m_strLrcSource.c_str(), szTitle, szArtist, szAlbum, szOffsetTime, szEncoding, szBy);
	}
	else
		bRet = FALSE;

	if (bRet)
	{
		m_lyrPropertiesOrg = m_lyrProperties;
	}

	return bRet;
}

void CMLData::ResetModificationFlag()
{
	tstring		strContent;

	m_lyrPropertiesOrg = m_lyrProperties;
	m_lyrConentTypeOrg = m_lyrConentType;
	ToString(strContent, FT_LYRICS_LRC, false);
	MD5Str((const char *)strContent.c_str(), strContent.size() * sizeof(TCHAR),
		m_szLyrContentMd5Org);
}

// COMMENT:
//		取得歌词的行数
int CMLData::GetLyricRowCount()
{
	return m_arrLyricsRows.size();
}

// COMMENT:
//		取得对应行 nRow 的歌词行
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
/*{
	int		nCount;
	PML_ROW	pRow;
	int		nBegRow = 0;
	int		nBegFrag = 0;
	int		nBegTime = 0;
	int		nEndRow = -1;
	int		nEndFrag = 0;
	int		nEndTime = 0;
	PLYRIC_FRAG	pFrag;

	while (NextFrag(nBegRow, nBegFrag, nEndRow, nEndFrag, &pFrag))
	{
		if (pFrag->nBegTime == -1)
		{
			pFrag->nBegTime = nBegTime;
		}

	}

	nCount = m_arrLyricsRows.size();

	for (int i = 0; i < nCount; i++)
	{
		pRow = m_arrLyricsRows[i];
		
	}
}*/
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

void CMLData::ClearLyrics()
{
	m_arrLyricsRows.erase(m_arrLyricsRows.begin(), m_arrLyricsRows.end());
	m_arrFileRows.clear();
}

void CMLData::Close()
{
	m_lyrPropertiesOrg.clear();
	m_lyrProperties.clear();
	EmptyStr(m_szLyrContentMd5Org);
	MD5Str((const char *)_T(""), 0,
		m_szLyrContentMd5Org);
	m_nTimeElapsed = 0;

	m_fileEncoding = FE_ANSI;

	m_dwDefShowStyle = LSA_SHOW_DEFAULT;
	m_lyrConentTypeOrg = m_lyrConentType = LCT_UNKNOWN;
	m_LyricFileType = FT_UNKNOWN;

	m_bHasLyrics = FALSE;

	m_lrcSourceType = LST_FILE;
	m_strSongFile.resize(0);;
	m_strLrcSource.resize(0);;

	m_id3v2SaveCacheInfo.clear();
	ClearLyrics();
}

BOOL CMLData::IsOK()
{
	return m_bHasLyrics;
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
		vector<PML_ROW>::iterator	itPos;

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

int CMLData::GetLyricsFileServerMark(TCHAR szMd5[])
{
	int				nRet = ERR_OK;
	LRC_SOURCE_TYPE lrcSourceType;

	EmptyStr(szMd5);

	lrcSourceType = GetLrcSourceType(m_strLrcSource.c_str());

	if (lrcSourceType == LST_LYRICS3V2)
	{
		stringex	strLyrics;
		CLyrics3v2		lyrics3v2;

		nRet = lyrics3v2.ReadLyrcsInfo(m_strSongFile.c_str(), strLyrics);
		if (nRet != ERR_OK)
			return nRet;

		MD5Str(strLyrics.c_str(), strLyrics.size(), szMd5);
	}
	else if (lrcSourceType == LST_ID3V2_SYLT || lrcSourceType == LST_ID3V2_USLT || lrcSourceType == LST_ID3V2_LYRICS)
	{
		return ERR_FAILED;
	}
	else
		MD5FileStr(m_strLrcSource.c_str(), szMd5);

	return ERR_OK;
}

// COMMENT:
//		设置时间偏移
void CMLData::SetPlayElapsedTime(int nTimeElapsed)
{
	m_nTimeElapsed = nTimeElapsed + m_lyrProperties.m_nTimeOffset;
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

	assert(nRow != -1);
	return nRow;
}

bool CMLData::DoesChooseNewFileName()
{
	return (m_strLrcSource.empty() || m_lyrConentType != m_lyrConentTypeOrg);
}

BOOL CMLData::IsModified()
{
	return !m_lyrProperties.Equal(m_lyrPropertiesOrg) || IsContentModified();
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
	MD5Str((const char *)strContent.c_str(), strContent.size() * sizeof(TCHAR),
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


// COMMENT:
//		设置歌词段的开始显示时间，在设置之后并且自动校正前面和后面设置相冲突的时间
void CMLData::SetLyricsFragBeginTime(int nRow, int nFrag, int nTime)
{
	PML_ROW			pRow;
	PLYRIC_FRAG		pFrag;
	int				nRowPrev;
	int				nFragPrev;
	int				nTimeBeg;
	int				nRowNext;
	int				nFragNext;
	int				nTimeEnd;

	assert((nRow >= 0) && (nRow <  GetLyricRowCount()));
	if ((nRow < 0) || (nRow >=  GetLyricRowCount()))
		return;

	pRow = GetLyricsRow(nRow);
	assert(pRow);

	assert((nFrag >= 0) && ((unsigned int)nFrag < pRow->vFrags.size()));
	if ((nFrag < 0) || ((unsigned int)nFrag >= pRow->vFrags.size()))
		return;

	pFrag = GetLyricsRow(nRow)->vFrags[nFrag];
	pFrag->dwAttrib &= ~LSA_TEMP_TIME_BEG;
	pFrag->nBegTime = nTime;

	// 重新校正从上一个设置了时间的歌词段到这个歌词段之前的时间。
	// 校正的规则：对于所有前面的时间大于nTime都设置为“临时时间”
	// |nFragPrev|....|nFragNext|nFrag|
	//                           ^

	// 取得前一段歌词
	if (PrevFrag(nRow, nFrag, nRowNext, nFragNext, &pFrag))
	{
		pFrag = NULL;
		nTimeEnd = nTime;
		nRowPrev = nRow;
		nFragPrev = nFrag;

		while (1)
		{
			if (!PrevFrag(nRowPrev, nFragPrev, nRowPrev, nFragPrev, &pFrag))
			{
				// 没有找到前一个，则到达了歌词头部
				nTimeBeg = LYRICS_BEGIN_TIME - GetOffsetTime();
				nRowPrev = 0;
				nFragPrev = 0;
				break;
			}
			if (!IsTempTimeEnd(pFrag->dwAttrib))
			{
				// 对于歌词时间不是临时的，如果时间值合理，则找到，否则，将此时间设置为临时时间
				// 歌词行之间的间隔过小，则进行修正，在此规定，每行歌词至少显示ROW_TIME_MIN_SPAN毫秒
				if (nTime > pFrag->nEndTime
					+ ROW_TIME_MIN_SPAN * (nRow - nRowPrev - 1))
				{
					// 歌词行之间的间隔过小，则进行修正，在此规定，每行歌词至少显示ROW_TIME_MIN_SPAN毫秒
					nTimeBeg = pFrag->nEndTime;
					if (!NextFrag(nRowPrev, nFragPrev, nRowPrev, nFragPrev))
						ERRLOG0(_T("Next Fragment does not exist?! Impossible!"));
					break;
				}
				pFrag->nEndTime = -1;
				pFrag->dwAttrib |= LSA_TEMP_TIME_END;
			}
			if (!IsTempTimeBeg(pFrag->dwAttrib))
			{
				// 对于歌词时间不是临时的，如果时间值合理，则找到，否则，将此时间设置为临时时间
				// 歌词行之间的间隔过小，则进行修正，在此规定，每行歌词至少显示ROW_TIME_MIN_SPAN毫秒
				if (nTime > pFrag->nBegTime + ROW_TIME_MIN_SPAN * (nRow - nRowPrev))
				{
					nTimeBeg = pFrag->nBegTime;
					break;
				}
				pFrag->nBegTime = -1;
				pFrag->dwAttrib |= LSA_TEMP_TIME_BEG;
			}
		}
		if ((nRowPrev < nRow) || ((nRowPrev == nRow) && (nFragPrev < nFrag)))
		{
			if (nTimeBeg >= nTimeEnd)
				nTimeBeg = nTimeEnd - ROW_TIME_MIN_SPAN * (nRowNext - nRowPrev + 1);
			CalFragTimeBetweenRows(nRowPrev, nFragPrev, nRowNext, nFragNext, nTimeBeg, nTimeEnd);
		}
	}

	// 重新校正从此段歌词到下一个设置了时间歌词段的所有时间。
	// |nFrag|nFragPrev|....|nFragNext|
	//  ^
	pFrag = NULL;
	nTimeBeg = nTime;
	nRowPrev = nRow;
	nFragPrev = nFrag;
	nRowNext = nRow;
	nFragNext = nFrag;

	pFrag = GetLyricsRow(nRow)->vFrags[nFrag];

	while (1)
	{
		if (!IsTempTimeEnd(pFrag->dwAttrib))
		{
			if (nTimeBeg + ROW_TIME_MIN_SPAN * (nRowNext - nRowPrev + 1) <= pFrag->nEndTime)
			{
				nTimeEnd = pFrag->nEndTime;
//				if (!NextFrag(nRowPrev, nFragPrev, nRowPrev, nFragPrev))
//				{
//					ERRLOG0(_T("Next Fragment does not exist?! Impossible!"));
//				}
				break;
			}

			pFrag->nEndTime = -1;
			pFrag->dwAttrib |= LSA_TEMP_TIME_END;
		}
		if (!NextFrag(nRowNext, nFragNext, nRowNext, nFragNext, &pFrag))
		{
			nTimeEnd = GetMediaLength() - LYRICS_END_TIME;
			nRowNext = GetLyricRowCount() - 1;
			assert(nRowNext >= 0);
			nFragNext = GetLyricsRow(nRowNext)->vFrags.size() - 1;
			break;
		}
		if (!IsTempTimeBeg(pFrag->dwAttrib))
		{
			if (nTimeBeg + ROW_TIME_MIN_SPAN * (nRowNext - nRowPrev) <= pFrag->nEndTime)
			{
				nTimeEnd = pFrag->nBegTime;
				if (!PrevFrag(nRowNext, nFragNext, nRowNext, nFragNext))
					ERRLOG0(_T("Previous Fragment does not exist?! Impossible!"));
				break;
			}

			pFrag->nBegTime = -1;
			pFrag->dwAttrib |= LSA_TEMP_TIME_BEG;
		}
	}
	if (nTimeBeg >= nTimeEnd)
		nTimeEnd = nTimeBeg + ROW_TIME_MIN_SPAN * (nRowNext - nRowPrev + 1);
	CalFragTimeBetweenRows(nRowPrev, nFragPrev, nRowNext, nFragNext, nTimeBeg, nTimeEnd);
}

// COMMENT:
//		设置歌词段的结束显示时间，在设置之后并且自动校正前面和后面设置相冲突的时间
void CMLData::SetLyricsFragEndTime(int nRow, int nFrag, int nTime)
{
	PML_ROW			pRow;
	PLYRIC_FRAG		pFrag;
	int				nRowPrev;
	int				nFragPrev;
	int				nTimeBeg;
	int				nRowNext;
	int				nFragNext;
	int				nTimeEnd;

	assert((nRow >= 0) && (nRow <  GetLyricRowCount()));
	if ((nRow < 0) || (nRow >=  GetLyricRowCount()))
		return;

	pRow = GetLyricsRow(nRow);
	assert(pRow);

	assert((nFrag >= 0) && ((unsigned int)nFrag < pRow->vFrags.size()));
	if ((nFrag < 0) || ((unsigned int)nFrag >= pRow->vFrags.size()))
		return;

	pFrag = GetLyricsRow(nRow)->vFrags[nFrag];
	pFrag->dwAttrib &= ~LSA_TEMP_TIME_END;
	pFrag->nEndTime = nTime;

	// 重新校正从上一个设置了时间的歌词段到这个歌词段之前的时间。
	// 校正的规则：对于所有前面的时间大于nTime都设置为“临时时间”
	// |nFragPrev|....|nFragNext|nFrag|
	//                               ^

	// 取得前一段歌词
//	if (PrevFrag(nRow, nFrag, nRowNext, nFragNext, &pFrag))
	{
		pFrag = NULL;
		nTimeEnd = nTime;
		nRowPrev = nRow;
		nFragPrev = nFrag;
		nRowNext = nRow;
		nFragNext = nFrag;

		pFrag = GetLyricsRow(nRow)->vFrags[nFrag];
		while (1)
		{
			if (!IsTempTimeBeg(pFrag->dwAttrib))
			{
				// 对于歌词时间不是临时的，如果时间值合理，则找到，否则，将此时间设置为临时时间
				// 歌词行之间的间隔过小，则进行修正，在此规定，每行歌词至少显示ROW_TIME_MIN_SPAN毫秒
				if (nTime > pFrag->nBegTime + ROW_TIME_MIN_SPAN * (nRow - nRowPrev))
				{
					nTimeBeg = pFrag->nBegTime;
					break;
				}
				pFrag->nBegTime = -1;
				pFrag->dwAttrib |= LSA_TEMP_TIME_BEG;
			}
			if (!PrevFrag(nRowPrev, nFragPrev, nRowPrev, nFragPrev, &pFrag))
			{
				// 没有找到前一个，则到达了歌词头部
				nTimeBeg = LYRICS_BEGIN_TIME - GetOffsetTime();
				nRowPrev = 0;
				nFragPrev = 0;
				break;
			}
			if (!IsTempTimeEnd(pFrag->dwAttrib))
			{
				// 对于歌词时间不是临时的，如果时间值合理，则找到，否则，将此时间设置为临时时间
				// 歌词行之间的间隔过小，则进行修正，在此规定，每行歌词至少显示ROW_TIME_MIN_SPAN毫秒
				if (nTime > pFrag->nEndTime + ROW_TIME_MIN_SPAN * (nRow - nRowPrev - 1))
				{
					// 歌词行之间的间隔过小，则进行修正，在此规定，每行歌词至少显示ROW_TIME_MIN_SPAN毫秒
					nTimeBeg = pFrag->nEndTime;
					if (!NextFrag(nRowPrev, nFragPrev, nRowPrev, nFragPrev))
						ERRLOG0(_T("Next Fragment does not exist?! Impossible!"));
					break;
				}
				pFrag->nEndTime = -1;
				pFrag->dwAttrib |= LSA_TEMP_TIME_END;
			}
		}
//		if ((nRowPrev < nRowNext) || (nFragPrev != nFragNext) || 
//			IsTempTimeBeg(GetLyricsRow(nRowPrev)->vFrags[nFragPrev]->dwAttrib))
		{
			if (nTimeBeg >= nTimeEnd)
				nTimeBeg = nTimeEnd - ROW_TIME_MIN_SPAN * (nRowNext - nRowPrev + 1);
			CalFragTimeBetweenRows(nRowPrev, nFragPrev, nRowNext, nFragNext, nTimeBeg, nTimeEnd);
		}
	}

	// 重新校正从此段歌词到下一个设置了时间歌词段的所有时间。
	// |nFrag|nFragPrev|....|nFragNext|
	//      ^
	pFrag = NULL;
	nTimeBeg = nTime;
//	nRowPrev = nRow;
//	nFragPrev = nFrag;
	nRowNext = nRow;
	nFragNext = nFrag;

	if (NextFrag(nRow, nFrag, nRowPrev, nFragPrev))
	{
		while (1)
		{
			if (!NextFrag(nRowNext, nFragNext, nRowNext, nFragNext, &pFrag))
			{
				nTimeEnd = GetMediaLength() - LYRICS_END_TIME;
				nRowNext = GetLyricRowCount() - 1;
				assert(nRowNext >= 0);
				nFragNext = GetLyricsRow(nRowNext)->vFrags.size() - 1;
				break;
			}
			if (!IsTempTimeBeg(pFrag->dwAttrib))
			{
				if (nTimeBeg + ROW_TIME_MIN_SPAN * (nRowNext - nRowPrev) <= pFrag->nEndTime)
				{
					nTimeEnd = pFrag->nBegTime;
					if (!PrevFrag(nRowNext, nFragNext, nRowNext, nFragNext))
						ERRLOG0(_T("Previous Fragment does not exist?! Impossible!"));
					break;
				}

				pFrag->nBegTime = -1;
				pFrag->dwAttrib |= LSA_TEMP_TIME_BEG;
			}
			if (!IsTempTimeEnd(pFrag->dwAttrib))
			{
				if (nTimeBeg + ROW_TIME_MIN_SPAN * (nRowNext - nRowPrev + 1) <= pFrag->nEndTime)
				{
					nTimeEnd = pFrag->nEndTime;
//					if (!NextFrag(nRowPrev, nFragPrev, nRowPrev, nFragPrev))
//					{
//						ERRLOG0(_T("Next Fragment does not exist?! Impossible!"));
//					}
					break;
				}

				pFrag->nEndTime = -1;
				pFrag->dwAttrib |= LSA_TEMP_TIME_END;
			}
		}
		if ((nRowNext > nRow) || ((nRowNext == nRow) && (nFragNext > nFrag)))
		{
			if (nTimeBeg >= nTimeEnd)
				nTimeEnd = nTimeBeg + ROW_TIME_MIN_SPAN * (nRowNext - nRowPrev + 1);
			CalFragTimeBetweenRows(nRowPrev, nFragPrev, nRowNext, nFragNext, nTimeBeg, nTimeEnd);
		}
	}
}

// COMMENT:
//		设置歌词行的结束显示时间，在设置之后并且自动校正前面和后面设置相冲突的时间
void CMLData::SetLyricsRowEndTime(int nRow, int nTime)
{
	if ((nRow < 0) || (nRow >= GetLyricRowCount()))
		return;

	SetLyricsFragEndTime(nRow, GetLyricsRow(nRow)->vFrags.size() - 1, nTime);
}

// 设置从当前行开始的偏移时间
void CMLData::SetOffsetTimeFromCurrentRow(int nRow, int nOffsetTime)
{
	if (nRow < 0 || nRow >= GetLyricRowCount())
		return;

	// 检查时间会不会冲突
	if (nRow > 0 && nOffsetTime < 0)
	{
		PML_ROW		pRowPrev, pRowCur;
		pRowPrev = GetLyricsRow(nRow - 1);
		ASSERT(pRowPrev);
		pRowCur = GetLyricsRow(nRow);
		ASSERT(pRowCur);

		if (pRowCur->nBegTime + nOffsetTime <= pRowPrev->nBegTime)
			nOffsetTime = pRowPrev->nBegTime - pRowCur->nBegTime + 100;	// 0.2ms at least
	}

	if (nOffsetTime == 0)
		return;

	PML_ROW			pRow;
	PLYRIC_FRAG		pFrag;
	for (int i = GetLyricRowCount() - 1; i >= nRow; i--)
	{
		pRow = GetLyricsRow(i);
		// SetLyricsFragBeginTime(i, 0, pRow->nBegTime + nOffsetTime);
		for (int k = 0; k < (int)pRow->vFrags.size(); k++)
		{
			pFrag = pRow->vFrags[k];
			pFrag->nBegTime += nOffsetTime;
			pFrag->nEndTime += nOffsetTime;
		}
		pRow->nBegTime += nOffsetTime;
		pRow->nEndTime += nOffsetTime;
	}
/*	if (nOffsetTime > 0)
	{
		for (int i = GetLyricRowCount() - 1; i >= nRow; i--)
		{
			pRow = GetLyricsRow(i);
			// SetLyricsFragBeginTime(i, 0, pRow->nBegTime + nOffsetTime);
			for (int k = 0; k < pRow->vFrags.size(); k++)
			{
				pFrag = pRow->vFrags[k];
				pFrag->nBegTime += nOffsetTime;
				pFrag->nEndTime += nOffsetTime;
			}
			pRow->nBegTime += nOffsetTime;
			pRow->nEndTime += nOffsetTime;
		}
	}
	else
	{
		for (int i = nRow; i < GetLyricRowCount(); i++)
		{
			pRow = GetLyricsRow(i);
			// SetLyricsFragBeginTime(i, 0, pRow->nBegTime + nOffsetTime);
			for (int k = 0; k < pRow->vFrags.size(); k++)
			{
				pFrag = pRow->vFrags[k];
				pFrag->nBegTime += nOffsetTime;
				pFrag->nEndTime += nOffsetTime;
			}
			pRow->nBegTime += nOffsetTime;
			pRow->nEndTime += nOffsetTime;
		}
	}*/
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

//
// Do not display the artist, album and title info of lyrics.
//
void CMLData::HideLyricsInfoTag()
{
	LPCTSTR		szTag[] = { _T("artist: "), _T("title: "), _T("album: "), _T("by: ") };
	int			nLen[CountOf(szTag)];
	int			i;

	for (i = 0; i < CountOf(szTag); i++)
		nLen[i] = _tcslen(szTag[i]);

	for (i = 0; i < (int)m_arrLyricsRows.size() && i <= 3; i++)
	{
		PML_ROW		pRow = m_arrLyricsRows[i];
		if (pRow->vFrags.size() == 1)
		{
			for (int k = 0; k < CountOf(szTag); k++)
			{
				if (_tcsnicmp(pRow->vFrags[0]->szLyric, szTag[k], nLen[k]) == 0)
				{
					// it's a tag, remove this line.
					m_arrLyricsRows.erase(m_arrLyricsRows.begin() + i);
					i--;
					break;
				}
			}
		}
	}
}

void CMLData::RemoveErrrorInfo()
{
	int		i;

	for (i = 0; i < (int)m_arrLyricsRows.size(); i++)
	{
		PML_ROW		pRow;
		pRow = m_arrLyricsRows[i];
		if (pRow->vFrags.size())
		{
			PLYRIC_FRAG		pFrag;
			pFrag = pRow->vFrags[0];
			if ((pFrag->dwAttrib & LAS_ERROR_INFO) == LAS_ERROR_INFO)
			{
				RemoveLyricsRow(i);
				i--;
			}
		}
	}
}

void CMLData::AddSongInfo(LPCTSTR szSongFile, LPCTSTR szSongTitle)
{
	PLYRIC_FRAG		pFrag;
	PML_ROW			pRow;

	//
	// 添加歌曲名等……
	vector<tstring>		vStr;
	int					nPrevEnd;
	int					nCount;

	vStr.push_back(szSongTitle);

	//
	// 仅仅在前30次显示提示用户搜索歌词的信息
	//
	nCount = g_Profile.GetProfileInt(_T("SearchLyricsPrompt"), 0);
	if (nCount <= 30)
	{
		nCount++;
		g_Profile.WriteProfileInt(_T("SearchLyricsPrompt"), nCount);
		vStr.push_back(_T("Please Click \"Search Lyrics\" button to get lyrics"));
	}

	nPrevEnd = 0;
	for (int i = 0; i < (int)vStr.size(); i++)
	{
		int		nEndTime;

		nEndTime = nPrevEnd + 10 * 1000;
		pFrag = NewLyricFrag(nPrevEnd, nEndTime, vStr[i].c_str(), vStr[i].size(), TRUE, TRUE, LSA_SHOW_SIMPLE | LSA_TEMP_FRAG);
		nPrevEnd = nEndTime;

		pRow = NewMLRow(pFrag->nBegTime, pFrag->nEndTime, (TCHAR *)NULL);
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
	pFrag = NewLyricFrag(nBegTime, -1, szText, -1, TRUE, TRUE, LSA_SHOW_DEFAULT | LAS_AD_FRAG | LSA_TEMP_FRAG);
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
		pFrag = NewLyricFrag(nBegTime, -1, szText, -1, TRUE, TRUE, LSA_SHOW_DEFAULT | LAS_AD_FRAG | LSA_TEMP_FRAG);
		pRow->vFrags.push_back(pFrag);
		AddRow(pRow);
	}

	CheckLyricsRowTime();
}

// COMMENT:
//		nFrag:	-1	该行歌词的结尾
int CMLData::GetFragTime(int nRow, int nFrag, BOOL bBegOfFrag)
{
	PML_ROW		pRow = NULL;
	PLYRIC_FRAG		pFrag;
	int			dwTime = 0;
	int			dwLength = 0;

	pRow = GetLyricsRow(nRow);
	if (pRow)
	{
		if (nFrag < 0 || (unsigned int)nFrag >= pRow->vFrags.size())
			nFrag = pRow->vFrags.size();

		pFrag = pRow->vFrags[nFrag];

		assert(pFrag);
		if (pFrag)
		{
			if (bBegOfFrag)
				dwTime = pFrag->nBegTime;
			else
				dwTime = pFrag->nEndTime;
		}

		dwTime -= GetOffsetTime();
		if (dwTime < 0)
			dwTime = 0;
		else
		{
			dwLength = GetMediaLength();
			if (dwTime > dwLength)
				dwTime = dwLength;
		}
	}
	return dwTime;
}

BOOL CMLData::LyricsRowToText(PML_ROW pRow, tstring &strLyricsRow)
{
	strLyricsRow.resize(0);
	if (!pRow)
		return FALSE;

	switch (pRow->nRowType)
	{
	case ML_ROW_UNKNOW:		// 不能识别的行
		assert(pRow->szContent);
		if (pRow->szContent)
			strLyricsRow = pRow->szContent;
		break;
	case ML_ROW_NULL:		// 空行
		break;
	case ML_ROW_LYRICS:		// 歌词行
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
//LYRIC_FRAG *CMLData::GetLyricsFrag(const MLDataPointer &pointer)
//{
//	return NULL;
//}
//
//BOOL CMLData::SetLyricsFrag(const MLDataPointer &pointer, LPCTSTR szLyricsFrag, LYRIC_FRAG_ATTRIBUTE dwShowAttrib/* = LSA_SHOW_SIMPLE*/, int nBegTime/* = -1*/, int nEndTime/* = -1*/)
//{
//	return TRUE;
//}

//
// 清除从nRow开始的行（包括该行）的所有时间标记
//
void CMLData::ClearTimeTagRromRow(int nRow)
{
	if ((nRow < 0) || (nRow >= GetLyricRowCount()))
		return;

	for (int i = nRow; i < (int)m_arrLyricsRows.size() - 1; i++)
	{
		PML_ROW	pRow;
		pRow = m_arrLyricsRows[i];
		pRow->nBegTime = -1;
		pRow->nEndTime = -1;
		for (int m = 0; m < (int)pRow->vFrags.size(); m++)
		{
			PLYRIC_FRAG	pFrag;
			pFrag = pRow->vFrags[m];
			pFrag->nBegTime = -1;
			pFrag->nEndTime = -1;
			pFrag->dwAttrib |= LSA_TEMP_TIME_BEG | LSA_TEMP_TIME_END;
		}
	}
}

/*
MLDataPointer CMLData::PrevFrag(const MLDataPointer &pointer)
{
}

MLDataPointer CMLData::NextFrag(const MLDataPointer &pointer)
{
}*/

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
							for (x = 0; x < 3 && isdigit(szNumb[x + 1]); x++)
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

bool CMLData::ToID3v2SynchLyrics(ID3v2SynchLyrics &lyrics, FILE_ENCODING fileEncoding, CHAR_ENCODING nEncodingID)
{
	strcpy_safe(lyrics.m_szLanguage, CountOf(lyrics.m_szLanguage), m_id3v2SaveCacheInfo.m_strLanguage.c_str());
	lyrics.m_bTimeStampMs = true;

	CLyricsTagReader<TCHAR>		lrctagReader;

	if (fileEncoding == FE_UCS2 || fileEncoding == FE_UTF8)
	{
		lyrics.m_encodingType = IET_UCS2LE_BOM;
		// convert content description
		stringex			strHeader;
		wstringex			strTemp;
		ConvertStr(m_id3v2SaveCacheInfo.m_strLrcDescription.c_str(), m_id3v2SaveCacheInfo.m_strLrcDescription.size(), lyrics.m_strUcs2ContentDesc, nEncodingID);
		strHeader.append((const char *)lyrics.m_strUcs2ContentDesc.c_str(), lyrics.m_strUcs2ContentDesc.size() * sizeof(WCHAR));
		lrctagReader.TxtWriteToStr(strHeader, true, 
			m_lyrProperties.m_strTitle.c_str(),
			m_lyrProperties.m_strArtist.c_str(),
			m_lyrProperties.m_strAlbum.c_str(),
			m_lyrProperties.GetEncoding());
		lyrics.m_strUcs2ContentDesc.clear();
		lyrics.m_strUcs2ContentDesc.append((const WCHAR *)strHeader.c_str(), strHeader.size() / sizeof(WCHAR));

		// convert lyrics rows
		for (int i = 0; i < (int)m_arrLyricsRows.size(); i++)
		{
			PML_ROW		pRow = m_arrLyricsRows[i];

			if (pRow->nRowType == ML_ROW_LYRICS && !pRow->IsTempRow())		// 歌词行
			{
				/*
				for (int k = 0; k < pRow->vFrags.size(); k++)
				{
					PLYRIC_FRAG				pFrag = pRow->vFrags[k];
					LrcSyllable<wchar_t>	syllable;
					syllable.bNewLine = false;
					syllable.strText = pFrag->szLyric;
					syllable.nTime = pFrag->nBegTime + m_lyrProperties.m_nTimeOffset;
					if (syllable.nTime < 0)
						syllable.nTime = 0;
					lyrics.m_vUcs2SynLyrics.push_back(syllable);
				}*/
				//
				// to compatiable with windows media player, the Karaoke info has to be lost.
				//
				LrcSyllable<wchar_t>	syllable;
				syllable.bNewLine = false;
				syllable.nTime = pRow->nBegTime - m_lyrProperties.m_nTimeOffset;
				if (syllable.nTime < 0)
					syllable.nTime = 0;
				for (int k = 0; k < (int)pRow->vFrags.size(); k++)
				{
					PLYRIC_FRAG				pFrag = pRow->vFrags[k];
#ifdef UNICODE
					syllable.strText += pFrag->szLyric;
#else
					ConvertStr(pFrag->szLyric, pFrag->nLen, strTemp, nEncodingID);
					syllable.strText += strTemp.c_str();
#endif
				}
				lyrics.m_vUcs2SynLyrics.push_back(syllable);

				if (!m_id3v2SaveCacheInfo.m_bAllSyllableIsNewLine)
				{
					LrcSyllable<wchar_t>	syllable;
					syllable.bNewLine = true;
					lyrics.m_vUcs2SynLyrics.push_back(syllable);
				}
			}
		}
	}
	else
	{
		// save as ansi encoding
		stringex			str;

		lyrics.m_encodingType = IET_ANSI;

		// convert content description
		ConvertStr(m_id3v2SaveCacheInfo.m_strLrcDescription.c_str(), m_id3v2SaveCacheInfo.m_strLrcDescription.size(), lyrics.m_strAnsiContentDesc, nEncodingID);
		lrctagReader.TxtWriteToStr(lyrics.m_strAnsiContentDesc, false, 
			m_lyrProperties.m_strTitle.c_str(),
			m_lyrProperties.m_strArtist.c_str(),
			m_lyrProperties.m_strAlbum.c_str(),
			m_lyrProperties.GetEncoding());

		// convert lyrics rows
		for (int i = 0; i < (int)m_arrLyricsRows.size(); i++)
		{
			PML_ROW		pRow = m_arrLyricsRows[i];

			if (pRow->nRowType == ML_ROW_LYRICS && !pRow->IsTempRow())		// 歌词行
			{
				/*
				for (int k = 0; k < pRow->vFrags.size(); k++)
				{
					PLYRIC_FRAG				pFrag = pRow->vFrags[k];
					LrcSyllable<char>		syllable;
					syllable.bNewLine = false;
					ConvertStr(pFrag->szLyric, pFrag->nLen, str, nEncodingID);
					syllable.strText = str;
					syllable.nTime = pFrag->nBegTime + m_lyrProperties.m_nTimeOffset;
					if (syllable.nTime < 0)
						syllable.nTime = 0;
					lyrics.m_vAnsiSynLyrics.push_back(syllable);
				}*/
				//
				// to compatiable with windows media player, the Karaoke info has to be lost.
				//
				LrcSyllable<char>	syllable;
				syllable.bNewLine = false;
				syllable.nTime = pRow->nBegTime - m_lyrProperties.m_nTimeOffset;
				if (syllable.nTime < 0)
					syllable.nTime = 0;
				for (int k = 0; k < (int)pRow->vFrags.size(); k++)
				{
					PLYRIC_FRAG				pFrag = pRow->vFrags[k];
#ifdef UNICODE
					ConvertStr(pFrag->szLyric, pFrag->nLen, str, nEncodingID);
					syllable.strText += str.c_str();
#else
					syllable.strText += pFrag->szLyric;
#endif
				}
				lyrics.m_vAnsiSynLyrics.push_back(syllable);

				if (!m_id3v2SaveCacheInfo.m_bAllSyllableIsNewLine)
				{
					LrcSyllable<char>	syllable;
					syllable.bNewLine = true;
					lyrics.m_vAnsiSynLyrics.push_back(syllable);
				}
			}
		}
	}

	return true;
}

bool CMLData::ToID3v2UnsynchLyrics(ID3v2UnsynchLyrics &lyrics, FILE_ENCODING fileEncoding, CHAR_ENCODING nEncodingID)
{
	tstring		str;

	ToString(str, FT_LYRICS_TXT, TRUE);

	strcpy_safe(lyrics.m_szLanguage, CountOf(lyrics.m_szLanguage), m_id3v2SaveCacheInfo.m_strLanguage.c_str());

	if (fileEncoding == FE_UCS2 || fileEncoding == FE_UTF8)
	{
		lyrics.m_encodingType = IET_UCS2LE_BOM;
		ConvertStr(m_id3v2SaveCacheInfo.m_strLrcDescription.c_str(), m_id3v2SaveCacheInfo.m_strLrcDescription.size(), lyrics.m_strUcs2ContentDesc, nEncodingID);
		ConvertStr(str.c_str(), str.size(), lyrics.m_strUcs2Lyrics, nEncodingID);
	}
	else
	{
		lyrics.m_encodingType = IET_ANSI;
		ConvertStr(m_id3v2SaveCacheInfo.m_strLrcDescription.c_str(), m_id3v2SaveCacheInfo.m_strLrcDescription.size(), lyrics.m_strAnsiContentDesc, nEncodingID);
		ConvertStr(str.c_str(), str.size(), lyrics.m_strAnsiLyrics, nEncodingID);
	}

	return true;
}

BOOL CMLData::ToString(tstring &str, MLFileType lrcFormat, BOOL bIncTags)
{
	if (lrcFormat == FT_LYRICS_LRC)
	{
		// 转换为 LRC 格式的字符串
		CLrcParser		parser(this);

		return parser.ToString(str, bIncTags);
	}
	else
	{
		ASSERT(lrcFormat == FT_LYRICS_TXT);
		// 转换为 TXT 格式的字符串
		CTextFileParser		parser(this);

		return parser.ToString(str, bIncTags);
	}
}

int CMLData::FromString(LPCVOID lpLrc, int nLen, FILE_ENCODING *fileEncoding, MLFileType lrcFormat, BOOL bStrict, BOOL bUnicode, bool bForceUseEncoding, CHAR_ENCODING nForceEncodingID)
{
	if (lrcFormat != FT_LYRICS_LRC)
		return ERR_FALSE;

	ClearLyrics();

	// 从LRC格式转换过来
	CLrcParser		parser(this);
	int				nRet;

	if (bForceUseEncoding)
	{
		// 设置缺省的编码
		SetEncoding(nForceEncodingID);
		if (nForceEncodingID == ED_UNICODE)
			bUnicode = TRUE;
	}

	nRet = parser.ParseString(bForceUseEncoding, lpLrc, nLen, bStrict, bUnicode);
	if (nRet == ERR_OK)
	{
		m_lyrConentType = parser.GetLyrContentType();
		m_bHasLyrics = TRUE;

		if (fileEncoding)
			*fileEncoding = parser.GetFileEncoding();

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
		const char *szContent, int nLen, ML_ROW_TYPE rowType)
{
	PML_ROW		pRow;

	pRow = new ML_ROW;
	pRow->nBegTime = nBegTime;
	pRow->nEndTime = nEndTime;
	pRow->nRowType = rowType;

	if (szContent)
	{
		tstringex	str;

		ConvertStr(szContent, nLen, str, GetEncodingIndex());
		if (str.empty() && nLen > 0)
			ConvertStr(szContent, nLen, str, ED_SYSDEF);

		pRow->szContent = new TCHAR[str.size() + 1];
		_tcsncpysz_safe(pRow->szContent, str.size() + 1, str.c_str(), str.size());
	}

	return pRow;
}

PML_ROW CMLData::NewMLRow(int nBegTime, int nEndTime, 
		const WCHAR *szContent, int nLen, ML_ROW_TYPE rowType)
{
	PML_ROW		pRow;

	pRow = new ML_ROW;
	pRow->nBegTime = nBegTime;
	pRow->nEndTime = nEndTime;
	pRow->nRowType = rowType;

	if (szContent)
	{
		tstringex	str;

		ConvertStr(szContent, nLen, str, GetEncodingIndex());

		pRow->szContent = new TCHAR[str.size() + 1];
		_tcsncpysz_safe(pRow->szContent, str.size() + 1, str.c_str(), str.size());
		// _tcscpy(pRow->szContent, str.c_str());
	}

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

VLyrLines &CMLData::GetRawLyrics()
{
	return m_arrLyricsRows;
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

void CMLData::GetProperty(LYRICS_PROP &prop, tstring &strValue)
{
	switch (prop)
	{
	case LP_ARTIST:
		strValue = m_lyrProperties.m_strArtist;
		break;
	case LP_ALBUM:
		strValue = m_lyrProperties.m_strAlbum;
		break;
	case LP_TITLE:
		strValue = m_lyrProperties.m_strTitle;
		break;
	case LP_ENCODING:
		strValue = m_lyrProperties.GetEncoding();
		break;
	case LP_BY:
		strValue = m_lyrProperties.m_strBy;
		break;
	case LP_OFFSET:
		strValue = CStrPrintf(_T("%d"), m_lyrProperties.m_nTimeOffset).c_str();
		break;
	}
}

void CMLData::SetProperty(LYRICS_PROP &prop, LPCTSTR szValue)
{
	switch (prop)
	{
	case LP_ARTIST:
		m_lyrProperties.m_strArtist = szValue;
		break;
	case LP_ALBUM:
		m_lyrProperties.m_strAlbum = szValue;
		break;
	case LP_TITLE:
		m_lyrProperties.m_strTitle = szValue;
		break;
	case LP_ENCODING:
		SetEncoding(szValue);
		break;
	case LP_BY:
		m_lyrProperties.m_strBy = szValue;
		break;
	case LP_OFFSET:
		m_lyrProperties.m_nTimeOffset = _ttoi(szValue);
		break;
	}

}

CMLData::LYRICS_PROP_NAME* CMLData::GetSupportedLyricsPropName()
{
	static LYRICS_PROP_NAME	_LyricsPropName[] = 
	{
		{ _T("Artist:"), LP_ARTIST },
		{ _T("Album:"), LP_ALBUM },
		{ _T("Title:"), LP_TITLE },
		{ _T("encoding:"), LP_ENCODING },
		{ _T("By:"), LP_BY },
		{ _T("Offset:"), LP_OFFSET },
		{ NULL, LP_ARTIST },
	};

	return _LyricsPropName;
}
