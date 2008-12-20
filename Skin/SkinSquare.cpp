// SkinSquare.cpp: implementation of the CSkinSquare class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MLSkin.h"
#include "SkinSquare.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinSquare, _T("Square"))

CSkinSquare::CSkinSquare()
{
	m_clrBg.set(0);
	m_brBg.CreateSolidBrush(m_clrBg);
}

CSkinSquare::~CSkinSquare()
{

}

void CSkinSquare::Draw(CRawGraph *pGraph)
{
	pGraph->FillRect(m_rcObj.left, m_rcObj.top, m_rcObj.Width(), m_rcObj.Height(), &m_brBg);
}


BOOL CSkinSquare::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (IsPropertyName(szProperty, _T("BgColor")))
	{
		GetColorValue(m_clrBg, szValue);
		m_brBg.Destroy();
		m_brBg.CreateSolidBrush(m_clrBg);
	}

	return TRUE;
}

void CSkinSquare::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	listProperties.AddPropColor(_T("BgColor"), m_clrBg, true);
}
