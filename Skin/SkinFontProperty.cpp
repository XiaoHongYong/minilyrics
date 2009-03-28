// SkinFontProperty.cpp: implementation of the CSkinFontProperty class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MLSkin.h"
#include "SkinFontProperty.h"



// szValue format: Verdana, 13, thin, 1, 0, Tahoma
// Latin font, height, weight(bold,normal,thin), italic, underline, Other font
bool FontPropertyValueFromStr(LPCTSTR szValue, int &nHeight, int &nWeight, BYTE &byItalic, tstring &strFaceNameLatin9, tstring &strFaceNameOthers)
{
	int				nUnderLine;
	tstring			strWeight;
	CCSVLineT		csvLine;

	csvLine.SetReadingLine(szValue);

	// latin font name
	csvLine.IgnoreChars(_T(""));
	csvLine.ReadFieldString(strFaceNameLatin9);
	TrimStr(strFaceNameLatin9);

	// height
	csvLine.IgnoreChars(_T(" "));
	csvLine.ReadFieldInt(nHeight);

	// font weight?
	csvLine.IgnoreChars(_T(" "));
	csvLine.ReadFieldString(strWeight);
	if (_tcsicmp(strWeight.c_str(), _T("bold")) == 0)
		nWeight = FW_BOLD;
	else if (_tcsicmp(strWeight.c_str(), _T("thin")) == 0)
		nWeight = FW_THIN;
	else
		nWeight = FW_NORMAL;

	// italic
	csvLine.IgnoreChars(_T(" "));
	csvLine.ReadFieldInt(byItalic);

	// underline
	csvLine.IgnoreChars(_T(" "));
	csvLine.ReadFieldInt(nUnderLine);

	// other character set font name.
	csvLine.IgnoreChars(_T(" "));
	csvLine.ReadFieldString(strFaceNameOthers);
	TrimStr(strFaceNameOthers);

	return true;
}

void FontPropertyValueToStr(tstring &strValue, int nHeight, int nWeight, BYTE byItalic, LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers)
{
#define			SZ_FEILD_SEP	_T(", ")
	TCHAR			szBuff[64];

	strValue.resize(0);
	strValue += szFaceNameLatin9;
	strValue += SZ_FEILD_SEP;

	_itot_s(nHeight, szBuff, CountOf(szBuff), 10);
	strValue += szBuff; strValue += SZ_FEILD_SEP;

	if (nWeight >= FW_BOLD)
		strValue += _T("bold, ");
	else if (nWeight <= FW_THIN)
		strValue += _T("thin, ");
	else
		strValue += _T("normal, ");

	_itot_s(byItalic, szBuff, CountOf(szBuff), 10);
	strValue += szBuff; strValue += SZ_FEILD_SEP;

	strValue += _T("0, ");

	strValue += szFaceNameOthers;
}


//////////////////////////////////////////////////////////////////////////

CSkinFontProperty::CSkinFontProperty()
{
	m_pRawFont = NULL;
	m_pSkin = NULL;
	m_nFlagProperties = 0;
	m_bCustomized = true;

	m_bOutlineText = false;
	m_clrText.set(RGB(0, 0, 0));
	m_clrTextOrg.set(RGB(0, 0, 0));
	m_clrOutlined.set(RGB(192, 192, 192));
	m_clrOutlinedOrg.set(RGB(192, 192, 192));
}


CSkinFontProperty::~CSkinFontProperty()
{
	if (m_pRawFont)
		delete m_pRawFont;
}


void CSkinFontProperty::Create(LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers, int nHeight, int nWeight, int nItalic, bool bUnderline)
{
	Clear();

	m_font.Create(szFaceNameLatin9, szFaceNameOthers, nHeight, nWeight, nItalic, bUnderline);
	m_nFlagProperties |= FP_FONT;
	m_bCustomized = true;
}


void CSkinFontProperty::Create(const CMLFont &font)
{
	Clear();

	m_font.CreateFrom(font);
	m_nFlagProperties |= FP_FONT;
	m_bCustomized = true;
}


void CSkinFontProperty::Clear()
{
	if (m_pRawFont)
	{
		delete m_pRawFont;
		m_pRawFont = NULL;
	}

	m_nFlagProperties = 0;
	m_bCustomized = true;
	m_font.m_strNameLatin9.resize(0);
	m_font.m_strNameOthers.resize(0);
}


bool CSkinFontProperty::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (IsPropertyName(szProperty, _T("CustomizedFont")))
		m_bCustomized = IsTRUE(szValue);
	else if (IsPropertyName(szProperty, _T("Font")))
	{
		// Set font property in one field: Verdana, 13, thin, 1, 0, Tahoma
		FontPropertyValueFromStr(szValue, m_font.m_nHeight, m_font.m_nWeight, m_font.m_nItalic, m_font.m_strNameLatin9, m_font.m_strNameOthers);
		m_nFlagProperties |= FP_FONT;
	}
	else if (IsPropertyName(szProperty, _T("FontName")))
	{
		m_font.m_strNameLatin9 = szValue;
		if (!m_font.m_strNameLatin9.empty())
			m_nFlagProperties |= FP_NAME_LATIN9;
		else
			m_nFlagProperties &= ~FP_NAME_LATIN9;
	}
	else if (IsPropertyName(szProperty, _T("FontNameOthers")))
	{
		m_font.m_strNameOthers = szValue;
		if (!m_font.m_strNameOthers.empty())
			m_nFlagProperties |= FP_NAME_OTHERS;
		else
			m_nFlagProperties &= ~FP_NAME_OTHERS;
	}
	else if (IsPropertyName(szProperty, _T("FontHeight")))
	{
		m_font.m_nHeight = _ttoi(szValue);
		if (m_font.m_nHeight >= 8 && m_font.m_nHeight < 255)
			m_nFlagProperties |= FP_HEIGHT;
		else
			m_nFlagProperties &= ~FP_HEIGHT;
	}
	else if (IsPropertyName(szProperty, _T("FontBold")))
	{
		m_font.m_nWeight = IsTRUE(szValue) ? FW_BOLD : FW_NORMAL;
		m_nFlagProperties |= FP_WEIGHT;
	}
	else if (IsPropertyName(szProperty, _T("FontWeight")))
	{
		if (IsPropertyName(szValue, _T("bold")) == 0)
			m_font.m_nWeight = FW_BOLD;
		else if (IsPropertyName(szValue, _T("thin")) == 0)
			m_font.m_nWeight = FW_THIN;
		else
			m_font.m_nWeight = FW_NORMAL;
		m_nFlagProperties |= FP_WEIGHT;
	}
	else if (IsPropertyName(szProperty, _T("FontItalic")))
	{
		m_font.m_nItalic = _ttoi(szValue);
		m_nFlagProperties |= FP_ITALIC;
	}
	else if (IsPropertyName(szProperty, _T("FontUnderLine")))
	{
		m_font.m_nItalic = _ttoi(szValue);
		m_nFlagProperties |= FP_UNDERLINE;
	}
	else if (IsPropertyName(szProperty, _T("TextOutlined")))
	{
		m_bOutlineText = IsTRUE(szValue);
		m_nFlagProperties |= FP_OUTLINED;
	}
	else if (_tcsicmp(szProperty, _T("TextColor")) == 0)
	{
		GetColorValue(m_clrTextOrg, szValue);
		m_clrText = m_clrTextOrg;
		m_nFlagProperties |= FP_TEXT_CLR;
	}
	else if (_tcsicmp(szProperty, _T("TextOutlinedColor")) == 0)
	{
		GetColorValue(m_clrOutlinedOrg, szValue);
		m_clrOutlined = m_clrOutlinedOrg;
		m_nFlagProperties |= FP_OUTLINED_CLR;
	}
	else
		return false;

	return true;
}


void CSkinFontProperty::EnumProperties(CUIObjProperties &listProperties)
{
	listProperties.AddPropBoolStr(_T("CustomizedFont"), m_bCustomized, !m_bCustomized);
	listProperties.AddPropFontName(_T("FontName"), m_font.GetName(), IsFlagSet(m_nFlagProperties, FP_NAME_LATIN9));
	listProperties.AddPropFontName(_T("FontNameOthers"), m_font.GetNameOthers(), IsFlagSet(m_nFlagProperties, FP_NAME_OTHERS));
	listProperties.AddPropInt(_T("FontHeight"), m_font.GetHeight(), IsFlagSet(m_nFlagProperties, FP_HEIGHT));
	listProperties.AddPropInt(_T("FontWeight"), m_font.GetWeight(), IsFlagSet(m_nFlagProperties, FP_WEIGHT));
	listProperties.AddPropInt(_T("FontItalic"), m_font.GetItalic(), IsFlagSet(m_nFlagProperties, FP_ITALIC));
	listProperties.AddPropInt(_T("FontUnderLine"), m_font.IsUnderline(), IsFlagSet(m_nFlagProperties, FP_UNDERLINE));
}


void CSkinFontProperty::EnableGdiplusAntialias(bool bGdiplusAntialias)
{
	if (m_pRawFont)
		m_pRawFont->EnableGdiplusAntialias(bGdiplusAntialias);
}


LPCTSTR CSkinFontProperty::GetName()
{
	if (IsPropertySet(FP_NAME_LATIN9))
		return m_font.GetName();
	else if (m_pSkin)
		return m_pSkin->GetFontProperty()->m_font.GetName();
	else
		return _T("");
}


LPCTSTR CSkinFontProperty::GetNameOthers()
{
	if (IsPropertySet(FP_NAME_OTHERS))
		return m_font.GetNameOthers();
	else if (m_pSkin)
		return m_pSkin->GetFontProperty()->m_font.GetNameOthers();
	else
		return _T("");
}


int CSkinFontProperty::GetHeight()
{
	if (IsPropertySet(FP_HEIGHT))
		return m_font.GetHeight();
	else if (m_pSkin)
		return m_pSkin->GetFontProperty()->m_font.GetHeight();
	else
		return 13;
}


int CSkinFontProperty::GetWeight()
{
	if (IsPropertySet(FP_WEIGHT))
		return m_font.GetWeight();
	else if (m_pSkin)
		return m_pSkin->GetFontProperty()->m_font.GetWeight();
	else
		return FW_NORMAL;
}


bool CSkinFontProperty::IsOutlined()
{
	if (IsPropertySet(FP_OUTLINED))
		return m_bOutlineText;
	else if (m_pSkin)
		return m_pSkin->GetFontProperty()->IsOutlined();
	else
		return false;
}


const MLColor &CSkinFontProperty::GetTextColor()
{
	if (IsPropertySet(FP_TEXT_CLR))
		return m_clrText;
	else if (m_pSkin)
		return m_pSkin->GetFontProperty()->GetTextColor();
	else
		return m_clrText;
}


const MLColor &CSkinFontProperty::GetColorOutlined()
{
	if (IsPropertySet(FP_OUTLINED_CLR))
		return m_clrOutlined;
	else if (m_pSkin)
		return m_pSkin->GetFontProperty()->GetColorOutlined();
	else
		return m_clrOutlined;
}


void CSkinFontProperty::OnCreate(CSkinWnd *pSkin)
{
	m_pSkin = pSkin;

	if (m_pSkin)
	{
		m_pSkin->GetSkinFactory()->GetAdjustedHueResult(m_clrText);
		m_pSkin->GetSkinFactory()->GetAdjustedHueResult(m_clrOutlined);
	}
}


void CSkinFontProperty::OnSkinFontChanged()
{
	ASSERT(m_pSkin);

	if ((m_nFlagProperties & FP_FONT) == 0 || !m_bCustomized || !m_pSkin)
		return;

	UpdateMLFontInfo();

	if (!m_pRawFont)
		m_pRawFont = new CRawBmpFont;
	ASSERT(m_pRawFont);

	if (!m_pRawFont->CreateFrom(m_font))
	{
		delete m_pRawFont;
		m_pRawFont = NULL;
	}
}


void CSkinFontProperty::OnAdjustHue(float hue, float saturation, float luminance)
{
	ASSERT(m_pSkin);
	if (!m_pSkin)
		return;

	m_clrText = m_clrTextOrg;
	m_pSkin->GetSkinFactory()->GetAdjustedHueResult(m_clrText);

	m_clrOutlined = m_clrOutlinedOrg;
	m_pSkin->GetSkinFactory()->GetAdjustedHueResult(m_clrOutlined);
}


void CSkinFontProperty::UpdateMLFontInfo()
{
	ASSERT(m_pSkin);
	if (!m_pSkin)
		return;

	CSkinFontProperty		*pFont;

	pFont = m_pSkin->GetFontProperty();

	if (!IsFlagSet(m_nFlagProperties, FP_NAME_LATIN9))
		m_font.m_strNameLatin9 = pFont->m_font.m_strNameLatin9;

	if (!IsFlagSet(m_nFlagProperties, FP_NAME_OTHERS))
		m_font.m_strNameOthers = pFont->m_font.m_strNameOthers;

	if (!IsFlagSet(m_nFlagProperties, FP_HEIGHT))
		m_font.m_nHeight = pFont->m_font.m_nHeight;

	if (!IsFlagSet(m_nFlagProperties, FP_WEIGHT))
		m_font.m_nWeight = pFont->m_font.m_nWeight;

	if (!IsFlagSet(m_nFlagProperties, FP_ITALIC))
		m_font.m_nItalic = pFont->m_font.m_nItalic;

	if (!IsFlagSet(m_nFlagProperties, FP_UNDERLINE))
		m_font.m_bUnderline = pFont->m_font.m_bUnderline;
}


CRawBmpFont *CSkinFontProperty::GetFont()
{
	if (m_bCustomized && (m_nFlagProperties & FP_FONT) != 0)
	{
		if (!m_pRawFont && m_pSkin)
			OnSkinFontChanged();

		if (!m_pRawFont)
		{
			m_pRawFont = new CRawBmpFont;
			if (!m_pRawFont)
				return NULL;

			m_pRawFont->CreateFrom(m_font);
		}

		return m_pRawFont;
	}

	if (m_pSkin)
		return m_pSkin->GetRawFont();
	else
		return NULL;
}

