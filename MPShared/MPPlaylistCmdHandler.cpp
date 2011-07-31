// MPPlaylistCmdHandler.cpp: implementation of the CMPPlaylistCmdHandler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerAppBase.h"
#include "MPPlaylistCmdHandler.h"
#include "MPHelper.h"
#include "DlgMediaInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPPlaylistCmdHandler::CMPPlaylistCmdHandler()
{

}

CMPPlaylistCmdHandler::~CMPPlaylistCmdHandler()
{

}

// if the command id is processed, return true.
bool CMPPlaylistCmdHandler::OnCommand(int nId)
{
	switch (nId)
	{
	case IDC_OPENDIR:
	case IDC_ADDDIR:
		{
			OnSongOpenDirCmd(m_pSkinWnd, nId == IDC_OPENDIR);
		}
		break;
	case IDC_OPENFILE:
	case IDC_ADDFILE:
		{
			OnSongOpenFileCmd(m_pSkinWnd, nId == IDC_OPENFILE);
		}
		break;
	case IDC_NEW_PL:
		g_Player.NewCurrentPlaylist();
		break;
	case IDC_OPEN_PL:
		{
			CFileOpenDlg		dlg(_T("Open Playlist"), g_Player.GetCurrentPlaylistFile().c_str(), _T("M3u file\0*.m3u\0All Files\0*.*\0\0"), 0);
			if (dlg.DoModal(m_pSkinWnd) == IDOK)
			{
				g_Player.ClearPlaylist();
				g_Player.LoadPlaylist(dlg.GetOpenFile());
			}
		}
		break;
	case IDC_SAVE_PL:
		{
			CFileSaveDlg		dlg(_T("Save Playlist"), g_Player.GetCurrentPlaylistFile().c_str(), _T("M3u file\0*.m3u\0All Files\0*.*\0\0"), 0);
			if (dlg.DoModal(m_pSkinWnd) == IDOK)
			{
				if (IsEmptyString(FileGetExt(dlg.GetSaveFile())))
				{
					TCHAR		szFile[MAX_PATH];
					_tcscpy_safe(szFile, CountOf(szFile), dlg.GetSaveFile());
					FileSetExt(szFile, CountOf(szFile), dlg.GetSelectedExt());
					g_Player.SaveCurrentPlaylistAs(szFile);
				}
				else
					g_Player.SaveCurrentPlaylistAs(dlg.GetSaveFile());
			}
		}
		break;
	case IDC_MOVE_UP:
		OnCustomCommand(CMD_PL_UP);
		break;
	case IDC_MOVE_DOWN:
		OnCustomCommand(CMD_PL_DOWN);
		break;
	case IDC_REMOVE:
		OnCustomCommand(CMD_PL_DEL);
		break;
	case IDC_CLEAR:
		{
			g_Player.ClearPlaylist();
			g_Player.SetPlaylistModified(true);
		}
		break;
	case IDC_MEDIA_INFO:
		{
			CSkinListCtrl	*plCtrl;
			CMPAutoPtr<IPlaylist>	playlist;
			CMPAutoPtr<IMedia>		media;

			plCtrl = (CSkinListCtrl*)m_pSkinWnd->GetUIObjectById(ID_PLAYLIST, CSkinListCtrl::ClassName());
			if (plCtrl && plCtrl->IsVisible() && plCtrl->IsParentVisible())
			{
				int				nSel;
				nSel = plCtrl->GetNextSelectedItem(-1);
				if (nSel == -1)
					break;

				if (g_Player.GetCurrentPlaylist(&playlist) != ERR_OK)
					break;

				if (playlist->GetItem(nSel, &media) != ERR_OK)
					break;
			}
			else
			{
				if (g_Player.GetCurrentMedia(&media) != ERR_OK)
					break;
			}

			ShowMediaInfoDialog(m_pSkinWnd, media);
			// dlg.DoModal(m_pSkinWnd, media);

/*			CXStr		str;
			media->GetSourceUrl(&str);

			CID3v2		id3v2;
			int			nRet;

			nRet = id3v2.Open(str.c_str(), false, false);
			if (nRet == ERR_OK)
			{
				CID3v2::FrameIterator		it, itEnd;

				itEnd = id3v2.FrameEnd();
				for (it = id3v2.FrameBegin(); it != itEnd; ++it)
				{
					CID3v2Frame		*fr = *it;
					DBGLOG2(_T("%S, %d"), fr->m_framehdr.szFrameID, fr->m_framehdr.nLen);
					// if (strcmp(fr->m_framehdr.szFrameID, "TIT2") == 0)
					if (fr->m_framehdr.szFrameID[0] == 'T')
					{
						CID3v2FrameParserText	txtParser;
						ID3v2Text		txt;
						nRet = txtParser.Parse(fr->m_framehdr, fr->m_frameData.c_str(), fr->m_frameData.size(), txt);
						if (nRet != ERR_OK)
							ERRLOG1(_T("Parse Txt frame Failed: %s"), (LPCTSTR)Error2Str(nRet));
						else
						{
							DBGLOG1(_T("Value: %s"), txt.GetValue());
						}
					}
					else if (strcmp(fr->m_framehdr.szFrameID, "COMM") == 0)
					{
						CID3v2FrameParserComment	parser;
						nRet = parser.Parse(*fr);
						if (nRet != ERR_OK)
							ERRLOG1(_T("Parse Txt frame Failed: %s"), (LPCTSTR)Error2Str(nRet));
						else
						{
							if (!parser.m_bUnicode)
							{
								ConvertStr(parser.m_strAnsiShortDesc.c_str(), -1, parser.m_strUcs2ShortDesc);
								ConvertStr(parser.m_strAnsiText.c_str(), -1, parser.m_strUcs2Text);
							}

							DBGLOG3(_T("Comment: %s, %s, %s"), parser.m_szLanguage, parser.m_strUcs2ShortDesc.c_str(), parser.m_strUcs2Text.c_str());
						}
					}
				}
			}
			else
				ERRLOG1(_T("Open Failed: %s"), (LPCTSTR)Error2Str(nRet));*/
		}
		break;
// 	case IDC_MEDIALIB:
// 		{
// 			if (!g_wndMediaLib.IsValid())
// 				g_wndMediaLib.Create(this);
// 		}
// 		break;
// 	case IDOK:
// 		g_wndLyricShow.UpdatePlaylist();
// 
// 		Destroy();
// 		break;
// 	case IDC_PLAY:
// 		{
// 			g_wndLyricShow.UpdatePlaylist();
// 
// 			int		n = m_playlist->GetNextSelectedItem(-1);
// 			if (n != -1)
// 				g_Player.PlayMedia(n);
// 			else
// 				g_Player.Play();
// 
// 			Destroy();
// 		}
// 		break;
	default:
		return false;
	}

	return true;
}


bool DeleteSelectedItemsInPlaylist(IPlaylist *playlist, vector<int> &vSelIndex)
{
	MLRESULT	nRet;
	long		n;
	long		count = 0;

	ASSERT(vSelIndex.size() > 0);
	if (vSelIndex.size() == 0)
		return false;

	sort(vSelIndex.begin(), vSelIndex.end());

	count = playlist->GetCount();
	if (count == 0)
		return false;

	for (n = 0; n < (long)vSelIndex.size(); n++)
	{
		ASSERT(vSelIndex[n] < count && vSelIndex[n] >= 0);
		nRet = playlist->RemoveItem(vSelIndex[n]);
		if (nRet != ERR_OK)
			break;
	}

	return nRet == ERR_OK;
}

bool OffsetAllSelectedRowInPlaylist(IPlaylist *playlist, vector<int> &vSelIndex, bool bDown)
{
	MLRESULT	nRet = ERR_OK;
	long		n;
	long		count = 0;

	ASSERT(vSelIndex.size() > 0);
	if (vSelIndex.size() == 0)
		return false;

	sort(vSelIndex.begin(), vSelIndex.end());

	count = playlist->GetCount();
	if (count == 0)
		return false;

	if (bDown)
	{
		for (n = (long)vSelIndex.size() - 1; n >= 0; n--)
		{
			ASSERT(vSelIndex[n] < count && vSelIndex[n] >= 0);
			nRet = playlist->MoveItem(vSelIndex[n], vSelIndex[n] + 1);
			if (nRet != ERR_OK)
				break;
		}
	}
	else
	{
		for (n = 0; n < (long)vSelIndex.size(); n++)
		{
			ASSERT(vSelIndex[n] < count && vSelIndex[n] >= 0);
			nRet = playlist->MoveItem(vSelIndex[n], vSelIndex[n] - 1);
			if (nRet != ERR_OK)
				break;
		}
	}

	return nRet == ERR_OK;
}

bool CMPPlaylistCmdHandler::OnCustomCommand(int nID)
{
	switch (nID)
	{
	case CMD_PL_DEL:
		{
			vector<int>		vSelItems;
			int				nItem = -1;
			CSkinListCtrl	*plCtrl;

			plCtrl = (CSkinListCtrl*)m_pSkinWnd->GetUIObjectById(ID_PLAYLIST, CSkinListCtrl::ClassName());
			if (!plCtrl)
				break;

			while ((nItem = plCtrl->GetNextSelectedItem(nItem)) != -1)
				vSelItems.push_back(nItem);

			if (vSelItems.empty())
				break;

			// plCtrl->DeleteSelectedItems();

			CMPAutoPtr<IPlaylist>	playlist;

			if (g_Player.GetCurrentPlaylist(&playlist) == ERR_OK)
			{
				if (DeleteSelectedItemsInPlaylist(playlist, vSelItems))
				{
					nItem = vSelItems[0];
					if (nItem >= plCtrl->GetItemCount())
						nItem = plCtrl->GetItemCount() - 1;
					if (nItem >= 0)
						plCtrl->SetItemSelectionState(nItem, true);
					g_Player.SetPlaylistModified(true);
				}
			}
		}
		break;
	case CMD_PL_UP:
	case CMD_PL_DOWN:
		{
			bool		bDown;
			CSkinListCtrl	*plCtrl;

			plCtrl = (CSkinListCtrl*)m_pSkinWnd->GetUIObjectById(ID_PLAYLIST, CSkinListCtrl::ClassName());
			if (!plCtrl)
				break;

			if (nID == CMD_PL_UP)
				bDown = false;
			else
				bDown = true;

			vector<int>		vSelItems;
			int				nItem = -1;

			while ((nItem = plCtrl->GetNextSelectedItem(nItem)) != -1)
				vSelItems.push_back(nItem);

			CMPAutoPtr<IPlaylist>	playlist;

			if (g_Player.GetCurrentPlaylist(&playlist) == ERR_OK)
			{
				if (OffsetAllSelectedRowInPlaylist(playlist, vSelItems, bDown))
				{
					// UpdateButtonState();
					g_Player.SetPlaylistModified(true);
				}
			}
		}
		break;
	default:
		return false;
	}

	return true;
}

bool CMPPlaylistCmdHandler::OnUIObjNotify(IUIObjNotify *pNotify)
{
	if (pNotify->nID == ID_PLAYLIST && pNotify->pUIObject->IsKindOf(CSkinListCtrl::ClassName()))
	{
		CSkinListCtrlEventNotify	*pListCtrlNotify = (CSkinListCtrlEventNotify *)pNotify;
		if (pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_DBL_CLICK ||
			pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_ENTER || 
			pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_KEY_DELETE)
		{
			CSkinListCtrl	*pCtrlPlaylist;

			pCtrlPlaylist = (CSkinListCtrl*)pNotify->pUIObject;
			ASSERT(pCtrlPlaylist);
			if (!pCtrlPlaylist)
				return true;

			if (pListCtrlNotify->cmd == CSkinListCtrlEventNotify::C_KEY_DELETE)
			{
				//
				// Remove selected items
				//
				int				n = pCtrlPlaylist->GetNextSelectedItem(-1);
				vector<int>		vItems;
				CMPAutoPtr<IPlaylist>	playlist;
				MLRESULT		nRet;
				nRet = g_Player.GetCurrentPlaylist(&playlist);
				if (nRet == ERR_OK)
				{
					while (n != -1)
					{
						vItems.push_back(n);
						n = pCtrlPlaylist->GetNextSelectedItem(n);
					}
					for (int i = vItems.size() - 1; i >= 0; i--)
					{
						playlist->RemoveItem(vItems[i]);
					}
				}
			}
			else
			{
				// play selected items
				int		n = pCtrlPlaylist->GetNextSelectedItem(-1);
				if (n != -1)
					g_Player.PlayMedia(n);
			}
		}

		return true;
	}

	return false;
}
