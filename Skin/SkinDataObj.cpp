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
		m_nId = m_pSkin->GetSkinFactory()->GetIDByName(szValue);
	}
	else if (IsPropertyName(szProperty, SZ_PN_NAME))
		m_strName = szValue;
	else
		return FALSE;

	return TRUE;
}


#ifdef _SKIN_EDITOR_
void CSkinDataObj::EnumProperties(CUIObjProperties &listProperties)
{
	listProperties.AddPropStr(SZ_PN_NAME, m_strName.c_str(), !m_strName.empty());

	listProperties.AddPropID(SZ_PN_ID, m_pSkin->GetSkinFactory()->GetStringOfID(m_nId).c_str(), m_nId != SKIN_UIOBJ_ID_UNDEFINE);
}
#endif // _SKIN_EDITOR_
