
#pragma once

#include "UIObject.h"

class CSkinFrameCtrl : public CUIObject  
{
UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	CSkinFrameCtrl();
	virtual ~CSkinFrameCtrl();

	void OnCreate();

	void OnAdjustHue(float hue, float saturation, float luminance);

	void Draw(CRawGraph *pGraph);
	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

protected:
	CSFImage			m_image;
	int					m_nRoundWidth;
	int					m_nThickWidth;
	int16				m_xBorder, m_yBorder;
	CSkinFontProperty	m_font;

};
