#include "StdAfx.h"
#include "Skin.h"
#include "SkinWndResizeObj.h"

UIOBJECT_CLASS_NAME_IMP(CSkinWndResizeObj, _T("WndResizer"));

CSkinWndResizeObj::CSkinWndResizeObj(void)
{
	m_nResizeDirection = 0;
}

CSkinWndResizeObj::~CSkinWndResizeObj(void)
{
	m_pSkin->m_wndResizer.RemoveResizeArea(m_nAutoUniID);
}

ID_TO_STRING	__idszResizeDirection[] = 
{
	{ CWndResizer::RD_LEFT, _T("left") },
	{ CWndResizer::RD_TOP, _T("top") },
	{ CWndResizer::RD_RIGHT, _T("right") },
	{ CWndResizer::RD_BOTTOM, _T("bottom") },
	{ 0, NULL }
};

BOOL CSkinWndResizeObj::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return true;

	if (IsPropertyName(szProperty, _T("Direction")))
	{
		m_nResizeDirection = GetCombinationValue(__idszResizeDirection, szValue);
	}

	return false;
}

#ifdef _SKIN_EDITOR_
void CSkinWndResizeObj::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);


	tstring		str;

	GetCombinationStrValue(__idszResizeDirection, m_nResizeDirection, str);
	listProperties.AddPropStr(_T("Direction"), str.c_str());
}
#endif // _SKIN_EDITOR_

void CSkinWndResizeObj::OnSize(int cx, int cy)
{
	CUIObject::OnSize(cx, cy);

	m_pSkin->m_wndResizer.SetResizeArea(m_nAutoUniID, m_nResizeDirection, m_rcObj);
}
