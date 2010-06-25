// SkinContainer.cpp: implementation of the CSkinContainer class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MLSkin.h"
#include "SkinContainer.h"


//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinContainer, _T("Container"))

CSkinContainer::CSkinContainer()
{
	m_nMsgWant = UO_MSG_WANT_ALL;
	m_pMenu = NULL;
	m_bClipChildren = false;
}

CSkinContainer::~CSkinContainer()
{
	Destroy();
}

bool CSkinContainer::OnCommand(int nID)
{
	int		nCount;

	nCount = (int)m_vUIObjs.size();
	for (int i = 0; i < nCount; i++)
	{
		CUIObject	*pObj = m_vUIObjs[i];

		if (pObj->IsContainer())
		{
			if (pObj->GetContainerIf()->OnCommand(nID))
			{
				return true;
			}
		}
	}

	return false;
}


void CSkinContainer::DispatchOnCreateMsg()
{
	for (int i = 0; i < (int)m_vUIObjs.size(); i++)
	{
		CUIObject		*pObj = m_vUIObjs[i];

		if (pObj->IsContainer())
		{
			CSkinContainer	*pContainer = pObj->GetContainerIf();
			pContainer->DispatchOnCreateMsg();
		}
		else if (!pObj->m_bCreated)
		{
			pObj->m_bCreated = true;
			pObj->OnCreate();
		}
	}

	OnCreate();
}


bool CSkinContainer::OnMouseMove(UINT nFlags, CPoint point) 
{
	int		nCount;
	bool	bMsgProceed = false;

	nCount = (int)m_vUIObjs.size();
	for (int i = 0; i < nCount; i++)
	{
		CUIObject	*pObj;
		pObj = m_vUIObjs[i];
		
		if (pObj->WantMsgMouseMove() && pObj->IsVisible()
			 && pObj->IsEnable() && pObj->IsPtIn(point))
		{
			if (pObj->OnMouseMove(nFlags, point))
			{
				bMsgProceed = true;
				break;
			}
		}
	}

	return bMsgProceed;
}

bool CSkinContainer::OnLButtonDown(UINT nFlags, CPoint point)
{
	int			nCount;
	bool		bMsgProceed = false;

	// send mouse move message to the ui objects that want to process
	nCount = (int)m_vUIObjs.size();
	for (int i = nCount - 1; i >= 0; i--)
	{
		CUIObject	*pObj;
		pObj = m_vUIObjs[i];
		assert(pObj);

		if (pObj->WantMsgLButton() && pObj->IsVisible()
			 && pObj->IsEnable() && pObj->IsPtIn(point))
		{
			if (!pObj->IsContainer() && pObj->WantMsgKey())
				m_pSkin->SetFocusUIObject(pObj);
			if (pObj->OnLButtonDown(nFlags, point))
			{
				bMsgProceed = true;
				break;
			}
		}
	}

	return bMsgProceed;
}

bool CSkinContainer::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	int			nCount;
	bool		bMsgProceed = false;

	// send mouse move message to the ui objects that want to process
	nCount = (int)m_vUIObjs.size();
	for (int i = nCount - 1; i >= 0; i--)
	{
		CUIObject	*pObj;
		pObj = m_vUIObjs[i];
		assert(pObj);

		if (pObj->WantMsgLButton() && pObj->IsVisible()
			 && pObj->IsEnable() && pObj->IsPtIn(point))
		{
			if (!pObj->IsContainer() && pObj->WantMsgKey())
				m_pSkin->SetFocusUIObject(pObj);
			if (pObj->OnLButtonDblClk(nFlags, point))
			{
				bMsgProceed = true;
				break;
			}
		}
	}

	return bMsgProceed;
}

bool CSkinContainer::OnLButtonUp(UINT nFlags, CPoint point)
{
	int			nCount;
	bool		bMsgProceed = false;

	// send mouse move message to the ui objects that want to process
	nCount = (int)m_vUIObjs.size();
	for (int i = nCount - 1; i >= 0; i--)
	{
		CUIObject	*pObj;
		pObj = m_vUIObjs[i];
		//		assert(pObj == NULL);
		
		if (pObj->WantMsgLButton() && pObj->IsVisible()
			 && pObj->IsEnable() && pObj->IsPtIn(point))
		{
			if (pObj->OnLButtonUp(nFlags, point))
			{
				bMsgProceed = true;
				break;
			}
		}
	}

	return bMsgProceed;
}

bool CSkinContainer::OnRButtonDown(UINT nFlags, CPoint point)
{
	int			nCount;
	bool		bMsgProceed = false;

	// send mouse move message to the ui objects that want to process
	nCount = (int)m_vUIObjs.size();
	for (int i = nCount - 1; i >= 0; i--)
	{
		CUIObject	*pObj;
		pObj = m_vUIObjs[i];
		//		assert(pObj == NULL);

		if (pObj->WantMsgRButton() && pObj->IsVisible()
			 && pObj->IsEnable() && pObj->IsPtIn(point))
		{
			if (pObj->OnRButtonDown(nFlags, point))
			{
				bMsgProceed = true;
				break;
			}
		}
	}

	return bMsgProceed;
}

bool CSkinContainer::OnRButtonUp(UINT nFlags, CPoint point)
{
	int			nCount;
	bool		bMsgProceed = false;

	// send mouse move message to the ui objects that want to process
	nCount = (int)m_vUIObjs.size();
	for (int i = nCount - 1; i >= 0; i--)
	{
		CUIObject	*pObj;
		pObj = m_vUIObjs[i];
		//		assert(pObj == NULL);

		if (pObj->WantMsgRButton() && pObj->IsVisible()
			 && pObj->IsEnable() && pObj->IsPtIn(point))
		{
			if (pObj->OnRButtonUp(nFlags, point))
			{
				bMsgProceed = true;
				break;
			}
		}
	}

	if (!bMsgProceed && m_pMenu)
	{
		POINT	pt;
		MLWidget::GetCursorPos(&pt);
		m_pMenu->TrackPopupMenu(pt.x, pt.y, m_pSkin);
		return true;
	}

	return bMsgProceed;
}

void CSkinContainer::OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt)
{
	int			nCount;

	// send mouse move message to every ui objects that want to process
	nCount = (int)m_vUIObjs.size();
	for (int i = nCount - 1; i >= 0; i--)
	{
		CUIObject	*pObj;
		pObj = m_vUIObjs[i];
		assert(pObj);
		
		if (pObj->WantMsgLButton() && pObj->IsVisible()
			 && pObj->IsEnable() && pObj->WantMsgMouseWheel())// && pObj->IsPtIn(point))
		{
			pObj->OnMouseWheel(nWheelDistance, nMkeys, pt);
			return;
		}
	}
}


bool CSkinContainer::OnMenuKey(UINT nChar, UINT nFlags)
{
	for (int i = m_vUIObjs.size() - 1; i >= 0; i--)
	{
		CUIObject	*pObj = m_vUIObjs[i];
		assert(pObj);

		if (pObj->WantMsgMenuKey() && pObj->IsVisible() && pObj->IsEnable())
		{
			if (pObj->OnMenuKey(nChar, nFlags))
				return true;
		}
	}

	return false;
}


void CSkinContainer::OnSize(int cx, int cy)
{
	RecalculateSizePos();
}

void CSkinContainer::OnLanguageChanged()
{
	if (m_pMenu)
	{
		delete m_pMenu;
		m_pMenu = NULL;

		m_pSkin->GetSkinFactory()->LoadMenu(m_pSkin, &m_pMenu, m_strContextMenu.c_str());
	}

	// Notify to child container...
	for (int i = 0; i < (int)m_vUIObjs.size(); i++)
	{
		m_vUIObjs[i]->OnLanguageChanged();
	}
}

void CSkinContainer::OnAdjustHue(float hue, float saturation, float luminance)
{
	for (int i = 0; i < (int)m_vUIObjs.size(); i++)
	{
		CUIObject	*pObj = m_vUIObjs[i];
		pObj->OnAdjustHue(hue, saturation, luminance);
	}
}


void CSkinContainer::OnSkinFontChanged()
{
	for (int i = 0; i < (int)m_vUIObjs.size(); i++)
	{
		CUIObject	*pObj = m_vUIObjs[i];
		pObj->OnSkinFontChanged();
	}
}


CRawGraph *CSkinContainer::GetMemGraph()
{
	if (m_pContainer)
		return m_pContainer->GetMemGraph();
	else
		return m_pSkin->GetMemGraphics();
}


void CSkinContainer::Draw(CRawGraph *pGraph)
{
	CRect	rc;

	if (m_bClipChildren)
	{
		pGraph->GetClipBoundBox(rc);
		pGraph->SetClipBoundBox(m_rcObj);
	}

	for (int i = 0; i < (int)m_vUIObjs.size(); i++)
	{
		CUIObject		*pObj = m_vUIObjs[i];
		if (pObj->IsVisible())
		{
/*			if (pObj->IsUseSkinBg() && pObj->IsUpdateTranslucencyWithSkin()
				&& m_pSkin->m_nTranslucencyAlphaDynamic != m_pSkin->m_nTranslucencyAlphaOnActive)
			{
				// Redraw background
				m_pSkin->RedrawBackground(pGraph, &pObj->m_rcObj, true);
			}*/

			CRawGraph::COpacityBlendAutoRecovery opacityAR(pGraph, pObj->GetOpacity());

			pObj->Draw(pGraph);
		}
	}

	if (m_bClipChildren)
		pGraph->ResetClipBoundBox(rc);
}

int CSkinContainer::FromXML(CSkinContainer *pContainer, SXNode *pXmlNode)
{
	CUIObject::FromXML(pContainer, pXmlNode);

	// recalculate position before create children.
	pContainer->RecalculateUIObjSizePos(this);

	// Create child CUIObject
	SXNode::iterator	it, itEnd;

	itEnd = pXmlNode->listChildren.end();
	for (it = pXmlNode->listChildren.begin(); it != itEnd; ++it)
	{
		SXNode			*pNode = *it;
		CUIObject		*pObj;

		pObj = m_pSkin->GetSkinFactory()->CreateUIObject(m_pSkin, pNode->strName.c_str(), this);
		// assert(pObj);

		if (pObj)
		{
			// 创建成功，首先设置它的属性
			pObj->FromXML(this, pNode);

			// 添加到数组中
			AddUIObject(pObj);
		}
	}

	return ERR_OK;
}


BOOL CSkinContainer::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, _T("ContextMenu")) == 0)
	{
		if (m_pMenu)
		{
			delete m_pMenu;
			m_pMenu = NULL;
		}

		m_strContextMenu = szValue;
		m_pSkin->GetSkinFactory()->LoadMenu(m_pSkin, &m_pMenu, szValue);
	}
	else if (IsPropertyName(szProperty, _T("ClipChildren")))
		m_bClipChildren = IsTRUE(szValue);
	else
		return FALSE;

	return true;
}

#ifdef _SKIN_EDITOR_
void CSkinContainer::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	listProperties.AddPropStr(_T("ContextMenu"), m_strContextMenu.c_str(), !m_strContextMenu.empty());
}
#endif // _SKIN_EDITOR_

void CSkinContainer::Destroy()
{
	if (m_pMenu)
	{
		delete m_pMenu;
		m_pMenu = NULL;
	}

	while (!m_vUIObjs.empty())
	{
		CUIObject		*pObj = m_vUIObjs.back();
		m_pSkin->UnregisterTimerObject(pObj);
		m_pSkin->OnRemoveUIObj(*(m_vUIObjs.begin() + m_vUIObjs.size() - 1));
		m_vUIObjs.erase(m_vUIObjs.begin() + m_vUIObjs.size() - 1);
		delete pObj;
	}
	m_vUIObjs.clear();
}

// functions for CSkinWnd
CUIObject *CSkinContainer::GetUIObjectByClassName(LPCTSTR szClassName)
{
	int nCount;

	// send mouse move message to every ui objects that want to process
	nCount = (int)m_vUIObjs.size();
	for (int i = 0; i < nCount; i++)
	{
		CUIObject	*pObj = m_vUIObjs[i];
		if (pObj->IsKindOf(szClassName))
			return pObj;
		if (pObj->IsContainer())
		{
			pObj = pObj->GetContainerIf()->GetUIObjectByClassName(szClassName);
			if (pObj)
				return pObj;
		}
	}

	return NULL;
}

CUIObject *CSkinContainer::GetUIObjectById(int nId, LPCTSTR szClassName)
{
	int nCount;

	if (nId == SKIN_UIOBJ_ID_UNDEFINE)
		return NULL;
	
	// send mouse move message to every ui objects that want to process
	nCount = (int)m_vUIObjs.size();
	for (int i = 0; i < nCount; i++)
	{
		CUIObject	*pObj = m_vUIObjs[i];
		if (pObj->m_nId == nId)
		{
			if (!szClassName)
				return pObj;
			else if (pObj->IsKindOf(szClassName))
				return pObj;
			else
				return NULL;
		}
		if (pObj->IsContainer())
		{
			pObj = pObj->GetContainerIf()->GetUIObjectById(nId, szClassName);
			if (pObj)
				return pObj;
		}
	}

	return NULL;
}

bool CSkinContainer::RemoveUIObject(CUIObject *pObj, bool bFree)
{
	for (int i = 0; i < int(m_vUIObjs.size()); i++)
	{
		CUIObject	*p = m_vUIObjs[i];
		if (p == pObj)
		{
			// found it,
			m_pSkin->UnregisterTimerObject(pObj);
			m_pSkin->OnRemoveUIObj(pObj);
			m_vUIObjs.erase(m_vUIObjs.begin() + i);
			m_pSkin->VerifyFocusUIObject();

			if (bFree)
				delete pObj;

			return true;
		}
		else if (p->IsContainer())
		{
			if (p->GetContainerIf()->RemoveUIObject(pObj, bFree))
				return true;
		}
	}

	return false;
}

CUIObject *CSkinContainer::RemoveUIObjectById(int nId)
{
	if (nId == SKIN_UIOBJ_ID_UNDEFINE)
		return NULL;

	for (int i = 0; i < (int)m_vUIObjs.size(); i++)
	{
		CUIObject	*pObj =  m_vUIObjs[i];
		
		if (pObj->m_nId == nId)
		{
			m_pSkin->UnregisterTimerObject(pObj);
			m_pSkin->OnRemoveUIObj(pObj);
			m_vUIObjs.erase(m_vUIObjs.begin() + i);
			m_pSkin->VerifyFocusUIObject();

			return pObj;
		}
		else if (pObj->IsContainer())
		{
			CUIObject	*p = pObj->GetContainerIf()->RemoveUIObjectById(nId);
			if (p)
				return p;
		}
	}
	
	return NULL;
}


bool CSkinContainer::AddUIObject(CUIObject *pObj)
{
	assert(pObj);
	if (pObj == NULL)
		return FALSE;

	pObj->m_nAutoUniID = m_pSkin->AllocAutoUniID();
	m_vUIObjs.push_back(pObj);

	RecalculateUIObjSizePos(pObj);
	m_pSkin->OnAddUIObj(pObj);

	return TRUE;
}

bool CSkinContainer::InsertUIObjectAt(CUIObject *pObjWhere, CUIObject *pObj)
{
	assert(pObj);
	if (pObj == NULL)
		return FALSE;

	V_UIOBJECT::iterator	it;

	pObj->m_nAutoUniID = m_pSkin->AllocAutoUniID();
	for (it = m_vUIObjs.begin(); it != m_vUIObjs.end(); ++it)
	{
		if (*it == pObjWhere)
		{
			break;
		}
	}
	m_vUIObjs.insert(it, pObj);

	if (m_bCreated)
		RecalculateUIObjSizePos(pObj);
	m_pSkin->OnAddUIObj(pObj);

	return TRUE;
}

bool CSkinContainer::AddUIObjectToFront(CUIObject *pObj)
{
	assert(pObj);
	if (pObj == NULL)
		return FALSE;

	pObj->m_nAutoUniID = m_pSkin->AllocAutoUniID();
	m_vUIObjs.insert(m_vUIObjs.begin(), pObj);

	m_pSkin->OnAddUIObj(pObj);

	return TRUE;
}

void CSkinContainer::RecalculateSizePos()
{
	// recalcuate all ui objects' position
	int nCount;
	FORMULA_VAR 	vars[] = {
		{'w', m_rcObj.Width()},
		{'h', m_rcObj.Height()},
		{0, 0}
	};

	// recalculate every ui objects' position on back buffer one by one
	nCount = (int)m_vUIObjs.size();
	for (int i = 0; i < nCount; i++)
	{
		m_vUIObjs[i]->ReCalculatePos(vars);
	}
}

void CSkinContainer::RecalculateUIObjSizePos(CUIObject *pObj)
{
	// recalculate all ui objects' position
	FORMULA_VAR 	vars[] = {
		{'w', m_rcObj.Width()},
		{'h', m_rcObj.Height()},
		{0, 0}
	};

	// recalculate every ui objects' position on back buffer one by one
	pObj->ReCalculatePos(vars);
}

bool CSkinContainer::IsChild(CUIObject *pObj)
{
	for (int i = 0; i < (int)m_vUIObjs.size(); i++)
	{
		CUIObject	*p = m_vUIObjs[i];

		if (pObj == p)
			return true;

		if (p->IsContainer())
		{
			if (p->GetContainerIf()->IsChild(pObj))
				return true;
		}
	}

	return false;
}

void CSkinContainer::InvalidateUIObject(CUIObject *pObj)
{
	if (m_pContainer)
		m_pContainer->InvalidateUIObject(pObj);
	else
		m_pSkin->InvalidateUIObject(pObj);
}

void CSkinContainer::UpdateMemGraphicsToScreen(const RECT* lpRect, CUIObject *pObjCallUpdate)
{
	if (m_pContainer)
		m_pContainer->UpdateMemGraphicsToScreen(lpRect, this);
	else
		m_pSkin->UpdateMemGraphicsToScreen(lpRect);
}

#ifdef _SKIN_EDITOR_
bool CSkinContainer::HasXMLChild()
{
	return !m_vUIObjs.empty();
}

void CSkinContainer::OnToXMLChild(CXMLWriter &xmlStream)
{
	for (int i = 0; i < (int)m_vUIObjs.size(); i++)
	{
		m_vUIObjs[i]->ToXML(xmlStream);
	}
}
#endif // _SKIN_EDITOR_

UIOBJECT_CLASS_NAME_IMP(CSkinClientArea, _T("ClientArea"))

