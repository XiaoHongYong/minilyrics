/********************************************************************
	Created  :	2002Äê1ÔÂ2ÈÕ 22:54:43
	FileName :	LyricsLocalSearch.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "stdafx.h"
#include "MPlayerApp.h"
#include "DownloadMgr.h"
#include "LyricsLocalSearch.h"

LPCTSTR 		SZ_FOLDER_PREFIX = _T("LyricFolder");

#define SZ_S2L_HEADER_V10	"MPS2LV1.0"
#define LEN_S2L_HEADER_V10	9

#define SZ_S2L_HEADER		"MPS2LV1.1"
#define LEN_S2L_HEADER		9

// rule: 1) http://xxxx	
//       2) without the extention of song file.
bool IsShoutcastMedia(LPCTSTR szMedia)
{
	if (_tcsnicmp(szMedia, _T("http://"), 7) == 0)
		return true;
	else if (_tcsnicmp(szMedia, _T("uvox://"), 7) == 0)
		return true;

	return false;
}

bool IsShoutcastAssociateKeyName(LPCTSTR szMedia)
{
	if (_tcsnicmp(szMedia, _T("http://"), 7) == 0)
	{
		LPCTSTR p = _tcsstr(szMedia + 7, _T("/[shoutcast]"));
		if (p)
			return true;
	}
	else if (_tcsnicmp(szMedia, _T("uvox://"), 7) == 0)
		return true;

	return false;
}

bool IsSupportedOpenFileType(MLFileType fileType)
{
	return fileType != FT_UNKNOWN && IsFlagSet(FT_LYRICS_TXT | FT_SUBTITLE_SRT | FT_LYRICS_LRC, fileType);
}

//
// Return it, if:
// * Name is same as song file name.
// * Title & artist are match.
//
void SearchMatchLyricsInDir(LPCTSTR szDir, CLyricsSearchParameter &searchParam, V_LRCSEARCHRESULT &vLyrics, BOOL bIncludeSub)
{
	if (IsEmptyString(szDir))
		return;

	CMLFileFind	finder;
	if (!finder.OpenDir(szDir))
	{
		ERRLOG1(_T("Failed to open dir: %s"), szDir);
		return;
	}

	while (finder.FindNext())
	{
		if (finder.IsCurDir())
		{
			if (bIncludeSub)
			{
				tstring subDir = DirStringJoint(szDir, finder.GetCurName());
				DirStringAddSlash(subDir);
				SearchMatchLyricsInDir(subDir.c_str(), searchParam, vLyrics, bIncludeSub);
				if (searchParam.IsBestMatchLyricsFound())
					return;
			}
		}
		else
		{
			MLFileType fileType = MLGetFileType(finder.GetCurName());
			if (!IsSupportedOpenFileType(fileType))
				continue;

			int nMatchValue = searchParam.CalMatchValueByName(szDir, finder.GetCurName(), fileType);
			if (nMatchValue > MATCH_VALUE_MIN)
			{
				if (searchParam.bOnlySearchBestMatch)
				{
					if (nMatchValue > searchParam.nMatchValueOfBest)
					{
						searchParam.strBestMatchLyrics = DirStringJoint(szDir, finder.GetCurName());
						searchParam.nMatchValueOfBest = nMatchValue;
						if (searchParam.IsBestMatchLyricsFound())
							return;
					}
					continue;
				}

				LrcSearchResult		LrcResult;
				LrcResult.nMatchValue = (float)nMatchValue;
				LrcResult.strUrl = DirStringJoint(szDir, finder.GetCurName());
				vLyrics.AddResult(LrcResult);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////

CLyricsLocalSearch::CLyricsLocalSearch()
{
	m_bSaved = TRUE;
}

CLyricsLocalSearch::~CLyricsLocalSearch()
{
}

void CLyricsLocalSearch::LoadLyricFolderCfg()
{
	for (int i = 0; ; i ++)
	{
		TCHAR	szKeyName[MAX_PATH];

		// get folder name
		_stprintf_s(szKeyName, CountOf(szKeyName), _T("%s%d"), SZ_FOLDER_PREFIX, i);
		tstring strFolder = CMLProfile::GetDir(SZ_SECT_SEARCH_FOLDER, szKeyName, _T(""));
		if (strFolder.size())
			m_vLyricsFolders.push_back(strFolder);
		else
			break;
	}
}

void CLyricsLocalSearch::SaveLyricFolderCfg()
{
	if (m_bSaved)
		return;

	m_bSaved = true;

	TCHAR	szKeyName[MAX_PATH];

	for (uint i = 0; i < m_vLyricsFolders.size(); i++)
	{
		// SAVE folder settings
		_stprintf_s(szKeyName, CountOf(szKeyName), _T("%s%d"), SZ_FOLDER_PREFIX, i);
		CMLProfile::WriteDir(SZ_SECT_SEARCH_FOLDER, szKeyName, m_vLyricsFolders[i].c_str());
	}

	// Set end positions.
	_stprintf_s(szKeyName, CountOf(szKeyName), _T("%s%d"), SZ_FOLDER_PREFIX, m_vLyricsFolders.size());
	g_Profile.WriteString(SZ_SECT_SEARCH_FOLDER, szKeyName, _T(""));
}

void CLyricsLocalSearch::SearchAllMatchLyrics(LPCTSTR szSongFile, LPCTSTR szArtist, LPCTSTR szTitle, V_LRCSEARCHRESULT &vLyrics)
{
	CLyricsSearchParameter searchParam(szSongFile, szArtist, szTitle);
	SearchLyrics(searchParam, vLyrics);
// 
// 	if (vLyrics.size() == 0 && !IsEmptyString(szSongFile))
// 	{
// 		tstring strArtist, strTitle;
// 		AnalysisLyricsFileNameEx(strArtist, strTitle, szSongFile);
// 		if (strTitle.empty() 
// 			|| _tcsicmp(szArtist, strArtist.c_str()) == 0 && _tcsicmp(szTitle, strTitle.c_str()) == 0)
// 			return;
// 
// 		CLyricsSearchParameter searchParam(szSongFile, szArtist, szTitle);
// 		if (searchParam.strTitleFiltered.size() > 0)
// 			SearchLyrics(searchParam, &vLyrics);
// 	}
}

// Search lyrics by artist and title.
void CLyricsLocalSearch::SearchLyrics(CLyricsSearchParameter &searchParam, V_LRCSEARCHRESULT &vLyrics)
{
	SearchEmbeddedLyrics(searchParam.szSongFile, vLyrics, searchParam.bOnlySearchBestMatch);
	if (searchParam.bOnlySearchBestMatch)
	{
		V_LRCSEARCHRESULT::iterator	it = vLyrics.GetTheBestMatchLyrics();
		if (it != vLyrics.end())
		{
			LrcSearchResult		&result = *it;
			if (result.nMatchValue >= MATCH_VALUE_EMBEDDED_LRC)
			{
				searchParam.strBestMatchLyrics = result.strUrl.c_str();
				searchParam.nMatchValueOfBest = (int)result.nMatchValue;
				return;
			}
		}
	}

	tstring	strDir;

	// Search lyrics in the same folder of song file.
	{
		if (!IsEmptyString(searchParam.szSongFile))
		{
			strDir = FileGetPath(searchParam.szSongFile);
			SearchMatchLyricsInDir(strDir.c_str(), searchParam, vLyrics, false);
			if (searchParam.IsBestMatchLyricsFound())
				return;
		}
	}

	// Search in lyrics download folder
	{
		strDir = g_LyricsDownloader.GetDefSavePath();
		DirStringAddSlash(strDir);

		SearchMatchLyricsInDir(strDir.c_str(), searchParam, vLyrics, true);
		if (searchParam.IsBestMatchLyricsFound())
			return;
	}

	// Search in the folder that user specified.
	for (int i = GetSearchFolerCount() - 1; i >= 0; i --)
	{
		strDir = m_vLyricsFolders[i].c_str();
		DirStringAddSlash(strDir);

		SearchMatchLyricsInDir(strDir.c_str(), searchParam, vLyrics, true);
		if (searchParam.IsBestMatchLyricsFound())
			return;
	}
}

int CLyricsLocalSearch::SearchEmbeddedLyrics(LPCTSTR szSongFile, V_LRCSEARCHRESULT &vLyrics, bool bOnlyListAvailable)
{
	VecString vLyricsNames;
	int nRet = CTagLib::GetEmbeddedLyrics(szSongFile, vLyricsNames);
	if (nRet != ERR_OK)
		return nRet;

	for (uint i = 0; i < vLyricsNames.size(); i++)
	{
		tstring &lyrName = vLyricsNames[i];
		LRC_SOURCE_TYPE lst = LyrSrcTypeFromName(lyrName.c_str());

		LrcSearchResult		result;
		result.strUrl = lyrName;
		result.strTitle = result.strSaveFileName = RemovePrefixOfAcckey(_TL(LyrSrcTypeToDesc(lst)));
		if (lst == LST_LYRICS3V2 || lst == LST_ID3V2_SYLT || lst == LST_ID3V2_LYRICS)
			result.nMatchValue = MATCH_VALUE_EMBEDDED_LRC;
		else
			result.nMatchValue = MATCH_VALUE_BETTER - 4;	// Larger than the text lyrics.

		string language;
		int index;
		if (GetEmbeddedLyricsNameInfo(lyrName.c_str(), language, index))
		{
			tstring strIndex;
			if (index > 0)
				strIndex = CStrPrintf(_T(": %d"), index).c_str();
			if (language == "eng")
				result.strArtist = _T("English");
			else if (language == "fre")
				result.strArtist = _T("French");
			else
				AnsiToTString(language.c_str(), -1, result.strArtist);
			result.strArtist += strIndex;
		}

		vLyrics.AddResult(result);
	}

	return ERR_OK;
}

int CLyricsLocalSearch::SearchEmbeddedLyrics(LPCTSTR szSongFile, uint32 &lrcSourceType)
{
	lrcSourceType = 0;

	VecString vLyricsNames;
	int nRet = CTagLib::GetEmbeddedLyrics(szSongFile, vLyricsNames);
	if (nRet != ERR_OK)
		return nRet;

	for (uint i = 0; i < vLyricsNames.size(); i++)
		lrcSourceType |= LyrSrcTypeFromName(vLyricsNames[i].c_str());

	return ERR_OK;
}

tstring CLyricsLocalSearch::GetFolder(int nIndex)
{
	if (nIndex >= 0 && nIndex < m_vLyricsFolders.size())
		return m_vLyricsFolders[nIndex];
	else
		return _T("");
}

BOOL CLyricsLocalSearch::SetFolder(LPCTSTR szFolder)
{
	for (int i = GetSearchFolerCount() - 1; i >= 0; i --)
	{
		if (_tcsicmp(m_vLyricsFolders[i].c_str(), szFolder) == 0)
			return FALSE;
	}

	m_bSaved = false;
	m_vLyricsFolders.push_back(szFolder);

	return TRUE;
}

int CLyricsLocalSearch::GetSearchFolerCount()
{
	return m_vLyricsFolders.size();
}

BOOL CLyricsLocalSearch::RemoveFolder(int nIndex)
{
	if (nIndex >= GetSearchFolerCount() || nIndex < 0)
		return FALSE;

	m_bSaved = FALSE;

	m_vLyricsFolders.erase(m_vLyricsFolders.begin() + nIndex);

	return TRUE;
}

void CLyricsLocalSearch::Init()
{
	LoadLyricFolderCfg();

	LoadLyricsAssociation();
}

void CLyricsLocalSearch::Quit()
{
	SaveLyricFolderCfg();
	m_vLyricsFolders.clear();
}

LPTSTR GetDiskImageFileExtEnd(LPCTSTR szFile)
{
	static LPCTSTR vImageExts[] = { _T(".cue"), _T(".tak"), _T(".wv") };

	for (int i = 0; i < CountOf(vImageExts); i++)
	{
		LPTSTR	p = stristr(szFile, vImageExts[i]);
		if (p)
			return p + _tcslen(vImageExts[i]);
	}

	return NULL;
}

tstring CLyricsLocalSearch::GetAssociateFileKeyword(LPCTSTR szSongFile, LPCTSTR szFulltitle)
{
	tstring		strAssociateKey;

	if (IsEmptyString(szSongFile))
		return szFulltitle;

	strAssociateKey = szSongFile;

	LPTSTR szEndPos = GetDiskImageFileExtEnd(szSongFile);
	if (szEndPos)
	{
		// to associate a .cue, .tak track correctly, the title will be appended to it.
		if (_tcscmp(szEndPos, _T(" - ")) == 0
			|| _tcscmp(szEndPos, _T(",")) == 0)
			*szEndPos = '\0';
		strAssociateKey += szFulltitle;
	}
	else if (IsShoutcastMedia(szSongFile))
	{
		// to associate a shoutcast track correctly, the title will be appended to it.
		strAssociateKey += _T("/[shoutcast]");
		strAssociateKey += szFulltitle;
	}

	return strAssociateKey;
}

bool CLyricsLocalSearch::AssociateLyrics(LPCTSTR szAssociateFileKeyword, LPCTSTR szLyricFile)
{
	assert(szAssociateFileKeyword);

	if (IsEmptyString(szAssociateFileKeyword))
		return false;

	SONG_LYRIC_MAP::iterator		it;
	tstring		strKey = ToAssociateKeyword(szAssociateFileKeyword);

	m_mutex.Acquire();
	// Is already associated and same?
	it = m_mapLyricsAssociate.find(strKey);
	if (it != m_mapLyricsAssociate.end())
	{
		if (_tcscmp((*it).second.c_str(), szLyricFile) == 0)
		{
			m_mutex.Release();
			return false;
		}
	}
	m_mapLyricsAssociate[strKey] = szLyricFile;
	m_mutex.Release();

	// save??
	SaveLyricsAssociation();

	return true;
}

BOOL CLyricsLocalSearch::CancelAssociate(LPCTSTR szAssociateFileKeyword)
{
	SONG_LYRIC_MAP::iterator		itLyric;
	CMutexAutolock					autoLock(m_mutex);

	itLyric = m_mapLyricsAssociate.find(ToAssociateKeyword(szAssociateFileKeyword));
	if (itLyric != m_mapLyricsAssociate.end())
	{
		m_mapLyricsAssociate.erase(itLyric);
		return TRUE;
	}

	SaveLyricsAssociation();

	return FALSE;
}

BOOL CLyricsLocalSearch::IsAssociatedLyrics(LPCTSTR szAssociateFileKeyword)
{
	assert(szAssociateFileKeyword);
	SONG_LYRIC_MAP::iterator		itLyric;
	CMutexAutolock					autoLock(m_mutex);

	itLyric = m_mapLyricsAssociate.find(ToAssociateKeyword(szAssociateFileKeyword));
	if (itLyric != m_mapLyricsAssociate.end())
		return TRUE;

	return FALSE;
}

BOOL CLyricsLocalSearch::IsAssociatedWithNoneLyrics(LPCTSTR szAssociateKeyword)
{
	TCHAR				szLyricsFile[MAX_PATH];

	if (GetAssociateLyrics(szAssociateKeyword, szLyricsFile, MAX_PATH))
	{
		if (_tcscmp(NONE_LYRCS, szLyricsFile) == 0)
			return TRUE;
	}
	return FALSE;
}

BOOL CLyricsLocalSearch::IsAssociatedLyrics(LPCTSTR szSongFile, LPCTSTR szFullTitle)
{
	return IsAssociatedLyrics(GetAssociateFileKeyword(szSongFile, szFullTitle).c_str());
}

inline bool FilePathIsBeginWithDriver(LPCTSTR szFile)
{
	return szFile[0] && szFile[1] == ':';
}

BOOL CLyricsLocalSearch::GetAssociateLyrics(LPCTSTR szAssociateFileKeyword, LPTSTR szLyricFile, int nMaxBuff)
{
	assert(szLyricFile);
	EmptyStr(szLyricFile);

	SONG_LYRIC_MAP::iterator		itLyric;
	CMutexAutolock					autoLock(m_mutex);

	itLyric = m_mapLyricsAssociate.find(ToAssociateKeyword(szAssociateFileKeyword));
	if (itLyric != m_mapLyricsAssociate.end())
	{
		_tcscpy_safe(szLyricFile, nMaxBuff, (*itLyric).second.c_str());

		if (IsLyricsExist(szLyricFile))
			return true;

		if (FilePathIsBeginWithDriver(szAssociateFileKeyword)
			&& FilePathIsBeginWithDriver(szLyricFile))
		{
			szLyricFile[0] = szAssociateFileKeyword[0];
			return IsLyricsExist(szLyricFile);
		}
	}

	return false;
}

bool CLyricsLocalSearch::IsLyricsExist(LPCTSTR szLyricSource)
{
	LRC_SOURCE_TYPE	sourceType = LyrSrcTypeFromName(szLyricSource);

	if (sourceType == LST_FILE)
	{
		if (IsFileExist(szLyricSource))
			return true;
		else
			return false;
	}
	else
		return true;
}

#ifdef _WIN32_DESKTOP
void CLyricsLocalSearch::LoadLyricsAssociationOld()
{
	SONG_LYRIC_MAP::iterator		it;
	char			szKey[MAX_PATH];
	char			szValue[MAX_PATH];
	FILE			*fp;
	TCHAR			szFile[MAX_PATH];
	tstringex		strKey, strValue;

	GetAppResourceDir(szFile);

	_tcscat_safe(szFile, CountOf(szFile), _T("MiniLyrics.S2L"));

	fp = _tfopen(szFile, _T("rb"));
	if (fp == NULL)
		return;

	CMutexAutolock					autoLock(m_mutex);

	while (!feof(fp))
	{
		// Key
		if (fread(szKey, 1, sizeof(szKey), fp) != sizeof(szKey))
			goto READ_FAILED;

		// Lyrics
		if (fread(szValue, 1, sizeof(szValue), fp) != sizeof(szValue))
			goto READ_FAILED;

		ConvertStr(szKey, -1, strKey);
		ConvertStr(szValue, -1, strValue);

		AssociateLyrics(strKey.c_str(), strValue.c_str());
	}

READ_FAILED:
	fclose(fp);
	DeleteFile(szFile);
}
#endif

void CLyricsLocalSearch::LoadLyricsAssociationV2()
{
	SONG_LYRIC_MAP::iterator		it;
	TCHAR			szKey[MAX_PATH];
	TCHAR			szValue[MAX_PATH];
	FILE			*fp;
	TCHAR			szFile[MAX_PATH];
	CMutexAutolock					autoLock(m_mutex);

#ifdef _WIN32_DESKTOP
	// Load old lyrics association
	LoadLyricsAssociationOld();
#endif

	GetAppDataDir(szFile);

	_tcscat_safe(szFile, CountOf(szFile), _T("MLLyrics.S2L"));

	fp = _tfopen(szFile, _T("rb"));
	if (fp == NULL)
		return;

	while (!feof(fp))
	{
		// Key
		if (fread(szKey, 1, sizeof(szKey), fp) != sizeof(szKey))
			goto R_END;

		// Lyrics
		if (fread(szValue, 1, sizeof(szValue), fp) != sizeof(szValue))
			goto R_END;

		AssociateLyrics(szKey, szValue);
	}

R_END:
	fclose(fp);
	DeleteFile(szFile);
}

void CLyricsLocalSearch::LoadLyricsAssociation()
{
	SONG_LYRIC_MAP::iterator		it;
	char			szBuff[1024];
	char			*szStr;
	int				nLen, nLenBuff;
	tstringex		strSong, strLyrics;
	FILE			*fp;
	TCHAR			szFile[MAX_PATH];
	CMutexAutolock	autoLock(m_mutex);
	bool			bConvertV10 = false;

#ifndef _WIN32_WCE
	BOOL			bConvertSave = FALSE;
	LoadLyricsAssociationV2();
	if (m_mapLyricsAssociate.size())
		bConvertSave = TRUE;
#endif

	GetAppDataDir(szFile);

	_tcscat_safe(szFile, CountOf(szFile), _T("MLyrics.S2L"));

	fp = _tfopen(szFile, _T("rb"));
	if (fp == NULL)
		goto R_END;

	if (!fgets(szBuff, CountOf(szBuff), fp))
		goto R_END;

	// Is valid s2l file?
	if (strncmp(szBuff, SZ_S2L_HEADER_V10, LEN_S2L_HEADER_V10) == 0)
		bConvertSave = bConvertV10 = true;
	else if (strncmp(szBuff, SZ_S2L_HEADER, LEN_S2L_HEADER) != 0)
		goto R_END;

	while (fgets(szBuff, CountOf(szBuff), fp))
	{
		szStr = szBuff;
		nLenBuff = strlen(szBuff);

		// get song file name.
		szStr = ReadInt_t(szStr, nLen); if (nLen > nLenBuff - int(szStr - szBuff)) continue;
		if (*szStr == ',') szStr++; else continue;

#ifdef _UNICODE
		Utf8ToUCS2(szStr, nLen, strSong);
#else
		strSong.resize(0);
		strSong.append(szStr, nLen);
#endif
		szStr += nLen;
		if (*szStr != ',')
			continue;
		szStr++;

		// get lyrics file name.
		szStr = ReadInt_t(szStr, nLen); if (nLen > nLenBuff - int(szStr - szBuff)) continue;
		if (*szStr == ',') szStr++; else continue;

#ifdef _UNICODE
		Utf8ToUCS2(szStr, nLen, strLyrics);
#else
		strLyrics.resize(0);
		strLyrics.append(szStr, nLen);
#endif

		if (bConvertV10)
			m_mapLyricsAssociate[ToAssociateKeyword(strSong.c_str())] = strLyrics.c_str();
		else
			m_mapLyricsAssociate[strSong.c_str()] = strLyrics.c_str();
	}

R_END:
	if (fp)
		fclose(fp);

#ifndef _WIN32_WCE
	if (bConvertSave)
	{
		SaveLyricsAssociation();
	}
#endif
}

void CLyricsLocalSearch::SaveLyricsAssociation()
{
	SONG_LYRIC_MAP::iterator		it;
	TCHAR			szFileName[MAX_PATH];
	FILE			*fp;
	TCHAR			szFile[MAX_PATH];
	stringex	str, strUtf8;
	char			szTemp[64];

	memset(szFileName, 0, sizeof(szFileName));

	GetAppDataDir(szFile);

	_tcscat_safe(szFile, CountOf(szFile), _T("MLyrics.S2L"));

	fp = _tfopen(szFile, _T("wb"));
	if (fp == NULL)
	{
		ERRLOG1(_T("Can't save lyrics association file: %s."), szFile);
		return;
	}

	if (fwrite(SZ_S2L_HEADER, 1, LEN_S2L_HEADER, fp) != LEN_S2L_HEADER)
		return;

	if (fwrite("\r\n", 1, 2, fp) != 2)
		return;

	CMutexAutolock	autoLock(m_mutex);

	for (it = m_mapLyricsAssociate.begin(); it != m_mapLyricsAssociate.end(); it++)
	{
		if (IsShoutcastAssociateKeyName((*it).first.c_str()))
			continue;

		str.resize(0);

		//
		// Write song file
		//
#ifdef _UNICODE
		UCS2ToUtf8((*it).first.c_str(), (*it).first.length(), strUtf8);
#else
		strUtf8 = (*it).first.c_str();
#endif

		// len
		sprintf_s(szTemp, CountOf(szTemp), "%d", strUtf8.size());
		str += szTemp; str += ",";

		// file
		str += strUtf8;
		str += ",";

		//
		// Write lyrics file
		//
#ifdef _UNICODE
		UCS2ToUtf8((*it).second.c_str(), (*it).second.length(), strUtf8);
#else
		strUtf8 = (*it).second.c_str();
#endif

		// len
		sprintf_s(szTemp, CountOf(szTemp), "%d", strUtf8.size());
		str += szTemp; str += ",";

		// file
		str += strUtf8;
		str += "\n";

		fwrite(str.c_str(), 1, str.size(), fp);
	}

	fclose(fp);
	return;
}

// Return the best match lyrics only
bool CLyricsLocalSearch::GetBestMatchLyrics(LPCTSTR szSongFile, LPCTSTR szArtist, LPCTSTR szTitle, tstring &strLyrFile)
{
	tstring fullTitle = FormatMediaTitle(szArtist, szTitle);
	TCHAR szLyricsFile[MAX_PATH];
	if (GetAssociateLyrics(GetAssociateFileKeyword(szSongFile, fullTitle.c_str()).c_str(), szLyricsFile, MAX_PATH))
	{
		strLyrFile = szLyricsFile;
		return true;
	}

	V_LRCSEARCHRESULT vLyrics;
	CLyricsSearchParameter searchParam(szSongFile, szArtist, szTitle, true);
	SearchLyrics(searchParam, vLyrics);
	strLyrFile = searchParam.strBestMatchLyrics;
	return strLyrFile.size() > 0;
}
