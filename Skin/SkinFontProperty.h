// SkinFontProperty.h: interface for the CSkinFontProperty class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _CSKINFONTPROPERTY_INC_
#define _CSKINFONTPROPERTY_INC_

#pragma once

class CUIObjProperties;

// szValue format: Verdana, 13, thin, 1, 0, Tahoma
// Latin font, height, weight(bold,normal,thin), italic, underline, Other font
bool FontPropertyValueFromStr(LPCTSTR szValue, int &nHeight, int &nWeight, BYTE &byItalic, tstring &strFaceNameLatin9, tstring &strFaceNameOthers);
void FontPropertyValueToStr(tstring &strValue, int nHeight, int nWeight, BYTE byItalic, LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers);


//
// This class is used to manage font properties of skin window and skin controls.
//
// Skin window has font property, every skin control(UIObject) inherits font property from it,
// but every control can use customized font name, size or weight etc, 
// And user may modify default font properties of skin winodw, 
// and the child controls should be affected, if it use default values.
//
class CSkinFontProperty
{
public:
	enum FontProperty
	{
		FP_NAME_LATIN9		= 0x1,
		FP_NAME_OTHERS		= 0x1 << 1,
		FP_HEIGHT			= 0x1 << 2,
		FP_WEIGHT			= 0x1 << 3,
		FP_ITALIC			= 0x1 << 4,
		FP_UNDERLINE		= 0x1 << 5,
		FP_OUTLINED			= 0x1 << 6,
		FP_TEXT_CLR			= 0x1 << 7,
		FP_OUTLINED_CLR		= 0x1 << 8,

		FP_FONT = FP_NAME_LATIN9 | FP_NAME_OTHERS | FP_HEIGHT | FP_WEIGHT | FP_ITALIC | FP_UNDERLINE
	};

	CSkinFontProperty();
	virtual ~CSkinFontProperty();

	void Create(LPCTSTR szFaceNameLatin9, LPCTSTR szFaceNameOthers, int nHeight, int nWeight, int nItalic, bool bUnderline);

	void Create(const CMLFont &font);

	void Clear();

	bool SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
	void EnumProperties(CUIObjProperties &listProperties);

	void EnableGdiplusAntialias(bool bGdiplusAntialias);

	LPCTSTR GetName();
	LPCTSTR GetNameOthers();
	int GetHeight();
	int GetWeight();
	bool IsOutlined();

	const MLColor &GetTextColor();
	const MLColor &GetColorOutlined();

	bool IsGood() { return m_pSkin != NULL || (m_bCustomized && IsFlagSet(m_nFlagProperties, FP_FONT)); }
	CRawBmpFont *GetFont();

	void UpdateMLFontInfo();

	CMLFont & GetMLFont() { return m_font; }

	void OnCreate(CSkinWnd *pSkin);

	void OnSkinFontChanged();

	void OnAdjustHue(float hue, float saturation, float luminance);

protected:
	inline bool IsPropertySet(uint32 fontProperty) { return IsFlagSet(m_nFlagProperties, fontProperty); }

protected:
	CSkinWnd			*m_pSkin;
	CRawBmpFont		*m_pRawFont;
	CMLFont			m_font;

	bool			m_bCustomized;			// This is used for skin editor to disable customized font
	uint32			m_nFlagProperties;

	bool			m_bOutlineText;
	MLColor			m_clrText, m_clrOutlined;
	MLColor			m_clrTextOrg, m_clrOutlinedOrg;

};


#endif // !_CSKINFONTPROPERTY_INC_
