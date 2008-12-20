// SkinSquare.h: interface for the CSkinSquare class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINSQUARE_H__2BE65AFD_A655_4209_A194_E1D50117EDDA__INCLUDED_)
#define AFX_SKINSQUARE_H__2BE65AFD_A655_4209_A194_E1D50117EDDA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UIObject.h"

class CSkinSquare : public CUIObject  
{
UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	CSkinSquare();
	virtual ~CSkinSquare();

	void Draw(CRawGraph *pGraph);
	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

	void EnumProperties(CUIObjProperties &listProperties);

protected:
	MLColor				m_clrBg;
	CMLBrush			m_brBg;

};

#endif // !defined(AFX_SKINSQUARE_H__2BE65AFD_A655_4209_A194_E1D50117EDDA__INCLUDED_)
