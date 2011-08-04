// MPlaylistCtrl.cpp: implementation of the CMPlaylistCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerAppBase.h"
#include "MPlaylistCtrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CMPlaylistCtrl, _T("Playlist"))

CMPlaylistCtrl::CMPlaylistCtrl()
{

}

CMPlaylistCtrl::~CMPlaylistCtrl()
{
	CMPlayerAppBase::GetEventsDispatcher()->UnregisterHanlder(ET_PLAYER_CUR_PLAYLIST_CHANGED, this);
	CMPlayerAppBase::GetEventsDispatcher()->UnregisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
}

void CMPlaylistCtrl::OnCreate()
{
	CSkinListCtrl::OnCreate();

	if (GetColumnCount() == 0)
		AddColumn(_T("Music"), 230);

	CMPlayerAppBase::GetEventsDispatcher()->RegisterHanlder(ET_PLAYER_CUR_PLAYLIST_CHANGED, this);
	CMPlayerAppBase::GetEventsDispatcher()->RegisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
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
		SetPlayingNow(g_Player.GetCurrentMediaIndex());
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
		SetPlayingNow(g_Player.GetCurrentMediaIndex());
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
		bool		bSelected = IsRowSelected(nIndexOld);
		if (nIndex >= 0 && nIndex < GetItemCount()
			&& nIndexOld >= 0 && nIndexOld < GetItemCount())
		{
			CMPAutoPtr<IMedia>	pMedia;
			if (playlist->GetItem(nIndex, &pMedia) == ERR_OK)
			{
				DeleteItem(nIndexOld, false);
				g_Player.FormatMediaTitle(pMedia, str);
				InsertItem(nIndex, str.c_str());
				if (bSelected)
					SetItemSelectionState(nIndex, bSelected);
			}
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
				SetItemSelectionState(nSel, true);
		}
	}
	else
		ASSERT(0);
}
