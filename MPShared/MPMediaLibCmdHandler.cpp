// MPMediaLibCmdHandler.cpp: implementation of the CMPMediaLibCmdHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerAppBase.h"
#include "MPMediaLibCmdHandler.h"
#ifdef _WIN32_WCE
#include "FolderBrowseDlg.h"
#endif
#include "MPHelper.h"

void CMediaLibTreeProvider::Init()
{
	Item		item;

	item.bUptodate = false;
	item.folderType = FT_MEDIA_LIB;
	item.strName = _T("Media Library");
	item.nImageIndex = II_MEDIA_LIB;
	m_tree.AddChild(item);

	m_tree.ChToChild(0);

	item.bUptodate = false;
	item.folderType = FT_ALL_SONGS_BY_ARTIST;
	item.strName = _T("All Songs By Artist");
	item.nImageIndex = II_MUSIC;
	m_tree.AddChild(item);

	item.folderType = FT_ALL_ARTIST;
	item.strName = _T("Artist");
	item.nImageIndex = II_ARTIST;
	m_tree.AddChild(item);

	item.folderType = FT_ALL_ALBUM;
	item.strName = _T("Album");
	item.nImageIndex = II_ALBUM;
	m_tree.AddChild(item);

	item.folderType = FT_ALL_PLAYLIST_FILES;
	item.strName = _T("Playlists");
	item.nImageIndex = II_PLAYLISTS;
	m_tree.AddChild(item);

	item.folderType = FT_TOP_PLAYED;
	item.strName = _T("Top 50 played");
	item.nImageIndex = II_ALBUM;
	m_tree.AddChild(item);

	item.folderType = FT_TOP_RATING;
	item.strName = _T("Top 50 rating");
	item.nImageIndex = II_ALBUM;
	m_tree.AddChild(item);

	item.folderType = FT_RECENT_PLAYED;
	item.strName = _T("Recent played");
	item.nImageIndex = II_ALBUM;
	m_tree.AddChild(item);

	item.folderType = FT_RECENT_ADDED;
	item.strName = _T("Recent Added");
	item.nImageIndex = II_ALBUM;
	m_tree.AddChild(item);

	item.folderType = FT_NOW_PLAYING;
	item.strName = _T("Now Playing");
	item.nImageIndex = II_NOW_PLAYING;
	m_tree.AddChild(item);

	g_Player.GetMediaLibrary(&m_mediaLib);
}

void CMediaLibTreeProvider::Close()
{
	m_mediaLib.Release();
	m_tree.Clear();
}

void CMediaLibTreeProvider::Update()
{

}

bool CMediaLibTreeProvider::EnumChildren(V_ITEMS &vItems)
{
	for (int i = 0; i < m_tree.GetChildrenCount(); i++)
	{
		vItems.push_back(m_tree.GetChild(i));
	}

	return true;
}

bool CMediaLibTreeProvider::ChToChild(int nIndex)
{
	if (!m_tree.ChToChild(nIndex))
		return false;

	Item	&item = m_tree.GetCurNodeData();

	if (item.bUptodate)
		return true;

	CMPAutoPtr<IPlaylist>	playlist;
	MLRESULT				nRet = ERR_OK;

	item.bUptodate = true;

	if (item.folderType == FT_ALL_PLAYLIST_FILES)
	{
		// nRet = m_mediaLib->GetAll(&playlist);
		int			nLevel = 3;
		vector<tstring>	vFiles;
		Item				newitem;

		newitem.bUptodate = false;
		newitem.folderType = FT_PLAYLIST_FILE;
		newitem.nImageIndex = II_PLAYLISTS;

		// EnumPlaylists(_T("\\"), nLevel, vFiles);
		EnumPlaylistsFast(vFiles);
		for (int i = 0; i < (int)vFiles.size(); i++)
		{
			newitem.strName = vFiles[i];
			m_tree.AddChild(newitem);
		}
	}
	else if (item.folderType == FT_PLAYLIST_FILE)
	{
		g_Player.ClearPlaylist();
		g_Player.AddToPlaylist(item.strName.c_str());
		g_Player.SaveCurrentPlaylist();
	}
	else if (item.folderType == FT_ALL_SONGS_BY_ARTIST)
	{
		nRet = m_mediaLib->GetAll(&playlist, MLOB_ARTIST, -1);
	}
	else if (item.folderType == FT_ALL_ARTIST)
	{
		CMPAutoPtr<IVXStr>	pvStr;
		MLRESULT			nRet;
		Item				newitem;

		newitem.bUptodate = false;
		newitem.folderType = FT_ARTIST;
		newitem.nImageIndex = II_ARTIST;

		nRet = m_mediaLib->GetAllArtist(&pvStr);
		if (nRet == ERR_OK)
		{
			int		nCount = pvStr->size();
			for (int i = 0; i < nCount; i++)
			{
				newitem.strName = pvStr->at(i);
				if (newitem.strName.empty())
				{
					newitem.strName = _T("Unknown artist");
					newitem.folderType = FT_ARTIST_UNKNOWN;
					m_tree.AddChild(newitem);
					newitem.folderType = FT_ARTIST;
					continue;
				}
				m_tree.AddChild(newitem);
			}
		}
	}
	else if (item.folderType == FT_ALL_ALBUM)
	{
		CMPAutoPtr<IVXStr>	pvStr;
		MLRESULT			nRet;
		Item				newitem;

		newitem.bUptodate = false;
		newitem.folderType = FT_ALBUM;
		newitem.nImageIndex = II_ALBUM;

		nRet = m_mediaLib->GetAllAlbum(&pvStr);
		if (nRet == ERR_OK)
		{
			int		nCount = pvStr->size();
			for (int i = 0; i < nCount; i++)
			{
				newitem.strName = pvStr->at(i);
				if (newitem.strName.empty())
				{
					newitem.strName = _T("Unknown Album");
					newitem.folderType = FT_ALBUM_UNKNOWN;
					m_tree.AddChild(newitem);
					newitem.folderType = FT_ALBUM;
					continue;
				}
				m_tree.AddChild(newitem);
			}
		}
	}
	else if ((item.folderType == FT_ARTIST || item.folderType == FT_ARTIST_UNKNOWN))
	{
		CMPAutoPtr<IVXStr>	pvStr;
		MLRESULT			nRet;
		Item				newitem;

		newitem.bUptodate = false;
		newitem.folderType = FT_ARTIST_ALL_MUSIC;
		newitem.nImageIndex = II_MUSIC;
		newitem.strName = _T("All songs by artist");
		m_tree.AddChild(newitem);

		newitem.bUptodate = false;
		newitem.folderType = FT_ARTIST_TOP_RATING;
		newitem.nImageIndex = II_MUSIC;
		newitem.strName = _T("top rated songs by artist");
		m_tree.AddChild(newitem);

		newitem.bUptodate = false;
		newitem.nImageIndex = II_ALBUM;
		newitem.folderType = FT_ARTIST_ALBUM;

		nRet = m_mediaLib->GetAlbumOfArtist(item.GetValue(), &pvStr);
		if (nRet == ERR_OK)
		{
			int		nCount = pvStr->size();
			for (int i = 0; i < nCount; i++)
			{
				newitem.strName = pvStr->at(i);
				if (newitem.strName.empty())
				{
					newitem.strName = _T("Unknown album");
					newitem.folderType = FT_ARTIST_ALBUM_UNKNOWN;
					m_tree.AddChild(newitem);
					newitem.folderType = FT_ARTIST_ALBUM;
					continue;
				}
				m_tree.AddChild(newitem);
			}
		}
		item.bUptodate = true;
	}
	else if (item.folderType == FT_ARTIST_ALBUM || 
		item.folderType == FT_ARTIST_ALBUM_UNKNOWN || 
		item.folderType == FT_ARTIST_TOP_RATING || 
		item.folderType == FT_ARTIST_ALL_MUSIC)
	{
		Item				&itemArtist = m_tree.GetParentNodeData();

		if (item.folderType == FT_ARTIST_ALL_MUSIC)
			nRet = m_mediaLib->GetByArtist(itemArtist.GetValue(), &playlist, MLOB_NONE, -1);
		else if (item.folderType == FT_ARTIST_TOP_RATING)
		{
			nRet = m_mediaLib->GetByArtist(itemArtist.GetValue(), &playlist, MLOB_RATING, -1);
			if (nRet == ERR_OK && playlist)
				g_Player.FilterLowRatingMedia(playlist);
		}
		else
			nRet = m_mediaLib->GetByAlbum(itemArtist.GetValue(), item.GetValue(), &playlist, MLOB_NONE, -1);
	}
	else if (item.folderType == FT_ALBUM_UNKNOWN || 
		item.folderType == FT_ALBUM)
	{
		nRet = m_mediaLib->GetByAlbum(item.GetValue(), &playlist, MLOB_NONE, -1);
	}
	else if (item.folderType == FT_TOP_RATING)
	{
		nRet = m_mediaLib->GetTopRating(16, &playlist);
		if (nRet == ERR_OK && playlist)
			g_Player.FilterLowRatingMedia(playlist);
	}
	else if (item.folderType == FT_TOP_PLAYED)
	{
		nRet = m_mediaLib->GetTopPlayed(16, &playlist);
	}
	else if (item.folderType == FT_RECENT_ADDED)
	{
		nRet = m_mediaLib->GetRecentAdded(16, &playlist);
	}
	else if (item.folderType == FT_RECENT_PLAYED)
	{
		nRet = m_mediaLib->GetRecentPlayed(16, &playlist);
	}

	if (nRet == ERR_OK && playlist.p)
	{
		AddPlaylistToTree(playlist);
		item.playlist = playlist;
	}

	return true;
}

bool CMediaLibTreeProvider::ChToParent()
{
	return m_tree.ChToParent();
}

bool CMediaLibTreeProvider::ChToPath(V_ITEMS &vPath)
{
	m_tree.ChToRoot();

	for (int i = 0; i < (int)vPath.size(); i++)
	{
		int		nCount = m_tree.GetChildrenCount();
		int		k;
		for (k = 0; k < nCount; k++)
		{
			if (_tcscmp(m_tree.GetChild(k).strName.c_str(), vPath[i].strName.c_str()) == 0)
			{
				ChToChild(k);
				break;
			}
		}
		if (k == nCount)
			return false;
	}

	return true;
}
// 
// bool CMediaLibTreeProvider::ChToPath(int nIndex)
// {
// 	return m_tree.ChToChild(nIndex);
// }

bool CMediaLibTreeProvider::GetPath(V_ITEMS &vPath)
{
	m_tree.GetPath(vPath);

	for (int i = 0; i < (int)vPath.size(); i++)
		vPath[i].playlist.Release();

	return true;
}

CMediaLibTreeProvider::Item CMediaLibTreeProvider::GetChildData(int n)
{
	return m_tree.GetChild(n);
}

bool CMediaLibTreeProvider::IsCurNodePlaylist()
{
	return m_tree.GetCurNodeData().playlist.p != NULL;
}

bool CMediaLibTreeProvider::GetCurNodePlaylist(IPlaylist **playlist)
{
	Item	&item = m_tree.GetCurNodeData();
	if (item.playlist.p)
	{
		*playlist = item.playlist;
		(*playlist)->AddRef();

		return true;
	}

	return false;
}

bool CMediaLibTreeProvider::IsCurNodePlaylistFile()
{
	Item	&item = m_tree.GetCurNodeData();
	if (item.folderType == FT_ALL_PLAYLIST_FILES)
		return true;

	return false;
}

bool CMediaLibTreeProvider::GetCurNodePlaylistFile(int nChildPos, tstring &strPlaylistFile)
{
	if (nChildPos >= 0 && nChildPos < m_tree.GetChildrenCount())
		strPlaylistFile = m_tree.GetChild(nChildPos).strName;
	else
		strPlaylistFile.resize(0);

	return true;
}

void CMediaLibTreeProvider::AddPlaylistToTree(IPlaylist *playlist)
{
	Item				newitem;
	tstring				str;
	
	newitem.bUptodate = true;
	newitem.folderType = FT_MEDIA_FILE;
	newitem.nImageIndex = II_MUSIC;

	int		nCount = playlist->GetCount();
	for (int i = 0; i < nCount; i++)
	{
		CMPAutoPtr<IMedia>	pMedia;
		if (playlist->GetItem(i, &pMedia) == ERR_OK)
		{
			g_Player.FormatMediaTitle(pMedia, str);
			newitem.strName = str;
			m_tree.AddChild(newitem);
		}
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPMediaLibCmdHandler::CMPMediaLibCmdHandler()
{

}

CMPMediaLibCmdHandler::~CMPMediaLibCmdHandler()
{

}

void CMPMediaLibCmdHandler::Init(CSkinWnd *pSkinWnd)
{
	ISkinCmdHandler::Init(pSkinWnd);
	
	m_mediaLibTree.Init();

	if (m_vPathLatest.size())
	{
		m_mediaLibTree.ChToPath(m_vPathLatest);
	}

	ReloadMedialibView();
	UpdateMediaList();
}

// if the command id is processed, return true.
bool CMPMediaLibCmdHandler::OnCommand(int nId)
{
	switch (nId)
	{
	case IDC_PLAY:
		OnDblClickMediaList();
		break;
	case IDOK:
		m_pSkinWnd->Destroy();
		break;
	case IDC_QUEUE_UP:
		{
			CSkinListCtrl	*pMediaList =  (CSkinListCtrl*)m_pSkinWnd->GetUIObjectById(ID_ML_MEDIA_LIST, CSkinListCtrl::ClassName());
			int		nSel = pMediaList->GetNextSelectedItem(-1);
			if (nSel == -1)
				break;

			CMPAutoPtr<IPlaylist>	playlist;
			if (m_mediaLibTree.GetCurNodePlaylist(&playlist))
			{
				CMPAutoPtr<IMedia>	media;
				if (playlist->GetItem(nSel, &media) == ERR_OK)
				{
					CMPAutoPtr<IPlaylist>	curPl;
					if (g_Player.GetCurrentPlaylist(&curPl) == ERR_OK)
					{
						curPl->InsertItem(-1, media);
					}
				}
			}
		}
		break;
	case IDC_NOWPLAYING:
		{
			SkinWndStartupInfo	startInfo(_T("MPSkin"), _T("Playlist"), _T("Playlist.xml"), NULL);
			CMPlayerAppBase::GetMPSkinFactory()->ActiveOrCreateSkinWnd(startInfo);
		}
		break;
	case IDC_ADD_DIR_TO_ML:
		//
		// Add Media to library
		//
		if (OnCmdSongAddDirToMediaLib(m_pSkinWnd))
			ReloadMedialibView();
		break;
	case IDC_REMOVE_FROM_ML:
		{
			CSkinListCtrl	*pMediaList =  (CSkinListCtrl*)m_pSkinWnd->GetUIObjectById(ID_ML_MEDIA_LIST, CSkinListCtrl::ClassName());
			if (pMediaList)
			{
				int		nSel = pMediaList->GetNextSelectedItem(-1);
				if (nSel == -1)
					return true;

				CMPAutoPtr<IPlaylist>		playlist;

				if (m_mediaLibTree.GetCurNodePlaylist(&playlist))
				{
					vector<int>				vIndex;
					CMPAutoPtr<IMedia>		media;
					while (nSel != -1)
					{
						vIndex.push_back(nSel);
						nSel = pMediaList->GetNextSelectedItem(nSel);
					}
					for (int i = vIndex.size() - 1; i >= 0; i--)
					{
						nSel = vIndex[i];
						if (playlist->GetItem(nSel, &media) == ERR_OK)
						{
							CMPAutoPtr<IMediaLibrary>	mediaLib;
							if (g_Player.GetMediaLibrary(&mediaLib) == ERR_OK)
								mediaLib->Remove(&(media.p), false);
							playlist->RemoveItem(nSel);
							pMediaList->DeleteItem(nSel, true);
							m_mediaLibTree.EraseChild(nSel);
						}
					}
					ReloadMedialibView();
				}
			}
		}
		break;
	default:
		return false;
	}

	return true;
}

bool CMPMediaLibCmdHandler::OnCustomCommand(int nID)
{
	switch (nID)
	{
	case CMD_ML_BACK:
		{
			BackHistoryPath();
		}
		break;
	default:
		return false;
	}

	return true;
}

bool CMPMediaLibCmdHandler::OnUIObjNotify(IUIObjNotify *pNotify)
{
	if (pNotify->nID == ID_ML_MEDIA_LIST)
	{
		if (pNotify->pUIObject->IsKindOf(CSkinListCtrl::ClassName()))
		{
			CSkinListCtrl	*pMediaList =  (CSkinListCtrl*)pNotify->pUIObject;
			int		nSel = pMediaList->GetNextSelectedItem(-1);
			if (nSel == -1)
				return true;

			CSkinListCtrlEventNotify	*pListCtrlNotify = (CSkinListCtrlEventNotify *)pNotify;
			if (pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_DBL_CLICK ||
				pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_ENTER ||
				pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_CLICK)
			{
				bool		bClose = false;
				if (m_mediaLibTree.IsCurNodePlaylist())
				{
					if (pListCtrlNotify->cmd != CSkinListCtrlEventNotify::C_CLICK)
					{
						// play current music...
						CMPAutoPtr<IPlaylist>	playlist;
						if (m_mediaLibTree.GetCurNodePlaylist(&playlist))
						{
							g_Player.SetCurrentPlaylist(playlist);
							g_Player.SetCurrentMediaInPlaylist(nSel);
							g_Player.Play();
							bClose = true;
						}
					}
				}
				else if (m_mediaLibTree.IsCurNodePlaylistFile())
				{
					if (pListCtrlNotify->cmd != CSkinListCtrlEventNotify::C_CLICK)
					{
						// play selected playlist
						tstring		strFile;
						if (m_mediaLibTree.GetCurNodePlaylistFile(nSel, strFile))
						{
							g_Player.ClearPlaylist();
							g_Player.AddToPlaylist(strFile.c_str());
							g_Player.SaveCurrentPlaylist();
							g_Player.Play();
							bClose = true;
						}
					}
				}
				else
				{
					CMediaLibTreeProvider::Item	item;

					item = m_mediaLibTree.GetChildData(nSel);
					if (item.folderType == CMediaLibTreeProvider::FT_NOW_PLAYING)
					{
						// 
						bClose = true;

						SkinWndStartupInfo	startInfo(_T("MPSkin"), _T("Playlist"), _T("Playlist.xml"), NULL);
						CMPlayerAppBase::GetMPSkinFactory()->ActiveOrCreateSkinWnd(startInfo);
					}
					else
					{
						AddHistoryPath();
						m_mediaLibTree.ChToChild(nSel);
						UpdateMediaList();
					}
				}

// 				if (bClose)
// 				{
// 					m_pSkinWnd->Destroy();
// 				}
			}
		}

		return true;
	}
/*	else if (pNotify->nID == CMD_ML_GUIDE)
	{
		if (pNotify->pUIObject->IsKindOf(CSkinDroplistCtrl::ClassName()))
		{
			//
			// Change to new path.
			//
			CSkinDroplistCtrl		*pCtrl = (CSkinDroplistCtrl*)pNotify->pUIObject;
			CSkinDroplistCtrlEventNotify	*pDroplistEvent = (CSkinDroplistCtrlEventNotify*)pNotify;
			if (pDroplistEvent->cmd == CSkinDroplistCtrlEventNotify::C_SEL_CHANGED)
			{
				vector<tstring>		vPath;
				CMediaLibTreeProvider::V_ITEMS	vPathNew;
				pCtrl->GetSelPath(vPath);
				m_mediaLibTree.GetPath(vPathNew);
				ASSERT(vPathNew.size() >= vPath.size());
				if (vPathNew.size() > vPath.size())
				{
					vPathNew.resize(vPath.size());
					AddHistoryPath();
					m_mediaLibTree.ChToPath(vPathNew);
					UpdateMediaList();
				}
			}
		}
		return true;
	}*/

	return false;
}

void CMPMediaLibCmdHandler::OnDblClickMediaList()
{
	CSkinListCtrl	*pMediaList = (CSkinListCtrl*)m_pSkinWnd->GetUIObjectById(ID_ML_MEDIA_LIST, CSkinListCtrl::ClassName());
	if (!pMediaList)
		return;

	CSkinListCtrlEventNotify	notify(pMediaList, -1, -1);
	notify.cmd = CSkinListCtrlEventNotify::C_DBL_CLICK;

	OnUIObjNotify(&notify);
}

void CMPMediaLibCmdHandler::ReloadMedialibView()
{
	m_mediaLibTree.Close();
	m_mediaLibTree.Init();
	m_mediaLibTree.GetPath(m_vPathLatest);
	if (m_vPathLatest.size())
	{
		m_mediaLibTree.ChToPath(m_vPathLatest);
		UpdateMediaList();
	}

}

void CMPMediaLibCmdHandler::UpdateMediaList()
{
	CSkinListCtrl	*plistCtrl;
	plistCtrl = (CSkinListCtrl*)m_pSkinWnd->GetUIObjectById(ID_ML_MEDIA_LIST, CSkinListCtrl::ClassName());
	if (plistCtrl)
	{
		CMediaLibTreeProvider::V_ITEMS	vItems;

		m_mediaLibTree.EnumChildren(vItems);

		if (plistCtrl->GetColumnCount() == 0)
			plistCtrl->AddColumn(_T(""), 230);

		plistCtrl->DeleteAllItems(false);

		for (int i = 0; i < (int)vItems.size(); i++)
		{
			CMediaLibTreeProvider::Item	&item = vItems[i];
			if (item.folderType == CMediaLibTreeProvider::FT_PLAYLIST_FILE)
			{
				TCHAR		szFileTitle[MAX_PATH];

				FileGetTitle(szFileTitle, CountOf(szFileTitle), item.strName.c_str());
				plistCtrl->InsertItem(plistCtrl->GetItemCount(), 
					szFileTitle, item.nImageIndex);
			}
			else
				plistCtrl->InsertItem(plistCtrl->GetItemCount(), 
					item.strName.c_str(), item.nImageIndex);
		}

		if (vItems.size())
			plistCtrl->SetItemSelectionState(0, true);
		plistCtrl->Invalidate();
	}

/*	CSkinDroplistCtrl	*pDroplist;
	pDroplist = (CSkinDroplistCtrl*)m_pSkinWnd->GetUIObjectById(CMD_ML_GUIDE, CSkinDroplistCtrl::ClassName());
	if (pDroplist)
	{
		CMediaLibTreeProvider::V_ITEMS	vItems;

		m_mediaLibTree.GetPath(vItems);

		pDroplist->Clear();

		for (int i = 0; i < (int)vItems.size(); i++)
		{
			pDroplist->Append(vItems[i].nImageIndex, vItems[i].strName.c_str());
		}

		pDroplist->Invalidate();
	}*/
}

void CMPMediaLibCmdHandler::AddHistoryPath()
{
	HistroyItem		item;
	CSkinListCtrl	*plistCtrl;
	plistCtrl = (CSkinListCtrl*)m_pSkinWnd->GetUIObjectById(ID_ML_MEDIA_LIST, CSkinListCtrl::ClassName());
	if (plistCtrl)
		item.nSelChild = plistCtrl->GetNextSelectedItem();
	else
		item.nSelChild = 0;

	m_mediaLibTree.GetPath(item.path);

	m_historyPath.push_back(item);
	if (m_historyPath.size() > 10)
		m_historyPath.erase(m_historyPath.begin());
}

void CMPMediaLibCmdHandler::BackHistoryPath()
{
	if (m_historyPath.size())
	{
		CSkinListCtrl	*plistCtrl;
		int				nSelRow = m_historyPath.back().nSelChild;

		m_mediaLibTree.ChToPath(m_historyPath.back().path);
		UpdateMediaList();
		m_historyPath.pop_back();

		plistCtrl = (CSkinListCtrl*)m_pSkinWnd->GetUIObjectById(ID_ML_MEDIA_LIST, CSkinListCtrl::ClassName());
		if (plistCtrl && nSelRow >= 0 && nSelRow < plistCtrl->GetItemCount())
		{
			plistCtrl->SetItemSelectionState(0, false);
			plistCtrl->SetItemSelectionState(nSelRow, true);
			plistCtrl->MakeSureRowVisible(nSelRow);
			plistCtrl->Invalidate();
		}
	}
}
