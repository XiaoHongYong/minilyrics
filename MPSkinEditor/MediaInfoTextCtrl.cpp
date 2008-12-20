// MediaInfoTextCtrl.cpp: implementation of the CMediaInfoTextCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MediaInfoTextCtrl.h"

ID_TO_STRING __mediaInfoType[] = 
{
	{ MA_ARTIST, _T("Artist") },
	{ MA_TITLE, _T("Title") },
	{ MA_ALBUM, _T("Album") },
	{ MA_TRACK_NUMB, _T("Track") },
	{ MA_YEAR, _T("Year") },
	{ MA_GENRE, _T("Genre") },
	{ MA_BITRATE, _T("Bit rate") },
	{ MA_SAMPLE_RATE, _T("Sample rate") },
	{ 0, NULL },
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CMediaInfoTextCtrl, _T("MediaInfoText"))

CMediaInfoTextCtrl::CMediaInfoTextCtrl()
{
	m_nSwitchTime = 5000;
	m_nCurrentItem = 0;
	m_bShowTitle = true;
}

CMediaInfoTextCtrl::~CMediaInfoTextCtrl()
{
	m_pSkin->UnregisterTimerObject(this);
}

void CMediaInfoTextCtrl::OnCreate()
{
	CSkinScrollText::OnCreate();

	if (m_vInfoType.size() > 1)
		m_pSkin->RegisterTimerObject(this, m_nSwitchTime);

	UpdateShowTrackInfo();
}

void CMediaInfoTextCtrl::OnTimer(int nId)
{
	if (nId == m_nIDTimerUpdateTrackInfo)
	{
		m_nCurrentItem++;
		if (m_nCurrentItem >= m_vInfoType.size())
			m_nCurrentItem = 0;
		UpdateShowTrackInfo();
		Invalidate();
	}
	else
		CSkinScrollText::OnTimer(nId);
}

BOOL CMediaInfoTextCtrl::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CSkinScrollText::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, _T("MediaInfoType")) == 0)
	{
		V_Str		vStr;

		m_vInfoType.clear();
		StrBreak(szValue, '|', vStr);
		for (int i = 0; i < vStr.size(); i++)
		{
			TrimStr(vStr[i]);
			m_vInfoType.push_back((MediaAttribute)StringToID(__mediaInfoType, vStr[i].c_str(), MA_TITLE));
		}
	}
	else if (_tcsicmp(szProperty, _T("SwitchTime")) == 0)
		m_nSwitchTime = _ttoi(szValue);
	else if (_tcsicmp(szProperty, _T("ShowTitle")) == 0)
		m_bShowTitle = IsTRUE(szValue);
	else
		return false;

	return true;
}

void CMediaInfoTextCtrl::EnumProperties(CUIObjProperties &listProperties)
{
	CSkinScrollText::EnumProperties(listProperties);

	CUIObjProperty	prop;
	int				i;

	prop.valueType = CUIObjProperty::VT_COMB_STR;
	prop.strName = _T("MediaInfoType");
	for (i = 0; i < m_vInfoType.size(); i++)
	{
		prop.strValue += IDToString(__mediaInfoType, m_vInfoType[i], _T("Title"));
		prop.strValue += _T("|");
	}
	if (!prop.strValue.empty())
		prop.strValue.resize(prop.strValue.size() - 1);

	for (i = 0; __mediaInfoType[i].szId != NULL; i++)
		prop.vOptions.push_back(__mediaInfoType[i].szId);
	listProperties.push_back(prop);

	listProperties.AddPropInt(_T("SwitchTime"), m_nSwitchTime, true);
	listProperties.AddPropBoolStr(_T("ShowTitle"), m_bShowTitle, !m_bShowTitle);
}

void CMediaInfoTextCtrl::UpdateShowTrackInfo()
{
	if (m_vInfoType.empty())
		return;

	if (m_nCurrentItem < 0 || m_nCurrentItem >= m_vInfoType.size())
		m_nCurrentItem = 0;
	MediaAttribute		infoType = m_vInfoType[m_nCurrentItem];

	m_strText = IDToString(__mediaInfoType, infoType, _T(""));
	m_strText += _T(": ");
}
