#include "stdafx.h"
#include "MPlayerApp.h"
#include "Player.h"
#include "EventsDispatcher.h"

static LPCTSTR		SZ_SUPPORTED_IMG_EXT[] = { _T(".jpg"), _T(".gif"), _T(".bmp"), _T(".png") };

bool IsSupportedImageFile(LPCTSTR szFile)
{
	LPCTSTR			szExt;
	int				i;

	szExt = _tcsrchr(szFile, '.');
	if (!szExt)
		return false;

	for (i = 0; i < CountOf(SZ_SUPPORTED_IMG_EXT); i++)
	{
		if (_tcsicmp(szExt, SZ_SUPPORTED_IMG_EXT[i]) == 0)
			return true;
	}

	return false;
}

/*
bool GetImageFileWithName(LPCTSTR szName, V_Str &vPicFiles)
{
	tstring				strPicFile;
	int			i;

	for (i = 0; i < CountOf(SZ_SUPPORTED_IMG_EXT); i++)
	{
		strPicFile = szName;
		strPicFile += SZ_SUPPORTED_IMG_EXT[i];
		if (IsFileExist(strPicFile.c_str()))
		{
			// Find it:)
			vPicFiles.push_back(strPicFile);
		}
	}

	return false;
}*/

bool GetCurrentMediaAlbumArtInSongDir(V_Str &vPicFiles)
{
	TCHAR		szDir[MAX_PATH], szFileTitle[MAX_PATH];
	tstring		strFile;
	LPCTSTR		szAlbumName = g_Player.GetAlbum(), szSongFile = g_Player.GetSrcMedia();
	V_Str		vOtherPicFiles;
	int			nFileCount = 0;

	if (!IsFileExist(szSongFile))
		return false;

	CMLFileFind		find;

	FileGetPath(szDir, CountOf(szDir), szSongFile);
	if (!find.OpenDir(szDir))
		return false;

	tstring		file;
	while (find.FindNext())
	{
		// enum every image file

		if (find.IsCurDir())
			continue;

		nFileCount++;
		if (!IsSupportedImageFile(find.GetCurName()))
			continue;

		strFile = szDir;
		strFile += find.GetCurName();
		FileGetTitle(szFileTitle, CountOf(szFileTitle), find.GetCurName());

		//
		// album.jpg
		//
		if (_tcsicmp(szFileTitle, szAlbumName) == 0)
		{
			vPicFiles.push_back(strFile);
			continue;
		}

		//
		// artist - album.jpg
		//
		tstring		strArAl;
		strArAl = g_Player.GetArtist();
		if (!IsEmptyString(szAlbumName))
		{
			if (!strArAl.empty())
				strArAl += _T(" - ");
			strArAl += szAlbumName;
		}
		if (strArAl.empty())
		{
			TCHAR		szTemp[MAX_PATH];
			FileGetTitle(szTemp, CountOf(szTemp), g_Player.GetSrcMedia());
			strArAl = szTemp;
		}
		if (_tcsicmp(szFileTitle, strArAl.c_str()) == 0)
		{
			vPicFiles.push_back(strFile);
			continue;
		}

		//
		// Folder.jpg
		//
		if (_tcsicmp(szFileTitle, _T("Folder")) == 0)
		{
			vPicFiles.push_back(strFile);
			continue;
		}

		// other files
		vOtherPicFiles.push_back(strFile);
	}

	if (nFileCount < 60 && vPicFiles.size() < 1)
	{
		vPicFiles.insert(vPicFiles.begin(), vOtherPicFiles.begin(), vOtherPicFiles.end());
	}

	return true;
}

CCurMediaAlbumArt::CCurMediaAlbumArt()
{
	m_bLoaded = false;
}

CCurMediaAlbumArt::~CCurMediaAlbumArt()
{
}

int CCurMediaAlbumArt::Load()
{
	tstring		strSongFile;
	int			nRet;

	Close();

	m_bLoaded = true;
	strSongFile = g_Player.GetSrcMedia();

	// Load id3v2 pictures...
	CID3v2IF		id3v2(ED_SYSDEF);
	nRet = id3v2.Open(strSongFile.c_str(), false, false);
	if (nRet == ERR_OK)
	{
		nRet = id3v2.GetPictures(m_id3v2Pic);
	}

	// load album art in song dir, with same album name.
	GetCurrentMediaAlbumArtInSongDir(m_vAlbumPicFile);

	if (GetPicCount() > 0)
		return ERR_OK;
	else
		return ERR_NOT_FOUND;
}

void CCurMediaAlbumArt::Close()
{
	m_id3v2Pic.Free();
	m_vAlbumPicFile.clear();
	m_bLoaded = false;
}

PRAW_IMAGE_DATA CCurMediaAlbumArt::LoadAlbumArtByIndex(int nIndex)
{
	if (nIndex < 0)
		return NULL;

	if (nIndex < (int)m_id3v2Pic.m_vItems.size())
	{
		ID3v2Pictures::ITEM	*pic = m_id3v2Pic.m_vItems[nIndex];
		return LoadRawImageDataFromMem(pic->m_buffPic.c_str(), pic->m_buffPic.size());
	}
	else
	{
		nIndex -= (int)m_id3v2Pic.m_vItems.size();
		if (nIndex < (int)m_vAlbumPicFile.size())
			return LoadRawImageDataFromFile(m_vAlbumPicFile[nIndex].c_str());
		else
			return NULL;
	}
}

int CCurMediaAlbumArt::GetPicCount()
{
	int		n = m_id3v2Pic.m_vItems.size();

	n += m_vAlbumPicFile.size();

	return n;
}

