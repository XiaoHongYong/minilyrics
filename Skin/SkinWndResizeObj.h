#pragma once

#include "UIObject.h"

class CSkinWndResizeObj :
	public CUIObject
{
	UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	CSkinWndResizeObj(void);
	~CSkinWndResizeObj(void);

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

	void OnSize();

protected:
	uint				m_nResizeDirection;

};
