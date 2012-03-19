// MPlaylistCtrl.cpp: implementation of the CMPlaylistCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerAppBase.h"
#include "MPlaylistCtrl.h"

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

//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CMPlaylistCtrl, _T("Playlist"))

CMPlaylistCtrl::CMPlaylistCtrl()
{
	m_nNowPlaying = -1;
}

CMPlaylistCtrl::~CMPlaylistCtrl()
{
}

void CMPlaylistCtrl::OnCreate()
{
	CSkinListCtrl::OnCreate();

	if (GetColumnCount() == 0)
		AddColumn(TOLOCAL_T("Media Files"), 230);

	RegisterHandler(CMPlayerAppBase::GetEventsDispatcher(), ET_PLAYER_CUR_PLAYLIST_CHANGED, ET_PLAYER_CUR_MEDIA_CHANGED);

	CMPAutoPtr<IPlaylist> playlist;
	int nRet = g_Player.GetCurrentPlaylist(&playlist);
	if (nRet != ERR_OK)
		return;

	for (int i = 0; i < playlist->GetCount(); i++)
	{
		CMPAutoPtr<IMedia>	media;
		tstring				strTitle;
		if (playlist->GetItem(i, &media) == ERR_OK)
		{
			g_Player.FormatMediaTitle(media, strTitle);
			InsertItem(i, strTitle.c_str(), 0);
		}
	}
}

void CMPlaylistCtrl::OnEvent(const IEvent *pEvent)
{
	if (pEvent->eventType == ET_PLAYER_CUR_PLAYLIST_CHANGED)
	{
		CEventPlaylistChanged	*pEventPlaylistChanged = (CEventPlaylistChanged*)pEvent;
		UpdatePlaylist(pEventPlaylistChanged);
	}
	else if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED)
	{
		SetNowPlaying();

		CMPAutoPtr<IMedia>	media;
		tstring				strTitle;
		if (g_Player.GetCurrentMedia(&media) == ERR_OK)
		{
			g_Player.FormatMediaTitle(media, strTitle);
			SetItemText(g_Player.GetCurrentMediaIndex(), 0, strTitle.c_str());
		}

		Invalidate();
	}
}

void CMPlaylistCtrl::OnKeyDown(UINT nChar, UINT nFlags)
{
	if (m_vRows.empty())
		return;

	if (nChar == VK_RETURN)
	{
		int n = GetNextSelectedItem(-1);
		if (n != -1)
			g_Player.PlayMedia(n);
	}
	else if (nChar == VK_DELETE)
	{
		DeleteSelectedItems();
		return;
	}
	if (nChar == VK_UP || nChar == VK_DOWN)
	{
		bool ctrl = IsKeyPressed(VK_CONTROL);
		if (ctrl)
			OffsetAllSelectedItems(nChar == VK_DOWN);
	}

	CSkinListCtrl::OnKeyDown(nChar, nFlags);
}

void CMPlaylistCtrl::SendNotifyEvent(CSkinListCtrlEventNotify::Command cmd, int nClickedRow, int nClickedCol)
{
	if (cmd == CSkinListCtrlEventNotify::C_DBL_CLICK)
		g_Player.PlayMedia(nClickedRow);

	CSkinListCtrl::SendNotifyEvent(cmd, nClickedRow, nClickedCol);
}

void CMPlaylistCtrl::DeleteSelectedItems()
{
	int				nItem = -1;
	vector<int>		vSelItems;

	while ((nItem = GetNextSelectedItem(nItem)) != -1)
		vSelItems.push_back(nItem);

	if (vSelItems.empty())
		return;

	CMPAutoPtr<IPlaylist>	playlist;

	if (g_Player.GetCurrentPlaylist(&playlist) == ERR_OK)
	{
		if (DeleteSelectedItemsInPlaylist(playlist, vSelItems))
		{
			nItem = vSelItems[0];
			if (nItem >= GetItemCount())
				nItem = GetItemCount() - 1;
			if (nItem >= 0)
				SetItemSelectionState(nItem, true);
			g_Player.SetPlaylistModified(true);
		}
	}
}

void CMPlaylistCtrl::OffsetAllSelectedItems(bool bMoveDown)
{
	vector<int>		vSelItems;
	int				nItem = -1;

	while ((nItem = GetNextSelectedItem(nItem)) != -1)
		vSelItems.push_back(nItem);

	CMPAutoPtr<IPlaylist>	playlist;

	if (g_Player.GetCurrentPlaylist(&playlist) == ERR_OK)
	{
		if (OffsetAllSelectedRowInPlaylist(playlist, vSelItems, bMoveDown))
		{
			// UpdateButtonState();
			g_Player.SetPlaylistModified(true);
		}
	}
}

void CMPlaylistCtrl::UpdatePlaylist(CEventPlaylistChanged *pEventPlaylistChanged)
{
	tstring					str;
	long					nCount;
	CMPAutoPtr<IPlaylist>	playlist;

	if (g_Player.GetCurrentPlaylist(&playlist) != ERR_OK)
		return;

	if (pEventPlaylistChanged == NULL || pEventPlaylistChanged->action == IMPEvent::PCA_FULL_UPDATE)
	{
		// reload playlist
		DeleteAllItems();

		nCount = playlist->GetCount();
		for (int i = 0;i < nCount; i++)
		{
			CMPAutoPtr<IMedia>	pMedia;
			if (playlist->GetItem(i, &pMedia) == ERR_OK)
			{
				g_Player.FormatMediaTitle(pMedia, str);
				InsertItem(i, str.c_str());
			}
		}

		SetNowPlaying();

		Invalidate();
	}
	else if (pEventPlaylistChanged->action == IMPEvent::PCA_CLEAR)
		DeleteAllItems();
	else if (pEventPlaylistChanged->action == IMPEvent::PCA_INSERT)
	{
		CMPAutoPtr<IMedia>	pMedia;
		if (playlist->GetItem(pEventPlaylistChanged->nIndex, &pMedia) == ERR_OK)
		{
			g_Player.FormatMediaTitle(pMedia, str);
			InsertItem(pEventPlaylistChanged->nIndex, str.c_str(), 0, 0, true);
		}
	}
	else if (pEventPlaylistChanged->action == IMPEvent::PCA_MOVE)
	{
		int			nIndex = pEventPlaylistChanged->nIndex;
		int			nIndexOld = pEventPlaylistChanged->nIndexOld;
		if (nIndex >= 0 && nIndex < GetItemCount()
			&& nIndexOld >= 0 && nIndexOld < GetItemCount())
		{
			LC_ROW *temp = m_vRows[nIndex];
			m_vRows[nIndex] = m_vRows[nIndexOld];
			m_vRows[nIndexOld] = temp;
		}
		Invalidate();
	}
	else if (pEventPlaylistChanged->action == IMPEvent::PCA_REMOVE)
	{
		int			nSel = GetNextSelectedItem();
		int			nIndex = pEventPlaylistChanged->nIndex;
		if (nIndex >= 0 && nIndex < GetItemCount())
		{
			DeleteItem(pEventPlaylistChanged->nIndex, true);
		}

		if (nSel != -1)
		{
			if (nSel >= GetItemCount())
				nSel = GetItemCount() - 1;
			if (nSel != -1)
			{
				m_pSkin->EnterInDrawUpdate();
				SetItemSelectionState(nSel, true);
				m_pSkin->LeaveInDrawUpdate();
			}
		}
	}
	else
		ASSERT(0);
}


int CMPlaylistCtrl::GetItemImageIndex(int nItem)
{
	ASSERT(nItem >= 0 && nItem < (int)m_vRows.size());
	if (nItem >= 0 && nItem < (int)m_vRows.size())
	{
		LC_ROW		*row = m_vRows[nItem];
		return row->nImageIndex;
	}

	return -1;
}

void CMPlaylistCtrl::SetNowPlaying()
{
	// Clear the old now playing item status.
	bool bOldStatusCleared = false;
	if (m_nNowPlaying >= 0 && m_nNowPlaying < (int)m_vRows.size())
	{
		LC_ROW *row = m_vRows[m_nNowPlaying];
		if (row->nImageIndex == IMAGE_NOW_PLAYING)
		{
			row->nImageIndex = IMAGE_NONE;
			bOldStatusCleared = true;
		}
	}

	if (!bOldStatusCleared)
	{
		for (int i = 0; i < (int)m_vRows.size(); i++)
		{
			LC_ROW *row = m_vRows[i];
			if (row->nImageIndex == IMAGE_NOW_PLAYING)
			{
				row->nImageIndex = IMAGE_NONE;
				break;
			}
		}
	}

	if (m_vRows.size() > 0)
		SetItemImageIndex(g_Player.GetCurrentMediaIndex(), IMAGE_NOW_PLAYING, false);
}
