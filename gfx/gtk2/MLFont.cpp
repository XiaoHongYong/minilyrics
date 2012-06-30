#include "StdAfx.h"
#include "FontInfo.h"
#include "MLFont.h"

///////////////////////////////////////////////////////////////////////
//
//Foundry			the company or organization where the font originated. 
//Family			the font family (a group of related font designs). 
//Weight			A name for the font's typographic weight For example, 'bold' or 'medium'). 
//Slant				The slant of the font. Common values are 'R' for Roman, 'I' for italoc, and 'O' for oblique. 
//Set Width			A name for the width of the font. For example, 'normal' or 'condensed'. 
//Add Style			Additional information to distinguish a font from other fonts of the same family. 
//Pixel Size		The body size of the font in pixels. 
//Point Size		The body size of the font in 10ths of a point. (A point is 1/72.27 inch)  
//Resolution X		The horizontal resolution that the font was designed for. 
//Resolution Y		The vertical resolution that the font was designed for . 
//Spacing			The type of spacing for the font - can be 'p' for proportional, 'm' for monospaced or 'c' for charcell. 
//Average Width		The average width of a glyph in the font. For monospaced and charcell fonts, all glyphs in the font have this width 
//Charset Registry	The registration authority that owns the encoding for the font. Together with the Charset Encoding field, this defines the character set for the font. 
//Charset Encoding	An identifier for the particular character set encoding. 

// sprintf(font_name, "-*-%s-bold-r-*--%d-*-*-*-p-*-*-*", szFaceNameLatin9, nHeight);
void FormatFontName(tstring &strFontName, LPCTSTR szFaceName, int nHeight, int nWeight, int nItalic, bool bUnderline)
{
	char	szTemp[64];

	strFontName = "-*-";
	if (IsEmptyString(szFaceName))
		strFontName += "*";
	else
		strFontName += szFaceName;
	strFontName += "-";

	if (nWeight >= FW_BOLD)
		strFontName += "bold";
	else if (nWeight >= FW_SEMIBOLD)
		strFontName += "semi-bold";
	else
		strFontName += "*";
	strFontName += "-";

	if (nItalic == FS_OBLIQUE)
		strFontName += "o";
	else if (nItalic == FS_ITALIC)
		strFontName += "i";
	else
		strFontName += "*";
	strFontName += "-*--";

	strFontName += itoa(nHeight, szTemp, 10);
	strFontName += "-*-*-*-p-*-*-*";
}

bool ScanFontName(LPCTSTR szFontName, tstring &strFaceName, int &nHeight, int &nWeight, int &nItalic, bool &bUnderline)
{
	// "-*-%s-bold-r-normal--%d-*-*-*-p-*-*-*", szFaceNameLatin9, nHeight);
	//  face name	: 2
	//	weight		: 3
	//	italic		: 4
	//	height		: 7
	V_Str		vStr;

	bUnderline = false;

	if (StrBreak(szFontName, '-', vStr) < 14)
		return false;

	strFaceName = vStr[2].c_str();
	if (_tcsicmp(vStr[3].c_str(), "bold") == 0)
		nWeight = FW_BOLD;
	else if (_tcsicmp(vStr[3].c_str(), "semi-bold") == 0)
		nWeight = FW_SEMIBOLD;
	else
		nWeight = FW_NORMAL;

	if (_tcsicmp(vStr[3].c_str(), "i") == 0)
		nItalic = FS_ITALIC;
	else  if (_tcsicmp(vStr[3].c_str(), "o") == 0)
		nItalic = FS_OBLIQUE;
	else
		nItalic = FS_NORMAL;

	nHeight = atoi(vStr[7].c_str());

	return true;
}

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

	FormatFontName(strFontName, szFaceNameLatin9, nHeight, nWeight, nItalic, bUnderline);

	//             Foundry-family-weight--
	// gchar	font_name[256];
	// sprintf(font_name, "-*-%s-bold-r-normal--%d-*-*-*-p-*-*-*", szFaceNameLatin9, nHeight);
	//////////////////////"-*-%s-Bold-R-Normal--%d-*-*-*-*-*-*-*");
	//							 slat      Pixel-Size
	MLDebug("Create Font: %s\n", strFontName.c_str());
	m_pFont = gdk_font_load(strFontName.c_str());
	if (!m_pFont)
	{
		strFontName.resize(0);
		FormatFontName(strFontName, "*", nHeight, nWeight, nItalic, bUnderline);
		MLDebug("Try to create font again: %s\n", strFontName.c_str());
		m_pFont = gdk_font_load(strFontName.c_str());
	}
	assert(m_pFont);
	// MLDebug("Font Height: %d\n", m_nHeight);

	return m_pFont != NULL;
}

bool CMLFont::Create(const CFontInfo &font)
{
	return Create(font.GetName(), font.GetNameOthers(), font.GetSize(), font.GetWeight(), font.GetItalic(), font.IsUnderline());
}

void CMLFont::Destroy()
{
	if (m_pFont)
	{
		gdk_font_unref(m_pFont);
		m_pFont = NULL;
	}
}
