#include "StdAfx.h"
#include "FontInfo.h"
#include "MLFont.h"

#ifdef _WINCE_
#ifndef OUT_TT_PRECIS
#define OUT_TT_PRECIS		OUT_DEFAULT_PRECIS
#endif
#ifndef FW_REGULAR
#define FW_REGULAR		400
#endif
#endif

#ifndef CLEARTYPE_QUALITY
#define CLEARTYPE_QUALITY	5
#endif

BYTE		g_byFontQuality = (GetOperationSystemType() >= OPS_WINXP) ? CLEARTYPE_QUALITY : ANTIALIASED_QUALITY;

CMLFont::CMLFont(void)
{
}

CMLFont::~CMLFont(void)
{
}

bool CMLFont::Create(LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers, int nHeight, int nWeight, int nItalic, bool bUnderline)
{
	CFontInfo::Set(szFaceNameLatin9, szFaceNameOthers, nHeight, nWeight, nItalic, bUnderline);

	return true;
}

bool CMLFont::Create(const CFontInfo &font)
{
	CFontInfo::Set(font);

	return true;
}

bool CMLFont::IsValid()
{
	return m_nHeight > 0 && m_nHeight < 200;
}
