// MPPPCWorkArea.cpp: implementation of the CMPPPCWorkArea class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPPPCWorkArea.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CMPPPCWorkArea, _T("WorkArea"))

CMPPPCWorkArea::CMPPPCWorkArea()
{
}

CMPPPCWorkArea::~CMPPPCWorkArea()
{
}

BOOL CMPPPCWorkArea::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, _T("DivideImage")) == 0)
	{
		m_strDivideImgFile = szValue;
	}
	else if (_tcsicmp(szProperty, _T("DivideImageRect")) == 0)
	{
		if (!GetRectValue(szValue, m_imgDivide))
			ERRLOG2(_T("Analysis Value: %s = %s FAILED."), szProperty, szValue);
	}
	else if (_tcsicmp(szProperty, _T("DivideImageBltMode")) == 0)
	{
		m_strDivideImgBltMode = szValue;
	}
	else
	{
		for (int i = 0; i < m_vProperties.size(); i += 2)
		{
			if (_tcsicmp(m_vProperties[i].c_str(), szProperty) == 0)
			{
				m_vProperties[i + 1] = szValue;
				return TRUE;
			}
		}

		m_vProperties.push_back(szProperty);
		m_vProperties.push_back(szValue);
	}

	return TRUE;
}

void CMPPPCWorkArea::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	listProperties.AddPropImage(_T("DivideImage"), _T("DivideImageRect"), m_strDivideImgFile.c_str(), m_imgDivide, !m_strDivideImgFile.empty());
	listProperties.AddPropBltMode(_T("DivideImageBltMode"), BltModeFromStr(m_strDivideImgBltMode.c_str()));

	for (int i = 0; i < m_vProperties.size(); i += 2)
	{
		listProperties.AddPropStr(m_vProperties[i].c_str(), m_vProperties[i + 1].c_str());
	}
}