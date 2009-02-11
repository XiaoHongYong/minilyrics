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

void CSkinDecorativeContainer::UpdateMemGraphicsToScreen(const RECT* lpRect)
{
	ASSERT(lpRect);

	CRawGraph		*pGraphMem = NULL;

	// Check for UIObject overlay, and redraw them?
	for (V_UIOBJECT::iterator it = m_vUIObjs.begin(); it != m_vUIObjs.end(); ++it)
	{
		CUIObject		*pObj = *it;
		CRect			rc;

		if (pObj->IsVisible() && rc.IntersectRect(&pObj->m_rcObj, lpRect))
		{
//			if (pObj->IsUseSkinBg() && m_bPaintBg)
//			{
//				// redraw background ?
//				RedrawBackground(pObj->m_rcObj);
//			}
			if (!pGraphMem)
			{
				pGraphMem = m_pSkin->GetMemGraphics();
				pGraphMem->SetClipBoundBox(*lpRect);
			}

			pObj->Draw(pGraphMem);
			if (pObj->IsUpdateTranslucencyWithSkin())
			{
				MLColor		clrAlpha;
				clrAlpha.setAlpha((byte)m_pSkin->m_nTranslucencyAlphaDynamic);
				if (pObj->m_imgMask.IsValid())
				{
					CRect	rcMask;
					rcMask.SetRect(pObj->m_imgMask.m_x, pObj->m_imgMask.m_y, pObj->m_imgMask.m_x + pObj->m_imgMask.m_cx, pObj->m_imgMask.m_y + pObj->m_imgMask.m_cy);
					m_pSkin->GetMemGraphics()->FillRect(&pObj->m_rcObj, pObj->m_imgMask.GetHandle(), &rcMask, clrAlpha, BPM_MULTIPLY);
				}
				else
					m_pSkin->GetMemGraphics()->FillRect(&pObj->m_rcObj, clrAlpha, BPM_MULTIPLY);
			}
		}
	}

	if (pGraphMem)
	{
		pGraphMem->RecoverClipBox();
	}

	m_pSkin->UpdateMemGraphicsToScreen(lpRect);
}
