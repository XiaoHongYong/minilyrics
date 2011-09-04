#pragma once

#include "../Skin/SkinLinearContainer.h"

class CLyricShowTxtContainer : public CSkinLinearContainer  
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinLinearContainer)
public:
	CLyricShowTxtContainer();
	virtual ~CLyricShowTxtContainer();

	void OnCreate();

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

protected:
	vector<tstring>			m_vProperties;
	CUIObject				*m_pLyricsShow;
	CUIObject				*m_pObjScrollBar;

};
