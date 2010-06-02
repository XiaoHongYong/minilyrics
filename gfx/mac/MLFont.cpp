#include "StdAfx.h"
#include "FontInfo.h"
#include "MLFont.h"

CMLFont::CMLFont()
{
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

	return true;
}

bool CMLFont::Create(const CFontInfo &font)
{
	return Create(font.GetName(), font.GetNameOthers(), font.GetHeight(), font.GetWeight(), font.GetItalic(), font.IsUnderline());
}

void CMLFont::Destroy()
{
}
