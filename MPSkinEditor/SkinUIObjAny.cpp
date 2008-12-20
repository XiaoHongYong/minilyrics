// SkinUIObjAny.cpp: implementation of the CSkinUIObjAny class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SkinUIObjAny.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinUIObjAny, _T("UIObjAny"))

CSkinUIObjAny::CSkinUIObjAny()
{
	m_nMsgWant = UO_MSG_WANT_ALL;
}

CSkinUIObjAny::~CSkinUIObjAny()
{

}

void CSkinUIObjAny::Draw(CRawGraph *pGraph)
{
}

BOOL CSkinUIObjAny::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

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

	return TRUE;
}

void CSkinUIObjAny::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObjProperty		property;

	CUIObject::EnumProperties(listProperties);

	for (int i = 0; i < m_vProperties.size(); i += 2)
	{
		property.strName = m_vProperties[i].c_str();
		property.strValue = m_vProperties[i + 1].c_str();;
		property.valueType = CUIObjProperty::VT_STR;
		listProperties.push_back(property);
	}
}
