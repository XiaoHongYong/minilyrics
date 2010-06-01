#include "StdAfx.h"
#include "MLFont.h"

CMLFont::CMLFont()
{
	m_pFont = NULL;
}

CMLFont::~CMLFont(void)
{
	Destroy();
}

bool CMLFont::Create(LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers, int nHeight, int nWeight, int nItalic, bool bUnderline)
{
	tstring	strFontName;

	CFontInfo::Set(szFaceNameLatin9, szFaceNameOthers, nHeight, nWeight, nItalic, bUnderline);

	ERRLOG0("Unimplemented fun CMLGraphics::");

	return m_pFont != NULL;
}

bool CMLFont::Create(const CFontInfo &font)
{
	return Create(font.GetName(), font.GetNameOthers(), font.GetHeight(), font.GetWeight(), font.GetItalic(), font.IsUnderline());
}

void CMLFont::Destroy()
{
}
