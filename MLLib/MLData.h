/********************************************************************
	Created :	2001/12/19    22:49
	FileName:	mldata.h
	Author  :	xhy
	Purpose :	各种歌词格式的歌词数据存储方式
*********************************************************************/

#pragma once

DECLAR_CPPUNIT_TEST_REG(CMLData)


#ifndef _IPHONE
#define _ID3V2_SUPPORT
#endif

#include "../TagLib/TagLib.h"
#include "../TagLib/LyricsData.h"

#include "../MLProtocol/MLUtils.h"
#include "LrcTag.h"

#include "LyrScrollActionRecorder.h"

MLFileType LyricsConentTypeToFileType(LYRICS_CONTENT_TYPE lyrContentType);

class CMLData  
{
public:
	CMLData();
	virtual ~CMLData();

	int NewLyrics(LPCTSTR szSongFile, int nMediaLength);

	int OpenLyrics(LPCTSTR szSongFile, int nMediaLength, LPCTSTR szLrcSource, bool bUseSpecifiedEncoding = FALSE, CHAR_ENCODING encodingSpecified = ED_SYSDEF);
	int OpenLyrics(LPCTSTR szSongFile, int nMediaLength, LPBYTE szLrc, int nLen);
#ifdef _IPHONE
	int OpenLyricsContent(LPCTSTR szSongFile, int nMediaLength, LPCTSTR szLyrContent);
#endif
	int ReopenLyrics(bool bUseSpecifiedEncoding, CHAR_ENCODING encodingSpecified);

	int SaveAsFile(LPCTSTR szFile, CHAR_ENCODING newEncoding, bool &bUseNewFileName);
	int Save();

#ifdef _ID3V2_SUPPORT
	int SaveLyricsInSongOfID3v2(VecString &vLyrNames);
	int SaveLyricsInSongOfLyrics3v2();
	int SaveLyricsInSongOfM4a();
#endif

	void ToString(tstring &str, MLFileType lrcFormat = FT_LYRICS_LRC, bool bIncTags = FALSE);
	// int FromString(LPCVOID lpLrc, int nLen, bool bAddTimeStampForTextLine = false, bool bUseSpecifiedEncoding = FALSE, CHAR_ENCODING encodingSpecified = ED_SYSDEF);
	int FromString(LPCTSTR szLyrics);


	int GetPlayElapsedTime();
	void SetPlayElapsedTime (int nTimeElapsed);

	int GetCurPlayLine(const CLyricsLines &lyrLines);

	size_t GetLyricsLinesCount() { return m_arrLyricsLines.size(); }
	LyricsLine *GetLyricsLine(int nLine);
	CLyricsLines &GetRawLyrics() { return m_arrLyricsLines; }
	CLyricsLines &GetFileLines() { return m_arrFileLines; }

	void CopyLyricsLines(CLyricsLines &lyrLines);

	void SetKaraokeStyle(bool bKaraoke = true) { m_bKaraokeStyle = bKaraoke; }
	bool IsKaraokeShowStyle() { return m_bKaraokeStyle; }

	// 检查歌词数据的时间顺序的正确性
	void CheckLyricsLineTime();

	void FixExtendedLrcTimeStamps();

	// 计算两个歌词段之间的所有歌词段开始时间和结束时间的值
	BOOL CalFragTimeBetweenFrags(int nLine, int nBegFrag, int nEndFrag, int nBegTime, int nEndTime);

	// 计算两个歌词行之间的所有歌词行的开始时间和结束时间的值
	BOOL CalFragTimeBetweenLines(int nBegLine, int nBegFrag, int nEndLine, int nEndFrag, int nBegTime, int nEndTime);

	// 添加歌词行
	void AddLine(LyricsLine *pFileLine);
	void AddLineByTime(LyricsLine *pLine);

	void AddTextInLyrics(V_Str &vText);

	bool RemoveLyricsLine(int nLine);

	void AddAdInLyrics(int nBegTimeOffset, LPCTSTR szText, LPCTSTR szLink);

	BOOL LyricsLineToText(LyricsLine *pLine, tstring &strLyricsLine);

	int FilterContents(void);

public:
	//
	// Interface for Lyrics Scroll Action Recorder (CLyrScrollActionRecorder).
	//
	void ClearRecordedLyrScrollActions(CLyricsLines &lyrLines) {
		ASSERT(GetLyrContentType() == LCT_TXT);
		m_lyrScrollActionRecorder.InitCreateAutoEvents(lyrLines);
		m_lyrScrollActionRecorder.InitCreateAutoEvents(GetRawLyrics());
	}

	bool IsUsingLyrScrollActionRecorder()
		{ return GetLyrContentType() == LCT_TXT && m_lyrScrollActionRecorder.HasActions(); }

	void UpdateTimeTagByLyrScrollActions(CLyricsLines &lyrLines)
		{ ASSERT(GetLyrContentType() == LCT_TXT); m_lyrScrollActionRecorder.UpdateTimeTagByEvents(lyrLines); }

	void LyrScrollActionsToTag() {
		if (GetLyrContentType() == LCT_TXT)
			m_lyrProperties.SetTxtLyrScrollEvents(m_lyrScrollActionRecorder.EventsDataToString().c_str());
	}

	void LyrScrollToLine(CLyricsLines &lyrLines, int nLine, int nTime, bool bUpdateTimeTag = true)
	{
		ASSERT(GetLyrContentType() == LCT_TXT);
		m_lyrScrollActionRecorder.ScrollToLine(lyrLines, nLine, nTime, bUpdateTimeTag);
	}

	bool IsOnlyAddLyrScrollActions();

public:
	tstring GetSuggestedLyricsFileName();
	LPCTSTR GetLyricsFileName();
	LPCTSTR GetSongFileName() { return m_strSongFile.c_str(); }

	LyricsProperties &Properties() { return m_lyrProperties; }

	void SetTxtLyrScrollEvents(LPCTSTR szScrEvents) { m_lyrProperties.SetTxtLyrScrollEvents(szScrEvents); }
	LPCTSTR GetTxtLyrScrollEvents() { return m_lyrProperties.GetTxtLyrScrollEvents(); }

	LRC_SOURCE_TYPE GetLyricsSourceType() { return m_lrcSourceType; }

	LYRICS_CONTENT_TYPE GetLyrContentType() { return m_lyrProperties.m_lyrContentType; }
	void SetLyrContentType(LYRICS_CONTENT_TYPE lyrContentType) { m_lyrProperties.m_lyrContentType = lyrContentType; }

	bool DoesChooseNewFileName();

	// 取得偏移时间量
	int GetOffsetTime() { return m_lyrProperties.GetOffsetTime(); }
	void SetOffsetTime(int nOffsetTime) { m_lyrProperties.SetOffsetTime(nOffsetTime); }

	LPCTSTR GetMediaSource() { return m_strSongFile.c_str(); }
	int GetMediaLength() { return m_nMediaLength; }

	BOOL HasLyricsOpened();
	BOOL IsModified();
	BOOL IsContentModified();
	void Close();
	void ClearLyrics();

	CHAR_ENCODING GetEncodingIndex() { return m_lyrProperties.GetEncodingIndex(); }

protected:
	int OpenLyricsFile(LPCTSTR szFile, bool bUseSpecifiedEncoding = FALSE, CHAR_ENCODING encodingSpecified = ED_SYSDEF);
#ifdef _ID3V2_SUPPORT
	int OpenLyricsInSongOfID3v2USLT(ID3v2UnsynchLyrics &lyrics, bool bUseSpecifiedEncoding = FALSE, CHAR_ENCODING encodingSpecified = ED_SYSDEF);
	int OpenLyricsInSongOfID3v2SYLT(ID3v2SynchLyrics &lyrics, bool bUseSpecifiedEncoding = FALSE, CHAR_ENCODING encodingSpecified = ED_SYSDEF);
	int OpenLyricsInSongOfLyrics3v2(const char *buffer, int len, bool bUseSpecifiedEncoding = FALSE, CHAR_ENCODING encodingSpecified = ED_SYSDEF);

	void ToID3v2SynchLyrics(ID3v2SynchLyrics &lyrics, CHAR_ENCODING encoding);
	void ToID3v2UnsynchLyrics(ID3v2UnsynchLyrics &lyrics, CHAR_ENCODING encoding);

	int UpdateTagToID3v2DescTxt();
#endif // #ifdef _ID3V2_SUPPORT
    
	void OnLyricsOpened();
    
	BOOL SaveTag();

	void ResetModificationFlag();

	// 取得下一个“歌词段”
	LyricsPiece *NextFrag(int nLine, int nFrag);
	BOOL NextFrag(int nLine, int nFrag, int &nNextLine, int &nNextFrag, LyricsPiece **ppPiece = NULL);

	// 取得上一个“歌词段”
	LyricsPiece *PrevFrag(int nLine, int nFrag);
	BOOL PrevFrag(int nLine, int nFrag, int &nPrevLine, int &nPrevFrag, LyricsPiece **ppPiece = NULL);

public:

	// All the file lines of the lyrics file.
	CLyricsLines			m_arrFileLines;

	int						m_nGlobalOffsetTime;

protected:
	// Only lyrics lines are saved here, it's just a sub copy of m_arrFileLines.
	CLyricsLines			m_arrLyricsLines;

protected:
	CLyrScrollActionRecorder	m_lyrScrollActionRecorder;

#ifdef _ID3V2_SUPPORT
	// save id3v2 lyrics information for cache.
	struct ID3v2LyricsSaveCacheInfo
	{
		ID3v2LyricsSaveCacheInfo();
		tstringex		m_strLrcDescription;	// save id3v2 lyrics description
		string			m_strLanguage;

		bool			m_bAllSyllableIsNewLine;
		void clear();
	};
	ID3v2LyricsSaveCacheInfo	m_id3v2SaveCacheInfo;
#endif // #ifdef _ID3V2_SUPPORT

	// 歌词文件
	tstring					m_strLrcSource;
	tstring					m_strSongFile;

	// lyrics source type: file, id3v2, lyrics3v2
	LRC_SOURCE_TYPE			m_lrcSourceType;

	// lyrics properties: tag(artist, title...)
	LyricsProperties		m_lyrProperties;

	// original lyrics info.
	TCHAR					m_szLyrContentMd5Org[33];	// used for lyrics modification compare.
	LyricsProperties		m_lyrPropertiesOrg;

	// lyrics display style
	bool					m_bKaraokeStyle;

	// current playing position of lyrics
	int						m_nTimeElapsed;

	// length of the current media file
	int						m_nMediaLength;

};
