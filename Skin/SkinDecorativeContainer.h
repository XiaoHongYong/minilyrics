// SkinDecorativeContainer.h: interface for the CSkinDecorativeContainer class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __SKINDECORATIVECONTAINER_H__
#define __SKINDECORATIVECONTAINER_H__

#include "UIObject.h"

//
// If any of the controls in this container call Invalidate, the whole container will be redrawed.
//
// Use this container ONLY if:
//	1) you want to solve UIObject overlapped problems,
//	2) you want to bind two UIObject, one redraw, another will be get notified.
//
class CSkinDecorativeContainer : public CSkinContainer  
{
UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	CSkinDecorativeContainer();
	virtual ~CSkinDecorativeContainer();

	void InvalidateUIObject(CUIObject *pObj);

};

#endif // __SKINDECORATIVECONTAINER_H__
