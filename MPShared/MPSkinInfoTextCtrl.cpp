// MPSkinInfoCtrl.cpp: implementation of the CMPSkinInfoTextCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerAppBase.h"
#include "MPSkinInfoTextCtrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CMPSkinInfoTextCtrl, _T("InfoText"))

CMPSkinInfoTextCtrl::CMPSkinInfoTextCtrl()
{
	m_nMsgWant = UO_MSG_WANT_LBUTTON | UO_MSG_WANT_MOUSEMOVE;
	m_nTimerIDHideInfo = 0;
	m_dwAlignText = DT_CENTER | DT_VCENTER | DT_SINGLELINE;
}

CMPSkinInfoTextCtrl::~CMPSkinInfoTextCtrl()
{
	if (m_nTimerIDHideInfo != 0)
		m_pSkin->UnregisterTimerObject(this, m_nTimerIDHideInfo);
}

void CMPSkinInfoTextCtrl::OnCreate()
{
	CSkinScrollText::OnCreate();

	RegisterHandler(CMPlayerAppBase::GetEventsDispatcher(), ET_UI_INFO_TEXT, ET_UI_LONG_ERROR_TEXT, ET_LICENSE_CHANGED, ET_UI_ACTIVATE_NOTICE);
	RegisterHandler(CMPlayerAppBase::GetEventsDispatcher(), ET_LYRICS_CHANGED);

	UpdateShowDefaultInfo();
	m_Cursor.LoadStdCursor(CCursor::C_HAND);
}

void CMPSkinInfoTextCtrl::OnEvent(const IEvent *pEvent)
{
	if (pEvent->eventType == ET_UI_INFO_TEXT)
	{
		SetText(pEvent->strValue.c_str());
		m_strCmd.clear();
		Invalidate();
		m_pSkin->UnregisterTimerObject(this);
		m_nTimerIDHideInfo = m_pSkin->RegisterTimerObject(this, 5000);
		SetVisible(true, true);
	}
	else if (pEvent->eventType == ET_UI_LONG_ERROR_TEXT)
	{
		SetText(pEvent->strValue.c_str());
		m_strCmd = pEvent->strName;
		Invalidate();
		m_pSkin->UnregisterTimerObject(this);
		m_nTimerIDHideInfo = m_pSkin->RegisterTimerObject(this, 30000);
		SetVisible(true, true);
	}
	else if (pEvent->eventType == ET_LICENSE_CHANGED || pEvent->eventType == ET_LYRICS_CHANGED || pEvent->eventType == ET_UI_ACTIVATE_NOTICE)
		UpdateShowDefaultInfo();
}

bool CMPSkinInfoTextCtrl::OnMouseMove(CPoint point)
{
	if (m_strCmd.empty())
		return false;

	MLWidget::SetCursor(m_Cursor);

	return true;
}

bool CMPSkinInfoTextCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_strCmd.empty())
		return false;

	LPCTSTR SZ_CMD = _T("cmd://");
	if (StrBeginWith(m_strCmd.c_str(), SZ_CMD))
	{
		tstring strIDs = m_strCmd.c_str() + _tcslen(SZ_CMD);
		VecString vIDs;
		StrSplit(strIDs.c_str(), '|', vIDs);

		if (vIDs.size() > 1)
		{
			CSkinMenu m_menu;

			m_menu.CreatePopupMenu();
			for (uint i = 0; i < vIDs.size(); i++)
			{
				int nId = m_pSkin->GetSkinFactory()->GetIDByName(vIDs[i].c_str());
				tstring tooltip;
				m_pSkin->GetSkinFactory()->GetTooltip(nId, tooltip);
				m_menu.AppendItem(m_pSkin->GetSkinFactory()->GetMenuIDByUID(nId), tooltip.c_str());
			}

			POINT point;
			MLWidget::GetCursorPos(&point);
			m_menu.TrackPopupMenu(point.x, point.y, m_pSkin);
		}
		else
		{
			int nId = m_pSkin->GetSkinFactory()->GetIDByName(vIDs[0].c_str());
			m_pSkin->PostCustomCommandMsg(nId);
		}
	}
	else if (StrBeginWith(m_strCmd.c_str(), _T("http://")))
	{
		OpenUrl(m_pSkin, m_strCmd.c_str());
	}

	MLWidget::SetCursor(m_Cursor);

	return true;
}

bool CMPSkinInfoTextCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_strCmd.empty())
		return false;

	MLWidget::SetCursor(m_Cursor);

	return true;
}

void CMPSkinInfoTextCtrl::OnTimer(int nId)
{
	if (nId == m_nTimerIDHideInfo)
	{
		m_pSkin->UnregisterTimerObject(this, m_nTimerIDHideInfo);
		m_nTimerIDHideInfo = 0;
		UpdateShowDefaultInfo();
		Invalidate();
	}
	else
		CSkinScrollText::OnTimer(nId);
}

void CMPSkinInfoTextCtrl::Draw(CRawGraph *pGraph)
{
	if (IsFlagSet(m_dwAlignText, AT_CENTER))
		CSkinStaticText::Draw(pGraph);
	else
		CSkinScrollText::Draw(pGraph);
}

void CMPSkinInfoTextCtrl::UpdateShowDefaultInfo()
{
	if (!g_licenseManager.IsLicenseValid())
	{
		tstring msg = _TLT("Do you think MiniLyrics is useful to you?");
		msg += _T(" ");
		msg += _TLT("Please activate MiniLyrics.");
		SetText(msg.c_str());
		m_strCmd = _T("cmd://") + m_pSkin->GetSkinFactory()->GetStringOfID(CMD_REGISTER);
	}
	else
	{
		// Hide itself.
		SetVisible(false, true);
	}
	// SetText(g_Player.GetFullTitle());
}
