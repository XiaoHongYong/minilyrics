#include "StdAfx.h"
#include "TagLib.h"

#include "../ID3Lib/ID3v1.h"
#include "../ID3Lib/ID3v2.h"
#include "../MLLib/Lyrics3v2.h"
#include "MidiTag.h"

#include "MP3InfoReader.h"

#define MIN_EMBEDDED_LYR_SIZE	10		// Minum file size of embedded lyrics

ID_TO_STRING	__id2strLyrSrcType[] = {
	{ LST_ID3V2_SYLT, SZ_SONG_ID3V2_SYLT },
	{ LST_ID3V2_USLT, SZ_SONG_ID3V2_USLT },
	{ LST_LYRICS3V2, SZ_SONG_LYRICS3V2 },
	{ LST_ID3V2_LYRICS, SZ_SONG_ID3V2_LYRICS },
	{ LST_M4A_LYRICS, SZ_SONG_M4A_LYRICS },
	{ LST_KAR_LYRICS, SZ_SONG_KAR_LYRICS },
	{ LST_UNKNOWN, NULL },
};

ID_TO_STRING	__id2strLyrSrcTypeDesc[] = {
	{ LST_ID3V2_SYLT, SZ_SONG_ID3V2_SYLT_DESC },
	{ LST_ID3V2_USLT, SZ_SONG_ID3V2_USLT_DESC },
	{ LST_LYRICS3V2, SZ_SONG_LYRICS3V2_DESC },
	{ LST_ID3V2_LYRICS, SZ_SONG_ID3V2_LYRICS_DESC },
	{ LST_M4A_LYRICS, SZ_SONG_M4A_LYRICS_DESC },
	{ LST_KAR_LYRICS, SZ_SONG_KAR_LYRICS_DESC },
	{ LST_NONE, SZ_SONG_NO_LYRICS_DESC },
	{ LST_UNKNOWN, NULL },
	// { ,  },
};

LRC_SOURCE_TYPE LyrSrcTypeFromName(LPCTSTR szLrcSource)
{
	if (_tcsicmp(szLrcSource, NONE_LYRCS) == 0)
		return LST_NONE;

	for (int i = 0; __id2strLyrSrcType[i].szId != NULL; i++)
	{
		if (StrBeginWith(szLrcSource, __id2strLyrSrcType[i].szId))
			return (LRC_SOURCE_TYPE)__id2strLyrSrcType[i].dwId;
	}

	return LST_FILE;
}

LPCTSTR LyrSrcTypeToName(LRC_SOURCE_TYPE lst)
{
	if (lst == LST_NONE)
		return NONE_LYRCS;
	return IDToString(__id2strLyrSrcType, lst, _T("Unknown"));
}

LPCTSTR LyrSrcTypeToDesc(LRC_SOURCE_TYPE lst)
{
	return IDToString(__id2strLyrSrcTypeDesc, lst, _T("Unknown"));
}

bool GetEmbeddedLyricsNameInfo(LPCTSTR szName, string &language, int &index)
{
	language.clear();
	index = 0;

	if (szName == NULL)
		return true; // No language or index is set.

	szName = szName + _tcslen(_T("song://"));
	VecString vStr;

	StrSplit(szName, '/', vStr);

	if (vStr.size() < 2)
		return false;

// 	type = vStr[0];
// 	subType = vStr[1];

	if (vStr.size() >= 3)
		TStringToUTF8(vStr[2].c_str(), -1, language);

	if (vStr.size() >= 4)
		index = _ttoi(vStr[3].c_str());

	return true;
}


//////////////////////////////////////////////////////////////////////////

CTagLib::CTagLib(void)
{
}

CTagLib::~CTagLib(void)
{
}

bool IsStrInArray(LPCTSTR szArray[], LPCTSTR str)
{
	for (int i = 0; szArray[i] != NULL; i++)
	{
		if (_tcsicmp(szArray[i], str) == 0)
			return true;
	}

	return false;
}

bool CTagLib::IsMediaTypeSupported(LPCTSTR szFile)
{
	LPCTSTR *szSupportedExt = CID3v1::GetSupportedExtArray();
	LPCTSTR szExt = FileGetExt(szFile);

	return IsStrInArray(szSupportedExt, szExt);
}

int CTagLib::GetTagFast(LPCTSTR szFile, tstring *pArtist, tstring *pTitle, tstring *pAlbum,
				  tstring *pYear, tstring *pGenre, tstring *pTrackNo, uint32 *pMediaLength)
{
	FILE	*fp;
	int		nRet;
	fp = _tfopen(szFile, _T("rb"));
	if (!fp)
	{
		SetCustomErrorDesc(CStrPrintf(_T("%s: %s"), OSError().Description(), szFile).c_str());
		return ERR_CUSTOM_ERROR;
	}

	LPCTSTR *szSupportedExt = CID3v1::GetSupportedExtArray();
	LPCTSTR szExt = FileGetExt(szFile);

	// .mp3
	if (IsStrInArray(szSupportedExt, szExt))
	{
		if (pMediaLength)
		{
			CMP3InfoReader	reader;
			reader.Attach(fp);
			*pMediaLength = reader.GetMediaLength();
		}

		CID3v2IF	id3v2(ED_SYSDEF);
		tstring		temp;
		nRet = id3v2.Open(fp, false);
		if (nRet == ERR_OK)
		{
			if (!pArtist) pArtist = &temp;
			if (!pAlbum) pAlbum = &temp;
			if (!pTitle) pTitle = &temp;
			if (!pTrackNo) pTrackNo = &temp;
			if (!pYear) pYear = &temp;
			if (!pGenre) pGenre = &temp;

			id3v2.GetTags(*pArtist, *pTitle, *pAlbum, temp, *pTrackNo, *pYear, *pGenre);
		}

		CID3v1		id3v1;
		ID3V1		v1tag;

		nRet = id3v1.GetTag(fp, &v1tag);
		if (nRet == ERR_OK)
		{
			if (pArtist && pArtist->empty())
				ConvertStr(v1tag.szArtist, -1, *pArtist);
			if (pAlbum && pAlbum->empty())
				ConvertStr(v1tag.szAlbum, -1, *pAlbum);
			if (pTitle && pTitle->empty())
				ConvertStr(v1tag.szTitle, -1, *pTitle);
			if (pYear && pYear->empty())
				ConvertStr(v1tag.szYear, -1, *pYear);
			if (pGenre && pGenre->empty())
				ConvertStr(id3v1.GetGenreDescription(v1tag.byGenre), -1, *pGenre);
			if (pTrackNo && pTrackNo->empty())
				*pTrackNo = CStrPrintf(_T("%d"), v1tag.byTrack).c_str();
		}
	}

	return ERR_OK;
}

int CTagLib::GetEmbeddedLyrics(LPCTSTR szFile, VecString &vLyricsNames)
{
	if (!IsFileExist(szFile))
		return ERR_NOT_FOUND;

	if (IsID3v2TagSupported(szFile))
	{
		// ID3v2 lyrics
		CID3v2IF		id3v2(ED_SYSDEF);
		int				nRet;

		nRet = id3v2.Open(szFile, false, false);
		if (nRet == ERR_OK)
		{
			VecString vLyrics;
			nRet = id3v2.ListLyrics(vLyricsNames);
			id3v2.Close();
			if (nRet != ERR_OK)
				return nRet;
		}
	}

	if (IsID3v2TagSupported(szFile))
	{
		// Lyrics3v2
		CLyrics3v2	lyrics3v2;
		stringex	strLyrics;

		if (lyrics3v2.ReadLyrcsInfo(szFile, strLyrics) == ERR_OK)
		{
			if (strLyrics.size() > MIN_EMBEDDED_LYR_SIZE)
				vLyricsNames.push_back(SZ_SONG_LYRICS3V2);
		}
	}

	if (IsM4aTagSupported(szFile))
	{
		CM4aTag	tag;
		int nRet = tag.Open(szFile, false);
		if (nRet != ERR_OK)
			return nRet;

		nRet = tag.ListLyrics(vLyricsNames);
		if (nRet != ERR_OK)
			return nRet;
	}

	if (IsKarTagSupported(szFile))
	{
		CMidiTag	tag;
		int nRet = tag.Open(szFile);
		if (nRet != ERR_OK)
			return nRet;

		nRet = tag.ListLyrics(vLyricsNames);
		if (nRet != ERR_OK)
			return nRet;
	}

	return ERR_OK;
}

bool CTagLib::IsEmbeddedLyricsSupported(LPCTSTR szFile)
{
	return IsID3v2TagSupported(szFile) || IsM4aTagSupported(szFile);
}

bool CTagLib::IsID3v2TagSupported(LPCTSTR szFile)
{
	return FileIsExtSame(szFile, _T(".mp3"));
}

bool CTagLib::IsM4aTagSupported(LPCTSTR szFile)
{
	return FileIsExtSame(szFile, _T(".m4a"));
}

bool CTagLib::IsKarTagSupported(LPCTSTR szFile)
{
	return FileIsExtSame(szFile, _T(".kar")) || FileIsExtSame(szFile, _T(".mid"));
}

int CTagLib::RemoveEmbeddedLyrics(LPCTSTR szMediaFile, VecString &vLyrNamesToRemove, int *succeededCount)
{
	uint32 uAllLstFlag = 0;
	for (uint i = 0; i < vLyrNamesToRemove.size(); i++)
		uAllLstFlag |= LyrSrcTypeFromName(vLyrNamesToRemove[i].c_str());

	int nRet;

	// Remove lyrics3v2 lyrics
	if (IsFlagSet(uAllLstFlag, LST_LYRICS3V2))
	{
		CLyrics3v2	lyrics3v2;
		nRet = lyrics3v2.ClearLyricsInfo(szMediaFile);
		if (nRet != ERR_OK && nRet != ERR_NOT_FIND_LRC3V2)
			return nRet;
	}

	// Remove ID3v2 embedded lyrics
	if ((uAllLstFlag & LST_ID3V2) > 0)
	{
		CID3v2IF		id3v2(ED_SYSDEF);
		nRet = id3v2.Open(szMediaFile, true, false);
		if (nRet != ERR_OK)
			return nRet;

		bool bRemoved = false;
		for (uint i = 0; i < vLyrNamesToRemove.size(); i++)
			if ((LyrSrcTypeFromName(vLyrNamesToRemove[i].c_str()) & LST_ID3V2) > 0)
			{
				nRet = id3v2.RemoveLyrics(vLyrNamesToRemove[i].c_str());
				if (nRet == ERR_OK)
					bRemoved = true;
			}

		if (bRemoved)
		{
			nRet = id3v2.Save();
			if (nRet == ERR_OK && succeededCount)
				(*succeededCount)++;
			return nRet;
		}

		return ERR_OK;
	}

	// Remove M4a lyrics
	nRet = ERR_OK;
	if (IsFlagSet(uAllLstFlag, LST_M4A_LYRICS))
	{
		CM4aTag tag;
		nRet = tag.Open(szMediaFile, true, false);
		if (nRet == ERR_OK)
		{
			nRet = tag.RemoveLyrics();
			if (nRet == ERR_OK)
			{
				nRet = tag.SaveClose();
				if (nRet == ERR_OK && succeededCount)
					(*succeededCount)++;
			}
		}
	}

	return nRet;
}
