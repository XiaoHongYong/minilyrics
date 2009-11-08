#include "StdAfx.h"
#include "MLSkin.h"
#include "SkinToggleCtrlContainer.h"

UIOBJECT_CLASS_NAME_IMP(CSkinToggleCtrlContainer, _T("ToggleCtrlContainer"))

CSkinToggleCtrlContainer::CSkinToggleCtrlContainer(void)
{
	m_nAnimateDuration = 2000;
	m_nToggleToCtrl = SKIN_UIOBJ_ID_UNDEFINE;
	m_nActiveCtrl = SKIN_UIOBJ_ID_UNDEFINE;
	m_nIDTimerAnimation = 0;
}

CSkinToggleCtrlContainer::~CSkinToggleCtrlContainer(void)
{
}

void CSkinToggleCtrlContainer::Draw(CRawGraph *pGraph)
{
	if (m_nIDTimerAnimation)
	{
		if (!m_memGraph.IsValid())
		{
			IMLGraphicsBase *pTemp = m_pSkin->GetGraphics();
			m_memGraph.Create(m_rcObj.Width(), m_rcObj.Height(), pTemp);
			m_pSkin->ReleaseGraphics(pTemp);
		}
		//m_memGraph.SetOrigin(CPoint(-m_rcObj.left, -m_rcObj.top));
		m_memGraph.SetClipBoundBox(m_rcObj);
		// m_memGraph.FillRect(&m_rcObj, MLColor(RGB(0, 0, 0)), BPM_COPY);
		m_memGraph.FillRect(&m_rcObj, m_pSkin->GetBgColor(), BPM_COPY);

		// Show child controls in animation
		CUIObject	*pObj = GetUIObjectById(m_nActiveCtrl, NULL);
		if (pObj)
			pObj->Draw(pGraph);
		int			nOpaque;

		pObj = GetUIObjectById(m_nToggleToCtrl, NULL);
		if (pObj)
		{
			nOpaque = (GetTickCount() - m_nTimeBeginAni) * 255 / m_nAnimateDuration;
			if (nOpaque > 255)
				nOpaque = 255;
			nOpaque = pGraph->SetOpacityPainting(nOpaque);

			pObj->Draw(&m_memGraph);

			pObj->UpdateTranslucencyWithSkin(pGraph);

			// pGraph->DrawGraph(&m_memGraph, m_rcObj.left, m_rcObj.top, m_rcObj.Width(), m_rcObj.Height(), m_rcObj.left, m_rcObj.top, BPM_BLEND);
			CRawImage	imgTemp;
			imgTemp.Attach(m_memGraph.GetRawBuff());

			imgTemp.Blt(pGraph, m_rcObj.left, m_rcObj.top, m_rcObj.Width(), m_rcObj.Height(), m_rcObj.left, m_rcObj.top, BPM_BLEND_RGB);

			imgTemp.Detach();

			pGraph->SetOpacityPainting(nOpaque);
		}
	}
	else
	{
		CSkinContainer::Draw(pGraph);

		if (!m_memGraph.IsValid())
			m_memGraph.Destroy();
	}
}

BOOL CSkinToggleCtrlContainer::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CSkinContainer::SetProperty(szProperty, szValue))
		return TRUE;

	if (IsPropertyName(szProperty, _T("CmdToggle")))
	{
		m_nToggleToCtrl = m_pSkin->GetSkinFactory()->GetIDByName(szValue);
		if (GetUIObjectById(m_nToggleToCtrl, NULL))
		{
			m_nIDTimerAnimation = m_pSkin->RegisterTimerObject(this, 100);

			// Hide all object
			CUIObject *pObj = GetUIObjectById(m_nActiveCtrl, NULL);
			if (pObj)
				pObj->SetVisible(false);

			m_nTimeBeginAni = GetTickCount();
		}
		else
			m_nToggleToCtrl = SKIN_UIOBJ_ID_UNDEFINE;
	}
	else
		return FALSE;

	return TRUE;
}

void CSkinToggleCtrlContainer::OnCreate()
{
	CSkinContainer::OnCreate();

	m_nActiveCtrl = 0;
}

void CSkinToggleCtrlContainer::OnTimer(int nId)
{
	if (nId == m_nIDTimerAnimation)
	{
		if (GetTickCount() >= m_nTimeBeginAni + m_nAnimateDuration)
		{
			// Stop animation timer
			m_pSkin->UnregisterTimerObject(this, m_nIDTimerAnimation);
			m_nIDTimerAnimation = 0;

			// Hide old Object
			CUIObject	*pObj = GetUIObjectById(m_nActiveCtrl, NULL);
			if (pObj)
			{
				pObj->SetVisible(false);
				m_nActiveCtrl = SKIN_UIOBJ_ID_UNDEFINE;
			}

			// Show new object
			pObj = GetUIObjectById(m_nToggleToCtrl, NULL);
			if (pObj)
			{
				pObj->SetVisible(true);
				m_nActiveCtrl = m_nToggleToCtrl;
			}
		}

		Invalidate();
	}
	else
		CSkinContainer::OnTimer(nId);
}
