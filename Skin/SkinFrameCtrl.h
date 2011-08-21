
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

	int GetBorderThick() { return m_nThickWidth; }

	void SetFocusIndicator(CUIObject *pObjFocusIndicator) { m_pObjFocusIndicator = pObjFocusIndicator; }

protected:
	CSFImage			m_image, m_imageFocus;
	BlendPixMode		m_bpm;
	int16				m_nRoundWidthTop, m_nRoundWidthBottom;
	int16				m_nThickWidth;
	int16				m_xBorder, m_yBorder;
	CSkinFontProperty	m_font;
	CUIObject			*m_pObjFocusIndicator;

};
