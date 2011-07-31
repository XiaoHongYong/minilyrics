// MPCmdHandlerOfMediaGuide.cpp: implementation of the CMPCmdHandlerOfMediaGuide class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerAppBase.h"
#include "MPCmdHandlerOfMediaGuide.h"
#ifdef _WIN32_WCE
#include "FolderBrowseDlg.h"
#endif
#include "Player.h"
#include "MPHelper.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPCmdHandlerOfMediaGuide::CMPCmdHandlerOfMediaGuide()
{

}

CMPCmdHandlerOfMediaGuide::~CMPCmdHandlerOfMediaGuide()
{

}

void CMPCmdHandlerOfMediaGuide::Init(CSkinWnd *pSkinWnd)
{
	ISkinCmdHandler::Init(pSkinWnd);

	m_mediaTree.Init();

	if (m_vPathLatest.size())
	{
		ISkinTreeNode	*pNode;
		pNode = m_mediaTree.GetNode(m_vPathLatest);
		if (pNode)
			m_mediaTree.SetSelNode(pNode);
	}

	CSkinTreeCtrl	*pTree = (CSkinTreeCtrl*)m_pSkinWnd->GetUIObjectById(CMD_MG_TREE_GUIDE, CSkinTreeCtrl::ClassName());
	if (pTree)
	{
		pTree->SetDataSrc(&m_mediaTree);
	}

	UpdateMediaList();
}

// if the command id is processed, return true.
bool CMPCmdHandlerOfMediaGuide::OnCommand(int nId)
{
	switch (nId)
	{
	case IDC_PLAY:
		OnDblClickMediaList();
		break;
	case IDC_QUEUE_UP:
		{
			CSkinListCtrl	*pMediaList =  (CSkinListCtrl*)m_pSkinWnd->GetUIObjectById(CMD_MG_MEDIA_LIST, CSkinListCtrl::ClassName());

			CMPAutoPtr<IPlaylist>	playlist;
			CMPAutoPtr<IPlaylist>	curPl;

			if (m_mediaTree.GetCurNodePlaylist(&playlist) &&
				g_Player.m_spPlayer->GetCurrentPlaylist(&curPl) == ERR_OK)
			{
				int		nSel = -1;
				while (1)
				{
					nSel = pMediaList->GetNextSelectedItem(nSel);
					if (nSel == -1)
						break;
					CMPAutoPtr<IMedia>	media;
					if (playlist->GetItem(nSel, &media) == ERR_OK)
					{
						curPl->InsertItem(-1, media);
					}
				}
				SaveCurrentPlaylist();
			}
		}
		break;
	case IDC_QUEUE_UP_ALL:
		{
			CMPAutoPtr<IPlaylist>	playlist;
			CMPAutoPtr<IPlaylist>	curPl;

			if (m_mediaTree.GetCurNodePlaylist(&playlist) &&
				g_Player.m_spPlayer->GetCurrentPlaylist(&curPl) == ERR_OK)
			{
				int		nCount = playlist->GetCount();
				for (int i = 0; i < nCount; i++)
				{
					CMPAutoPtr<IMedia>	media;
					if (playlist->GetItem(i, &media) == ERR_OK)
					{
						curPl->InsertItem(-1, media);
					}
				}
				SaveCurrentPlaylist();
			}
		}
		break;
	case IDC_NOWPLAYING:
		g_mpApp.GetSkinFactory()->ActiveOrCreateSkinWnd(false, _T("CMPSkin"), _T("Playlist"), _T("Playlist"));
		break;
	case IDC_ADD_DIR_TO_ML:
		//
		// Add Media to library
		//
		if (OnCmdSongAddDirToMediaLib(m_pSkinWnd))
			ReloadMediaGuideView();
		break;
	case IDC_ADD_FILE_TO_ML:
		//
		// Add files to library
		//
		if (OnCmdSongAddFilesToMediaLib(m_pSkinWnd))
			ReloadMediaGuideView();
		break;
	case IDC_REMOVE_FROM_ML:
		{
			// remove media from media library
			CSkinListCtrl	*pMediaList =  (CSkinListCtrl*)m_pSkinWnd->GetUIObjectById(CMD_MG_MEDIA_LIST, CSkinListCtrl::ClassName());
			if (pMediaList)
			{
				int		nSel = pMediaList->GetNextSelectedItem(-1);
				if (nSel == -1)
					return true;

				CMPAutoPtr<IPlaylist>		playlist;

				if (m_mediaTree.GetCurNodePlaylist(&playlist))
				{
					vector<int>				vIndex;
					while (nSel != -1)
					{
						vIndex.push_back(nSel);
						nSel = pMediaList->GetNextSelectedItem(nSel);
					}
					for (int i = vIndex.size() - 1; i >= 0; i--)
					{
						CMPAutoPtr<IMedia>		media;

						nSel = vIndex[i];
						if (playlist->GetItem(nSel, &media) == ERR_OK)
						{
							CMPAutoPtr<IMediaLibrary>	mediaLib;

							if (g_Player.m_spPlayer->GetMediaLibrary(&mediaLib) == ERR_OK)
								mediaLib->Remove(&media, false);

							playlist->RemoveItem(nSel);
							pMediaList->DeleteItem(nSel, true);
						}
					}
					// ReloadMediaGuideView();
				}
			}
		}
		break;
	default:
		return false;
	}

	return true;
}

bool CMPCmdHandlerOfMediaGuide::OnCustomCommand(int nID)
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

bool CMPCmdHandlerOfMediaGuide::OnUIObjNotify(IUIObjNotify *pNotify)
{
	if (pNotify->nID == CMD_MG_MEDIA_LIST)
	{
		if (pNotify->pUIObject->IsKindOf(CSkinListCtrl::ClassName()))
		{
			IMPMediaTreeNode	*pNode;
			CSkinListCtrl	*pMediaList =  (CSkinListCtrl*)pNotify->pUIObject;
			int		nSel = pMediaList->GetNextSelectedItem(-1);
			if (nSel == -1)
				return true;

			pNode = (IMPMediaTreeNode*)m_mediaTree.GetSelNode();
			if (!pNode)
				return true;

			CSkinListCtrlEventNotify	*pListCtrlNotify = (CSkinListCtrlEventNotify *)pNotify;
			if (pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_DBL_CLICK ||
				pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_ENTER)
			{
				if (pNode->m_playlist.p)
				{
					g_Player.m_spPlayer->SetCurrentPlaylist(pNode->m_playlist);
					g_Player.m_spPlayer->SetCurrentMediaInPlaylist(nSel);
					g_Player.Play();
					SaveCurrentPlaylist();
					return true;
				}
				if (pNode->nodeType == MGNT_NOW_PLAYING)
				{
					g_Player.m_spPlayer->SetCurrentMediaInPlaylist(nSel);
				}
			}
		}

		return true;
	}
	else if (pNotify->nID == CMD_MG_TREE_GUIDE)
	{
		if (pNotify->pUIObject->IsKindOf(CSkinTreeCtrl::ClassName()))
		{
			//
			// Change to new node of media tree guide, update media guide view.
			//
			CSkinTreeCtrl		*pCtrl = (CSkinTreeCtrl*)pNotify->pUIObject;
			CSkinTreeCtrlEventNotify	*pEvent = (CSkinTreeCtrlEventNotify*)pNotify;
			if (pEvent->cmd == CSkinTreeCtrlEventNotify::C_SEL_CHANGED)
			{
				IMPMediaTreeNode *pNode = (IMPMediaTreeNode*)m_mediaTree.GetSelNode();

				// Update media guide view.
				if (pNode)
				{
					UpdateMediaList();
				}
			}
		}
		return true;
	}

	return false;
}

void CMPCmdHandlerOfMediaGuide::OnDblClickMediaList()
{
	CSkinListCtrl	*pMediaList = (CSkinListCtrl*)m_pSkinWnd->GetUIObjectById(CMD_MG_MEDIA_LIST, CSkinListCtrl::ClassName());
	if (!pMediaList)
		return;

	CSkinListCtrlEventNotify	notify(pMediaList);
	notify.cmd = CSkinListCtrlEventNotify::C_DBL_CLICK;

	OnUIObjNotify(&notify);
}

void CMPCmdHandlerOfMediaGuide::ReloadMediaGuideView()
{
	m_mediaTree.Close();
	m_mediaTree.Init();
	m_mediaTree.GetCurPath(m_vPathLatest);

	if (m_vPathLatest.size())
	{
		ISkinTreeNode	*pNode;
		pNode = m_mediaTree.GetNode(m_vPathLatest);
		if (pNode)
			m_mediaTree.SetSelNode(pNode);
	}
	UpdateMediaList();
}

void CMPCmdHandlerOfMediaGuide::UpdateMediaList()
{
	IMPMediaTreeNode *pNode = (IMPMediaTreeNode*)m_mediaTree.GetSelNode();

	CSkinListCtrl	*plistCtrl;
	plistCtrl = (CSkinListCtrl*)m_pSkinWnd->GetUIObjectById(CMD_MG_MEDIA_LIST, CSkinListCtrl::ClassName());
	if (!plistCtrl)
		return;

	if (plistCtrl->GetColumnCount() == 0)
		plistCtrl->AddColumn(_T(""), 230);

	plistCtrl->DeleteAllItems(false);

	// Update media guide view.
	if (pNode == NULL || pNode->m_playlist.p == NULL)
	{
		plistCtrl->Invalidate();
		return;
	}

	int		nCount = pNode->m_playlist->GetCount();
	tstring	str;

	for (int i = 0; i < nCount; i++)
	{
		CMPAutoPtr<IMedia>	media;
		if (pNode->m_playlist->GetItem(i, &media) == ERR_OK)
		{
			g_Player.FormatMediaTitle(media, str);

			plistCtrl->InsertItem(plistCtrl->GetItemCount(), 
				str.c_str(), II_MUSIC);
		}
	}

	if (plistCtrl->GetItemCount())
		plistCtrl->SetItemSelectionState(0, true);
	plistCtrl->Invalidate();
}

void CMPCmdHandlerOfMediaGuide::AddHistoryPath()
{
	HistroyItem		item;
	CSkinListCtrl	*plistCtrl;
	plistCtrl = (CSkinListCtrl*)m_pSkinWnd->GetUIObjectById(CMD_MG_MEDIA_LIST, CSkinListCtrl::ClassName());
	if (plistCtrl)
		item.nSelChild = plistCtrl->GetNextSelectedItem();
	else
		item.nSelChild = 0;

	m_mediaTree.GetCurPath(item.path);

	m_historyPath.push_back(item);
	if (m_historyPath.size() > 10)
		m_historyPath.erase(m_historyPath.begin());
}

void CMPCmdHandlerOfMediaGuide::BackHistoryPath()
{
	if (m_historyPath.size())
	{
		CSkinListCtrl	*plistCtrl;
		int				nSelRow = m_historyPath.back().nSelChild;
		ISkinTreeNode	*pNode;

		plistCtrl = (CSkinListCtrl*)m_pSkinWnd->GetUIObjectById(CMD_MG_MEDIA_LIST, CSkinListCtrl::ClassName());
		pNode = m_mediaTree.GetNode(m_historyPath.back().path);
		if (plistCtrl && pNode)
		{
			m_mediaTree.SetSelNode(pNode);

			UpdateMediaList();

			if (plistCtrl && nSelRow >= 0 && nSelRow < plistCtrl->GetItemCount())
			{
				plistCtrl->SetItemSelectionState(0, false);
				plistCtrl->SetItemSelectionState(nSelRow, true);
				plistCtrl->MakeSureRowVisible(nSelRow);
				plistCtrl->Invalidate();
			}

			CSkinTreeCtrl	*pTree = (CSkinTreeCtrl*)m_pSkinWnd->GetUIObjectById(CMD_MG_TREE_GUIDE, CSkinTreeCtrl::ClassName());
			if (pTree)
				pTree->Invalidate();
		}
		m_historyPath.pop_back();
	}
}
