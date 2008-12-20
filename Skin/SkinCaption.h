// SkinCaption.h: interface for the CSkinCaption class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINCAPTION_H__FE68860E_70C8_4775_82AA_58E760E09473__INCLUDED_)
#define AFX_SKINCAPTION_H__FE68860E_70C8_4775_82AA_58E760E09473__INCLUDED_

#include "UIObject.h"

class CSkinCaption : public CUIObject  
{
UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	CSkinCaption();
	virtual ~CSkinCaption();

	void Draw(CRawGraph *pGraph);

	bool OnLButtonDown(UINT nFlags, CPoint point);
	bool OnLButtonUp(UINT nFlags, CPoint point);
	bool OnLButtonDblClk(UINT nFlags, CPoint point);

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
	void EnumProperties(CUIObjProperties &listProperties);

protected:
	CSFImage		m_imgBk, m_imgFocus;

	tstring			m_strBmpBkFile;
	tstring			m_strBmpFocusFile;

	bool			m_bEnableMaximize;

	long			m_nCutPos1, m_nCutPos2, m_nCutPos3, m_nCutPos4;

	BlendPixMode	m_bpm;

};

#endif // !defined(AFX_SKINCAPTION_H__FE68860E_70C8_4775_82AA_58E760E09473__INCLUDED_)
