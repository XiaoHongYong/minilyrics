#include "StdAfx.h"
#include "TagLib.h"

#include "../ID3Lib/ID3v1.h"
#include "../ID3Lib/ID3v2.h"

#include "MP3InfoReader.h"

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
		SetCustomErrorDesc(CStrPrintf(_T("%s: %s"), szFile, (LPCTSTR)CWin32Error()).c_str());
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

			nRet = id3v2.GetTags(*pArtist, *pTitle, *pAlbum, temp, *pTrackNo, *pYear, *pGenre);
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
