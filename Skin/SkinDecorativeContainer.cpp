// SkinDecorativeContainer.cpp: implementation of the CSkinDecorativeContainer class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Skin.h"
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
	CRect			rcClipBoxOrg;

	// Check for UIObject overlay, and redraw them?
	for (VecUIObjects::iterator it = m_vUIObjs.begin(); it != m_vUIObjs.end(); ++it)
	{
		CUIObject		*pObj = *it;
		CRect			rc;

		if (pObj != pObjCallUpdate && pObj->IsVisible()
			&& rc.IntersectRect(&pObj->m_rcObj, lpRect))
		{
//			if (pObj->IsUseParentBg() && m_bPaintBg)
//			{
//				// redraw background ?
//				RedrawBackground(pGraphMem, pObj->m_rcObj);
//			}
			if (!pGraphMem)
			{
				pGraphMem = m_pContainer->GetMemGraph();
				pGraphMem->GetClipBoundBox(rcClipBoxOrg);
				pGraphMem->SetClipBoundBox(*lpRect);
			}

			CRawGraph::COpacityBlendAutoRecovery opacityAR(pGraphMem, pObj->GetOpacity());

			pObj->TryToCallInitialUpdate();
			pObj->Draw(pGraphMem);
		}
	}

	if (pGraphMem)
	{
		pGraphMem->ResetClipBoundBox(rcClipBoxOrg);
	}

	if (m_pContainer)
		m_pContainer->UpdateMemGraphicsToScreen(lpRect, this);
	else
		m_pSkin->UpdateMemGraphicsToScreen(lpRect);
}
