// MPSkinWnd.cpp: implementation of the CMPSkinWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "EventsDispatcher.h"
#include "MPSkinWnd.h"
#include "MPCommonCmdHandler.h"
#ifdef _MPLAYER
#include "MPPlaylistCmdHandler.h"
#include "MPMediaLibCmdHandler.h"
#include "MPCmdHandlerOfMediaGuide.h"
#endif
#include "SkinListCtrl.h"
#include "SkinSeekCtrl.h"
#include "SkinRateCtrl.h"
#include "SkinToolBar.h"
#include "LyricShowObj.h"
#include "LyricShowTextEditObj.h"
#include "LyricShowAgentObj.h"

/*
class IUIStateUpdater
{
public:
	IUIStateUpdater(void *pObj) { m_hObj = (HANDLE)pObj; }
	virtual ~IUIStateUpdater() { }
	virtual void Enable(bool bEnable) = 0;
	virtual void SetCheck(bool bCheck) = 0;

	HANDLE			m_hObj;

};

class CUIStateUpdaterButton : public IUIStateUpdater
{
public:
	CUIStateUpdaterButton(void *pObj) : IUIStateUpdater(pObj) { }
	void Enable(bool bEnable)
	{
		CUIObject	*pObj;
		pObj = (CUIObject *)m_hObj;
		if (pObj->IsEnable() != bEnable)
		{
			pObj->SetEnable(bEnable);
			pObj->Invalidate();
		}
	}
	void SetCheck(bool bCheck)
	{
		CSkinButton	*pButton;
		pButton = (CSkinButton *)m_hObj;
		if (pButton->IsCheck() != bCheck)
		{
			pButton->SetCheck(bCheck);
		}
	}

};


class LIST_UISTATE_UPDATER : public list<IUIStateUpdater *>
{
public:
	void Add(IUIStateUpdater *pUpdater)
	{
		iterator	it, itEnd;
		itEnd = end();
		for (it = begin(); it != itEnd; ++it)
		{
			IUIStateUpdater	*pItem = *it;
			if (pItem->m_hObj = pUpdater->m_hObj)
				return;
		}

		push_back(pUpdater);
	}
	void Del(HANDLE hObj)
	{
		iterator	it, itEnd;
		itEnd = end();
		for (it = begin(); it != itEnd; ++it)
		{
			IUIStateUpdater	*pItem = *it;
			if (pItem->m_hObj = hObj)
			{
				erase(it);
				return;
			}
		}
		ASSERT(0);
	}
};

class CUIPlayerStatusHandler : public IEventHandler
{
public:
	virtual void OnEvent(const IEvent *pEvent)
	{
		if (pEvent->eventType == ET_PLAYER_STATUS_CHANGED)
		{
			CEventPlayerStatusChanged	*pPlayerStatus = (CEventPlayerStatusChanged *)pEvent;
			// if ()
			{
				LIST_UISTATE_UPDATER::iterator	it, itEnd;

				itEnd = m_listStatePlaypause.end();
				for (it = m_listStatePlaypause.begin(); it != itEnd; ++it)
				{
					IUIStateUpdater		*pUpdater = *it;
					pUpdater->SetCheck(pPlayerStatus->status == PS_PLAYING);
				}
			}
		}
		else if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED)
		{
			
		}
	}

	virtual void AddPlayPauseUpdater(IUIStateUpdater *pUpdater)
	{
		if (m_listStatePlaypause.empty())
			g_eventDispather.RegisterHanlder(ET_PLAYER_STATUS_CHANGED, this);
		m_listStatePlaypause.Add(pUpdater);
	}

	virtual void DelPlayPauseUpdater(HANDLE hObj)
	{
		m_listStatePlaypause.Del(hObj);
		if (m_listStatePlaypause.empty())
			g_eventDispather.UnregisterHanlder(ET_PLAYER_STATUS_CHANGED, this);
	}

	LIST_UISTATE_UPDATER		m_listStatePlaypause;

};


CUIPlayerStatusHandler		g_UIPlayerStatusHandler;*/

class CPlayerShuffleEventHandler : public IEventHandlerEx
{
public:
	virtual void OnEvent(const IEvent *pEvent)
	{
		if (pEvent->eventType == ET_PLAYER_SETTING_CHANGED)
		{
			CEventPlayerSettingChanged	*pSettingEvt = (CEventPlayerSettingChanged *)pEvent;
			if (pSettingEvt->settingType == IMPEvent::MPS_SHUFFLE)
			{
				if (Tobool(pSettingEvt->nValue) != Tobool(m_pShuffle->GetStatus()))
					m_pShuffle->SetStatus(g_Player.IsShuffle() ? 1 : 0);
			}
		}
	}

	void Register(CSkinNStatusButton *pShuffle)
	{
		m_pShuffle = pShuffle;
		if (g_Player.IsShuffle() != Tobool(m_pShuffle->GetStatus()))
			pShuffle->SetStatus(g_Player.IsShuffle() ? 1 : 0);
		g_eventDispather.RegisterHanlder(ET_PLAYER_SETTING_CHANGED, this);
	}

	void Unregister()
	{
		g_eventDispather.UnregisterHanlder(ET_PLAYER_SETTING_CHANGED, this);
	}

protected:
	CSkinNStatusButton			*m_pShuffle;

};

class CPlayerLoopEventHandler : public IEventHandlerEx
{
public:
	virtual void OnEvent(const IEvent *pEvent)
	{
		if (pEvent->eventType == ET_PLAYER_SETTING_CHANGED)
		{
			CEventPlayerSettingChanged	*pSettingEvt = (CEventPlayerSettingChanged *)pEvent;

			if (pSettingEvt->settingType == IMPEvent::MPS_LOOP)
			{
				if (pSettingEvt->nValue != m_pLoop->GetStatus())
					m_pLoop->SetStatus(pSettingEvt->nValue);
			}
		}
	}

	void Register(CSkinNStatusButton *pLoopBt)
	{
		m_pLoop = pLoopBt;
		if ((int)g_Player.GetLoop() != pLoopBt->GetStatus())
			m_pLoop->SetStatus(g_Player.GetLoop());
		g_eventDispather.RegisterHanlder(ET_PLAYER_SETTING_CHANGED, this);
	}

	void Unregister()
	{
		g_eventDispather.UnregisterHanlder(ET_PLAYER_SETTING_CHANGED, this);
	}

protected:
	CSkinNStatusButton			*m_pLoop;

};

class CPlayerVolumeEventHandler : public IEventHandlerEx
{
public:
	virtual void OnEvent(const IEvent *pEvent)
	{
		if (pEvent->eventType == ET_PLAYER_SETTING_CHANGED)
		{
			CEventPlayerSettingChanged	*pSettingEvt = (CEventPlayerSettingChanged *)pEvent;
			if (pSettingEvt->settingType == IMPEvent::MPS_VOLUME)
			{
				if (pSettingEvt->nValue != m_pVolume->GetScrollPos())
					m_pVolume->SetScrollPos(pSettingEvt->nValue);
			}
		}
	}

	void Register(CSkinSeekCtrl *pVolume)
	{
		long		nVolume = 0;
		m_pVolume = pVolume;
		g_Player.GetVolume(nVolume);
		m_pVolume->SetScrollInfo(0, MP_VOLUME_MAX, 0, nVolume);
		g_eventDispather.RegisterHanlder(ET_PLAYER_SETTING_CHANGED, this);
	}

	void Unregister()
	{
		g_eventDispather.UnregisterHanlder(ET_PLAYER_SETTING_CHANGED, this);
	}

protected:
	CSkinSeekCtrl		*m_pVolume;

};

class CPlayerSeekEventHandler : public IEventHandlerEx
{
public:
	virtual void OnEvent(const IEvent *pEvent)
	{
		if (pEvent->eventType == ET_PLAYER_POS_UPDATE || pEvent->eventType == ET_PLAYER_SEEK)
		{
			m_pSeekCtrl->SetScrollPos(g_Player.GetPlayPos());
		}
		else if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED)
		{
			m_pSeekCtrl->SetScrollInfo(0, g_Player.GetMediaLength(), 0, 0);
		}
		else if (pEvent->eventType == ET_PLAYER_STATUS_CHANGED)
		{
			if (g_Player.GetPlayerState() == PS_STOPED)
				m_pSeekCtrl->SetScrollPos(0);
		}
	}

	void Register(CSkinSeekCtrl *pSeekCtrl)
	{
		m_pSeekCtrl = pSeekCtrl;
		pSeekCtrl->SetScrollInfo(0, g_Player.GetMediaLength(), 0, 0);
		g_eventDispather.RegisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
		g_eventDispather.RegisterHanlder(ET_PLAYER_POS_UPDATE, this);
		g_eventDispather.RegisterHanlder(ET_PLAYER_STATUS_CHANGED, this);
	}

	void Unregister()
	{
		g_eventDispather.UnregisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
		g_eventDispather.UnregisterHanlder(ET_PLAYER_POS_UPDATE, this);
		g_eventDispather.UnregisterHanlder(ET_PLAYER_STATUS_CHANGED, this);
	}

protected:
	CSkinSeekCtrl		*m_pSeekCtrl;

};

class CPlayerPlayPauseBtHandler : public IEventHandlerEx
{
public:
	virtual void OnEvent(const IEvent *pEvent)
	{
		if (pEvent->eventType == ET_PLAYER_STATUS_CHANGED)
		{
			int		bCheck;

			bCheck = g_Player.GetPlayerState() == PS_PLAYING;
			if (m_pButtn->GetStatus() != bCheck)
			{
				m_pButtn->SetStatus(bCheck);
			}
		}
	}

	void Register(CSkinNStatusButton *pButton)
	{
		int		bCheck;

		bCheck = g_Player.GetPlayerState() == PS_PLAYING;

		m_pButtn = pButton;
		if (m_pButtn->GetStatus() != bCheck)
		{
			m_pButtn->SetStatus(bCheck);
		}
		g_eventDispather.RegisterHanlder(ET_PLAYER_STATUS_CHANGED, this);
	}

	void Unregister()
	{
		g_eventDispather.UnregisterHanlder(ET_PLAYER_STATUS_CHANGED, this);
	}

protected:
	CSkinNStatusButton		*m_pButtn;

};

class CPlayerPlayPauseToolbarHandler : public IEventHandlerEx
{
public:
	virtual void OnEvent(const IEvent *pEvent)
	{
		if (pEvent->eventType == ET_PLAYER_STATUS_CHANGED)
		{
			bool		bCheck;

			bCheck = g_Player.GetPlayerState() == PS_PLAYING;
			if (m_pToolbar->IsCheck(CMD_PLAYPAUSE) != bCheck)
			{
				m_pToolbar->SetCheck(CMD_PLAYPAUSE, bCheck);
			}
		}
	}

	void Register(CSkinToolbar *pToolbar)
	{
		bool		bCheck;

		bCheck = g_Player.GetPlayerState() == PS_PLAYING;

		m_pToolbar = pToolbar;
		if (m_pToolbar->IsCheck(CMD_PLAYPAUSE) != bCheck)
		{
			m_pToolbar->SetCheck(CMD_PLAYPAUSE, bCheck, false);
		}
		g_eventDispather.RegisterHanlder(ET_PLAYER_STATUS_CHANGED, this);
	}

	void Unregister()
	{
		g_eventDispather.UnregisterHanlder(ET_PLAYER_STATUS_CHANGED, this);
	}

protected:
	CSkinToolbar		*m_pToolbar;

};

class CPlayerShuffleRepeatToolbarHandler : public IEventHandlerEx
{
public:
	virtual void OnEvent(const IEvent *pEvent)
	{
		if (pEvent->eventType == ET_PLAYER_SETTING_CHANGED)
		{
			CEventPlayerSettingChanged	*pSettingEvt = (CEventPlayerSettingChanged *)pEvent;
			if (pSettingEvt->settingType == IMPEvent::MPS_SHUFFLE)
			{
				if (pSettingEvt->nValue != m_pToolbar->GetBtStatus(CMD_SHUFFLE))
					m_pToolbar->SetBtStatus(CMD_SHUFFLE, g_Player.IsShuffle() ? 1 : 0, true);
			}
			else if (pSettingEvt->settingType == IMPEvent::MPS_LOOP)
			{
				if (pSettingEvt->nValue != m_pToolbar->GetBtStatus(CMD_LOOP))
					m_pToolbar->SetBtStatus(CMD_LOOP, g_Player.GetLoop(), true);
			}
		}
	}

	void Register(CSkinToolbar *pToolbar)
	{
		m_pToolbar = pToolbar;

		if (g_Player.IsShuffle() != Tobool(pToolbar->GetBtStatus(CMD_SHUFFLE)))
			pToolbar->SetBtStatus(CMD_SHUFFLE, g_Player.IsShuffle() ? 1 : 0, false);

		if (g_Player.GetLoop() != m_pToolbar->GetBtStatus(CMD_LOOP))
			m_pToolbar->SetBtStatus(CMD_LOOP, g_Player.GetLoop(), false);

		g_eventDispather.RegisterHanlder(ET_PLAYER_SETTING_CHANGED, this);
	}

	void Unregister()
	{
		g_eventDispather.UnregisterHanlder(ET_PLAYER_SETTING_CHANGED, this);
	}

protected:
	CSkinToolbar		*m_pToolbar;

};

#ifdef _MPLAYER
//
// Player controls
//

// Media
class CMediaStereoStatusEventHandler : public IEventHandlerEx
{
public:
	virtual void OnEvent(const IEvent *pEvent)
	{
		if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED)
		{
			CMPAutoPtr<IMedia>			pMedia;
			if (g_Player.m_spPlayer->GetCurrentMedia(&pMedia) == ERR_OK)
			{
				int		nChannels;
				pMedia->GetAttribute(MA_CHANNELS, &nChannels);
				if (nChannels >= 2)
					m_pStereoStatusCtrl->SetPropertyInt(_T("CurStatus"), 1);
				else
					m_pStereoStatusCtrl->SetPropertyInt(_T("CurStatus"), 0);
			}
		}
	}

	void Register(CUIObject *pCtrl)
	{
		m_pStereoStatusCtrl = pCtrl;
		CMPAutoPtr<IMedia>			pMedia;
		if (g_Player.m_spPlayer->GetCurrentMedia(&pMedia) == ERR_OK)
		{
			int		nChannels;
			pMedia->GetAttribute(MA_CHANNELS, &nChannels);
			if (nChannels >= 2)
				m_pStereoStatusCtrl->SetPropertyInt(_T("CurStatus"), 1);
			else
				m_pStereoStatusCtrl->SetPropertyInt(_T("CurStatus"), 0);
		}
		g_eventDispather.RegisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
	}

	void Unregister()
	{
		g_eventDispather.UnregisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
	}

protected:
	CUIObject		*m_pStereoStatusCtrl;;

};

class CPlayerMediaChangedRateEventHandler : public IEventHandlerEx
{
public:
	virtual void OnEvent(const IEvent *pEvent)
	{
		if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED || pEvent->eventType == ET_PLAYER_CUR_MEDIA_INFO_CHANGED)
		{
			CMPAutoPtr<IMedia>			pMedia;
			if (g_Player.m_spPlayer->GetCurrentMedia(&pMedia) == ERR_OK)
			{
				int		nRating = 0;
				pMedia->GetAttribute(MA_RATING, &nRating);
				m_pRateCtrl->SetRating(nRating);
			}
		}
	}

	void Register(CSkinRateCtrl *pRateCtrl)
	{
		m_pRateCtrl = pRateCtrl;
		CMPAutoPtr<IMedia>			pMedia;
		if (g_Player.m_spPlayer->GetCurrentMedia(&pMedia) == ERR_OK)
		{
			int		nRating = 0;
			pMedia->GetAttribute(MA_RATING, &nRating);
			m_pRateCtrl->SetRating(nRating);
		}
		g_eventDispather.RegisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
		g_eventDispather.RegisterHanlder(ET_PLAYER_CUR_MEDIA_INFO_CHANGED, this);
	}

	void Unregister()
	{
		g_eventDispather.UnregisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
		g_eventDispather.UnregisterHanlder(ET_PLAYER_CUR_MEDIA_INFO_CHANGED, this);
	}

protected:
	CSkinRateCtrl		*m_pRateCtrl;

};

// current playlist changed event handler
class CPlayerCurPlaylistChangedEventHandler : public IEventHandlerEx
{
public:
	virtual void OnEvent(const IEvent *pEvent)
	{
		if (pEvent->eventType == ET_PLAYER_CUR_PLAYLIST_CHANGED)
		{
			CEventPlaylistChanged	*pEventPlaylistChanged = (CEventPlaylistChanged*)pEvent;
			UpdatePlaylist(pEventPlaylistChanged);
		}
		else if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED)
		{
			m_plistCtrl->SetPlayingNow(g_Player.GetCurrentMediaIndex());
			m_plistCtrl->Invalidate();
		}
	}

	void Register(CSkinListCtrl *plistCtrl)
	{
		m_plistCtrl = plistCtrl;
		if (plistCtrl->GetColumnCount() == 0)
		{
			plistCtrl->InsertColum(_T("Music"), 230);
			plistCtrl->m_pContainer->RecalculateUIObjSizePos(plistCtrl);
		}
		UpdatePlaylist();
		g_eventDispather.RegisterHanlder(ET_PLAYER_CUR_PLAYLIST_CHANGED, this);
		g_eventDispather.RegisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
	}

	void Unregister()
	{
		g_eventDispather.UnregisterHanlder(ET_PLAYER_CUR_PLAYLIST_CHANGED, this);
		g_eventDispather.UnregisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
	}

	void UpdatePlaylist(CEventPlaylistChanged *pEventPlaylistChanged = NULL)
	{
		tstring					str;
		long					nCount;
		CMPAutoPtr<IPlaylist>	playlist;

		if (g_Player.m_spPlayer->GetCurrentPlaylist(&playlist) != ERR_OK)
			return;

		if (pEventPlaylistChanged == NULL || pEventPlaylistChanged->action == IMPEvent::PCA_FULL_UPDATE)
		{
			// reload playlist
			m_plistCtrl->DeleteAllItems();

			nCount = playlist->GetCount();
			for (int i = 0;i < nCount; i++)
			{
				CMPAutoPtr<IMedia>	pMedia;
				if (playlist->GetItem(i, &pMedia) == ERR_OK)
				{
					g_Player.FormatMediaTitle(pMedia, str);
					m_plistCtrl->InsertItem(i, str.c_str());
				}
			}
			m_plistCtrl->SetPlayingNow(g_Player.GetCurrentMediaIndex());
			m_plistCtrl->Invalidate();
		}
		else if (pEventPlaylistChanged->action == IMPEvent::PCA_CLEAR)
			m_plistCtrl->DeleteAllItems();
		else if (pEventPlaylistChanged->action == IMPEvent::PCA_INSERT)
		{
			CMPAutoPtr<IMedia>	pMedia;
			if (playlist->GetItem(pEventPlaylistChanged->nIndex, &pMedia) == ERR_OK)
			{
				g_Player.FormatMediaTitle(pMedia, str);
				m_plistCtrl->InsertItem(pEventPlaylistChanged->nIndex, str.c_str(), 0, 0, true);
			}
		}
		else if (pEventPlaylistChanged->action == IMPEvent::PCA_MOVE)
		{
			int			nIndex = pEventPlaylistChanged->nIndex;
			int			nIndexOld = pEventPlaylistChanged->nIndexOld;
			bool		bSelected = m_plistCtrl->IsRowSelected(nIndexOld);
			if (nIndex >= 0 && nIndex < m_plistCtrl->GetItemCount()
				&& nIndexOld >= 0 && nIndexOld < m_plistCtrl->GetItemCount())
			{
				CMPAutoPtr<IMedia>	pMedia;
				if (playlist->GetItem(nIndex, &pMedia) == ERR_OK)
				{
					m_plistCtrl->DeleteItem(nIndexOld, false);
					g_Player.FormatMediaTitle(pMedia, str);
					m_plistCtrl->InsertItem(nIndex, str.c_str());
					if (bSelected)
						m_plistCtrl->SetItemSelectionState(nIndex, bSelected);
				}
			}
			m_plistCtrl->Invalidate();
		}
		else if (pEventPlaylistChanged->action == IMPEvent::PCA_REMOVE)
		{
			int			nSel = m_plistCtrl->GetNextSelectedItem();
			int			nIndex = pEventPlaylistChanged->nIndex;
			if (nIndex >= 0 && nIndex < m_plistCtrl->GetItemCount())
			{
				m_plistCtrl->DeleteItem(pEventPlaylistChanged->nIndex, true);
			}

			if (nSel != -1)
			{
				if (nSel >= m_plistCtrl->GetItemCount())
					nSel = m_plistCtrl->GetItemCount() - 1;
				if (nSel != -1)
					m_plistCtrl->SetItemSelectionState(nSel, true);
			}
		}
		else
			ASSERT(0);
	}

protected:
	CSkinListCtrl		*m_plistCtrl;

};

#endif // _MPLAYER

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPSkinWnd::CMPSkinWnd()
{
	m_bEnableVolumeSwitch = false;
	m_bUseVolumeSeek = false;

	m_bTopmost = false;

	m_rcReal.top = m_rcReal.left = 0;
	m_rcReal.right = 400;
	m_rcReal.bottom = 250;

#ifdef _MPLAYER
	m_autoSwitchToLyrShow.m_pSkinWnd = this;
#endif
}

CMPSkinWnd::~CMPSkinWnd()
{

}

void CMPSkinWnd::OnActivate(bool bActived)
{
#ifdef _MPLAYER
	if (m_wndToolTip.IsWindow())
		m_autoSwitchToLyrShow.OnActivate(bActived);
#endif

	CSkinWnd::OnActivate(bActived);

	if (bActived && !IsIconic())
		g_mpApp.GetSkinFactory()->OnSkinWndActivate(this);
}

bool CMPSkinWnd::OnCreate()
{
	if (!CSkinWnd::OnCreate())
		return false;

#ifdef _WIN32_DESKTOP
	::SendMessage(m_hWnd, WM_SETICON, ICON_BIG, (WPARAM)LoadIcon(GetAppInstance(), MAKEINTRESOURCE(IDI_MPLAYER)));

	// 允许拖放歌词文件
	DragAcceptFiles(m_hWnd, TRUE);
#endif

	return true;
}

void CMPSkinWnd::OnEvent(const IEvent *pEvent)
{
#ifdef _MPLAYER
	m_autoSwitchToLyrShow.OnEvent(pEvent);
#endif
}

void CMPSkinWnd::OnContexMenu(int xPos, int yPos)
{
	if (m_rootConainter.GetMenu())
		m_rootConainter.GetMenu()->TrackPopupMenu(xPos, yPos, this);
}

void CMPSkinWnd::OnCommand(UINT uID, UINT nNotifyCode)
{
#ifdef _WIN32_DESKTOP
	switch (uID)
	{
	case IDC_CLICK_THROUGH:
		{
			//bool		bValue = !g_Profile.GetProfileBool(GetCfgSectName(), _T("ClickThrough"), FALSE);
			//g_Profile.WriteProfileInt(GetCfgSectName(), _T("ClickThrough"), bValue);
			if (m_bIndividualProperties)
			{
				m_bClickThrough = !m_bClickThrough;
				LoadTransparentSettings();
			}
			else
			{
				g_mpApp.GetSkinFactory()->SetClickThrough(!g_mpApp.GetSkinFactory()->GetClickThrough());
				g_mpApp.GetSkinFactory()->AllUpdateTransparent();
			}
		}
		return;
	case IDC_SETTOPMOST:
		{
			bool		bTopmost = !IsTopmost();

			if (m_bIndividualProperties)
			{
				g_Profile.WriteProfileInt(GetCfgSectName(), _T("topmost"), bTopmost);
				SetTopmost(bTopmost);
			}
			else
			{
				g_Profile.WriteProfileInt(SZ_SECT_UI, _T("topmost"), bTopmost);
				g_mpApp.GetSkinFactory()->TopmostAll(bTopmost);
			}
		}
		return;
// 	case IDC_USE_TRANSPT_BK_COLOR:
// 		g_Profile.WriteProfileInt(GetCfgSectName(), _T("TransparentBkColor"), 
// 			!g_Profile.GetProfileBool(GetCfgSectName(), _T("TransparentBkColor"), FALSE));
// 		if (m_bIndividualProperties)
// 			LoadTransparentSettings();
// 		else
// 			g_mpApp.GetSkinFactory()->AllUpdateTransparent();
// 		return;
	case IDC_SET_OPAQUE_100:
	case IDC_SET_OPAQUE_90:
	case IDC_SET_OPAQUE_80:
	case IDC_SET_OPAQUE_70:
	case IDC_SET_OPAQUE_60:
	case IDC_SET_OPAQUE_50:
	case IDC_SET_OPAQUE_40:
	case IDC_SET_OPAQUE_30:
	case IDC_SET_OPAQUE_20:
	case IDC_SET_OPAQUE_10:
		{
			int		nPercent = 100;

			if (uID == IDC_SET_OPAQUE_100)	nPercent = 100;
			else if (uID == IDC_SET_OPAQUE_100)	nPercent = 100;
			else if (uID == IDC_SET_OPAQUE_90)	nPercent = 90;
			else if (uID == IDC_SET_OPAQUE_80)	nPercent = 80;
			else if (uID == IDC_SET_OPAQUE_70)	nPercent = 70;
			else if (uID == IDC_SET_OPAQUE_60)	nPercent = 60;
			else if (uID == IDC_SET_OPAQUE_50)	nPercent = 50;
			else if (uID == IDC_SET_OPAQUE_40)	nPercent = 40;
			else if (uID == IDC_SET_OPAQUE_30)	nPercent = 30;
			else if (uID == IDC_SET_OPAQUE_20)	nPercent = 20;
			else if (uID == IDC_SET_OPAQUE_10)	nPercent = 10;

			g_Profile.WriteProfileInt(GetCfgSectName(), _T("WndOpacity"), false);
			g_Profile.WriteProfileInt(GetCfgSectName(), _T("WndOpaquePercent"), nPercent);
			if (m_bIndividualProperties)
				LoadTransparentSettings();
			else
				g_mpApp.GetSkinFactory()->AllUpdateTransparent();
		}
		return;
	case IDC_LDS_MULTI_LINE:
	case IDC_LDS_STATIC_TXT:
	case IDC_LDS_TWO_LINE:
	case IDC_LDS_HORZ_SCROLL:
	case IDC_LDS_SINGLE_LINE:
	case IDC_LDS_VOBSUB:
		{
			// special lyrics display style
			tstring		strName;
			if (uID == IDC_LDS_MULTI_LINE)
				strName = CLyricShowMultiRowObj::ClassName();
			else if (uID == IDC_LDS_STATIC_TXT)
				strName = CLyricShowTxtObj::ClassName();
			else if (uID == IDC_LDS_TWO_LINE)
				strName = CLyricShowTwoRowObj::ClassName();
			else if (uID == IDC_LDS_HORZ_SCROLL)
				strName = CLyricShowHorzScrollObj::ClassName();
			else if (uID == IDC_LDS_SINGLE_LINE)
				strName = CLyricShowSingleRowObj::ClassName();
			else if (uID == IDC_LDS_VOBSUB)
				strName = CLyricShowVobSub::ClassName();
			else
				ASSERT(0);

			CLyricShowAgentObj::SetLyrDispStyleSettings(this, strName.c_str());
		}
		return;
	default:
		break;
	}
#endif

	CSkinWnd::OnCommand(uID, nNotifyCode);

	LIST_SKINCMDHANDLER::iterator	it, itEnd;

	itEnd = m_listSkinCmdHandler.end();
	for (it = m_listSkinCmdHandler.begin(); it != itEnd; ++it)
	{
		ISkinCmdHandler		*pHandler = *it;
		if (pHandler->OnCommand(uID))
			return;
	}

	if (m_rootConainter.OnCommand(uID))
		return;
}

bool CMPSkinWnd::OnCustomCommand(int nId)
{
#ifdef _MPLAYER
	m_autoSwitchToLyrShow.OnCustomCmdNotify(nId);
#endif

	if (CSkinWnd::OnCustomCommand(nId))
		return true;

	switch (nId)
	{
	case CMD_TOPMOST:
		OnCommand(IDC_SETTOPMOST, 0);
		return true;
#ifdef _WIN32_DESKTOP
	case CMD_CLICK_THROUGH:
		OnCommand(IDC_CLICK_THROUGH, 0);
		return true;
#endif
	}

	LIST_SKINCMDHANDLER::iterator	it, itEnd;

	itEnd = m_listSkinCmdHandler.end();
	for (it = m_listSkinCmdHandler.begin(); it != itEnd; ++it)
	{
		ISkinCmdHandler		*pHandler = *it;
		if (pHandler->OnCustomCommand(nId))
			return true;
	}

	if (nId == CMD_VOL_SWITCH)
	{
		if (m_bEnableVolumeSwitch)
		{
			m_bUseVolumeSeek = !m_bUseVolumeSeek;
			g_Profile.WriteProfileInt(_T("UseVolumeSeek"), m_bUseVolumeSeek);
			UpdateVolumeCtrl();
			// SetInfoText(TOLOCAL(_T("Switch Volume Control")));
			return true;
		}
	}

	return false;
}

void CMPSkinWnd::OnUIObjNotify(IUIObjNotify *pNotify)
{
	LIST_SKINCMDHANDLER::iterator	it, itEnd;

	itEnd = m_listSkinCmdHandler.end();
	for (it = m_listSkinCmdHandler.begin(); it != itEnd; ++it)
	{
		ISkinCmdHandler		*pHandler = *it;
		if (pHandler->OnUIObjNotify(pNotify))
			return;
	}
}

void CMPSkinWnd::OnKeyDown(UINT nChar, UINT nFlags)
{
	CMPSkinMainWnd		*pMainWnd = g_mpApp.GetMainWnd();
	if (pMainWnd)
	{
		if (g_mpApp.GetHotkey().OnKeyDown(this, nChar, nFlags))
			return;
	}

	CSkinWnd::OnKeyDown(nChar, nFlags);

#ifdef _MPLAYER
	m_autoSwitchToLyrShow.OnUserAction();
#endif
}

void CMPSkinWnd::OnSizeModeChanged(WndSizeMode sizeMode)
{
	CSkinWnd::OnSizeModeChanged(sizeMode);

	// fix the bug auto width with lyrics when the windows is Minimized.
	if (sizeMode == WndSizeMode_Normal)
	{
		CUIObject		*pObj = GetUIObjectByClassName(CLyricShowObj::ClassName());
		if (pObj && !pObj->IsKindOf(CLyricShowTextEditObj::ClassName()))
		{
			CLyricShowObj	*pLyrObj = (CLyricShowObj*)pObj;
			IEvent	evt;
			evt.eventType = ET_LYRICS_CHANGED;
			pLyrObj->OnEvent(&evt);
		}

		CSkinNStatusButton *pMaximizeBt = (CSkinNStatusButton *)GetUIObjectById(CMD_MAXIMIZE, CSkinNStatusButton::ClassName());
		if (pMaximizeBt)
			pMaximizeBt->SetStatus(0);
	}
	else if (sizeMode == WndSizeMode_Minimized)
	{
		// If it is a tool window, minimize it.
		if (IsToolWindow() && IsVisible())
			ShowWindow(SW_HIDE);
	}
	else
	{
		CSkinNStatusButton *pMaximizeBt = (CSkinNStatusButton *)GetUIObjectById(CMD_MAXIMIZE, CSkinNStatusButton::ClassName());
		if (pMaximizeBt)
			pMaximizeBt->SetStatus(1);
	}
}

void CMPSkinWnd::OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar)
{
	if (pScrollBar->GetID() == CMD_VOLUME)
	{
		long		vol = 0;

		// set volume
		g_Player.SetVolume(pScrollBar->GetScrollPos());
		g_Player.GetVolume(vol);

		g_eventDispather.DispatchInfoText(CStrPrintf(_T("%s %d%%"), TOLOCAL(_T("Set Volume")), vol).c_str());
	}
	else if (pScrollBar->GetID() == CMD_SEEK)
	{
		int		nPos, nPercent, nMediaLength;
		TCHAR	szPos[256], szLength[256];

		nPos = pScrollBar->GetScrollPos();
		nMediaLength = g_Player.GetMediaLength();
		if (nMediaLength != 0)
		{
			nPercent = nPos * 100 / nMediaLength;
			FormatPlayTime(szPos, nPos);
			FormatPlayTime(szLength, nMediaLength);
			g_eventDispather.DispatchInfoText(CStrPrintf(_T("%s %s/%s (%d%%)"), TOLOCAL(_T("Seek")),
				szPos, szLength, nPercent).c_str(), _T("seek"));
		}

		// seek lyrics too...
		g_Player.UISeekbarOnSeek(nPos);
	}
}

void CMPSkinWnd::OnSkinLoaded()
{
#ifdef _MPLAYER
	m_autoSwitchToLyrShow.OnCreate(this);
#endif

	CSkinWnd::OnSkinLoaded();

	// load new skin command handler
	vector<tstring>		vHandler;
	StrBreak(m_strCmdHandler.c_str(), ',', vHandler);
	for (int i = 0; i < (int)vHandler.size(); i++)
	{
		TrimStr(vHandler[i]);
		AddCmdHandler(vHandler[i].c_str());
	}
	if (vHandler.empty())
		AddCmdHandler(_T("ch_common"));

#ifdef _WIN32_DESKTOP
	// m_bClickThrough = g_Profile.GetProfileBool(GetCfgSectName(), _T("ClickThrough"), false);
	m_bTopmost = g_Profile.GetProfileBool(GetCfgSectName(), _T("topmost"), true);

	SetTopmost(m_bTopmost);

	LoadTransparentSettings();
#endif

	CSkinNStatusButton	*pBt;

	// maximize button status
	pBt = (CSkinNStatusButton *)GetUIObjectById(CMD_MAXIMIZE, CSkinNStatusButton::ClassName());
	if (pBt && IsZoomed())
		pBt->SetStatus(1);

	// topmost button status
	pBt = (CSkinNStatusButton *)GetUIObjectById(CMD_TOPMOST, CSkinNStatusButton::ClassName());
	if (pBt && IsTopmost())
		pBt->SetStatus(1);

	UpdateVolumeCtrl();
}

void CMPSkinWnd::OnLoadWndSizePos()
{
#ifdef _WIN32_WCE
// 	rcWnd.left = CW_USEDEFAULT;
// 	rcWnd.right =  CW_USEDEFAULT;
// 	rcWnd.top = 26;
// 	rcWnd.bottom = 294;
#else
#ifdef _WIN32
	if (::GetParent(m_hWnd))
		return;
#endif

	// WndSizeMode		sizeMode;
	CRect			rcNew;
	DWORD			dwShowFlag = 0;

	if (IsZoomed())
		ShowWindow(SW_RESTORE);
		// dwShowFlag = SW_RESTORE;

	GetWindowRect(&m_rcReal);

	rcNew = m_rcReal;

	rcNew.left = g_Profile.GetProfileInt(GetSkinWndName(), _T("wnd_left"), rcNew.left);
	rcNew.top = g_Profile.GetProfileInt(GetSkinWndName(), _T("wnd_top"), rcNew.top);
	rcNew.right = rcNew.left + g_Profile.GetProfileInt(GetSkinWndName(), _T("wnd_width"), m_nWidth);
	rcNew.bottom = rcNew.top + g_Profile.GetProfileInt(GetSkinWndName(), _T("wnd_height"), m_nHeight);

	if (m_wndResizer.IsFixedWidth())
		rcNew.right = long(rcNew.left + m_nWidth * m_dbScale);
	if (m_wndResizer.IsFixedHeight())
		rcNew.bottom = long(rcNew.top + m_nHeight * m_dbScale);

	if (rcNew.Width() < m_wndResizer.GetMinCx())
		rcNew.right = rcNew.left + m_wndResizer.GetMinCx();
	if (rcNew.Height() < m_wndResizer.GetMinCy())
		rcNew.bottom = rcNew.top + m_wndResizer.GetMinCy();

	if (!rcNew.EqualRect(&m_rcReal))
		MoveWindowSafely(rcNew.left, rcNew.top, rcNew.Width(), rcNew.Height(), true);

/*	if (!m_wndResizer.IsFixedWidth() && !m_wndResizer.IsFixedHeight())
	{
// 		sizeMode = (WndSizeMode)g_Profile.GetProfileInt(GetSkinWndName(), _T("SizeMode"), WndSizeMode_Normal);
// 		if (sizeMode == WndSizeMode_Maximized)
// 			dwShowFlag = SW_MAXIMIZE;
// 			// ShowWindow(SW_MAXIMIZE);
// 		else if (sizeMode == WndSizeMode_Minimized)
// 			dwShowFlag = SW_MINIMIZE;
			// ShowWindow(SW_MINIMIZE);
	}*/

/*	WINDOWPLACEMENT		wp;
	memset(&wp, 0, sizeof(wp));
	wp.length = sizeof(wp);
	wp.flags = 0;
	wp.ptMaxPosition.y = wp.ptMaxPosition.x = -1;
	wp.ptMinPosition.y = wp.ptMinPosition.x = -1;
	wp.rcNormalPosition = rcNew;
	wp.showCmd = dwShowFlag;
	SetWindowPlacement(GetHandle(), &wp);*/
#endif
}

void CMPSkinWnd::OnAddUIObj(CUIObject *pObj)
{
	CSkinWnd::OnAddUIObj(pObj);

	int			nID = pObj->GetID();
	if (nID == CMD_PLAYPAUSE)
	{
		if (pObj->IsKindOf(CSkinNStatusButton::ClassName()))
		{
			CPlayerPlayPauseBtHandler	*pHandler;
			pHandler = new CPlayerPlayPauseBtHandler;
			pHandler->Register((CSkinNStatusButton*)pObj);
			m_mapEventHandlers[pObj] = pHandler;
		}
	}
	else if (nID == CMD_SEEK)
	{
		if (pObj->IsKindOf(CSkinSeekCtrl::ClassName()))
		{
			CPlayerSeekEventHandler *pHandler;
			pHandler = new CPlayerSeekEventHandler;
			pHandler->Register((CSkinSeekCtrl*)pObj);
			m_mapEventHandlers[pObj] = pHandler;
		}
	}
	else if (nID == CMD_VOLUME)
	{
		if (pObj->IsKindOf(CSkinSeekCtrl::ClassName()))
		{
			CPlayerVolumeEventHandler *pHandler;
			pHandler = new CPlayerVolumeEventHandler;
			pHandler->Register((CSkinSeekCtrl*)pObj);
			m_mapEventHandlers[pObj] = pHandler;
		}
	}
	else if (nID == CMD_SHUFFLE)
	{
		if (pObj->IsKindOf(CSkinNStatusButton::ClassName()))
		{
			CPlayerShuffleEventHandler *pHandler;
			pHandler = new CPlayerShuffleEventHandler;
			pHandler->Register((CSkinNStatusButton*)pObj);
			m_mapEventHandlers[pObj] = pHandler;
		}
	}
	else if (nID == CMD_LOOP)
	{
		if (pObj->IsKindOf(CSkinNStatusButton::ClassName()))
		{
			CPlayerLoopEventHandler *pHandler;
			pHandler = new CPlayerLoopEventHandler;
			pHandler->Register((CSkinNStatusButton*)pObj);
			m_mapEventHandlers[pObj] = pHandler;
		}
	}
#ifdef _MPLAYER
	else if (nID == CMD_RATE)
	{
		if (pObj->IsKindOf(CSkinRateCtrl::ClassName()))
		{
			CPlayerMediaChangedRateEventHandler *pHandler;
			pHandler = new CPlayerMediaChangedRateEventHandler;
			pHandler->Register((CSkinRateCtrl*)pObj);
			m_mapEventHandlers[pObj] = pHandler;
		}
	}
	else if (nID == ID_PLAYLIST)
	{
		if (pObj->IsKindOf(CSkinListCtrl::ClassName()))
		{
			CPlayerCurPlaylistChangedEventHandler *pHandler;
			pHandler = new CPlayerCurPlaylistChangedEventHandler;
			pHandler->Register((CSkinListCtrl*)pObj);
			m_mapEventHandlers[pObj] = pHandler;
		}
	}
	else if (nID == ID_STEREO_STAT)
	{
		if (pObj->IsKindOf(CSkinNStatusImage::ClassName()))
		{
			CMediaStereoStatusEventHandler *pHandler;
			pHandler = new CMediaStereoStatusEventHandler;
			pHandler->Register(pObj);
			m_mapEventHandlers[pObj] = pHandler;
		}
	}
#endif
	else
	{
		if (pObj->IsKindOf(CSkinToolbar::ClassName()))
		{
			CSkinToolbar	*pToolBar = (CSkinToolbar*)pObj;

			if (pToolBar->IsButtonExist(CMD_PLAYPAUSE))
			{
				CPlayerPlayPauseToolbarHandler *pHandler;
				pHandler = new CPlayerPlayPauseToolbarHandler;
				pHandler->Register(pToolBar);
				m_mapEventHandlers[pObj] = pHandler;
			}

			if (pToolBar->IsButtonExist(CMD_SHUFFLE) || pToolBar->IsButtonExist(CMD_LOOP))
			{
				CPlayerShuffleRepeatToolbarHandler *pHandler;
				pHandler = new CPlayerShuffleRepeatToolbarHandler;
				pHandler->Register(pToolBar);
				m_mapEventHandlers[pObj] = pHandler;
			}
		}
	}
}

void CMPSkinWnd::OnRemoveUIObj(CUIObject *pObj)
{
	MAP_EVENT_HANDLER::iterator	it;
	it = m_mapEventHandlers.find(pObj);
	if (it != m_mapEventHandlers.end())
	{
		IEventHandlerEx		*pHandler = (*it).second;
		pHandler->Unregister();
		m_mapEventHandlers.erase(it);
		delete pHandler;
	}

	CSkinWnd::OnRemoveUIObj(pObj);
}

#ifdef _MPLAYER
BOOL CMPSkinWnd::SetProperty(SXNode *pXmlNode)
{
	LPCTSTR		szName = pXmlNode->GetPropertySafe(SZ_PN_NAME);
	ASSERT(szName);
	if (!szName)
		return FALSE;

	if (IsPropertyName(szName, _T("LyrAutoSwitch")))
	{
		m_autoSwitchToLyrShow.m_bEnabled = true;
		m_autoSwitchToLyrShow.m_nCmdShowLyr = m_pSkinFactory->GetIDByName(
			pXmlNode->GetPropertySafe(_T("CmdSwithToLyrics")));
		m_autoSwitchToLyrShow.m_nCmdShowPlaylist = m_pSkinFactory->GetIDByName(
			pXmlNode->GetPropertySafe(_T("CmdSwithToPlaylist")));
		m_autoSwitchToLyrShow.m_nCmdShowInfoText = m_pSkinFactory->GetIDByName(
			pXmlNode->GetPropertySafe(_T("CmdSwithToInfoText")));

		return TRUE;
	}

	return CSkinWnd::SetProperty(pXmlNode);
}
#endif // #ifdef _MPLAYER

BOOL CMPSkinWnd::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (_tcsicmp(szProperty, _T("CmdHandler")) == 0)
	{
		m_strCmdHandler = szValue;
	}
	else if (_tcsicmp(szProperty, _T("EnableVolumeSwitch")) == 0)
	{
		m_bEnableVolumeSwitch = IsTRUE(szValue);
		m_bUseVolumeSeek = g_Profile.GetProfileBool(_T("UseVolumeSeek"), false);
	}
	else if (CSkinWnd::SetProperty(szProperty, szValue))
	{
		if (m_bSkinOpened && IsPropertyName(szProperty, _T("BgColor")))
		{
// 			if (g_Profile.GetProfileBool(GetCfgSectName(), _T("TransparentBkColor"), FALSE))
// 				LoadTransparentSettings();
		}
		return true;
	}
	else
		return false;

	return true;
}

void CMPSkinWnd::CloseSkin()
{
	if (m_bSkinOpened)
	{
		// the skin was opened before, so save latest settings.
		SaveWndPos();

#ifdef _MPLAYER
		m_autoSwitchToLyrShow.OnClose();
#endif
	}

	CSkinWnd::CloseSkin();

	LIST_SKINCMDHANDLER::iterator	it, itEnd;

	itEnd = m_listSkinCmdHandler.end();
	for (it = m_listSkinCmdHandler.begin(); it != itEnd; ++it)
	{
		ISkinCmdHandler		*pHandler = *it;
		delete pHandler;
	}
	m_listSkinCmdHandler.clear();
	m_strCmdHandler.resize(0);

	m_bTopmost = false;

	m_bEnableVolumeSwitch = false;
	m_bUseVolumeSeek = false;
}

// IUICheckStatus interface
bool CMPSkinWnd::GetChecked(UINT nID, bool &bChecked)
{
	switch (nID)
	{
	case IDC_SETTOPMOST:
		bChecked = IsTopmost();
		// bChecked = g_Profile.GetProfileBool(GetCfgSectName(), _T("topmost"), true);
		break;
#ifdef _WIN32
	case IDC_CLICK_THROUGH:
		// bChecked = g_Profile.GetProfileBool(GetCfgSectName(), _T("ClickThrough"), false);
		if (m_bIndividualProperties)
			bChecked = m_bClickThrough;
		else
			bChecked = g_mpApp.GetSkinFactory()->GetClickThrough();
		break;
#endif
// 	case IDC_USE_TRANSPT_BK_COLOR:
// 		bChecked = g_Profile.GetProfileBool(GetCfgSectName(), _T("TransparentBkColor"), FALSE);
// 		break;
	default:
		{
			LIST_SKINCMDHANDLER::iterator	it, itEnd;

			itEnd = m_listSkinCmdHandler.end();
			for (it = m_listSkinCmdHandler.begin(); it != itEnd; ++it)
			{
				ISkinCmdHandler		*p = *it;
				if (p->GetChecked(nID, bChecked))
					return true;
			}
		}
		return false;
	}

	return true;
}

bool CMPSkinWnd::GetRadioChecked(vector<UINT> &vIDs, UINT &nIDChecked)
{
	ASSERT(!vIDs.empty());
	if (vIDs.empty())
		return false;

	if (vIDs[0] == IDC_SET_OPAQUE_100)
	{
		if (g_Profile.GetProfileBool(GetCfgSectName(), _T("WndOpacity"), true))
			nIDChecked = IDC_SET_OPAQUE_100;
		else
		{
			int		nPercent = g_Profile.GetProfileInt(GetCfgSectName(), _T("WndOpaquePercent"), 70);
			if (nPercent >= 95) nIDChecked = IDC_SET_OPAQUE_100;
			else if (nPercent >= 85) nIDChecked = IDC_SET_OPAQUE_90;
			else if (nPercent >= 75) nIDChecked = IDC_SET_OPAQUE_80;
			else if (nPercent >= 65) nIDChecked = IDC_SET_OPAQUE_70;
			else if (nPercent >= 55) nIDChecked = IDC_SET_OPAQUE_60;
			else if (nPercent >= 45) nIDChecked = IDC_SET_OPAQUE_50;
			else if (nPercent >= 35) nIDChecked = IDC_SET_OPAQUE_40;
			else if (nPercent >= 25) nIDChecked = IDC_SET_OPAQUE_30;
			else if (nPercent >= 15) nIDChecked = IDC_SET_OPAQUE_20;
			else nIDChecked = IDC_SET_OPAQUE_10;
		}
		return true;
	}
	else if (vIDs[0] == IDC_LDS_MULTI_LINE)
	{
		tstring		strName;

		CLyricShowAgentObj::GetLyrDispStyleSettings(this, strName);

		if (IsPropertyName(strName.c_str(), CLyricShowMultiRowObj::ClassName()))
			nIDChecked = IDC_LDS_MULTI_LINE;
		else if (IsPropertyName(strName.c_str(), CLyricShowTxtObj::ClassName()))
			nIDChecked = IDC_LDS_STATIC_TXT;
		else if (IsPropertyName(strName.c_str(), CLyricShowTwoRowObj::ClassName()))
			nIDChecked = IDC_LDS_TWO_LINE;
		else if (IsPropertyName(strName.c_str(), CLyricShowHorzScrollObj::ClassName()))
			nIDChecked = IDC_LDS_HORZ_SCROLL;
		else if (IsPropertyName(strName.c_str(), CLyricShowSingleRowObj::ClassName()))
			nIDChecked = IDC_LDS_SINGLE_LINE;
		else if (IsPropertyName(strName.c_str(), CLyricShowVobSub::ClassName()))
			nIDChecked = IDC_LDS_VOBSUB;
		else
			nIDChecked = IDC_LDS_MULTI_LINE;

		return true;
	}
	else
	{
		LIST_SKINCMDHANDLER::iterator	it, itEnd;

		itEnd = m_listSkinCmdHandler.end();
		for (it = m_listSkinCmdHandler.begin(); it != itEnd; ++it)
		{
			ISkinCmdHandler		*p = *it;
			if (p->GetRadioChecked(vIDs, nIDChecked))
				return true;
		}
	}

	return false;
}

void CMPSkinWnd::AddCmdHandler(LPCTSTR szName)
{
	ISkinCmdHandler		*pHandler = NULL;

	if (_tcsicmp(szName, _T("ch_common")) == 0)
		pHandler = new CMPCommonCmdHandler;
#ifdef _MPLAYER
	else if (_tcsicmp(szName, _T("ch_playlist")) == 0)
		pHandler = new CMPPlaylistCmdHandler;
	else if (_tcsicmp(szName, _T("ch_medialib_ppc")) == 0)
		pHandler = new CMPMediaLibCmdHandler;
	else if (_tcsicmp(szName, _T("ch_mediaguide")) == 0)
		pHandler = new CMPCmdHandlerOfMediaGuide;
#endif
	else if (_tcsicmp(szName, _T("ch_floating_lyr")) == 0)
		pHandler = new CMPCommonCmdHandler(true);
	else
	{
		ERRLOG1(_T("Unknow command handler: %s"), szName);
		return;
	}

	pHandler->Init(this);
	m_listSkinCmdHandler.push_back(pHandler);
}

#ifdef _WIN32_DESKTOP
void CMPSkinWnd::OnDropFiles(HDROP hDrop)
{
	TCHAR		szFile[MAX_PATH];
	int			n;
	TCHAR		szAssociateKeyWord[MAX_PATH];

	n = DragQueryFile(hDrop, (UINT)-1, szFile, CountOf(szFile));

	if (n == 1)
	{
		DragQueryFile(hDrop, 0, szFile, CountOf(szFile));
		if (FileIsExtSame(szFile, _T(".lrc")) || FileIsExtSame(szFile, _T(".txt"))
			|| FileIsExtSame(szFile, _T(".srt")))
		{
			g_mpApp.GetAssociateFileKeyword(szAssociateKeyWord, MAX_PATH);
			g_LyricSearch.AssociateLyrics(szAssociateKeyWord, szFile);
			g_mpApp.ResearchLyrics();

			DragFinish(hDrop);
			return;
		}
	}

#ifdef _MPLAYER
	g_Player.ClearPlaylist();
	g_Player.SetPlaylistModified(true);

	for (int i = 0; i < n; ++i)
	{
		DragQueryFile(hDrop, i, szFile, CountOf(szFile));

		if (IsDirExist(szFile))
			g_Player.AddDirToPlaylist(szFile);
		else
			g_Player.AddToPlaylist(szFile);
	}

	g_Player.Play();
#endif

	DragFinish(hDrop);
}

void CMPSkinWnd::LoadTransparentSettings()
{
	if (!IsLayeredWndSupported())
		return;

#ifndef _MPLAYER
	if (::GetParent(m_hWnd))
		return;
#endif

	BYTE			byAlpha = 255;
	bool			bTransparentClr = false;

	if (!g_Profile.GetProfileBool(GetCfgSectName(), _T("WndOpacity"), true))
	{
		// 半透明值
		int		nOpaque = g_Profile.GetProfileInt(GetCfgSectName(), _T("WndOpaquePercent"), 70);
		if (nOpaque < 10)
			nOpaque = 10;
		else if (nOpaque > 100)
			nOpaque = 100;
		byAlpha = nOpaque * 255 / 100;
	}

	// bTransparentClr = g_Profile.GetProfileBool(GetCfgSectName(), _T("TransparentBkColor"), FALSE);

#ifdef _WIN32
	if (!m_bIndividualProperties && g_mpApp.GetSkinFactory()->GetClickThrough())
		m_bClickThrough = true;

	SetTransparent(byAlpha, bTransparentClr, m_clrBg.get(), m_bClickThrough);
#endif
}

#endif

void CMPSkinWnd::UpdateVolumeCtrl()
{
	if (m_bEnableVolumeSwitch)
	{
		CUIObject	*pObj;
		if (m_bUseVolumeSeek)
		{
			pObj = GetUIObjectById(CMD_VOL_INC);
			if (pObj)
				pObj->SetVisible(false);
			pObj = GetUIObjectById(CMD_VOL_DEC);
			if (pObj)
				pObj->SetVisible(false);
			pObj = GetUIObjectById(ID_VOL_BT_IMG);
			if (pObj)
				pObj->SetVisible(false);
			pObj = GetUIObjectById(CMD_VOLUME);
			if (pObj)
				pObj->SetVisible(true);
		}
		else
		{
			pObj = GetUIObjectById(CMD_VOL_INC);
			if (pObj)
				pObj->SetVisible(true);
			pObj = GetUIObjectById(CMD_VOL_DEC);
			if (pObj)
				pObj->SetVisible(true);
			pObj = GetUIObjectById(ID_VOL_BT_IMG);
			if (pObj)
				pObj->SetVisible(true);
			pObj = GetUIObjectById(CMD_VOLUME);
			if (pObj)
				pObj->SetVisible(false);
		}
		InvalidateRect();
	}
}

// COMMENT:
//		save current skin window size pos.
void CMPSkinWnd::SaveWndPos()
{
#ifdef _WIN32_DESKTOP
	if (::GetParent(m_hWnd))
		return;

	WINDOWPLACEMENT	wp;
	CRect			rcToSave;

	wp.length = sizeof(wp);
	if (GetWindowPlacement(m_hWnd, &wp))
	{
		rcToSave = wp.rcNormalPosition;
	}
	else
		rcToSave = m_rcReal;

	WndSizeMode		sizeMode;
	if (IsFlagSet(wp.showCmd, SW_SHOWMAXIMIZED))
		sizeMode = WndSizeMode_Maximized;
	else if (IsFlagSet(wp.showCmd, SW_SHOWMINIMIZED))
		sizeMode = WndSizeMode_Minimized;
	else
		sizeMode = WndSizeMode_Normal;

	g_Profile.WriteProfileInt(GetSkinWndName(), _T("SizeMode"), sizeMode);

	g_Profile.WriteProfileInt(GetSkinWndName(), _T("wnd_left"), rcToSave.left);
	g_Profile.WriteProfileInt(GetSkinWndName(), _T("wnd_top"), rcToSave.top);
	g_Profile.WriteProfileInt(GetSkinWndName(), _T("wnd_width"), rcToSave.Width());
	g_Profile.WriteProfileInt(GetSkinWndName(), _T("wnd_height"), rcToSave.Height());
#endif
}

void CMPSkinWnd::SetTopmost(bool bTopmost)
{
	GetParentOrSelf()->SetTopmost(bTopmost);
}

CBaseWnd *CMPSkinWnd::GetParentOrSelf()
{
#ifdef _WIN32
	HWND		hParent = m_hWnd, hParentNew = m_hWnd;
	while (hParentNew)
	{
		hParent = hParentNew;
		hParentNew = ::GetParent(hParent);
	}
	if (hParent != m_hWnd)
		return CBaseWnd::FromHandle(hParent);
	else
		return this;
#else
	return this;
#endif
}

LPCTSTR CMPSkinWnd::GetCfgSectName()
{
	if (m_bIndividualProperties)
		return GetSkinWndName();
	else
		return SZ_SECT_UI;
}
