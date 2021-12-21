// MPMediaTree.cpp: implementation of the CMPMediaTree class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPMediaTree.h"
#include "MPHelper.h"

class CSortVInt : public vector<int>
{
public:
	void Add(int n)
	{
		iterator	it, itEnd;
		itEnd = end();
		it = begin();
		for (; it != itEnd; ++it)
		{
			if (n > *it)
				continue;
			else if (n == *it)
				return;
			else
			{
				insert(it, n);
				return;
			}
		}
		push_back(n);
	}

};

void CMPMTNMediaLibrary::OnUpdate()
{
	CMPAutoPtr<IMediaLibrary>	mediaLib;

	if (g_Player.GetMediaLibrary(&mediaLib) != ERR_OK)
		return;

	m_playlist.Release();

	if (folderType == FT_ALL_SONGS_BY_ARTIST)
	{
		mediaLib->GetAll(&m_playlist, MLOB_ARTIST, -1);
	}
	else if (folderType == FT_ALL_ARTIST)
	{
		CMPAutoPtr<IVXStr>	pvStr;
		MLRESULT			nRet;

		nRet = mediaLib->GetAllArtist(&pvStr);
		if (nRet == ERR_OK)
		{
			int		nCount = pvStr->size();
			for (int i = 0; i < nCount; i++)
			{
				if (IsEmptyString(pvStr->at(i)))
					AddNewNode(_T("Unknown artist"), FT_ARTIST_UNKNOWN, II_ARTIST, false);
				else
					AddNewNode(pvStr->at(i), FT_ARTIST, II_ARTIST, false);
			}
		}
	}
	else if (folderType == FT_ALL_ALBUM)
	{
		CMPAutoPtr<IVXStr>	pvStr;
		MLRESULT			nRet;

		nRet = mediaLib->GetAllAlbum(&pvStr);
		if (nRet == ERR_OK)
		{
			int		nCount = pvStr->size();
			for (int i = 0; i < nCount; i++)
			{
				if (IsEmptyString(pvStr->at(i)))
					AddNewNode(_T("Unknown Album"), FT_ALBUM_UNKNOWN, II_ALBUM, false);
				else
					AddNewNode(pvStr->at(i), FT_ALBUM, II_ALBUM, false);
			}
		}
	}
	else if (folderType == FT_ARTIST || 
		folderType == FT_ARTIST_UNKNOWN)
	{
		CMPAutoPtr<IVXStr>	pvStr;

#ifdef _WIN32_WCE
		AddNewNode(_T("All songs by artist"), FT_ARTIST_ALL_MUSIC, II_MUSIC, false);
#endif

		AddNewNode(_T("Top rated songs by artist"), FT_ARTIST_TOP_RATING, II_MUSIC, false);

		if (folderType == FT_ARTIST_UNKNOWN)
		{
			mediaLib->GetByArtist(_T(""), &m_playlist, MLOB_NONE, -1);
			mediaLib->GetAlbumOfArtist(_T(""), &pvStr);
		}
		else
		{
			mediaLib->GetByArtist(m_strName.c_str(), &m_playlist, MLOB_NONE, -1);
			mediaLib->GetAlbumOfArtist(m_strName.c_str(), &pvStr);
		}

		int		nCount = pvStr->size();
		for (int i = 0; i < nCount; i++)
		{
			if (IsEmptyString(pvStr->at(i)))
				AddNewNode(_T("Unknown album"), FT_ARTIST_ALBUM_UNKNOWN, II_ALBUM, false);
			else
				AddNewNode(pvStr->at(i), FT_ARTIST_ALBUM, II_ALBUM, false);
		}
	}
	else if (folderType == FT_ARTIST_ALBUM || 
		folderType == FT_ARTIST_ALBUM_UNKNOWN || 
		folderType == FT_ARTIST_TOP_RATING || 
		folderType == FT_ARTIST_ALL_MUSIC)
	{
		if (!m_pParent)
			return;

		tstring		strArtist;
		CMPMTNMediaLibrary	*pNodeArtist = (CMPMTNMediaLibrary *)m_pParent;
		if (pNodeArtist->folderType != FT_ARTIST_UNKNOWN)
			strArtist = pNodeArtist->m_strName;

		if (folderType == FT_ARTIST_ALL_MUSIC)
			mediaLib->GetByArtist(strArtist.c_str(), &m_playlist, MLOB_NONE, -1);
		else if (folderType == FT_ARTIST_TOP_RATING)
		{
			MLRESULT nRet = mediaLib->GetByArtist(strArtist.c_str(), &m_playlist, MLOB_RATING, -1);
			if (nRet == ERR_OK && m_playlist)
				g_Player.FilterLowRatingMedia(m_playlist);
		}
		else
		{
			mediaLib->GetByAlbum(strArtist.c_str(), 
				folderType == FT_ARTIST_ALBUM_UNKNOWN ? _T("") : m_strName.c_str(),
				&m_playlist, MLOB_NONE, -1);
		}
	}
	else if (folderType == FT_ALBUM_UNKNOWN || 
		folderType == FT_ALBUM)
	{
		mediaLib->GetByAlbum(
			folderType == FT_ALBUM_UNKNOWN ? _T("") : m_strName.c_str(), &m_playlist, MLOB_NONE, -1);
	}
	//////////////////////////////////////////////////////////////////////////
	// Genre
	else if (folderType == FT_ALL_GENRE)
	{
		CMPAutoPtr<IVXStr>	pvStr;
		MLRESULT			nRet;

		nRet = mediaLib->GetAllGenre(&pvStr);
		if (nRet == ERR_OK)
		{
			int		nCount = pvStr->size();
			for (int i = 0; i < nCount; i++)
			{
				if (IsEmptyString(pvStr->at(i)))
					AddNewNode(_T("Unknown Genre"), FT_GENRE_UNKNOWN, II_GENRE, false);
				else
					AddNewNode(pvStr->at(i), FT_GENRE, II_GENRE, false);
			}
		}
	}
	else if (folderType == FT_GENRE_UNKNOWN || 
		folderType == FT_GENRE)
	{
		mediaLib->GetByGenre(
			folderType == FT_GENRE_UNKNOWN ? _T("") : m_strName.c_str(), &m_playlist, MLOB_NONE, -1);
	}
	// Year
	else if (folderType == FT_ALL_YEAR)
	{
		// 对year 进行分类，每10年为一个子节点
		CMPAutoPtr<IVInt>	pvInt;
		MLRESULT			nRet;
		CSortVInt			vTenYear;
		int					i;
		TCHAR				szName[256];

		nRet = mediaLib->GetAllYear(&pvInt);
		if (nRet == ERR_OK)
		{
			int		nCount = pvInt->size();
			for (i = 0; i < nCount; i++)
			{
				vTenYear.Add(pvInt->at(i) / 10);
// 				if (pvInt->at(i) == 0)
// 					AddNewNode(_T("Unknown Genre"), FT_GENRE_UNKNOWN, II_ALBUM, false);
// 				else
// 					AddNewNode(pvStr->at(i), FT_GENRE, II_ALBUM, false);
			}
		}

		for (i = 0; i < (int)vTenYear.size(); i++)
		{
			CMPMTNMediaLibrary		*pNewNode;
			int						nDecadeYear = vTenYear[i];

			if (nDecadeYear == 0)
			{
				AddNewNode(_T("Unknown"), FT_YEAR, II_YEAR, false);
				continue;
			}

			pNewNode = new CMPMTNMediaLibrary;
			pNewNode->SetUpdated(true);
			pNewNode->folderType = FT_NOT_SET;
			pNewNode->m_nImageIndex = II_YEAR;
			_stprintf(szName, _T("%d's"), nDecadeYear * 10);
			pNewNode->m_strName = szName;
			AddChildBack(pNewNode);
			for (int k = 0; k < (int)pvInt->size(); k++)
			{
				if (vTenYear[i] == pvInt->at(k) / 10)
				{
					_stprintf(szName, _T("%d"), pvInt->at(k));
					pNewNode->AddNewNode(szName, FT_YEAR, II_YEAR, false);
				}
			}
		}
	}
	else if (folderType == FT_YEAR)
	{
		int		nYear;
		nYear = _ttoi(m_strName.c_str());
		mediaLib->GetByYear(nYear, &m_playlist, MLOB_NONE, -1);
	}
	// rating
	else if (folderType == FT_ALL_RATING)
	{
		int					i;
		tstring				strName = _T("*****");

		AddNewNode(_T("Not rated"), FT_NOT_RATED, II_RATING, false);

		for (i = 4; i >= 0; i--)
		{
			strName.resize(i + 1);
			AddNewNode(strName.c_str(), FT_RATING_1 + i, II_RATING, false);
		}
//		mediaLib->GetTopRating(16, &m_playlist);
//		if (nRet == ERR_OK && m_playlist)
//			g_Player.FilterLowRatingMedia(m_playlist);
	}
	else if (folderType == FT_NOT_RATED || folderType == FT_RATING_1 ||
		folderType == FT_RATING_2 || folderType == FT_RATING_3 ||
		folderType == FT_RATING_4 || folderType == FT_RATING_5)
	{
		mediaLib->GetByRating(folderType - FT_NOT_RATED, &m_playlist, MLOB_RATING, -1);
	}
	else if (folderType == FT_TOP_PLAYED)
	{
		mediaLib->GetTopPlayed(50, &m_playlist);
		SetUpdated(false);
	}
	else if (folderType == FT_RECENT_ADDED)
	{
		mediaLib->GetRecentAdded(32, &m_playlist);
		SetUpdated(false);
	}
	else if (folderType == FT_RECENT_PLAYED)
	{
		mediaLib->GetRecentPlayed(32, &m_playlist);
		SetUpdated(false);
	}
}

void CMPMTNDiskDevice::OnUpdate()
{
	if (m_nImageIndex == II_MYCOMPUTER)
	{
		// root of my computer
#ifdef _WIN32
		TCHAR	szStr[512] = _T("");
		int		nRet;
		V_Str	vStr;
		int		nImage;
		UINT	nType;

		nRet = GetLogicalDriveStrings(CountOf(szStr), szStr);
		MultiStrToVStr(szStr, vStr);
		for (int i = 0; i < (int)vStr.size(); i++)
		{
			nType = GetDriveType(vStr[i].c_str());
			if (nType == DRIVE_REMOVABLE)
				nImage = II_DRIVE_REMOVABLE;
			else if (nType == DRIVE_FIXED)
				nImage = II_DRIVE_FIXED;
			else if (nType == DRIVE_REMOTE)
				nImage = II_DRIVE_REMOTE;
			else if (nType == DRIVE_CDROM)
				nImage = II_DRIVE_CDROM;
			else if (nType == DRIVE_RAMDISK)
				nImage = II_DRIVE_RAMDISK;
			else
				continue;

			AddNewNode(vStr[i].c_str(), vStr[i].c_str(), nImage, nImage);
		}
#else
#endif
	}
	else
	{
		CMLFileFind		finder;
		tstring			strFile;

		if (finder.OpenDir(m_strDir.c_str()))
		{
			while (finder.FindNext())
			{
				if (finder.IsCurDir())
				{
					if (_tcscmp(finder.GetCurName(), _T(".")) != 0 && 
						_tcscmp(finder.GetCurName(), _T("..")) != 0)
						AddNewNode(finder.GetCurName(), (m_strDir + finder.GetCurName()).c_str(), II_FOLDER_CLOSED, II_FOLDER_OPENED);
				}
				else
				{
					// Is file an audio media?
					if (g_Player.IsExtAudioFile(FileGetExt(finder.GetCurName())))
					{
						if (!m_playlist)
						{
							if (g_Player.NewPlaylist(&m_playlist) != ERR_OK)
								return;
						}
						strFile = m_strDir + finder.GetCurName();
						CMPAutoPtr<IMedia>	media;
						if (g_Player.NewMedia(&media, strFile.c_str()) == ERR_OK)
							m_playlist->InsertItem(-1, media);
					}
				}
			}
		}
	}
}

void CMPMTNPlaylists::OnUpdate()
{
	if (folderType == FT_ALL_PLAYLISTS)
	{
		// enum all the playlist files
		vector<tstring>	vFiles;

		EnumPlaylistsFast(vFiles);
		for (int i = 0; i < (int)vFiles.size(); i++)
		{
			CMPMTNPlaylists		*playlistNode;

			playlistNode = new CMPMTNPlaylists;
			playlistNode->m_strName = FileGetName(vFiles[i].c_str());
			playlistNode->m_nImageIndex = II_PLAYLISTS;
			playlistNode->folderType = FT_PLAYLIST_FILE;
			playlistNode->m_strPlaylistFile = vFiles[i];
			AddChildBack(playlistNode);
		}
	}
	else if (folderType == FT_PLAYLIST_FILE)
	{
		// 
		if (g_Player.NewPlaylist(&m_playlist) != ERR_OK)
			return;
		LoadPlaylist(g_Player.GetIMPlayer(), m_playlist, m_strPlaylistFile.c_str());
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPMediaTree::CMPMediaTree()
{

}

CMPMediaTree::~CMPMediaTree()
{

}

bool CMPMediaTree::Init()
{
	IMPMediaTreeNode	*pNodeChild;
	CMPMTNMediaLibrary	*pMediaLibNode;

	pMediaLibNode = new CMPMTNMediaLibrary;
	pMediaLibNode->m_strName = _T("Media Library");
	pMediaLibNode->m_nImageIndex = II_MEDIA_LIB;
	pMediaLibNode->SetExpanded(true);
	pMediaLibNode->SetUpdated(true);
	m_root.AddChildBack(pMediaLibNode);


	pMediaLibNode->AddNewNode(_T("Location"), CMPMTNMediaLibrary::FT_LOCATION, II_LOCATION, false);
	pMediaLibNode->AddNewNode(_T("All Songs By Artist"), CMPMTNMediaLibrary::FT_ALL_SONGS_BY_ARTIST, II_MUSIC, false);
	pMediaLibNode->AddNewNode(_T("Artist"), CMPMTNMediaLibrary::FT_ALL_ARTIST, II_ARTIST, false);
	pMediaLibNode->AddNewNode(_T("Album"), CMPMTNMediaLibrary::FT_ALL_ALBUM, II_ALBUM, false);
	pMediaLibNode->AddNewNode(_T("Genre"), CMPMTNMediaLibrary::FT_ALL_GENRE, II_GENRE, false);
	pMediaLibNode->AddNewNode(_T("Year"), CMPMTNMediaLibrary::FT_ALL_YEAR, II_YEAR, false);
	pMediaLibNode->AddNewNode(_T("Rating"), CMPMTNMediaLibrary::FT_ALL_RATING, II_RATING, false);
	pMediaLibNode->AddNewNode(_T("Top 50 played"), CMPMTNMediaLibrary::FT_TOP_PLAYED, II_ALBUM, false);
	pMediaLibNode->AddNewNode(_T("Recent played"), CMPMTNMediaLibrary::FT_RECENT_PLAYED, II_ALBUM, false);
	pMediaLibNode->AddNewNode(_T("Recent Added"), CMPMTNMediaLibrary::FT_RECENT_ADDED, II_ALBUM, false);

	pNodeChild = new CMPMTNPlaylists;
	pNodeChild->m_strName = _T("Playlists");
	pNodeChild->m_nImageIndex = II_PLAYLISTS;
	m_root.AddChildBack(pNodeChild);

	CMPMTNDiskDevice	*pDeviceNode;
	pDeviceNode = new CMPMTNDiskDevice;
	pDeviceNode->SetUpdated(false);
	pDeviceNode->m_strName = _T("My Computer");
	pDeviceNode->m_nImageIndex = II_MYCOMPUTER;
	m_root.AddChildBack(pDeviceNode);

	return true;
}

void CMPMediaTree::Close()
{
	
}

bool CMPMediaTree::GetCurNodePlaylist(IPlaylist **playlist)
{
	IMPMediaTreeNode	*pNode;

	pNode = (IMPMediaTreeNode *)GetSelNode();
	if (!pNode)
		return false;

	if (pNode->m_playlist.p)
	{
		*playlist = pNode->m_playlist;
		(*playlist)->AddRef();

		return true;
	}

	return false;
}
