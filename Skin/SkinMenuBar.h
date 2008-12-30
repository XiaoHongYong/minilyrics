// SkinMenuBar.h: interface for the CSkinMenuBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_SKIN_MENU_BAR_H_)
#define _SKIN_MENU_BAR_H_

#include "UIObject.h"

class CSkinMenuBar : public CUIObject  
{
UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	CSkinMenuBar();
	virtual ~CSkinMenuBar();

	void Draw(CRawGraph *pGraph);

	bool OnLButtonDown(UINT nFlags, CPoint point);
	bool OnLButtonUp(UINT nFlags, CPoint point);
	bool OnLButtonDblClk(UINT nFlags, CPoint point);

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
	void EnumProperties(CUIObjProperties &listProperties);

protected:
	CSFImage		m_imgNormal, m_imgHover, m_imgSel;
	tstring			m_strImgNormal, m_strImgHover, m_strImgSel;

	BlendPixMode	m_bpm;

};

#endif // !defined(_SKIN_MENU_BAR_H_)
