// SkinDecorativeContainer.cpp: implementation of the CSkinDecorativeContainer class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
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

//
// Redraw this whole container.
//
void CSkinDecorativeContainer::UpdateMemGraphicsToScreen(const RECT* lpRect, CUIObject *pObjCallUpdate)
{
	ASSERT(lpRect);

	CRawGraph		*pGraphMem = NULL;

	// Check for UIObject overlay, and redraw them?
	for (V_UIOBJECT::iterator it = m_vUIObjs.begin(); it != m_vUIObjs.end(); ++it)
	{
		CUIObject		*pObj = *it;
		CRect			rc;

		if (pObj != pObjCallUpdate && pObj->IsVisible()
			&& rc.IntersectRect(&pObj->m_rcObj, lpRect))
		{
//			if (pObj->IsUseSkinBg() && m_bPaintBg)
//			{
//				// redraw background ?
//				RedrawBackground(pGraphMem, pObj->m_rcObj);
//			}
			if (!pGraphMem)
			{
				pGraphMem = m_pContainer->GetMemGraph();
				pGraphMem->SetClipBoundBox(*lpRect);
			}

			pObj->Draw(pGraphMem);

			pObj->UpdateTranslucencyWithSkin(pGraphMem);
		}
	}

	if (pGraphMem)
	{
		pGraphMem->RecoverClipBox();
	}

	if (m_pContainer)
		m_pContainer->UpdateMemGraphicsToScreen(lpRect, this);
	else
		m_pSkin->UpdateMemGraphicsToScreen(lpRect);
}
