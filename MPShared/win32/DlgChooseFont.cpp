// DlgChooseFont.cpp: implementation of the CDlgChooseFont class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "DlgChooseFont.h"

// #define FLAG_BOLD		1
// #define FLAG_ITALIC		(0x1 << 1)
// 
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDlgChooseFont::CDlgChooseFont()
{
}

CDlgChooseFont::~CDlgChooseFont()
{

}

int CDlgChooseFont::DoModal(CBaseWnd *pWndParent, LPCTSTR szFontFaceName, int nFontSize, int nWeight, int nItalic)
{
	LOGFONT lgfont = { 0 };
	CHOOSEFONT	choosefont = { 0 };

	m_strFontFaceName = szFontFaceName;
	m_nFontSize = nFontSize;
	m_nWeight = nWeight;
	m_nItalic = nItalic;

	lgfont.lfSize = nFontSize;
	lgfont.lfWeight = nWeight;
	lgfont.lfItalic = nItalic;
	_tcscpy_safe(lgfont.lfFaceName, CountOf(lgfont.lfFaceName), szFontFaceName);

	choosefont.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCALABLEONLY | CF_SCREENFONTS | CF_NOVERTFONTS;// | CF_EFFECTS;
	choosefont.lpLogFont = &lgfont;
	choosefont.hwndOwner = pWndParent->GetHandle();
	choosefont.lStructSize = sizeof(choosefont);
	if (ChooseFont(&choosefont))
	{
		m_strFontFaceName = lgfont.lfFaceName;
		m_nFontSize = lgfont.lfSize;
		m_nWeight = lgfont.lfWeight;
		m_nItalic = lgfont.lfItalic;

		return IDOK;
	}

	return IDCANCEL;
}

LPCTSTR CDlgChooseFont::GetFaceName()
{
	return m_strFontFaceName.c_str();
}

int CDlgChooseFont::GetSize()
{
	return m_nFontSize;
}

int CDlgChooseFont::GetWeight()
{
	return m_nWeight;
}

int CDlgChooseFont::GetItalic()
{
	return m_nItalic;
}
