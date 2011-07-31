// MPSkinNumInfoCtrl.cpp: implementation of the CMPSkinMediaNumInfoCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerAppBase.h"
#include "MPSkinMediaNumInfoCtrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CMPSkinMediaNumInfoCtrl, _T("NumInfo"))

CMPSkinMediaNumInfoCtrl::CMPSkinMediaNumInfoCtrl()
{
	m_infoType = IT_BITRATE;
}

CMPSkinMediaNumInfoCtrl::~CMPSkinMediaNumInfoCtrl()
{
	CMPlayerAppBase::GetEventsDispatcher()->UnregisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
	CMPlayerAppBase::GetEventsDispatcher()->UnregisterHanlder(ET_PLAYER_CUR_MEDIA_INFO_CHANGED, this);
}

void CMPSkinMediaNumInfoCtrl::OnCreate()
{
	CSkinPicText::OnCreate();

	CMPlayerAppBase::GetEventsDispatcher()->RegisterHanlder(ET_PLAYER_CUR_MEDIA_CHANGED, this);
	CMPlayerAppBase::GetEventsDispatcher()->RegisterHanlder(ET_PLAYER_CUR_MEDIA_INFO_CHANGED, this);

	UpdateShowTrackInfo();
}

void CMPSkinMediaNumInfoCtrl::OnEvent(const IEvent *pEvent)
{
	if (pEvent->eventType == ET_PLAYER_CUR_MEDIA_CHANGED || pEvent->eventType == ET_PLAYER_CUR_MEDIA_INFO_CHANGED)
	{
		UpdateShowTrackInfo();
		Invalidate();
	}
}

BOOL CMPSkinMediaNumInfoCtrl::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CSkinPicText::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, _T("InfoType")) == 0)
	{
		if (_tcsicmp(szValue, _T("kbps")) == 0)
			m_infoType = IT_BITRATE;
		else
			m_infoType = IT_SAMPLERATE;
	}
	else
		return FALSE;

	return TRUE;
}

#ifdef _SKIN_EDITOR_
void CMPSkinMediaNumInfoCtrl::EnumProperties(CUIObjProperties &listProperties)
{

}
#endif // _SKIN_EDITOR_

void CMPSkinMediaNumInfoCtrl::UpdateShowTrackInfo()
{
	CMPAutoPtr<IMedia>			pMedia;
	int			nValue = 0;
	TCHAR		szText[64];

	if (g_Player.GetCurrentMedia(&pMedia) != ERR_OK)
	{
		SetText(_T(""));
		return;
	}

	if (m_infoType == IT_BITRATE)
	{
		pMedia->GetAttribute(MA_BITRATE, &nValue);
		nValue /= 1000;
	}
	else
		pMedia->GetAttribute(MA_SAMPLE_RATE, &nValue);

	_stprintf(szText, _T("%d"), nValue);
	SetText(szText);
}
