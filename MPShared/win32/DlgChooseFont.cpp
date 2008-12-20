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

int CDlgChooseFont::DoModal(CBaseWnd *pWndParent, LPCTSTR szFontFaceName, int nFontHeight, int nWeight, int nItalic)
{
	LOGFONT lgfont = {25, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_ROMAN, _T("system")};
	CHOOSEFONT	choosefont;

	m_strFontFaceName = szFontFaceName;
	m_nFontHeight = nFontHeight;
	m_nWeight = nWeight;
	m_nItalic = nItalic;

	lgfont.lfHeight = nFontHeight;
	lgfont.lfWeight = nWeight;
	lgfont.lfItalic = nItalic;
	_tcscpy_safe(lgfont.lfFaceName, CountOf(lgfont.lfFaceName), szFontFaceName);

	memset(&choosefont, 0, sizeof(choosefont));
	choosefont.Flags = CF_INITTOLOGFONTSTRUCT | CF_TTONLY | CF_SCREENFONTS | CF_NOVERTFONTS;// | CF_EFFECTS;
	choosefont.lpLogFont = &lgfont;
	choosefont.hwndOwner = pWndParent->GetHandle();
	choosefont.lStructSize = sizeof(choosefont);
	if (ChooseFont(&choosefont))
	{
		m_strFontFaceName = lgfont.lfFaceName;
		m_nFontHeight = lgfont.lfHeight;
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

int CDlgChooseFont::GetHeight()
{
	return m_nFontHeight;
}

int CDlgChooseFont::GetWeight()
{
	return m_nWeight;
}

int CDlgChooseFont::GetItalic()
{
	return m_nItalic;
}
