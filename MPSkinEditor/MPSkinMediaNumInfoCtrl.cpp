// MPSkinNumInfoCtrl.cpp: implementation of the CMPSkinMediaNumInfoCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
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
}

void CMPSkinMediaNumInfoCtrl::OnCreate()
{
	CSkinPicText::OnCreate();

	UpdateShowTrackInfo();
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

		UpdateShowTrackInfo();
	}
	else
		return FALSE;

	return TRUE;
}

void CMPSkinMediaNumInfoCtrl::EnumProperties(CUIObjProperties &listProperties)
{
	CSkinPicText::EnumProperties(listProperties);

	CUIObjProperty		prop;

	prop.strName = _T("InfoType");
	if (m_infoType == IT_BITRATE)
		prop.strValue = _T("kbps");
	else
		prop.strValue = _T("samplerate");
	prop.valueType = CUIObjProperty::VT_COMB_STR;
	prop.vOptions.push_back(_T("kbps"));
	prop.vOptions.push_back(_T("samplerate"));
	listProperties.push_back(prop);
}

void CMPSkinMediaNumInfoCtrl::UpdateShowTrackInfo()
{
	if (m_infoType == IT_SAMPLERATE)
		SetText(_T("44100"));
	else
		SetText(_T("192"));
}
