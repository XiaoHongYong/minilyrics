#pragma once

#include "../ID3Lib/ID3v1.h"
#include "ID3v2IF.h"
#include "M4aTag.h"

enum LRC_SOURCE_TYPE
{
	LST_NONE			= 0,
	LST_UNKNOWN			= 0x1 << 1,
	LST_FILE			= 0x1 << 2,
	LST_ID3V2_SYLT		= 0x1 << 3,
	LST_ID3V2_USLT		= 0x1 << 4,
	LST_LYRICS3V2		= 0x1 << 5,
	LST_ID3V2_LYRICS	= 0x1 << 6,
	LST_M4A_LYRICS		= 0x1 << 7,		// Embedded lyrics in M4A file.
	LST_KAR_LYRICS		= 0x1 << 8,		// Embedded lyrics in KAR file.

	LST_ID3V2			= LST_ID3V2_SYLT | LST_ID3V2_USLT | LST_ID3V2_LYRICS,
};

LRC_SOURCE_TYPE LyrSrcTypeFromName(LPCTSTR szLrcSource);
LPCTSTR LyrSrcTypeToName(LRC_SOURCE_TYPE lst);
LPCTSTR LyrSrcTypeToDesc(LRC_SOURCE_TYPE lst);

#define SZ_SONG_ID3V2_SYLT		_T("song://id3v2/sylt/")
#define SZ_SONG_ID3V2_USLT		_T("song://id3v2/uslt/")
#define SZ_SONG_LYRICS3V2		_T("song://lyrics3v2")
#define SZ_SONG_ID3V2_LYRICS	_T("song://id3v2/lyrics(nonstandard)")
#define SZ_SONG_M4A_LYRICS		_T("song://m4a/lyrics")
#define SZ_SONG_KAR_LYRICS		_T("song://kar/lyrics")

#define NONE_LYRCS	_T("")

#define SZ_SONG_ID3V2_SYLT_DESC	_TLM("Embedded lyrics for Windows Media Player (ID3v2 synchronized lyrics)")
#define SZ_SONG_ID3V2_USLT_DESC	_TLM("Embedded lyrics for iPod, iTunes (ID3v2 unsynchronized lyrics)")
#define SZ_SONG_LYRICS3V2_DESC	_TLM("Embedded lyrics for MiniLyrics (Lyrics3v2 synchronized lyrics)")
#define SZ_SONG_ID3V2_LYRICS_DESC	_TLM("Embedded ID3v2 lyrics (Nonstandard)")
#define SZ_SONG_M4A_LYRICS_DESC	_TLM("Embedded lyrics for iPod, iTunes (For &AAC (M4A) file)")
#define SZ_SONG_KAR_LYRICS_DESC	_TLM("Embedded lyrics for Karaoke (KAR file)")
#define SZ_SONG_NO_LYRICS_DESC	_TLM("No suitable lyrics for the song file")

enum TagType
{
	TT_ID3V1,
	TT_ID3V2,
	TT_APE,
	TT_M4A,
};

class TagFields
{
public:
	tstring		strName;
	tstring		strValue;

};
typedef list<TagFields>		ListTagFields;

class CTag
{
public:
	ListTagFields		m_listFields;
	TagType				m_tagType;
	bool				m_bRemovable;		// Can the tag be removed
	bool				m_bEmpty;			// Is this an empty tag?

};

class CListTags: public list<CTag *>
{
public:
	CListTags();
	virtual ~CListTags();

};


class CTagLib
{
public:
	CTagLib(void);
	virtual ~CTagLib(void);

public:
	// pMediaLength: in ms
	static int GetTagFast(LPCTSTR szFile, tstring *pArtist, tstring *pTitle, tstring *pAlbum,
		tstring *pYear, tstring *pGenre, tstring *pTrackNo, uint32 *pMediaLength);

	static int GetAllTags(LPCTSTR szFile, CListTags &listTags);

	static bool IsMediaTypeSupported(LPCTSTR szFile);

	static int GetEmbeddedLyrics(LPCTSTR szFile, VecString &vLyricsNames);

	static bool IsEmbeddedLyricsSupported(LPCTSTR szFile);
	static bool IsID3v2TagSupported(LPCTSTR szFile);
	static bool IsM4aTagSupported(LPCTSTR szFile);
	static bool IsKarTagSupported(LPCTSTR szFile);

	static int RemoveEmbeddedLyrics(LPCTSTR szMediaFile, VecString &vLyrNamesToRemove, int *succeededCount = NULL);

};

