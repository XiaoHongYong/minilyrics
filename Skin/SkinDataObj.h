#ifndef _SKINDATAOBJ_H_
#define _SKINDATAOBJ_H_

#pragma once

#include "uiobject.h"

//
// CSkinDataObj is a special UIObject, only do data processing, invisible, no drawing and no other UI message processing.
//
class CSkinDataObj : public CUIObject
{
	UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	CSkinDataObj();
	~CSkinDataObj();

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
	void EnumProperties(CUIObjProperties &listProperties);

};


#endif // _SKINDATAOBJ_H_
