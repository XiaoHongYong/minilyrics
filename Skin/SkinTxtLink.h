
#pragma once

class CSkinTxtLink : public CUIObject  
{
	UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	CSkinTxtLink();
	virtual ~CSkinTxtLink();

	void Draw(CRawGraph *pGraph);
	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

	void OnCreate();

	void OnSize();

	bool OnLButtonUp(UINT nFlags, CPoint point);

	bool OnLButtonDown(UINT nFlags, CPoint point);

	bool OnMouseMove(UINT nFlags, CPoint point);

protected:
	tstring				m_strLink;
	DWORD				m_dwAlignText;
	CSkinFontProperty	m_font;
	CCursor				m_Cursor;
	bool				m_bNeedResize;

};
