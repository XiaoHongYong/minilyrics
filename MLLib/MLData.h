/********************************************************************
	Created :	2001/12/19    22:49
	FileName:	mldata.h
	Author  :	xhy
	Purpose :	各种歌词格式的歌词数据存储方式
*********************************************************************/

#if !defined(AFX_MLDATA_H__89324577_421C_48B2_B812_3D645089F044__INCLUDED_)
#define AFX_MLDATA_H__89324577_421C_48B2_B812_3D645089F044__INCLUDED_

#pragma once


DECLAR_CPPUNIT_TEST_REG(CMLData)


#ifndef _IPHONE
#define _ID3V2_SUPPORT
#endif

#ifdef _ID3V2_SUPPORT
#include "../ID3Lib/ID3v2.h"
#endif

#include "../MLProtocol/MLUtils.h"
#include "LrcTag.h"

///////////////////////////////////////////////////////////////////////////////
// BEGIN 歌词数据的数据结构定义
typedef enum LYRIC_FRAG_ATTRIBUTE
{
	// LYRIC_FRAG::dwAttrib 的值的描述

	// 前面的3位（1 - 3)为显示风格
	LSA_SHOW_DEFAULT = 0,			// 缺省的显示风格，为：LSA_SHOW_SIMPLE
	LSA_SHOW_SIMPLE = 1,			// 简单的显示风格
	LSA_SHOW_KARAOKE = 2,			// 逐字显示风格

	// 第4位开始时间是否为临时的标志
	LSA_TEMP_TIME_BEG = 0x1 << 3,		// 开始时间是否为临时计算出的？

	// 第5位结束时间是否为临时的标志
	LSA_TEMP_TIME_END = 0x1 << 4,		// 结束时间是否为临时计算出的？

	// 第6位为临时歌词位：如插入的广告、注册提示等临时信息
	LSA_TEMP_FRAG		= 0x1 << 5,
	// 第7位为提示信息标志位
	LSA_ERROR_INFO		= 0x1 << 6,
	LSA_AD_FRAG			= 0x1 << 7,
}LYRIC_FRAG_ATTRIBUTE;

#define GetShowStyle(dwAttrib)		(dwAttrib & 0x7)

#define SetShowStyle(dwAttrib, dwStyle)		(dwAttrib = (dwAttrib & (~0x7)) | dwStyle)

#define IsValidShowStyle(dwAttrib)		((dwAttrib == LSA_SHOW_DEFAULT) || (dwAttrib == LSA_SHOW_SIMPLE) || (dwAttrib == LSA_SHOW_KARAOKE))

#define IsTempTimeBeg(dwAttrib)		((dwAttrib & LSA_TEMP_TIME_BEG) == LSA_TEMP_TIME_BEG)

#define IsTempTimeEnd(dwAttrib)		((dwAttrib & LSA_TEMP_TIME_END) == LSA_TEMP_TIME_END)

#define IsTempLyricsFrag(dwAttrib)		((dwAttrib & LSA_TEMP_FRAG) == LSA_TEMP_FRAG)

typedef struct LYRIC_FRAG
{
	DWORD				dwAttrib;		// 属性，表示此段数据的信息LYRIC_FRAG_ATTRIBUTE
	int					nBegTime;		// begin time
	int					nEndTime;		// end time
	int16				nDrawWidth;		// 显示出来的宽度
	int16				nLen;			// length of szLyric
	TCHAR				szLyric[1];
	LYRIC_FRAG()
	{
		nDrawWidth = 0;
		nLen = 0;
	}
	bool IsAdFrag() { return (dwAttrib & LSA_AD_FRAG) == LSA_AD_FRAG; }
	bool IsTempFrag() { return IsTempLyricsFrag(dwAttrib); }
	bool IsTempBegTime() { return IsTempTimeBeg(dwAttrib); }

}LYRIC_FRAG, *PLYRIC_FRAG, *LPLYRIC_FRAG;

typedef vector<LPLYRIC_FRAG>		V_FRAG;

typedef enum ML_ROW_TYPE
{
	ML_ROW_LYRICS,		// 歌词行
	ML_ROW_UNKNOW,		// 不能识别的行
}ML_ROW_TYPE;

// 歌词中的每一行，在 ML 文件中，它表示歌词中的一行
class ML_ROW
{
public:
	ML_ROW()
	{
		szContent = NULL;
	}
	~ML_ROW()
	{
		if (szContent)
			delete szContent;

		for (int k = 0; k < (int)vFrags.size(); k++)
		{
			delete[] (BYTE*)vFrags[k];
		}
	}
	int						nRowType;			// 表示此行的类型，必须是ML_ROW_TYPE中定义的一种类型。

	// 当 nRowType == ML_ROW_LYRICS 时使用以下成员
	vector<PLYRIC_FRAG>		vFrags;			// words in the row
	int						nBegTime;		// begin time
	int						nEndTime;		// end time

	// 当 nRowType == ML_ROW_UNKNOWN 时使用以下成员
	// int					nLen;			// length of szContent
	TCHAR					*szContent;		// content of the row

	bool IsTempBegTime()
	{
		if (vFrags.size())
			return vFrags[0]->IsTempBegTime();

		return false;
	}

	bool IsAdRow()
	{
		if (vFrags.size())
			return vFrags[0]->IsAdFrag();

		return false;
	}
	bool IsTempRow()
	{
		if (vFrags.size())
			return vFrags[0]->IsTempFrag();

		return false;
	}
};

typedef ML_ROW*		PML_ROW;
typedef ML_ROW*		LPML_ROW;

typedef vector<PML_ROW>		VLyrLines;

class CMLyrLines : public VLyrLines
{
public:
	~CMLyrLines()
	{
		clear();
	}

	void clear()
	{
		iterator	it, itEnd;
		itEnd = end();
		for (it = begin(); it != itEnd; ++it)
		{
			PML_ROW		pRow = *it;
			delete pRow;
		}
		vector<PML_ROW>::clear();
	}

	void ClearDrawContextWidth()
	{
		iterator	it, itEnd;
		itEnd = end();
		for (it = begin(); it != itEnd; ++it)
		{
			PML_ROW		pRow = *it;
			for (int k = 0; k < (int)pRow->vFrags.size(); k++)
			{
				pRow->vFrags[k]->nDrawWidth = 0;
			}
		}
	}

};


enum LRC_SOURCE_TYPE
{
	LST_NONE			= 0,
	LST_UNKNOWN			= 0x1 << 1,
	LST_FILE			= 0x1 << 2,
	LST_ID3V2_SYLT		= 0x1 << 3,
	LST_ID3V2_USLT		= 0x1 << 4,
	LST_LYRICS3V2		= 0x1 << 5,
	LST_ID3V2_LYRICS	= 0x1 << 6,
};

LRC_SOURCE_TYPE GetLrcSourceType(LPCTSTR szLrcSource);

// END 歌词数据的数据结构定义
///////////////////////////////////////////////////////////////////////////////

MLFileType LyricsConentTypeToFileType(LYRICS_CONTENT_TYPE lyrContentType);

#include "LyrScrollActionRecorder.h"

class CMLData  
{
public:
	CMLData();
	virtual ~CMLData();

	// 打开歌词文件
	int OpenLyrics(LPCTSTR szSongFile, int nMediaLength, LPCTSTR szLrcSource, bool bUseSpecifiedEncoding = FALSE, CHAR_ENCODING encodingSpecified = ED_SYSDEF);
#ifdef _IPHONE
	int OpenLyrics(LPCTSTR szSongFile, int nMediaLength, LPCTSTR szLyrContent);
#endif
	int ReopenLyrics(bool bUseSpecifiedEncoding, CHAR_ENCODING encodingSpecified);

	int SaveAsFile(LPCTSTR szFile, CHAR_ENCODING newEncoding, bool &bUseNewFileName);
	int Save();
	
	int SaveToTextFile(LPCTSTR szFile);
#ifdef _ID3V2_SUPPORT
	int SaveLyricsInSongOfID3v2(bool bUslt, bool bSylt, bool bUserDefinedLyr);
	int SaveLyricsInSongOfLyrics3v2(CHAR_ENCODING encoding);
#endif

	BOOL ToString(tstring &str, MLFileType lrcFormat = FT_LYRICS_LRC, bool bIncTags = FALSE);
	// int FromString(LPCVOID lpLrc, int nLen, bool bAddTimeStampForTextLine = false, bool bUseSpecifiedEncoding = FALSE, CHAR_ENCODING encodingSpecified = ED_SYSDEF);
	int FromString(LPCTSTR szLyrics);


	int GetPlayElapsedTime();
	void SetPlayElapsedTime (int nTimeElapsed);

	int GetCurPlayLine(const VLyrLines &lyrLines);

	int GetLyricRowCount() { return m_arrLyricsRows.size(); }
	PML_ROW GetLyricsRow(int nRow);
	VLyrLines &GetRawLyrics();

	void CopyMLyrLines(CMLyrLines &lyrLines);

	// 设置缺省的显示风格
	void SetDefShowStyle(DWORD dwDefShowStyle) { m_dwDefShowStyle = dwDefShowStyle; }
	bool IsKaraokeShowStyle() { return m_dwDefShowStyle == LSA_SHOW_KARAOKE; }

	// 检查歌词数据的时间顺序的正确性
	void CheckLyricsRowTime();

	void FixExtendedLrcTimeStamps();

	// 计算两个歌词段之间的所有歌词段开始时间和结束时间的值
	BOOL CalFragTimeBetweenFrags(int nRow, int nBegFrag, int nEndFrag, int nBegTime, int nEndTime);

	// 计算两个歌词行之间的所有歌词行的开始时间和结束时间的值
	BOOL CalFragTimeBetweenRows(int nBegRow, int nBegFrag, int nEndRow, int nEndFrag, int nBegTime, int nEndTime);

	PLYRIC_FRAG NewLyricFrag(int nBegTime, int nEndTime, 
		const char * szLyrics, int nLen, BOOL bTempBegTime, BOOL bTempEndTime, DWORD dwStyle = LSA_SHOW_DEFAULT);

	PLYRIC_FRAG NewLyricFrag(int nBegTime, int nEndTime, 
		const WCHAR * szLyrics, int nLen, BOOL bTempBegTime, BOOL bTempEndTime, DWORD dwStyle = LSA_SHOW_DEFAULT);

	PML_ROW NewMLRow(int nBegTime, int nEndTime, 
		LPCTSTR szContent = NULL, int nLen = 0, ML_ROW_TYPE rowType = ML_ROW_LYRICS);

	static PML_ROW CopyMLRow(PML_ROW pRowSrc);

	// 添加歌词行
	void AddRow(PML_ROW pFileRow);
	void AddRowByTime(PML_ROW pRow);

	void AddTextInLyrics(V_Str &vText, DWORD dwLyrFlagAttr = LSA_SHOW_SIMPLE);

	bool RemoveLyricsRow(int nRow);

	void AddAdInLyrics(int nBegTimeOffset, LPCTSTR szText, LPCTSTR szLink);

	BOOL LyricsRowToText(PML_ROW pRow, tstring &strLyricsRow);

	int FilterContents(void);

public:
	//
	// Interface for Lyrics Scroll Action Recorder (CLyrScrollActionRecorder).
	//
	void ClearRecordedLyrScrollActions(VLyrLines &lyrLines) {
		ASSERT(GetLyrContentType() == LCT_TXT);
		m_lyrScrollActionRecorder.InitCreateAutoEvents(lyrLines);
		m_lyrScrollActionRecorder.InitCreateAutoEvents(GetRawLyrics());
	}

	bool IsUsingLyrScrollActionRecorder()
		{ return GetLyrContentType() == LCT_TXT && m_lyrScrollActionRecorder.HasActions(); }

	void UpdateTimeTagByLyrScrollActions(VLyrLines &lyrLines)
		{ ASSERT(GetLyrContentType() == LCT_TXT); m_lyrScrollActionRecorder.UpdateTimeTagByEvents(lyrLines); }

	void LyrScrollActionsToTag() {
		if (GetLyrContentType() == LCT_TXT)
			m_lyrProperties.SetTxtLyrScrollEvents(m_lyrScrollActionRecorder.EventsDataToString().c_str());
	}

	void LyrScrollToLine(VLyrLines &lyrLines, int nLine, int nTime, bool bUpdateTimeTag = true)
	{
		ASSERT(GetLyrContentType() == LCT_TXT);
		m_lyrScrollActionRecorder.ScrollToLine(lyrLines, nLine, nTime, bUpdateTimeTag);
	}

	bool IsOnlyAddLyrScrollActions();

public:
	LPCTSTR GetLyricsFileName();
	LPCTSTR GetSongFileName() { return m_strSongFile.c_str(); }

	LyricsProperties &Properties() { return m_lyrProperties; }

	void SetTxtLyrScrollEvents(LPCTSTR szScrEvents) { m_lyrProperties.SetTxtLyrScrollEvents(szScrEvents); }
	LPCTSTR GetTxtLyrScrollEvents() { return m_lyrProperties.GetTxtLyrScrollEvents(); }

	LRC_SOURCE_TYPE GetLyricsSourceType() { return m_lrcSourceType; }

	LYRICS_CONTENT_TYPE GetLyrContentType() { return m_lyrConentType; }

	bool DoesChooseNewFileName();

	// 取得偏移时间量
	int GetOffsetTime() { return m_lyrProperties.GetOffsetTime(); }
	void SetOffsetTime(int nOffsetTime) { m_lyrProperties.SetOffsetTime(nOffsetTime); }

	int GetMediaLength() { return m_nMediaLength; }

	BOOL HasLyricsOpened();
	BOOL IsModified();
	BOOL IsContentModified();
	void Close();
	void ClearLyrics();

protected:
	int OpenLyricsFile(LPCTSTR szFile, bool bUseSpecifiedEncoding = FALSE, CHAR_ENCODING encodingSpecified = ED_SYSDEF);
#ifdef _ID3V2_SUPPORT
	int OpenLyricsInSongOfID3v2USLT(ID3v2UnsynchLyrics &lyrics, bool bUseSpecifiedEncoding = FALSE, CHAR_ENCODING encodingSpecified = ED_SYSDEF);
	int OpenLyricsInSongOfID3v2SYLT(ID3v2SynchLyrics &lyrics, bool bUseSpecifiedEncoding = FALSE, CHAR_ENCODING encodingSpecified = ED_SYSDEF);
	int OpenLyricsInSongOfLyrics3v2(const char *buffer, int len, bool bUseSpecifiedEncoding = FALSE, CHAR_ENCODING encodingSpecified = ED_SYSDEF);

	bool ToID3v2SynchLyrics(ID3v2SynchLyrics &lyrics, CHAR_ENCODING encoding);
	bool ToID3v2UnsynchLyrics(ID3v2UnsynchLyrics &lyrics, CHAR_ENCODING encoding);

	int UpdateTagToID3v2DescTxt();
#endif // #ifdef _ID3V2_SUPPORT
	BOOL SaveTag();

	void ResetModificationFlag();

	// 取得下一个“歌词段”
	PLYRIC_FRAG NextFrag(int nRow, int nFrag);
	BOOL NextFrag(int nRow, int nFrag, int &nNextRow, int &nNextFrag, PLYRIC_FRAG *ppFrag = NULL);

	// 取得上一个“歌词段”
	PLYRIC_FRAG PrevFrag(int nRow, int nFrag);
	BOOL PrevFrag(int nRow, int nFrag, int &nPrevRow, int &nPrevFrag, PLYRIC_FRAG *ppFrag = NULL);

	CHAR_ENCODING GetEncodingIndex() { return m_lyrProperties.GetEncodingIndex(); }

public:

	// 歌词文件的歌词部分，包含了不能识别的歌词、数据、注释等。
	// ML_ROW_LYRICS 之外的其他行都存在此处。
	CMLyrLines				m_arrFileRows;

	int						m_nGlobalOffsetTime;

protected:
	// Only lyrics lines are saved here, it's just a sub copy of m_arrFileRows.
	CMLyrLines				m_arrLyricsRows;
	bool					m_bHasLyricsOpened;

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

	// lyrics content type: txt, lrc, kar?
	LYRICS_CONTENT_TYPE		m_lyrConentType;

	// original lyrics info.
	TCHAR					m_szLyrContentMd5Org[33];	// used for lyrics modification compare.
	LYRICS_CONTENT_TYPE		m_lyrConentTypeOrg;
	LyricsProperties		m_lyrPropertiesOrg;

	// lyrics display style
	DWORD					m_dwDefShowStyle;

	// current playing position of lyrics
	int						m_nTimeElapsed;

	// length of the current media file
	int						m_nMediaLength;

};

#endif // !defined(AFX_MLDATA_H__89324577_421C_48B2_B812_3D645089F044__INCLUDED_)
