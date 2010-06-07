#include "StdAfx.h"
#include "FontInfo.h"
#include "MLFont.h"

CMLFont::CMLFont()
{
	m_font = NULL;
}

CMLFont::~CMLFont(void)
{
	Destroy();
}

bool CMLFont::Create(LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers, int nHeight, int nWeight, int nItalic, bool bUnderline)
{
	tstring	strFontName;

	CFontInfo::Set(szFaceNameLatin9, szFaceNameOthers, nHeight, nWeight, nItalic, bUnderline);

	//if (szFaceNameLatin9 == NULL || IsEmptyString(szFaceNameLatin9))
	szFaceNameLatin9 = "Helvetica";
	CFStringRef	name = CFStringCreateWithCString(NULL, szFaceNameLatin9, kCFStringEncodingUTF8);
	m_font = CGFontCreateWithFontName(name);
	CFRelease(name);

	return true;
}

bool CMLFont::Create(const CFontInfo &font)
{
	return Create(font.GetName(), font.GetNameOthers(), font.GetHeight(), font.GetWeight(), font.GetItalic(), font.IsUnderline());
}

void CMLFont::Destroy()
{
	if (m_font != NULL)
	{
		CGFontRelease(m_font);
		m_font = NULL;
	}
}
