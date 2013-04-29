/********************************************************************
	Created  :	2002/01/04	21:42
	FileName :	SkinStaticText.h
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#if !defined(AFX_SKINSTATICTEXT_H__7ED53727_726A_11D5_9E04_02608CAD9330__INCLUDED_)
#define AFX_SKINSTATICTEXT_H__7ED53727_726A_11D5_9E04_02608CAD9330__INCLUDED_


class CSkinStaticText : public CUIObject  
{
	UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	CSkinStaticText();
	virtual ~CSkinStaticText();

	void Draw(CRawGraph *pGraph);

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

	void OnCreate();

	void OnMeasureSizeByContent();

	void OnAdjustHue(float hue, float saturation, float luminance);

	bool OnLButtonUp(UINT nFlags, CPoint point);

protected:
	DWORD			m_dwAlignText;
	bool			m_bCaptionText;

	int				m_nLeftMargin;

	CSkinFontProperty	m_font;

};

#endif // !defined(AFX_SKINSTATICTEXT_H__7ED53727_726A_11D5_9E04_02608CAD9330__INCLUDED_)
