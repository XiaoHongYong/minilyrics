// MPSkinWnd.cpp: implementation of the CMPSkinWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "MPSkinWnd.h"
#include "MPCommonCmdHandler.h"

#include "MPPlaylistCmdHandler.h"
#ifdef _MPLAYER
#include "MPMediaLibCmdHandler.h"
#endif

#include "SkinRateCtrl.h"
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
			CMPlayerAppBase::GetEventsDispatcher()->RegisterHandler(ET_PLAYER_STATUS_CHANGED, this);
		m_listStatePlaypause.Add(pUpdater);
	}

	virtual void DelPlayPauseUpdater(HANDLE hObj)
	{
		m_listStatePlaypause.Del(hObj);
		if (m_listStatePlaypause.empty())
			CMPlayerAppBase::GetEventsDispatcher()->UnRegisterHandler(ET_PLAYER_STATUS_CHANGED, this);
	}

	LIST_UISTATE_UPDATER		m_listStatePlaypause;

};


CUIPlayerStatusHandler		g_UIPlayerStatusHandler;*/

class CPlayerShuffleEventHandler : public IEventHandler
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
		RegisterHandler(CMPlayerAppBase::GetEventsDispatcher(), ET_PLAYER_SETTING_CHANGED);
	}

protected:
	CSkinNStatusButton			*m_pShuffle;

};

class CPlayerLoopEventHandler : public IEventHandler
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
		RegisterHandler(CMPlayerAppBase::GetEventsDispatcher(), ET_PLAYER_SETTING_CHANGED);
	}

protected:
	CSkinNStatusButton			*m_pLoop;

};

class CPlayerVolumeEventHandler : public IEventHandler
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
		m_pVolume = pVolume;
		m_pVolume->SetScrollInfo(0, MP_VOLUME_MAX, 0, g_Player.GetVolume());
		RegisterHandler(CMPlayerAppBase::GetEventsDispatcher(), ET_PLAYER_SETTING_CHANGED);
	}

protected:
	CSkinSeekCtrl		*m_pVolume;

};

class CPlayerSeekEventHandler : public IEventHandler
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
		RegisterHandler(CMPlayerAppBase::GetEventsDispatcher(), ET_PLAYER_CUR_MEDIA_CHANGED, ET_PLAYER_POS_UPDATE, ET_PLAYER_STATUS_CHANGED);
	}

protected:
	CSkinSeekCtrl		*m_pSeekCtrl;

};

class CPlayerPlayPauseBtHandler : public IEventHandler
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
		RegisterHandler(CMPlayerAppBase::GetEventsDispatcher(), ET_PLAYER_STATUS_CHANGED);
	}

protected:
	CSkinNStatusButton		*m_pButtn;

};

class CPlayerPlayPauseToolbarHandler : public IEventHandler
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
		RegisterHandler(CMPlayerAppBase::GetEventsDispatcher(), ET_PLAYER_STATUS_CHANGED);
	}

protected:
	CSkinToolbar		*m_pToolbar;

};

class CPlayerShuffleRepeatToolbarHandler : public IEventHandler
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

		RegisterHandler(CMPlayerAppBase::GetEventsDispatcher(), ET_PLAYER_SETTING_CHANGED);
	}

protected:
	CSkinToolbar		*m_pToolbar;

};

#ifdef _MPLAYER
//
// Player controls
//

// Media
class CMediaStereoStatusEventHandler : public IEventHandler
{
public:
	virtual void OnEvent(const IEvent *pEvent)
	{
		if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED)
		{
			CMPAutoPtr<IMedia>			pMedia;
			if (g_Player.GetCurrentMedia(&pMedia) == ERR_OK)
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
		if (g_Player.GetCurrentMedia(&pMedia) == ERR_OK)
		{
			int		nChannels;
			pMedia->GetAttribute(MA_CHANNELS, &nChannels);
			if (nChannels >= 2)
				m_pStereoStatusCtrl->SetPropertyInt(_T("CurStatus"), 1);
			else
				m_pStereoStatusCtrl->SetPropertyInt(_T("CurStatus"), 0);
		}
		RegisterHandler(CMPlayerAppBase::GetEventsDispatcher(), ET_PLAYER_CUR_MEDIA_CHANGED);
	}

protected:
	CUIObject		*m_pStereoStatusCtrl;;

};

class CPlayerMediaChangedRateEventHandler : public IEventHandler
{
public:
	virtual void OnEvent(const IEvent *pEvent)
	{
		if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED || pEvent->eventType == ET_PLAYER_CUR_MEDIA_INFO_CHANGED)
		{
			CMPAutoPtr<IMedia>			pMedia;
			if (g_Player.GetCurrentMedia(&pMedia) == ERR_OK)
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
		if (g_Player.GetCurrentMedia(&pMedia) == ERR_OK)
		{
			int		nRating = 0;
			pMedia->GetAttribute(MA_RATING, &nRating);
			m_pRateCtrl->SetRating(nRating);
		}
		RegisterHandler(CMPlayerAppBase::GetEventsDispatcher(), ET_PLAYER_CUR_MEDIA_CHANGED, ET_PLAYER_CUR_MEDIA_INFO_CHANGED);
	}

protected:
	CSkinRateCtrl		*m_pRateCtrl;

};

#endif // _MPLAYER

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMPSkinWnd::CMPSkinWnd()
{
	m_autoSwitchToLyrShow.m_pSkinWnd = this;
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
}

void CMPSkinWnd::OnPreCreate(bool &bTopmost, bool &bVisible)
{
	bTopmost = SettingGetTopmost();
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
	m_autoSwitchToLyrShow.OnEvent(pEvent);
}

void CMPSkinWnd::OnCommand(UINT uID, UINT nNotifyCode)
{
	int	nUID = m_pSkinFactory->GetUIDByMenuID(uID);
	if (nUID != UID_INVALID)
		CMPlayerApp::GetAppAnalytics().IncreaseCmdClickCount(nUID, CAppAnalyticsClient::CAM_MENU);

#ifdef _WIN32_DESKTOP
	switch (uID)
	{
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

			SettingSetOpaquePercent(nPercent);
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
}

bool CMPSkinWnd::OnCustomCommand(int nId)
{
	m_autoSwitchToLyrShow.OnCustomCmdNotify(nId);

	if (CSkinWnd::OnCustomCommand(nId))
		return true;

	switch (nId)
	{
	case CMD_TOPMOST:
		SettingReverseTopmost();
		return true;
	case CMD_CLICK_THROUGH:
		SettingReverseClickThrough();
		return true;
	case CMD_LDS_MULTI_LINE:
	case CMD_LDS_STATIC_TXT:
	case CMD_LDS_TWO_LINE:
	case CMD_LDS_HORZ_SCROLL:
	case CMD_LDS_SINGLE_LINE:
	case CMD_LDS_VOBSUB:
		{
			// special lyrics display style
			tstring		strName;
			if (nId == CMD_LDS_MULTI_LINE)
				strName = CLyricShowMultiRowObj::ClassName();
			else if (nId == CMD_LDS_STATIC_TXT)
				strName = SZ_TXT_LYR_CONTAINER;
			else if (nId == CMD_LDS_TWO_LINE)
				strName = CLyricShowTwoRowObj::ClassName();
			else if (nId == CMD_LDS_HORZ_SCROLL)
				strName = CLyricShowHorzScrollObj::ClassName();
			else if (nId == CMD_LDS_SINGLE_LINE)
				strName = CLyricShowSingleRowObj::ClassName();
			else if (nId == CMD_LDS_VOBSUB)
				strName = CLyricShowVobSub::ClassName();
			else
				ASSERT(0);

			CLyricShowAgentObj::SetLyrDispStyleSettings(this, strName.c_str());
		}
		return true;
	}

	LIST_SKINCMDHANDLER::iterator	it, itEnd;

	itEnd = m_listSkinCmdHandler.end();
	for (it = m_listSkinCmdHandler.begin(); it != itEnd; ++it)
	{
		ISkinCmdHandler		*pHandler = *it;
		if (pHandler->OnCustomCommand(nId))
			return true;
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
	CMPSkinMainWnd		*pMainWnd = CMPlayerAppBase::GetMainWnd();
	if (pMainWnd)
	{
		if (CMPlayerAppBase::GetHotkey().OnKeyDown(this, nChar, nFlags))
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
		// set volume
		g_Player.SetVolume(pScrollBar->GetScrollPos());
		long vol = g_Player.GetVolume();

		CMPlayerAppBase::GetInstance()->DispatchInfoText(CStrPrintf(_T("%s %d%%"), _TLT("Set Volume"), vol).c_str());
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
			FormatPlayTime(nPos, szPos);
			FormatPlayTime(nMediaLength, szLength);
			CMPlayerAppBase::GetInstance()->DispatchInfoText(CStrPrintf(_T("%s %s/%s (%d%%)"), _TLT("Seek"),
				szPos, szLength, nPercent).c_str(), _T("seek"));
		}

		// seek lyrics too...
		g_Player.UISeekbarOnSeek(nPos);
	}
}

void CMPSkinWnd::OnSkinLoaded()
{
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

	// 半透明值
	int		nOpaque = SettingGetOpaquePercent();
	SetTransparent(OpaquePercentToAlpha(nOpaque), SettingGetClickThrough());

	CSkinNStatusButton	*pBt;

	// maximize button status
	pBt = (CSkinNStatusButton *)GetUIObjectById(CMD_MAXIMIZE, CSkinNStatusButton::ClassName());
	if (pBt && IsZoomed())
		pBt->SetStatus(1);

	// topmost button status
	pBt = (CSkinNStatusButton *)GetUIObjectById(CMD_TOPMOST, CSkinNStatusButton::ClassName());
	if (pBt && IsTopmost())
		pBt->SetStatus(1);

	m_autoSwitchToLyrShow.OnCreate(this);
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
		IEventHandler		*pHandler = (*it).second;
		pHandler->UnregisterHandler();
		m_mapEventHandlers.erase(it);
		delete pHandler;
	}

	CSkinWnd::OnRemoveUIObj(pObj);
}

BOOL CMPSkinWnd::SetProperty(LPCTSTR szProperty, SXNode *pXmlNode)
{
	ASSERT(szProperty);
	if (IsPropertyName(szProperty, _T("LyrAutoSwitch")))
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

	return CSkinWnd::SetProperty(szProperty, pXmlNode);
}

BOOL CMPSkinWnd::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (_tcsicmp(szProperty, _T("CmdHandler")) == 0)
	{
		m_strCmdHandler = szValue;
	}
	else
		return CSkinWnd::SetProperty(szProperty, szValue);

	return true;
}

void CMPSkinWnd::CloseSkin()
{
	if (!m_bSkinOpened)
		return;

	CSkinWnd::CloseSkin();

	m_autoSwitchToLyrShow.OnClose();

	LIST_SKINCMDHANDLER::iterator	it, itEnd;

	itEnd = m_listSkinCmdHandler.end();
	for (it = m_listSkinCmdHandler.begin(); it != itEnd; ++it)
	{
		ISkinCmdHandler		*pHandler = *it;
		delete pHandler;
	}
	m_listSkinCmdHandler.clear();
	m_strCmdHandler.resize(0);
}

// IUICheckStatus interface
bool CMPSkinWnd::GetChecked(UINT nID, bool &bChecked)
{
	switch (nID)
	{
	case IDC_SETTOPMOST:
		bChecked = IsTopmost();
		break;
	case IDC_CLICK_THROUGH:
		bChecked = m_bClickThrough;
		break;
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
		int		nPercent = SettingGetOpaquePercent();
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

		return true;
	}
	else if (vIDs[0] == IDC_LDS_MULTI_LINE)
	{
		tstring		strName;

		CLyricShowAgentObj::GetLyrDispStyleSettings(this, strName);

		if (IsPropertyName(strName.c_str(), CLyricShowMultiRowObj::ClassName()))
			nIDChecked = IDC_LDS_MULTI_LINE;
		else if (IsPropertyName(strName.c_str(), SZ_TXT_LYR_CONTAINER))
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

void CMPSkinWnd::PostCustomCommandMsg(int nId)
{
	CSkinWnd::PostCustomCommandMsg(nId);

	CMPlayerApp::GetAppAnalytics().IncreaseCmdClickCount(nId, CAppAnalyticsClient::CAM_BUTTON);
}

void CMPSkinWnd::PostShortcutKeyCmd(int nId)
{
	CSkinWnd::PostShortcutKeyCmd(nId);

	CMPlayerApp::GetAppAnalytics().IncreaseCmdClickCount(nId, CAppAnalyticsClient::CAM_HOTKEY);
}

void CMPSkinWnd::AddCmdHandler(LPCTSTR szName)
{
	ISkinCmdHandler		*pHandler = NULL;

	if (_tcsicmp(szName, _T("ch_common")) == 0)
		pHandler = new CMPCommonCmdHandler();
#ifdef _MPLAYER
	else if (_tcsicmp(szName, _T("ch_playlist")) == 0)
		pHandler = new CMPPlaylistCmdHandler();
	else if (_tcsicmp(szName, _T("ch_medialib_ppc")) == 0)
		pHandler = new CMPMediaLibCmdHandler();
//	else if (_tcsicmp(szName, _T("ch_mediaguide")) == 0)
//		pHandler = new CMPCmdHandlerOfMediaGuide;
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

	n = DragQueryFile(hDrop, (UINT)-1, szFile, CountOf(szFile));

	if (n == 1)
	{
		DragQueryFile(hDrop, 0, szFile, CountOf(szFile));
		if (FileIsExtSame(szFile, _T(".lrc")) || FileIsExtSame(szFile, _T(".txt"))
			|| FileIsExtSame(szFile, _T(".srt")))
		{
			g_LyricSearch.AssociateLyrics(CMPlayerAppBase::GetAssociateFileKeyword().c_str(), szFile);
			CMPlayerAppBase::GetInstance()->DispatchResearchLyrics();

			DragFinish(hDrop);
			return;
		}
	}

	g_Player.ClearPlaylist();

	for (int i = 0; i < n; ++i)
	{
		DragQueryFile(hDrop, i, szFile, CountOf(szFile));

		if (IsDirExist(szFile))
			g_Player.AddDirToPlaylist(szFile);
		else
			g_Player.AddToPlaylist(szFile);
	}

	g_Player.Play();

	DragFinish(hDrop);
}
#endif

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

bool CMPSkinWnd::SettingGetTopmost()
{
	return g_Profile.GetBool(SZ_SECT_UI, _T("topmost"), true);
}

int CMPSkinWnd::SettingGetOpaquePercent()
{
	return g_Profile.GetInt(SZ_SECT_UI, _T("WindowOpaquePercent"), 100);
}

bool CMPSkinWnd::SettingGetClickThrough()
{
	return CMPlayerAppBase::GetMPSkinFactory()->GetClickThrough();
}

void CMPSkinWnd::SettingReverseTopmost()
{
	bool		bTopmost = !IsTopmost();

	g_Profile.WriteInt(SZ_SECT_UI, _T("topmost"), bTopmost);
	CMPlayerAppBase::GetMPSkinFactory()->TopmostAll(bTopmost);
}

void CMPSkinWnd::SettingSetOpaquePercent(int nPercent)
{
	g_Profile.WriteInt(SZ_SECT_UI, _T("WindowOpaquePercent"), nPercent);
	CMPlayerAppBase::GetMPSkinFactory()->AllUpdateTransparent();
}

void CMPSkinWnd::SettingReverseClickThrough()
{
	CMPlayerAppBase::GetMPSkinFactory()->SetClickThrough(!CMPlayerAppBase::GetMPSkinFactory()->GetClickThrough());
}
