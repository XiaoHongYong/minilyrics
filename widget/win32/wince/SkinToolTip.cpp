// SkinToolTip.cpp: implementation of the CSkinToolTip class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SkinToolTip.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkinToolTip::CSkinToolTip()
{
// 	m_hWnd = NULL;
// 	m_hWndParent = NULL;
}

CSkinToolTip::~CSkinToolTip()
{

}

BOOL CSkinToolTip::Create(CBaseWnd *pWndParent)
{
	return true;
}

BOOL CSkinToolTip::AddTool(LPCTSTR szText, LPCRECT lpRectTool, UINT nIDTool)
{
	return true;
}

void CSkinToolTip::DelTool(UINT nIDTool)
{
}

/*
void CSkinToolTip::RelayEvent(LPMSG lpMsg)
{
}
*/

bool CSkinToolTip::IsValid()
{
	return true;
}

bool CSkinToolTip::IsWindow()
{
	return true;
}

void CSkinToolTip::Destroy()
{
}
