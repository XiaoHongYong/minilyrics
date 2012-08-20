/********************************************************************
	Created  :	2002年1月2日 22:54:43
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

// COMMENT:
//		分析类似Winamp的歌曲名称标题：
//		exp. "叶丽仪 - 上海滩" ----> 上海滩
//		      artist - title
//			  "作者 - 歌曲名(注释:)" ----->
//			  "artist - title.mp3"
//			  "artist - title.ra" .....
//		输出的串会进行过滤，最后只会留下字母、文字、数字等，其他字符都会过滤掉
// INPUT:
//		szFullTitle	-	Winamp 窗口显示的类似标题
// OUTPUT:
//		szTitle		-	标题
//		szArtist	-	作者
void AnalysisSongArTitle(TCHAR szTitle[], BOOL &bTitleIgnoreNumb, TCHAR szArtist[], BOOL &bArtIgnoreNumb, LPCTSTR szFile)
{
	tstring		strArtist, strTitle;

	tstring fileTitle = FileGetTitle(szFile);
	if (!StrSplit(fileTitle.c_str(), _T(" - "), strArtist, strTitle)
		&& !StrSplit(fileTitle.c_str(), '-', strArtist, strTitle))
	{
		strArtist.resize(0);
		strTitle = fileTitle;
	}

	_tcscpy_safe(szArtist, 256, strArtist.c_str());
	bArtIgnoreNumb = IsIgnoreNumber(szArtist);
	FilterChars(szArtist, 256, szArtist, bArtIgnoreNumb);

	_tcscpy_safe(szTitle, 256, strTitle.c_str());
	bTitleIgnoreNumb = IsIgnoreNumber(szTitle);
	FilterChars(szTitle, 256, szTitle, bTitleIgnoreNumb);
}

// PURPOSE:
//		在目录 szDir 中查找符合的文件
// INPUT:
//		szDir	-	要搜索的目录
//		szTitle	-	歌词标题
//		szArtist-	作者
// OUTPUT:
//		pvLyrics	-	所有找到的歌词列表
void SearchMatchLyricsInDir(LPCTSTR szDir, LPCTSTR szTitle, BOOL bTitleIgnoreNumb, LPCTSTR szArtist, BOOL bArtIgnoreNumb, V_LRCSEARCHRESULT *pvLyrics, BOOL bIncludeSub)
{
	if (IsEmptyString(szDir))
		return;

	TCHAR				szFileName[MAX_FILE_NAME];
	TCHAR				szFindDir[MAX_FILE_NAME];
	CMLFileFind			finder;
	LrcSearchResult		LrcResult;

	_tcscpy_safe(szFindDir, CountOf(szFindDir), szDir);
	DirStringFormat(szFindDir);

	if (!finder.OpenDir(szFindDir))
		return;

	while (finder.FindNext())
	{
		if (finder.IsCurDir())
		{
			if (bIncludeSub)
			{
				// search in sub directory
				TCHAR	szSubDir[MAX_PATH];

				_tcscpy_safe(szSubDir, CountOf(szSubDir), szDir);
				_tcscat_safe(szSubDir, CountOf(szSubDir), finder.GetCurName());
				_tcscat_safe(szSubDir, CountOf(szSubDir), _T(SZ_DIR_SLASH));

				SearchMatchLyricsInDir(szSubDir, szTitle, bTitleIgnoreNumb, szArtist, 
					bArtIgnoreNumb, pvLyrics, bIncludeSub);
			}
		}
		else
		{
			MLFileType fileType = MLGetFileType(finder.GetCurName());
			if (IsSupportedOpenFileType(fileType))
			{
				LrcResult.nMatchValue = (float)CalLyricsMatchValue(finder.GetCurName(), szTitle, bTitleIgnoreNumb, szArtist, bArtIgnoreNumb);
				if (LrcResult.nMatchValue > MATCH_VALUE_MIN)
				{
					_tcscpy_safe(szFileName, CountOf(szFileName), szDir);
					_tcscat_safe(szFileName, CountOf(szFileName), finder.GetCurName());
					LrcResult.strUrl = szFileName;
					pvLyrics->AddResult(LrcResult);
				}
			}
		}
	}
}

// PURPOSE:
//		在目录 szDir 中查找符合的文件
// INPUT:
//		szDir	-	要搜索的目录
//		szTitle	-	歌词标题
//		szArtist-	作者
// OUTPUT:
//		LrcResult	-	找到的歌词
//		LrcResult.nMatchValue	= MATCH_VALUE_MAX	- 成功找到最匹配的歌词
void SearchMatchLyricsInDir(LPCTSTR szDir, LPCTSTR szTitle, BOOL bTitleIgnoreNumb, LPCTSTR szArtist, BOOL bArtIgnoreNumb, LrcSearchResult &LrcResult, BOOL bIncludeSub)
{
	TCHAR				szFileName[MAX_FILE_NAME];
	TCHAR				szFindDir[MAX_FILE_NAME];
	CMLFileFind			finder;
	MLFileType			nFileType;
	int					nMatchValue;

	if (IsEmptyString(szDir))
		return;

	if (IsEmptyString(szTitle))
		return;

	_tcscpy_safe(szFindDir, CountOf(szFindDir), szDir);
	DirStringFormat(szFindDir);

	if (!finder.OpenDir(szFindDir))
		return;

	while (finder.FindNext())
	{
		if (finder.IsCurDir())
		{
			if (bIncludeSub)
			{
				// search in sub directory
				TCHAR	szSubDir[MAX_PATH];

				_tcscpy_safe(szSubDir, CountOf(szSubDir), szDir);
				_tcscat_safe(szSubDir, CountOf(szSubDir), finder.GetCurName());
				_tcscat_safe(szSubDir, CountOf(szSubDir), _T(SZ_DIR_SLASH));

				SearchMatchLyricsInDir(szSubDir, szTitle, bTitleIgnoreNumb, szArtist, bArtIgnoreNumb, LrcResult, bIncludeSub);
				if (LrcResult.nMatchValue == MATCH_VALUE_MAX)
					return;
			}
		}
		else
		{
			nFileType = MLGetFileType(finder.GetCurName());
			if (IsSupportedOpenFileType(nFileType))
			{
				nMatchValue = CalLyricsMatchValue(finder.GetCurName(), szTitle, bTitleIgnoreNumb, szArtist, bArtIgnoreNumb);
				if (nMatchValue > LrcResult.nMatchValue)
				{
					_tcscpy_safe(szFileName, CountOf(szFileName), szDir);
					_tcscat_safe(szFileName, CountOf(szFileName), finder.GetCurName());
					LrcResult.strUrl = szFileName;
					LrcResult.nMatchValue = (float)nMatchValue;
					if (LrcResult.nMatchValue == MATCH_VALUE_MAX)
						return;
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
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
	g_Profile.WriteProfileString(SZ_SECT_SEARCH_FOLDER, szKeyName, _T(""));
}

// COMMENT:
//		搜索与歌曲szSongName，所有相关得歌词 
BOOL CLyricsLocalSearch::SearchAllMatchLyrics(LPCTSTR szSongFile, LPCTSTR szSongName, V_LRCSEARCHRESULT &vLyrics)
{
	// 将 szSongName 转换为歌名.
	// exp. "叶丽仪 - 上海滩" ----> 上海滩
	TCHAR	szTitle[MAX_PATH];
	TCHAR	szArtist[MAX_PATH];
	BOOL	bTitleIgnoreNumb, bArtIgnoreNumb;

	SearchEmbeddedLyrics(szSongFile, vLyrics, true);

	AnalysisSongArTitle(szTitle, bTitleIgnoreNumb, szArtist, bArtIgnoreNumb, szSongName);

	if (!IsEmptyString(szTitle))
		SearchLyrics(szSongFile, szTitle, bTitleIgnoreNumb, szArtist, bArtIgnoreNumb, &vLyrics);

	if (vLyrics.size() == 0 && !IsEmptyString(szSongFile))
	{
		AnalysisSongArTitle(szTitle, bTitleIgnoreNumb, szArtist, bArtIgnoreNumb, szSongFile);
		if (!IsEmptyString(szTitle))
			SearchLyrics(szSongFile, szTitle, bTitleIgnoreNumb, szArtist, bArtIgnoreNumb, &vLyrics);
	}

	return TRUE;
}

// COMMENT:
//		搜索与歌曲szSongName，相匹配的歌词
// INPUT:
//		szTitle		-	标题
//		szArtist	-	作者
// OUTPUT:
//		pvLyrics	-	搜索到的歌词列表
// RETURN:
//		
BOOL CLyricsLocalSearch::SearchLyrics(LPCTSTR szSongFile, LPCTSTR szTitle, BOOL bTitleIgnoreNumb, LPCTSTR szArtist, BOOL bArtIgnoreNumb, V_LRCSEARCHRESULT *pvLyrics)
{
	tstring			strDir;

	//
	// 到歌曲所在目录下搜索
	//
	{
		if (!IsEmptyString(szSongFile))
		{
			strDir = FileGetPath(szSongFile);

			SearchMatchLyricsInDir(strDir.c_str(), szTitle, bTitleIgnoreNumb, szArtist, bArtIgnoreNumb, pvLyrics, FALSE);
		}

		TCHAR		szLyricFile[MAX_PATH];

		//
		// 同目录下面的lrc文件
		//
		_tcscpy_safe(szLyricFile, CountOf(szLyricFile), szSongFile);
		FileSetExt(szLyricFile, CountOf(szLyricFile), _T(".lrc"));
		if (IsFileExist(szLyricFile))
		{
			LrcSearchResult		item;
			item.nMatchValue = MATCH_VALUE_WITH_LRC;
			item.strUrl = szLyricFile;
			pvLyrics->AddResult(item);
		}

		//
		// 同目录下面的txt文件
		//
		_tcscpy_safe(szLyricFile, CountOf(szLyricFile), szSongFile);
		FileSetExt(szLyricFile, CountOf(szLyricFile), _T(".txt"));
		if (IsFileExist(szLyricFile))
		{
			LrcSearchResult		item;
			item.nMatchValue = MATCH_VALUE_OK;
			item.strUrl = szLyricFile;
			pvLyrics->AddResult(item);
		}
	}

	//
	// 搜索歌词下载目录
	//
	{
		strDir = g_LyricsDownloader.GetDefSavePath();
		DirStringFormat(strDir);

		SearchMatchLyricsInDir(strDir.c_str(), szTitle, bTitleIgnoreNumb, szArtist, bArtIgnoreNumb, pvLyrics, TRUE);
	}

	//
	// 搜索用户设置的目录
	//
	for (int i = GetSearchFolerCount() - 1; i >= 0; i --)
	{
		strDir = m_vLyricsFolders[i].c_str();
		DirStringAddSlash(strDir);

		SearchMatchLyricsInDir(strDir.c_str(), szTitle, bTitleIgnoreNumb, szArtist, bArtIgnoreNumb, pvLyrics, true);
	}

	return TRUE;
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
		result.strSaveFileName = RemovePrefixOfAcckey(_TL(LyrSrcTypeToDesc(lst)));
		if (lst == LST_LYRICS3V2 || lst == LST_ID3V2_SYLT || lst == LST_ID3V2_LYRICS)
			result.nMatchValue = MATCH_VALUE_EMBEDDED_LRC;
		else
			result.nMatchValue = MATCH_VALUE_OK + 2;

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

// 关闭歌词搜索模块
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

// COMMENT:
//		使歌词szLyricFile 和歌曲szSongFile 相关连
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

// COMMENT:
//		取消与歌曲szSongFile 相关连的歌词
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

// COMMENT:
//		搜索是否有与歌曲szSongFile 相关连的歌词
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

// COMMENT:
//		搜索与歌曲 szAssociateFileKeyword 相关连的歌词
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
// COMMENT:
//		先加载旧的歌词关联表，并删除之
void CLyricsLocalSearch::LoadLyricsAssociationOld()
{
	SONG_LYRIC_MAP::iterator		it;
	char			szKey[MAX_FILE_NAME];
	char			szValue[MAX_FILE_NAME];
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
		// 读入key - 歌曲名
		if (fread(szKey, 1, sizeof(szKey), fp) != sizeof(szKey))
			goto READ_FAILED;

		// 读入value - 歌词文件名
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
	TCHAR			szKey[MAX_FILE_NAME];
	TCHAR			szValue[MAX_FILE_NAME];
	FILE			*fp;
	TCHAR			szFile[MAX_PATH];
	CMutexAutolock					autoLock(m_mutex);

#ifdef _WIN32_DESKTOP
	// 先加载旧的歌词关联表，并删除之
	LoadLyricsAssociationOld();
#endif

	GetAppDataDir(szFile);

	_tcscat_safe(szFile, CountOf(szFile), _T("MLLyrics.S2L"));

	fp = _tfopen(szFile, _T("rb"));
	if (fp == NULL)
		return;

	while (!feof(fp))
	{
		// 读入key - 歌曲名
		if (fread(szKey, 1, sizeof(szKey), fp) != sizeof(szKey))
			goto R_END;

		// 读入value - 歌词文件名
		if (fread(szValue, 1, sizeof(szValue), fp) != sizeof(szValue))
			goto R_END;

		AssociateLyrics(szKey, szValue);
	}

R_END:
	fclose(fp);
	DeleteFile(szFile);
}

// COMMENT:
//		从文件中读入歌词关连表
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
	// 先加载旧的歌词关联表，并删除之
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

// COMMENT:
//		保存歌词关连表到文件中
void CLyricsLocalSearch::SaveLyricsAssociation()
{
	SONG_LYRIC_MAP::iterator		it;
	TCHAR			szFileName[MAX_FILE_NAME];
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

// COMMENT:
//		取得合szSongName最匹配的歌词文件名
// INPUT:
//		szSongName	-	歌曲名,格式可以为: artist + title
// OUTPUT:
//		szLyricFile	-	匹配的歌曲文件
BOOL CLyricsLocalSearch::GetBestMatchLyrics(LPCTSTR szSongFile, LPCTSTR szArtist, LPCTSTR szTitle, TCHAR *szLyricFile)
{
	TCHAR			szEmbeddedLyrics[MAX_PATH];

	szLyricFile[0] = '\0';

	tstring fullTitle = FormatMediaTitle(szArtist, szTitle);

	// 首先到关连表中搜索.
	if (GetAssociateLyrics(GetAssociateFileKeyword(szSongFile, fullTitle.c_str()).c_str(), szLyricFile, MAX_PATH))
		return TRUE;

	V_LRCSEARCHRESULT		vLyrics;

	//
	// 搜索嵌入的歌词
	//
	EmptyStr(szEmbeddedLyrics);
	SearchEmbeddedLyrics(szSongFile, vLyrics, true);
	V_LRCSEARCHRESULT::iterator	it = vLyrics.GetTheBestMatchLyrics();
	if (it != vLyrics.end())
	{
		LrcSearchResult		&result = *it;
		if (result.nMatchValue >= MATCH_VALUE_EMBEDDED_LRC)
		{
			_tcscpy_safe(szLyricFile, MAX_PATH, result.strUrl.c_str());
			return TRUE;
		}
		_tcscpy_safe(szEmbeddedLyrics, CountOf(szEmbeddedLyrics), result.strUrl.c_str());
	}

	//
	// 同目录下的lrc歌词
	//
	_tcscpy_safe(szLyricFile, MAX_PATH, szSongFile);
	FileSetExt(szLyricFile, MAX_PATH, _T(".lrc"));
	if (IsFileExist(szLyricFile))
		return TRUE;

	//
	// 同目录下的srt字幕
	//
	_tcscpy_safe(szLyricFile, MAX_PATH, szSongFile);
	FileSetExt(szLyricFile, MAX_PATH, _T(".srt"));
	if (IsFileExist(szLyricFile))
		return TRUE;

	// Search lyrics by artist and title.
	if (!IsEmptyString(szTitle))
	{
		BOOL	bTitleIgnoreNumb, bArtIgnoreNumb;

		bTitleIgnoreNumb = IsIgnoreNumber(szTitle);
		bArtIgnoreNumb = IsIgnoreNumber(szArtist);

		SearchLyrics(szSongFile, szTitle, bTitleIgnoreNumb, szArtist, bArtIgnoreNumb, &vLyrics);
		V_LRCSEARCHRESULT::iterator itBest = vLyrics.GetTheBestMatchLyrics(TRUE);
		if (itBest != vLyrics.end())
		{
			// 自动选择的最优歌词，匹配值必须大于 MATCH_VALUE_OK
			if ((*itBest).nMatchValue >= MATCH_VALUE_OK)
			{
				_tcscpy_safe(szLyricFile, MAX_PATH, (*itBest).strUrl.c_str());
				return TRUE;
			}
		}
	}

	// Search lyrics by song file name
	{
		BOOL	bTitleIgnoreNumb, bArtIgnoreNumb;
		TCHAR	szArtist[MAX_PATH], szTitle[MAX_PATH];

		AnalysisSongArTitle(szTitle, bTitleIgnoreNumb, szArtist, bArtIgnoreNumb, szSongFile);

		if (!IsEmptyString(szTitle))
		{
			SearchLyrics(szSongFile, szTitle, bTitleIgnoreNumb, szArtist, bArtIgnoreNumb, &vLyrics);
			V_LRCSEARCHRESULT::iterator itBest = vLyrics.GetTheBestMatchLyrics(TRUE);
			if (itBest != vLyrics.end())
			{
				// 自动选择的最优歌词，匹配值必须大于 MATCH_VALUE_OK
				if ((*itBest).nMatchValue >= MATCH_VALUE_OK)
				{
					_tcscpy_safe(szLyricFile, MAX_PATH, (*itBest).strUrl.c_str());
					return TRUE;
				}
			}
		}
	}

	// embedded text lyrics?
	if (!IsEmptyString(szEmbeddedLyrics))
	{
		_tcscpy_safe(szLyricFile, MAX_PATH, szEmbeddedLyrics);
		return true;
	}

	// same name text lyrics in folder
	_tcscpy_safe(szLyricFile, MAX_PATH, szSongFile);
	FileSetExt(szLyricFile, MAX_PATH, _T(".txt"));
	if (IsFileExist(szLyricFile))
		return TRUE;

	if (!vLyrics.empty())
	{
		V_LRCSEARCHRESULT::iterator itBest = vLyrics.GetTheBestMatchLyrics(TRUE);
		if (itBest != vLyrics.end())
		{
			LrcSearchResult		&result = *itBest;
			// 自动选择的最优歌词，匹配值必须大于 MATCH_VALUE_OK
			if (result.nMatchValue >= MATCH_VALUE_OK)
			{
				_tcscpy_safe(szLyricFile, MAX_PATH, result.strUrl.c_str());
				return TRUE;
			}

			// Artist = "", Min match value >= MATCH_VALUE_MIN_DL
			if (IsEmptyString(szArtist) && result.nMatchValue > MATCH_VALUE_MIN_DL)
			{
				_tcscpy_safe(szLyricFile, MAX_PATH, result.strUrl.c_str());
				return TRUE;
			}
		}
	}

	return FALSE;
}
