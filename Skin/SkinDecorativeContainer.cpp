// SkinDecorativeContainer.cpp: implementation of the CSkinDecorativeContainer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MLSkin.h"
#include "SkinDecorativeContainer.h"


UIOBJECT_CLASS_NAME_IMP(CSkinDecorativeContainer, _T("DecorativeContainer"))


CSkinDecorativeContainer::CSkinDecorativeContainer()
{
}


CSkinDecorativeContainer::~CSkinDecorativeContainer()
{
}


void CSkinDecorativeContainer::InvalidateUIObject(CUIObject *pObj)
{
	// Redraw this whole container.

	if (m_pContainer)
		m_pContainer->InvalidateUIObject(this);
	else
		m_pSkin->InvalidateUIObject(this);
}
