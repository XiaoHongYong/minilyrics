#include "StdAfx.h"
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
	m_nHeight = 0;
	m_nWeight = FW_NORMAL;
	m_nItalic = 0;
	m_bUnderline = false;
}

CMLFont::~CMLFont(void)
{
}

bool CMLFont::Create(LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers, int nHeight, int nWeight, int nItalic, bool bUnderline)
{
	ASSERT(g_byFontQuality == CLEARTYPE_QUALITY);
	m_strNameLatin9 = szFaceNameLatin9;
	if (!szFaceNameOthers || IsEmptyString(szFaceNameOthers))
		m_strNameOthers = szFaceNameLatin9;
	else
		m_strNameOthers = szFaceNameOthers;
	m_nHeight = nHeight;
	m_nWeight = nWeight;
	m_nItalic = nItalic;
	m_bUnderline = bUnderline;

	return true;
}

bool CMLFont::CreateFrom(const CMLFont &font)
{
	m_strNameLatin9 = font.GetName();
	if (IsEmptyString(font.GetNameOthers()))
		m_strNameOthers = font.GetName();
	else
		m_strNameOthers = font.GetNameOthers();
	m_nHeight = font.GetHeight();
	m_nWeight = font.GetWeight();
	m_nItalic = font.GetItalic();
	m_bUnderline = font.IsUnderline();

	return true;
}

bool CMLFont::IsValid()
{
	return m_nHeight > 0 && m_nHeight < 200;
}
