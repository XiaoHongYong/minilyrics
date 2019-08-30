// LyricsDBMgr.cpp: implementation of the CLyricsDBMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MLServer.h"
#include "LyricsDBMgr.h"
#include "MLHelperFun.h"
#include "LyricsInfoFiles.h"
#include "LyricsUpdateMgr.h"
#include "../../base/CharEncoding.h"
#include "../../MLLib/LyricsKeywordFilter.h"
#include <time.h>


CLyricsDBMgr		g_LyricsDBMgr;

int		g_nSameLrcExistMax		= 4;

LPCTSTR GenRandLyrFilePrefix()
{
	static TCHAR		str[4];

	str[0] = 'a' + rand() % ('z' - 'a');
	str[1] = 'a' + rand() % ('z' - 'a');
	str[2] = '_';
	str[3] = '\0';

	return str;
};

// Convert UTF-8 lyrics content to most proper file content, add FILE BOM
void ConvertUtf8ToProperFileData(LPCSTR szUtf8, int nLenUtf8, CCharBuffer &bufData)
{
	bufData.clear();

	if (IsAnsiStr(szUtf8))
		bufData.append(szUtf8, nLenUtf8);
	else
	{
		// Try to convert to UCS2 to see whether the size of lyrics will be decrease.
		wstringex		ucs2;
		Utf8ToUCS2(szUtf8, nLenUtf8, ucs2);
		if (ucs2.size() * sizeof(WCHAR) <= (size_t)nLenUtf8)
		{
			bufData.append(SZ_FE_UCS2);
			bufData.append((LPCSTR)ucs2.c_str(), ucs2.size() * sizeof(WCHAR));
		}
		else
		{
			bufData.append(SZ_FE_UTF8);
			bufData.append(szUtf8, nLenUtf8);
		}
	}
}

int SaveDataAsFileSafe(LPCTSTR szFile, LPCSTR szData, int nLen)
{
	if (!SaveDataAsFile(szFile, szData, nLen))
	{
        return ERR_OPEN_FILE;
	}

	return ERR_OK;
}

LPCTSTR GenRandLyrFilePrefix();

int CLyricsDBMgr::SaveUploadedLyricsFile(LPCTSTR szLyricsId, LYRICS_CONTENT_TYPE lyrContentType, stringex &strLyrContentUtf8, tstring &strFile)
{
	if (strFile.empty())
	{
		strFile = m_lyricsDir;
		strFile += m_relatedUploadDir;

		time_t now = time(NULL);
        tm *t = localtime(&now);

        char    szTemp[64];
		sprintf(szTemp, "%02d%02d%02d/", t->tm_year + 1900 - 2000, t->tm_mon + 1, t->tm_mday);
		strFile += szTemp;

        if (!IsDirExist(strFile.c_str())) {
            CreateDirectoryAll(strFile.c_str());
        }
	}
	strFile += GenRandLyrFilePrefix();
	strFile += szLyricsId;

	if (lyrContentType == LCT_TXT)
		strFile += ".txt";
	else
		strFile += ".lrc";

	return SaveDataAsFileSafe(strFile.c_str(), strLyrContentUtf8.c_str(), strLyrContentUtf8.size());
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLyricsDBMgr::CLyricsDBMgr()
{

}

CLyricsDBMgr::~CLyricsDBMgr()
{

}

void UpdateLyrIdTagAndConvertLyrToFileData(long nLyricsId, const tstring &strLyrContent, CLrcTag &lyrProp, CCharBuffer &bufLyrData)
{
	LyricsProperties	propNew;

	MLIDEncrypt(nLyricsId, propNew.m_strId);
	propNew.SetEncoding(lyrProp.GetEncodingIndex());

	CLrcTag		lrcTag(lyrProp, propNew, LTT_ID);

	// Write lyrics id in tag.
	stringex	strLyrContentNew;
	strLyrContentNew.append(strLyrContent.c_str(), strLyrContent.size());
	lrcTag.WriteToText(strLyrContentNew, lyrProp.m_lyrContentType == LCT_LRC);

	ConvertUtf8ToProperFileData(strLyrContentNew.c_str(), strLyrContentNew.size(), bufLyrData);
}

// New uploaded lyrics.
int CLyricsDBMgr::AddNewUploadedLyrics(long nUploaderId, LPCTSTR szUploader, 
					   CLrcTag &lyrProp, tstring &strLyrContent, long &nLyricsId, long nDerivedFromId)
{
	CLyricsInfo	lyrInfo;

	lyrInfo.SetInfo(lyrProp, nUploaderId, szUploader);

    assert(nUploaderId > 0);
	int nRet = AddLyrics(lyrInfo, lyrProp, strLyrContent, NULL, nDerivedFromId);
	if (nRet != ERR_OK)
		return nRet;

	nLyricsId = lyrInfo.nUniID;

	return ERR_OK;
}

int CLyricsDBMgr::AddLyrics(CLyricsInfo	&lyrInfo, CLrcTag &lyrProp, tstring &strLyrContent, LPCTSTR szAddToDir, long nDerivedFromId)
{
	// Add lyrics in DB first
	int nRet = g_LyricsDB.AddLyrics(lyrInfo);
	if (nRet != ERR_OK)
		return nRet;
    assert(lyrInfo.nUniID != 0);

	CCharBuffer	bufLyrData;
	UpdateLyrIdTagAndConvertLyrToFileData(lyrInfo.nUniID, strLyrContent, lyrProp, bufLyrData);

	tstring		strFile;
	tstring		strLyricsId;
	MLIDEncrypt(lyrInfo.nUniID, strLyricsId);

	if (szAddToDir && !IsEmptyString(szAddToDir))
	{
		strFile = szAddToDir;
		DirStringAddSlash(strFile);
	}

	// Save lyrics file
	nRet = SaveUploadedLyricsFile(strLyricsId.c_str(),
		lyrProp.m_lyrContentType, bufLyrData, strFile);
	if (nRet != ERR_OK)
		return nRet;

	// Update file link in lyrics db.
	TCHAR	szRelatedLink[MAX_PATH];
	GetRelatedDir(strFile.c_str(), m_lyricsDir.c_str(), szRelatedLink);
	strrep(szRelatedLink, '\\', '/');

	nRet = g_LyricsDB.UpdateLyricsLinkById(lyrInfo.nUniID, szRelatedLink);
	if (nRet != ERR_OK)
		return nRet;

	// Add to lyrics update manager for review
	LyrUpdateItem lyrUpdateItem(lyrInfo.strEditedBy.c_str(), RS_OPTIONAL, lyrProp.GetMD5(), true);
	if (lyrInfo.nUploadUserID == 0)
		lyrUpdateItem.strUploader.clear();
	if (szAddToDir && !IsEmptyString(szAddToDir))
		lyrUpdateItem.reviewStatus = RS_REVIEWED;

	nRet = g_lyrUpdateMgr.AddUpdate(lyrInfo.nUniID, 
		strFile.c_str(), lyrUpdateItem, bufLyrData, nDerivedFromId);

	return ERR_OK;
}

int CLyricsDBMgr::UpdateLyrics(RetLyrInfo &lyrInfo, CLrcTag &lyrProp, tstring &strLyrContent, LPCTSTR szUploader)
{
	// Write update history, and save in review manager.
	tstring		strFile;

    getLyricsFile(lyrInfo.strLink, strFile);

	// Convert lyrics data to proper file encoding.
	CCharBuffer	bufLyrData;
	UpdateLyrIdTagAndConvertLyrToFileData(lyrInfo.lyricsID, strLyrContent, lyrProp, bufLyrData);

	// Add update for review
	// Add to lyrics update manager for review
	LyrUpdateItem lyrUpdateItem(szUploader, RS_OPTIONAL, lyrProp.GetMD5(), false);
	g_lyrUpdateMgr.AddUpdate(lyrInfo.lyricsID,
		strFile.c_str(), lyrUpdateItem, bufLyrData);

	int nRet = SaveDataAsFileSafe(strFile.c_str(), bufLyrData.c_str(), bufLyrData.size());
	if (nRet != ERR_OK)
		return nRet;

	nRet = g_LyricsDB.UpdateLyricsPropById(lyrInfo.lyricsID, lyrProp);
	if (nRet != ERR_OK)
		return nRet;

	return ERR_OK;
}

void CLyricsDBMgr::getLyricsFile(const string &relatedLink, tstring &outFileName)
{
    outFileName = m_lyricsDir;
    outFileName += relatedLink;
}

bool CLyricsDBMgr::IsLyricsExists(LyricsProperties &lyrProp, LPCTSTR szMD5, int nUploaderId, uint32 *pOrgLyricsId)
{
	stringex		strArtistCmp, strTitleCmp;

	CLyricsKeywordFilter::Filter(lyrProp.m_strArtist.c_str(), strArtistCmp);
	CLyricsKeywordFilter::Filter(lyrProp.m_strTitle.c_str(), strTitleCmp);

	RetLyrInfoList	vLyrics;
	g_LyricsDB.SearchLyricsByArtistTitleContentType(strArtistCmp.c_str(), strTitleCmp.c_str(),
		vLyrics, lyrProp.m_lyrContentType);
	if (vLyrics.empty())
		return false;

	tstring			strFile;
    for (auto it = vLyrics.begin(); it != vLyrics.end(); ++it) {
		RetLyrInfo &info = *it;
		if (lyrProp.m_strId.empty() && nUploaderId != 0 && pOrgLyricsId) {
			// Find the original lyrics with same artist, album, title.
			if (nUploaderId == info.uploaderID
				&& _tcsicmp(lyrProp.m_strArtist.c_str(), info.strArtist.c_str()) == 0
				&& _tcsicmp(lyrProp.m_strAlbum.c_str(), info.strAlbum.c_str()) == 0
				&& _tcsicmp(lyrProp.m_strTitle.c_str(), info.strTitle.c_str()) == 0)
				*pOrgLyricsId = info.lyricsID;
		}

		getLyricsFile(info.strLink, strFile);
		CLyrUpdateHistory	file;
		if (file.Open(strFile.c_str()) == ERR_OK)
		{
			if (file.IsUpdateExists(szMD5))
				return true;
		}
	}

	return false;
}
