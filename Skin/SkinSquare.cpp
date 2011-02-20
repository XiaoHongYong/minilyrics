// SkinSquare.cpp: implementation of the CSkinSquare class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Skin.h"
#include "SkinSquare.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinSquare, _T("Square"))

CSkinSquare::CSkinSquare()
{
	m_clrBg.set(0);
}

CSkinSquare::~CSkinSquare()
{

}

void CSkinSquare::Draw(CRawGraph *pGraph)
{
	pGraph->FillRect(&m_rcObj, m_clrBg);
}


BOOL CSkinSquare::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (IsPropertyName(szProperty, _T("BgColor")))
	{
		GetColorValue(m_clrBg, szValue);
	}

	return TRUE;
}

#ifdef _SKIN_EDITOR_
void CSkinSquare::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	listProperties.AddPropColor(_T("BgColor"), m_clrBg, true);
}
#endif // _SKIN_EDITOR_
