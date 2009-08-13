/********************************************************************
	Created  :	2002/01/04	21:42
	FileName :	SkinButton.h
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#if !defined(AFX_SKINBUTTON_H__FD258491_71A3_11D5_9E04_02608CAD9330__INCLUDED_)
#define AFX_SKINBUTTON_H__FD258491_71A3_11D5_9E04_02608CAD9330__INCLUDED_

#include "UIObject.h"
#include "SkinNStatusButton.h"

class CSkinButton : public CSkinNStatusButton  
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinNStatusButton)
public:
	CSkinButton();
	virtual ~CSkinButton();

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

	bool IsCheck();
	void SetCheck(bool bCheck);

};

class CSkinActiveButton : public CSkinButton
{
UIOBJECT_CLASS_NAME_DECLARE(CSkinButton)

public:
	void Draw(CRawGraph *pGraph);
	
	CSkinActiveButton();
	virtual ~CSkinActiveButton();

};

#endif // !defined(AFX_SKINBUTTON_H__FD258491_71A3_11D5_9E04_02608CAD9330__INCLUDED_)
