// SkinToolTip.cpp: implementation of the CSkinToolTip class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WidgetLite.h"
#include "SkinToolTip.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkinToolTip::CSkinToolTip()
{
	m_hWnd = NULL;
}

CSkinToolTip::~CSkinToolTip()
{

}

BOOL CSkinToolTip::Create(CBaseWnd *pWndParent)
{
	return (m_hWnd != NULL);
}

BOOL CSkinToolTip::AddTool(LPCTSTR szText, LPCRECT lpRectTool, UINT nIDTool)
{
	return TRUE;
}

void CSkinToolTip::DelTool(UINT nIDTool)
{
}

void CSkinToolTip::Destroy()
{
}

