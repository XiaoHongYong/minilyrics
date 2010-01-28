/********************************************************************
	Created :	2001/12/19    22:49
	FileName:	mldata.h
	Author  :	xhy
	Purpose :	各种歌词格式的歌词数据存储方式
*********************************************************************/

#if !defined(AFX_MLDATA_H__89324577_421C_48B2_B812_3D645089F044__INCLUDED_)
#define AFX_MLDATA_H__89324577_421C_48B2_B812_3D645089F044__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../ID3Lib/ID3v2.h"
#include "../MLProtocol//MLUtils.h"

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
	// bool IsTempTimeBeg() { }

}LYRIC_FRAG, *PLYRIC_FRAG, *LPLYRIC_FRAG;

typedef vector<LPLYRIC_FRAG>		V_FRAG;

typedef enum ML_ROW_TYPE
{
	ML_ROW_LYRICS,		// 歌词行
	ML_ROW_UNKNOW,		// 不能识别的行
	ML_ROW_NULL,		// 空行
}ML_ROW_TYPE;

// 歌词中的每一行，在 ML 文件中，它表示歌词中的一行
class ML_ROW
{
public:
	ML_ROW()
	{
		szContent = NULL;
		// nLen = 0;
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


// txt < lrc < karaoke
// precision of timestamps is higher
enum LYRICS_CONTENT_TYPE
{
	LCT_UNKNOWN,
	LCT_TXT,
	LCT_LRC,
	LCT_KARAOKE,		// karaoke content, full syncd.
};

MLFileType LyricsConentTypeToFileType(LYRICS_CONTENT_TYPE lyrContentType);

struct LyricsProperties
{
	LyricsProperties()
	{
		m_nTimeOffset = 0;
		m_nEncodingID = ED_SYSDEF;
	}
	bool Equal(const LyricsProperties &other) const
	{
		return m_strArtist == other.m_strArtist &&
			m_strTitle == other.m_strTitle &&
			m_strAlbum == other.m_strAlbum &&
			m_strBy == other.m_strBy &&
			m_nTimeOffset == other.m_nTimeOffset &&
			m_nEncodingID == other.m_nEncodingID; 
	}

	void clear()
	{
		m_strTitle.resize(0);
		m_strArtist.resize(0);
		m_strAlbum.resize(0);
		m_strBy.resize(0);
		m_strEncoding.resize(0);
		m_nTimeOffset = 0;
		m_nEncodingID = ED_SYSDEF;
	}

	CHAR_ENCODING GetEncodingIndex() { return m_nEncodingID; }
	LPCTSTR GetEncoding() const { return m_strEncoding.c_str(); }
	void SetEncoding(CHAR_ENCODING nEncodingID);
	void SetEncoding(LPCTSTR szEncoding);

	tstring					m_strArtist;
	tstring					m_strTitle;
	tstring					m_strAlbum;
	tstring					m_strBy;
	// offset time of timestamps in lyrics, in ms
	int						m_nTimeOffset;
protected:
	// Character encoding
	tstring					m_strEncoding;
	CHAR_ENCODING			m_nEncodingID;

};

class CMLData  
{
public:
	enum LYRICS_PROP
	{
		LP_NONE,
		LP_ARTIST,
		LP_ALBUM,
		LP_TITLE,
		LP_ENCODING,
		LP_BY,
		LP_OFFSET,
		LP_TIME_TAG,
	};

	struct LYRICS_PROP_NAME
	{
		LPCTSTR			szName;
		LYRICS_PROP		prop;
	};

public:
	CMLData();
	virtual ~CMLData();

	// 打开歌词文件
	int OpenLyrics(LPCTSTR szSongFile, int nMediaLength, LPCTSTR szLrcSource, bool bForceUseEncoding = FALSE, CHAR_ENCODING nForceEncodingID = ED_SYSDEF);
	int ReopenLyrics(bool bForceUseEncoding, CHAR_ENCODING nForceEncodingID);

	int SaveAsFile(LPCTSTR szFile, FILE_ENCODING fileEncoding, bool &bUseNewFileName);
	int Save();
	
	int SaveToTextFile(LPCTSTR szFile, bool bSaveErrorInfo = false);
// 	int SaveLyricsInSongOfID3v2USLT(FILE_ENCODING fileEncoding, CHAR_ENCODING nEncodingID);
// 	int SaveLyricsInSongOfID3v2SYLT(FILE_ENCODING fileEncoding, CHAR_ENCODING nEncodingID);
	int SaveLyricsInSongOfID3v2(FILE_ENCODING fileEncoding, bool bUslt, bool bSylt, bool bUserDefinedLyr, CHAR_ENCODING nEncodingID);
	int SaveLyricsInSongOfLyrics3v2(FILE_ENCODING fileEncoding, CHAR_ENCODING nEncodingID);

	BOOL ToString(tstring &str, MLFileType lrcFormat = FT_LYRICS_LRC, BOOL bIncTags = FALSE);
	int FromString(LPCVOID lpLrc, int nLen, FILE_ENCODING *fileEncoding, MLFileType lrcFormat = FT_LYRICS_LRC, BOOL bStrict = TRUE, BOOL bUnicode = FALSE, bool bForceUseEncoding = FALSE, CHAR_ENCODING nForceEncodingID = ED_SYSDEF);


	int GetPlayElapsedTime();
	void SetPlayElapsedTime (int nTimeElapsed);

	int GetCurPlayLine(const VLyrLines &lyrLines);

	// 访问歌词行
	int GetLyricRowCount();
	PML_ROW GetLyricsRow(int nRow);

	VLyrLines &GetRawLyrics();

	void CopyMLyrLines(CMLyrLines &lyrLines);

	void ClearTimeTagRromRow(int nRow);

	// 设置缺省的显示风格
	void SetDefShowStyle(DWORD dwDefShowStyle) { m_dwDefShowStyle = dwDefShowStyle; }
	DWORD GetDefShowStyle() { return m_dwDefShowStyle; }
	bool IsKaraokeShowStyle() { return m_dwDefShowStyle == LSA_SHOW_KARAOKE; }

	// 检查歌词数据的时间顺序的正确性
	void CheckLyricsRowTime();

	void SetLyricsRowEndTime (int nRow, int nTime);

	// 设置从当前行开始的偏移时间
	void SetOffsetTimeFromCurrentRow(int nRow, int nOffsetTime);

	// 设置歌词段的开始显示时间，在设置之后并且自动校正前面和后面设置相冲突的时间
	void SetLyricsFragEndTime(int nRow, int nFrag, int nTime);
	// 设置歌词段的结束显示时间，在设置之后并且自动校正前面和后面设置相冲突的时间
	void SetLyricsFragBeginTime(int nRow, int nFrag, int nTime);

	// 计算两个歌词段之间的所有歌词段开始时间和结束时间的值
	BOOL CalFragTimeBetweenFrags(int nRow, int nBegFrag, int nEndFrag, int nBegTime, int nEndTime);

	// 计算两个歌词行之间的所有歌词行的开始时间和结束时间的值
	BOOL CalFragTimeBetweenRows(int nBegRow, int nBegFrag, int nEndRow, int nEndFrag, int nBegTime, int nEndTime);

	PLYRIC_FRAG NewLyricFrag(int nBegTime, int nEndTime, 
		const char * szLyrics, int nLen, BOOL bTempBegTime, BOOL bTempEndTime, DWORD dwStyle = LSA_SHOW_DEFAULT);

	PLYRIC_FRAG NewLyricFrag(int nBegTime, int nEndTime, 
		const WCHAR * szLyrics, int nLen, BOOL bTempBegTime, BOOL bTempEndTime, DWORD dwStyle = LSA_SHOW_DEFAULT);

	PML_ROW NewMLRow(int nBegTime, int nEndTime, 
		const char *szContent = NULL, int nLen = 0, ML_ROW_TYPE rowType = ML_ROW_LYRICS);

	PML_ROW NewMLRow(int nBegTime, int nEndTime, 
		const WCHAR *szContent = NULL, int nLen = 0, ML_ROW_TYPE rowType = ML_ROW_LYRICS);

	static PML_ROW CopyMLRow(PML_ROW pRowSrc);

	// 添加歌词行
	void AddRow(PML_ROW pFileRow);
	void AddRowByTime(PML_ROW pRow);

	void AddTextInLyrics(V_Str &vText, DWORD dwLyrFlagAttr = LSA_SHOW_SIMPLE);

	bool RemoveLyricsRow(int nRow);

	void AddAdInLyrics(int nBegTimeOffset, LPCTSTR szText, LPCTSTR szLink);

	int FilterContents(void);

	BOOL LyricsRowToText(PML_ROW pRow, tstring &strLyricsRow);

public:
	LPCTSTR GetLyricsFileName();
	LPCTSTR GetSongFileName() { return m_strSongFile.c_str(); }
	int GetLyricsFileServerMark(TCHAR szMd5[]);

	FILE_ENCODING GetFileEncoding() { return m_fileEncoding; }

	void GetProperty(LYRICS_PROP &prop, tstring &strValue);
	void SetProperty(LYRICS_PROP &prop, LPCTSTR szValue);
	LyricsProperties &Properties() { return m_lyrProperties; }
	static LYRICS_PROP_NAME* GetSupportedLyricsPropName();

	CHAR_ENCODING GetEncodingIndex() { return m_lyrProperties.GetEncodingIndex(); }
	LPCTSTR GetEncoding() { return m_lyrProperties.GetEncoding(); }
	void SetEncoding(CHAR_ENCODING nEncodingID) { m_lyrProperties.SetEncoding(nEncodingID); }
	void SetEncoding(LPCTSTR szEncoding) { m_lyrProperties.SetEncoding(szEncoding); }

	LRC_SOURCE_TYPE GetLyricsSourceType() { return m_lrcSourceType; }

	//		取得当前文件类型
	MLFileType GetLyricsFileType() { return m_LyricFileType; }

	LYRICS_CONTENT_TYPE GetLyrContentType() { return m_lyrConentType; }
	// void SetLyrContentType(LYRICS_CONTENT_TYPE lyrContentType)	{ m_lyrConentType = lyrContentType; }

	bool DoesChooseNewFileName();

	// 取得偏移时间量
	int GetOffsetTime() { return m_lyrProperties.m_nTimeOffset; }
	void SetOffsetTime(int nOffsetTime) { m_lyrProperties.m_nTimeOffset = nOffsetTime; }

	int GetMediaLength() { return m_nMediaLength; }

	BOOL HasLyricsOpened();
	// bool IsEncodingModified();
	BOOL IsModified();
	BOOL IsContentModified();
	void Close();
	void ClearLyrics();

protected:
	// int OpenLyricsFile(LPCTSTR szFile, bool bForceUseEncoding = FALSE, CHAR_ENCODING nForceEncodingID = ED_SYSDEF);

	int OpenLyricsFile(LPCTSTR szFile, bool bForceUseEncoding = FALSE, CHAR_ENCODING nForceEncodingID = ED_SYSDEF);
	int OpenLyricsInSongOfID3v2USLT(ID3v2UnsynchLyrics &lyrics, bool bForceUseEncoding = FALSE, CHAR_ENCODING nForceEncodingID = ED_SYSDEF);
	int OpenLyricsInSongOfID3v2SYLT(ID3v2SynchLyrics &lyrics, bool bForceUseEncoding = FALSE, CHAR_ENCODING nForceEncodingID = ED_SYSDEF);
	int OpenLyricsInSongOfLyrics3v2(const char *buffer, int len, bool bForceUseEncoding = FALSE, CHAR_ENCODING nForceEncodingID = ED_SYSDEF);

	bool ToID3v2SynchLyrics(ID3v2SynchLyrics &lyrics, FILE_ENCODING fileEncoding, CHAR_ENCODING nEncodingID);
	bool ToID3v2UnsynchLyrics(ID3v2UnsynchLyrics &lyrics, FILE_ENCODING fileEncoding, CHAR_ENCODING nEncodingID);

	int UpdateTagToID3v2DescTxt();
	BOOL SaveTag();

	void ResetModificationFlag();

	// 取得下一个“歌词段”
	PLYRIC_FRAG NextFrag(int nRow, int nFrag);
	BOOL NextFrag(int nRow, int nFrag, int &nNextRow, int &nNextFrag, PLYRIC_FRAG *ppFrag = NULL);

	// 取得上一个“歌词段”
	PLYRIC_FRAG PrevFrag(int nRow, int nFrag);
	BOOL PrevFrag(int nRow, int nFrag, int &nPrevRow, int &nPrevFrag, PLYRIC_FRAG *ppFrag = NULL);

public:

	// 歌词文件的歌词部分，包含了不能识别的歌词、数据、注释等。
	// ML_ROW_LYRICS 之外的其他行都存在此处。
	CMLyrLines				m_arrFileRows;

	int						m_nGlobalOffsetTime;

protected:
	// Only lyrics lines are saved here, it's just a sub copy of m_arrFileRows.
	CMLyrLines				m_arrLyricsRows;

protected:
	// save id3v2 lyrics infomation for cache.
	struct ID3v2LyricsSaveCacheInfo
	{
		ID3v2LyricsSaveCacheInfo();
		tstringex		m_strLrcDescription;	// save id3v2 lyrics description
		string			m_strLanguage;

		bool			m_bAllSyllableIsNewLine;
		void clear();
	};
	ID3v2LyricsSaveCacheInfo	m_id3v2SaveCacheInfo;

	// 是否有歌词内容？
	BOOL					m_bHasLyrics;

	// 文件编码：ansi unicode？
	FILE_ENCODING			m_fileEncoding;

	// 歌词文件
	tstring					m_strLrcSource;
	tstring					m_strSongFile;

	// lyrics source type: file, id3v2, lyrics3v2
	LRC_SOURCE_TYPE			m_lrcSourceType;

	// lyrics file type: txt, lrc, etc
	MLFileType				m_LyricFileType;

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

	// current playing pos of lyrics
	int						m_nTimeElapsed;

	// length of the current media file
	int						m_nMediaLength;

};

#endif // !defined(AFX_MLDATA_H__89324577_421C_48B2_B812_3D645089F044__INCLUDED_)
