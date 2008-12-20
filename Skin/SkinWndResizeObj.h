#pragma once

#include "uiobject.h"

class CSkinWndResizeObj :
	public CUIObject
{
	UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	CSkinWndResizeObj(void);
	~CSkinWndResizeObj(void);

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
	void EnumProperties(CUIObjProperties &listProperties);

	void OnSize(int cx, int cy);

protected:
	uint				m_nResizeDirection;

};
