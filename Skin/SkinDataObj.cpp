#include "StdAfx.h"
#include "MLSkin.h"
#include "SkinDataObj.h"


UIOBJECT_CLASS_NAME_IMP(CSkinDataObj, _T("DataObj"))

CSkinDataObj::CSkinDataObj()
{
	m_bVisible = false;
}


CSkinDataObj::~CSkinDataObj()
{
}


BOOL CSkinDataObj::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (IsPropertyName(szProperty, SZ_PN_ID))
	{
		m_strID = szValue;
		m_nId = m_pSkin->GetSkinFactory()->GetIDByName(szValue);
	}
	else if (IsPropertyName(szProperty, SZ_PN_NAME))
		m_strName = szValue;
	else
		return FALSE;

	return TRUE;
}


void CSkinDataObj::EnumProperties(CUIObjProperties &listProperties)
{
	listProperties.AddPropStr(SZ_PN_NAME, m_strName.c_str(), !m_strName.empty());

	listProperties.AddPropID(SZ_PN_ID, m_strID.c_str(), !m_strID.empty());
}
