// SkinContainer.cpp: implementation of the CSkinContainer class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Skin.h"
#include "SkinContainer.h"

#define SZ_EX_POOL		_T("ExPool.")
#define LEN_EX_POOL		(CountOf(SZ_EX_POOL) - 1)

//////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinContainer, _T("Container"))

CSkinContainer::CSkinContainer()
{
	// m_nMsgWant = UO_MSG_WANT_ALL;
	m_pMenu = NULL;
	m_bClipChildren = false;
	m_nFocusUIObj = -1;
	m_bDrawBg = false;

	m_BgBpm = BPM_COPY;
}

CSkinContainer::~CSkinContainer()
{
	Destroy();
}

bool CSkinContainer::OnCommand(int nID)
{
	for (int i = 0; i < (int)m_vUIObjs.size(); i++)
	{
		CUIObject	*pObj = m_vUIObjs[i];

		if (!pObj->IsVisible())
			continue;

		if (pObj->IsContainer() || pObj->WantMsgMenuCmd())
		{
			if (pObj->OnCommand(nID))
				return true;
		}
	}

	return false;
}

bool CSkinContainer::OnCustomCommand(int nId)
{
	for (int i = 0; i < (int)m_vUIObjs.size(); i++)
	{
		CUIObject	*pObj = m_vUIObjs[i];

		if (!pObj->IsVisible())
			continue;

		if (pObj->IsContainer() || pObj->WantMsgCustomCmd())
		{
			if (pObj->OnCustomCommand(nId))
				return true;
		}
	}

	return false;
}

void CSkinContainer::OnSwitchTo()
{
	if (m_strText.size())
		m_pSkin->SetCaptionText(_TL(m_strText.c_str()));
}

bool CSkinContainer::OnUserMessage(int nMessageID, LPARAM param)
{
	for (uint i = 0; i < m_vUIObjs.size(); i++)
	{
		CUIObject *pObj = m_vUIObjs[i];
		if (!pObj->IsContainer())
			continue;
		if (pObj->GetContainerIf()->OnUserMessage(nMessageID, param))
			return true;
	}

	return false;
}

bool CSkinContainer::OnClose()
{
	for (uint i = 0; i < m_vUIObjs.size(); i++)
	{
		CUIObject *pObj = m_vUIObjs[i];
		if (!pObj->IsVisible() || !pObj->IsContainer())
			continue;
		if (!pObj->GetContainerIf()->OnClose())
			return false;
	}

	return true;
}

bool CSkinContainer::OnOK()
{
	for (uint i = 0; i < m_vUIObjs.size(); i++)
	{
		CUIObject *pObj = m_vUIObjs[i];
		if (!pObj->IsVisible() || !pObj->IsContainer())
			continue;
		if (!pObj->GetContainerIf()->OnOK())
			return false;
	}

	return true;
}

bool CSkinContainer::OnCancel()
{
	for (uint i = 0; i < m_vUIObjs.size(); i++)
	{
		CUIObject *pObj = m_vUIObjs[i];
		if (!pObj->IsVisible() || !pObj->IsContainer())
			continue;
		if (!pObj->GetContainerIf()->OnCancel())
			return false;
	}

	return true;
}

void CSkinContainer::DispatchOnCreateMsg()
{
	if (m_bCreated)
		return;
	m_bCreated = true;

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

void CSkinContainer::OnInitialUpdate()
{
	if (m_strDefaultPageClass.empty())
		return;

	CSkinContainer *pToActivate = (CSkinContainer *)GetChildByClass(m_strDefaultPageClass.c_str());
	if (pToActivate)
	{
		PageViewItem pvToActivate(pToActivate, false, 0);
		m_vStackPageView.push(pvToActivate);
		pToActivate->OnSwitchTo();
		pToActivate->SetVisible(true, false);
	}
	else
	{
		ERRLOG1(_T("Inexistant default Page: %s"), m_strDefaultPageClass.c_str());
		return;
	}
}

void CSkinContainer::OnDestroy()
{
	for (int i = 0; i < (int)m_vUIObjs.size(); i++)
	{
		CUIObject		*pObj = m_vUIObjs[i];

		if (pObj->m_bCreated)
		{
			pObj->OnDestroy();
			pObj->m_bCreated = false;
		}
	}

	Destroy();

	m_bCreated = false;
	m_bInitialUpdated = false;
}

void CSkinContainer::OnKeyUp(UINT nChar, UINT nFlags)
{
	if (m_nFocusUIObj >= 0 && m_nFocusUIObj < (int)m_vUIObjs.size())
	{
		CUIObject *pObj = m_vUIObjs[m_nFocusUIObj];
		pObj->OnKeyUp(nChar, nFlags);
	}
}

void CSkinContainer::OnKeyDown(UINT nChar, UINT nFlags)
{
	if (m_nFocusUIObj >= 0 && m_nFocusUIObj < (int)m_vUIObjs.size())
	{
		CUIObject *pObj = m_vUIObjs[m_nFocusUIObj];
		pObj->OnKeyDown(nChar, nFlags);
	}
}

bool CSkinContainer::OnMouseDrag(CPoint point)
{
	bool	bMsgProceed = false;

	for (int i = 0; i < (int)m_vUIObjs.size(); i++)
	{
		CUIObject	*pObj;
		pObj = m_vUIObjs[i];
		
		if ((pObj->WantMsgMouseMove() || pObj->IsContainer()) && pObj->IsVisible()
			 && pObj->IsEnable() && pObj->IsPtIn(point))
		{
			if (pObj->OnMouseDrag(point))
			{
				bMsgProceed = true;
				break;
			}
		}
	}

	return bMsgProceed;
}

bool CSkinContainer::OnMouseMove(CPoint point) 
{
	bool	bMsgProceed = false;

	for (int i = 0; i < (int)m_vUIObjs.size(); i++)
	{
		CUIObject	*pObj;
		pObj = m_vUIObjs[i];

		if ((pObj->WantMsgMouseMove() || pObj->IsContainer()) && pObj->IsVisible()
			&& pObj->IsEnable() && pObj->IsPtIn(point))
		{
			if (pObj->OnMouseMove(point))
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

		if ((pObj->WantMsgLButton() || pObj->IsContainer()) && pObj->IsVisible()
			 && pObj->IsEnable() && pObj->IsPtIn(point))
		{
			if (pObj->WantMsgKey())
				pObj->SetFocus();
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

		if ((pObj->WantMsgLButton() || pObj->IsContainer()) && pObj->IsVisible()
			 && pObj->IsEnable() && pObj->IsPtIn(point))
		{
			if (pObj->WantMsgKey())
				pObj->SetFocus();
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
		
		if ((pObj->WantMsgLButton() || pObj->IsContainer()) && pObj->IsVisible()
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

		if ((pObj->WantMsgRButton() || pObj->IsContainer()) && pObj->IsVisible()
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

		if ((pObj->WantMsgRButton() || pObj->IsContainer()) && pObj->IsVisible()
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
		
		if ((pObj->WantMsgMouseWheel() || pObj->IsContainer()) && pObj->IsVisible()
			 && pObj->IsEnable())// && pObj->IsPtIn(point))
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

		if ((pObj->WantMsgMenuKey() || pObj->IsContainer()) && pObj->IsVisible() && pObj->IsEnable())
		{
			if (pObj->OnMenuKey(nChar, nFlags))
				return true;
		}
	}

	return false;
}

void CSkinContainer::OnSize()
{
	CUIObject::OnSize();

	// recalculate all UIObjects' position
	FORMULA_VAR 	vars[] = {
		{'w', m_rcObj.Width()},
		{'h', m_rcObj.Height()},
		{0, 0}
	};

	// recalculate every UIObjects' position on back buffer one by one
	for (uint i = 0; i < m_vUIObjs.size(); i++)
	{
		m_vUIObjs[i]->OnMeasureSizePos(vars);
		m_vUIObjs[i]->OnSize();
	}
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

CUIObject *CSkinContainer::GetChildByClass(LPCTSTR szClassName)
{
	for (int i = 0; i < (int)m_vUIObjs.size(); i++)
	{
		CUIObject	*pObj = m_vUIObjs[i];
		if (pObj->IsKindOf(szClassName))
			return pObj;
	}

	return NULL;
}

void CSkinContainer::OnAdjustHue(float hue, float saturation, float luminance)
{
	m_clrBg = m_clrBgOrg;
	m_pSkin->GetSkinFactory()->GetAdjustedHueResult(m_clrBg);

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

MLColor CSkinContainer::GetBgColor() const
{
	if (m_bDrawBg && !m_imageBg.IsValid())
		return m_pSkin->GetTranslucencyColor(m_clrBg);

	if (m_pContainer)
		return m_pContainer->GetBgColor();

	return m_clrBg;
}

bool CSkinContainer::IsDrawBgImage() const
{
	// Draw with background image
	if (m_imageBg.IsValid())
		return true;

	// Draw with background color?
	if (m_bDrawBg)
		return false;

	if (m_pContainer)
		return m_pContainer->IsDrawBgImage();

	return false;
}

void CSkinContainer::RedrawBackground(CRawGraph *pGraph, const RECT &rc)
{
	ASSERT(pGraph);
	if (!pGraph)
		return;

	if (!m_bDrawBg)
	{
		if (m_pContainer)
			m_pContainer->RedrawBackground(pGraph, rc);
		return;
	}

	MLColor clrBg = m_pSkin->GetTranslucencyColor(m_clrBg);

	CRect	rcClip, rcNewClip;
	pGraph->GetClipBoundBox(rcClip);
	rcNewClip.IntersectRect(&rcClip, &rc);

	if (m_pSkin->GetCurrentTranslucency() == 0)
	{
		// Optimize for totally transparent background
		MLColor	clr(RGB(0, 0, 0));
		clr.setAlpha(0);
		pGraph->FillRect(&rcNewClip, clr);
		return;
	}

	CRawGraph::CClipBoxAutoRecovery	autoCBR(pGraph);
	pGraph->SetClipBoundBox(rcNewClip);

	if (m_imageBg.IsValid())
	{
		// Draw Background with image
		CDrawImageFunMask	funcDraw(pGraph, &m_imageBg, NULL, m_BgBpm);

		if (m_imageBgMask.IsValid())
			funcDraw.m_pImageMask = &m_imageBgMask;

		// Fill background
		if (m_imageBgMask.IsValid())
			pGraph->FillRect(&rcNewClip, clrBg);
		else if (m_pSkin->GetCurrentTranslucency() != 255)
		{
// 			MLColor	clr(RGB(0, 0, 0));
// 			clr.setAlpha(0);
// 			pGraph->FillRect(&rcNewClip, clr);
		}

		m_bgImagePainter.Blt(m_rcObj.left, m_rcObj.top, m_rcObj.Width(), m_rcObj.Height(), funcDraw);

//		if (m_pSkin->GetCurrentTranslucency() != 255)
//			pGraph->MultiplyAlpha(m_pSkin->GetCurrentTranslucency(), &rcNewClip);
	}
	else
		pGraph->FillRect(&rcNewClip, clrBg);
}

void CSkinContainer::Draw(CRawGraph *pGraph)
{
	CRect	rc;

	if (m_bClipChildren)
	{
		pGraph->GetClipBoundBox(rc);
		pGraph->SetClipBoundBox(m_rcObj);
	}

	if (m_bDrawBg)
		RedrawBackground(pGraph, m_rcObj);

	for (int i = 0; i < (int)m_vUIObjs.size(); i++)
	{
		CUIObject		*pObj = m_vUIObjs[i];
		if (pObj->IsVisible())
		{
			CRawGraph::COpacityBlendAutoRecovery opacityAR(pGraph, pObj->GetOpacity());

			pObj->TryToCallInitialUpdate();
			pObj->Draw(pGraph);
		}
	}

	if (m_bClipChildren)
		pGraph->ResetClipBoundBox(rc);
}

int CSkinContainer::FromXML(SXNode *pXmlNode)
{
	CUIObject::FromXML(pXmlNode);

	// recalculate position before create children.
	ASSERT(m_pContainer);
	m_pContainer->RecalculateUIObjSizePos(this);

	CreateChild(pXmlNode);

	return ERR_OK;
}

void CSkinContainer::CreateChild(SXNode *pXmlNode)
{
	// Create child CUIObject
	for (SXNode::iterator it = pXmlNode->listChildren.begin();
		it != pXmlNode->listChildren.end(); ++it)
	{
		SXNode			*pNode = *it;

		if (IsPropertyName(pNode->strName.c_str(), SZ_PN_PROPERTY))
		{
			// Is it a property node?
			LPCTSTR szPropName = pNode->GetPropertySafe(SZ_PN_NAME);
			if (!IsEmptyString(szPropName) && !SetProperty(szPropName, pNode))
				DBGLOG2(_T("Unknow Property: %s, name: %s"), pNode->strName.c_str(), szPropName);
			continue;
		}
		else if (IsPropertyName(pNode->strName.c_str(), _T("PlaceHolder")))
		{
			CreateChild(pNode);
			continue;
		}

		CUIObject *pObj = m_pSkin->GetSkinFactory()->CreateUIObject(m_pSkin, pNode->strName.c_str(), this);
		if (pObj)
		{
			// Set its properties
			pObj->FromXML(pNode);

			// Add to child list
			AddUIObject(pObj);
		}
		else
			DBGLOG1(_T("Failed to create UIObject: %s"), pNode->strName.c_str());
	}
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
	else if (IsPropertyName(szProperty, _T("DefaultPage")))
		m_strDefaultPageClass = szValue;
	else if (IsPropertyName(szProperty, _T("BgColor")))
	{
		m_bDrawBg = true;
		GetColorValue(m_clrBg, szValue);
		m_clrBgOrg = m_clrBg;
	}
	else if (IsPropertyName(szProperty, _T("BgImage.Image")))
	{
		m_imageBg.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);
	}
	else if (StrBeginWith(szProperty, SZ_EX_POOL))
	{
		ASSERT(LEN_EX_POOL == _tcslen(SZ_EX_POOL));
		szProperty += LEN_EX_POOL;
		if (!IsEmptyString(szProperty))
			m_pSkin->m_mapExchangePool[szProperty] = szValue;
	}
	else
		return FALSE;

	return true;
}

BOOL CSkinContainer::SetProperty(LPCTSTR szProperty, SXNode *pXmlNode)
{
	if (m_pSkin->SetProperty(szProperty, pXmlNode))
		return TRUE;

	if (IsPropertyName(szProperty, _T("BgImage")))
	{
		// Set background image
		m_imageBg.LoadFromSRM(m_pSkin->GetSkinFactory(), pXmlNode->GetPropertySafe(SZ_PN_IMAGE));
		LPCTSTR szValue = pXmlNode->GetProperty(_T("ImageMask"));
		if (szValue)
			m_imageBgMask.LoadFromSRM(m_pSkin->GetSkinFactory(), szValue);

		// Image Rect
		szValue = pXmlNode->GetProperty(SZ_PN_IMAGERECT);
		if (szValue)
		{
			GetRectValue(szValue, m_imageBg);
			if (m_imageBgMask.IsValid())
				GetRectValue(szValue, m_imageBgMask);
		}

		szValue = pXmlNode->GetPropertySafe(_T("HorzExtendPos"));
		if (!Scan2IntX(szValue, m_bgImagePainter.nSrcHorzExtendStart, m_bgImagePainter.nSrcHorzExtendEnd))
			m_bgImagePainter.nSrcHorzExtendStart = m_bgImagePainter.nSrcHorzExtendEnd = m_imageBg.m_x + m_imageBg.m_cx;

		szValue = pXmlNode->GetPropertySafe(_T("VertExtendPos"));
		if (!Scan2IntX(szValue, m_bgImagePainter.nSrcVertExtendStart, m_bgImagePainter.nSrcVertExtendEnd))
			m_bgImagePainter.nSrcVertExtendStart = m_bgImagePainter.nSrcVertExtendEnd = m_imageBg.m_y + m_imageBg.m_cy;

		szValue = pXmlNode->GetProperty(_T("BlendPixMode"));
		if (szValue)
			m_BgBpm = BlendPixModeFromStr(szValue);

		m_bgImagePainter.nSrcX = m_imageBg.m_x;
		m_bgImagePainter.nSrcY = m_imageBg.m_y;
		m_bgImagePainter.nSrcWidth = m_imageBg.m_cx;
		m_bgImagePainter.nSrcHeight = m_imageBg.m_cy;
		m_bgImagePainter.bDrawCenterArea = !m_imageBgMask.IsValid();

		if (m_imageBg.IsValid())
			m_bDrawBg = true;

		return true;
	}

	return false;
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
	for (int i = 0; i < (int)m_vUIObjs.size(); i++)
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
	if (nId == UID_INVALID)
		return NULL;
	
	// send mouse move message to every ui objects that want to process
	for (int i = 0; i < (int)m_vUIObjs.size(); i++)
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
		else if (pObj->IsContainer())
		{
			pObj = pObj->GetContainerIf()->GetUIObjectById(nId, szClassName);
			if (pObj)
				return pObj;
		}
	}

	return NULL;
}

CUIObject *CSkinContainer::GetUIObjectById(LPCTSTR szId, LPCTSTR szClassName)
{
	return GetUIObjectById(GetIDByName(szId), szClassName);
}

bool CSkinContainer::EnableUIObject(int nId, bool bEnable, bool bRedraw)
{
	CUIObject *pObj = GetUIObjectById(nId, NULL);
	if (!pObj)
	{
		DBGLOG1(_T("UIObject: %s doesn't exist."), GetSkinFactory()->GetStringOfID(nId).c_str());
		return false;
	}

	if (Tobool(pObj->m_bEnable) != bEnable)
	{
		pObj->m_bEnable = bEnable;

		if (bRedraw)
			pObj->Invalidate();
	}

	return true;
}

bool CSkinContainer::EnableUIObject(LPCTSTR szId, bool bEnable, bool bRedraw)
{
	return EnableUIObject(GetIDByName(szId), bEnable, bRedraw);
}

bool CSkinContainer::SetUIObjectProperty(int nId, LPCTSTR szProperty, LPCTSTR szValue)
{
	CUIObject *pObj = GetUIObjectById(nId, NULL);
	if (!pObj)
	{
		DBGLOG1(_T("UIObject: %s doesn't exist."), GetSkinFactory()->GetStringOfID(nId).c_str());
		return false;
	}

	return Tobool(pObj->SetProperty(szProperty, szValue));
}

bool CSkinContainer::SetUIObjectProperty(LPCTSTR szId, LPCTSTR szProperty, LPCTSTR szValue)
{
	return SetUIObjectProperty(GetSkinFactory()->GetIDByName(szId), szProperty, szValue);
}

void CSkinContainer::SetUIObjectVisible(int nId, bool bVisible, bool bRedraw)
{
	CUIObject *pObj = GetUIObjectById(nId, NULL);
	if (!pObj)
	{
		DBGLOG1(_T("UIObject: %s doesn't exist."), GetSkinFactory()->GetStringOfID(nId).c_str());
		return;
	}

	pObj->SetVisible(bVisible, bRedraw);
}

void CSkinContainer::SetUIObjectVisible(LPCTSTR szId, bool bVisible, bool bRedraw)
{
	SetUIObjectVisible(GetSkinFactory()->GetIDByName(szId), bVisible, bRedraw);
}

void CSkinContainer::InvalidateUIObject(int nId)
{
	CUIObject *pObj = GetUIObjectById(nId, NULL);
	if (!pObj)
	{
		DBGLOG1(_T("UIObject: %s doesn't exist."), GetSkinFactory()->GetStringOfID(nId).c_str());
		return;
	}

	InvalidateUIObject(pObj);
}

void CSkinContainer::InvalidateUIObject(LPCTSTR szId)
{
	InvalidateUIObject(GetSkinFactory()->GetIDByName(szId));
}

tstring CSkinContainer::GetUIObjectText(int nId)
{
	CUIObject *pObj = GetUIObjectById(nId, NULL);
	if (!pObj)
	{
		DBGLOG1(_T("UIObject: %s doesn't exist."), GetSkinFactory()->GetStringOfID(nId).c_str());
		return _T("");
	}

	return pObj->GetText();
}

tstring CSkinContainer::GetUIObjectText(LPCTSTR szId)
{
	return GetUIObjectText(GetSkinFactory()->GetIDByName(szId));
}

bool CSkinContainer::GetUIObjectRect(int nId, CRect &rc)
{
	CUIObject		*p = GetUIObjectById(nId, NULL);
	if (!p)
		return false;

	rc = p->m_rcObj;
	return true;
}

bool CSkinContainer::GetUIObjectRect(LPCTSTR szId, CRect &rc)
{
	return GetUIObjectRect(GetIDByName(szId), rc);
}

void CSkinContainer::SetUIObjectText(int nId, LPCTSTR szText, bool bRedraw)
{
	CUIObject *pObj = GetUIObjectById(nId, NULL);
	if (!pObj)
	{
		DBGLOG1(_T("UIObject: %s doesn't exist."), GetSkinFactory()->GetStringOfID(nId).c_str());
		return;
	}

	pObj->SetProperty(SZ_PN_TEXT, szText);
	if (bRedraw)
		pObj->Invalidate();
}

void CSkinContainer::SetUIObjectText(LPCTSTR szId, LPCTSTR szText, bool bRedraw)
{
	SetUIObjectText(GetSkinFactory()->GetIDByName(szId), szText, bRedraw);
}

void CSkinContainer::CheckButton(int nId, bool bCheck)
{
	CSkinNStatusButton *pButton = (CSkinNStatusButton*)GetUIObjectById(nId, CSkinNStatusButton::ClassName());
	if (pButton)
		pButton->SetStatus(bCheck);
}

void CSkinContainer::CheckButton(LPCTSTR szId, bool bCheck)
{
	CheckButton(GetIDByName(szId), bCheck);
}

bool CSkinContainer::IsButtonChecked(int nId)
{
	CSkinNStatusButton *pButton = (CSkinNStatusButton*)GetUIObjectById(nId, CSkinNStatusButton::ClassName());
	if (pButton)
	{
		return Tobool(pButton->GetStatus());
	}

	return false;
}

bool CSkinContainer::IsButtonChecked(LPCTSTR szId)
{
	return IsButtonChecked(GetIDByName(szId));
}

void CSkinContainer::CheckToolbarButton(LPCTSTR szToolbarId, LPCTSTR szButtonId, bool bCheck)
{
	CheckToolbarButton(GetIDByName(szToolbarId), GetIDByName(szButtonId), bCheck);
}

void CSkinContainer::CheckToolbarButton(int nToolbarId, int nButtonId, bool bCheck)
{
	CSkinToolbar *pToolbar = (CSkinToolbar*)GetUIObjectById(
		nToolbarId, CSkinToolbar::ClassName());
	if (pToolbar)
		pToolbar->SetCheck(nButtonId, bCheck);
}

void CSkinContainer::SetFocusUIObject(int nId)
{
	CUIObject *pObj = GetUIObjectById(nId, NULL);
	if (pObj)
		pObj->SetFocus();
}

void CSkinContainer::SetFocusUIObject(LPCTSTR szId)
{
	SetFocusUIObject(GetIDByName(szId));
}

CUIObject *CSkinContainer::GetFocusUIObject()
{
	if (m_nFocusUIObj >= 0 && m_nFocusUIObj < (int)m_vUIObjs.size())
	{
		CUIObject *pObj = m_vUIObjs[m_nFocusUIObj];
		if (pObj->WantMsgKey())
			return pObj;
		else if (pObj->GetContainerIf())
			return pObj->GetContainerIf()->GetFocusUIObject();
		return pObj;
	}

	return NULL;
}

void CSkinContainer::SetFocusChild(CUIObject *pNewFocusObj)
{
	CUIObject *pOldFocusObj = NULL;
	if (m_nFocusUIObj >= 0 && m_nFocusUIObj < (int)m_vUIObjs.size())
		pOldFocusObj = m_vUIObjs[m_nFocusUIObj];

	if (pOldFocusObj == pNewFocusObj)
		return;

	int nNewFocusObj = -1;

	// Get new Focus Object
	if (pNewFocusObj != NULL)
	{
		for (nNewFocusObj = 0; nNewFocusObj < (int)m_vUIObjs.size(); nNewFocusObj++)
		{
			if (m_vUIObjs[nNewFocusObj] == pNewFocusObj)
				break;
		}
		if (nNewFocusObj >= (int)m_vUIObjs.size())
		{
			ASSERT(0 && "New focus UIObject is not the child of this container");
			return;
		}
	}

	m_nFocusUIObj = nNewFocusObj;

	if (pOldFocusObj)
	{
		// Kill Focus for old child.
		if (pOldFocusObj->WantMsgKey())
			pOldFocusObj->OnKillFocus();
		else if (pOldFocusObj->IsContainer())
			pOldFocusObj->GetContainerIf()->SetFocusChild(NULL);
		else
			pOldFocusObj->OnKillFocus();
	}

	if (pNewFocusObj)
	{
		// Let parent container set focus child as "this"
		if (pNewFocusObj->WantMsgKey())
			pNewFocusObj->OnSetFocus();
		if (m_pContainer)
			m_pContainer->SetFocusChild(this);
	}
}

int CSkinContainer::FocusToNext()
{
	if (m_vUIObjs.empty())
	{
		ASSERT(m_nFocusUIObj == -1);
		return false;
	}

	int i = m_nFocusUIObj;
	if (m_nFocusUIObj < 0 || m_nFocusUIObj >= (int)m_vUIObjs.size())
		i = 0;
	else if (m_vUIObjs[m_nFocusUIObj]->WantMsgKey())
		i++;

	for (int k = 0; k <= (int)m_vUIObjs.size(); k++, i++)
	{
		CUIObject *pObj = m_vUIObjs[i % m_vUIObjs.size()];

		if (!pObj->IsVisible())
			continue;

		if (pObj->WantMsgKey())
		{
			pObj->SetFocus();
			return true;
		}
		else if (pObj->IsContainer())
		{
			if (pObj->GetContainerIf()->FocusToNext())
				return true;
		}
	}

	if (!m_pContainer)
	{
		CUIObject *pObj = GetFocusUIObject();
		if (pObj)
			SetFocusChild(NULL);
	}

	return false;
}

int CSkinContainer::FocusToPrev()
{
	if (m_vUIObjs.empty())
	{
		ASSERT(m_nFocusUIObj == -1);
		return false;
	}

	int i = m_nFocusUIObj;
	if (m_nFocusUIObj < 0 || m_nFocusUIObj >= (int)m_vUIObjs.size())
		i = 0;
	else if (m_vUIObjs[m_nFocusUIObj]->WantMsgKey())
		i--;

	for (int k = 0; k <= (int)m_vUIObjs.size(); k++, i--)
	{
		CUIObject *pObj = m_vUIObjs[(i + m_vUIObjs.size()) % m_vUIObjs.size()];

		if (!pObj->IsVisible())
			continue;

		if (pObj->WantMsgKey())
		{
			pObj->SetFocus();
			return true;
		}
		else if (pObj->IsContainer())
		{
			if (pObj->GetContainerIf()->FocusToPrev())
				return true;
		}
	}

	if (!m_pContainer)
	{
		CUIObject *pObj = GetFocusUIObject();
		if (pObj)
			SetFocusChild(NULL);
	}

	return false;
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
			if (bFree)
				delete pObj;

			// Change UIObject focus, if possible.
			if (i == m_nFocusUIObj)
			{
				m_nFocusUIObj = -1;
				m_pSkin->GetRootContainer()->FocusToNext();
			}

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
	if (nId == UID_INVALID)
		return NULL;

	for (int i = 0; i < (int)m_vUIObjs.size(); i++)
	{
		CUIObject	*pObj =  m_vUIObjs[i];
		
		if (pObj->m_nId == nId)
		{
			m_pSkin->UnregisterTimerObject(pObj);
			m_pSkin->OnRemoveUIObj(pObj);
			m_vUIObjs.erase(m_vUIObjs.begin() + i);

			// Change UIObject focus, if possible.
			if (i == m_nFocusUIObj)
			{
				m_nFocusUIObj = -1;
				m_pSkin->GetRootContainer()->FocusToNext();
			}

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

	OnAdd(pObj);

	return TRUE;
}

bool CSkinContainer::InsertUIObjectAt(CUIObject *pObjWhere, CUIObject *pObj)
{
	assert(pObj);
	if (pObj == NULL)
		return FALSE;

	VecUIObjects::iterator	it;

	pObj->m_nAutoUniID = m_pSkin->AllocAutoUniID();
	for (it = m_vUIObjs.begin(); it != m_vUIObjs.end(); ++it)
	{
		if (*it == pObjWhere)
			break;
	}
	m_vUIObjs.insert(it, pObj);

	OnAdd(pObj);

	return TRUE;
}

BOOL CSkinContainer::RecalculateVarValue(CFormula &form, int &nRetValue)
{
	FORMULA_VAR 	vars[] = {
		{'w', m_rcObj.Width()},
		{'h', m_rcObj.Height()},
		{0, 0}
	};

	return form.CalCualteValue(vars, nRetValue);
}

void CSkinContainer::RecalculateUIObjSizePos(CUIObject *pObj)
{
	// recalculate all ui objects' position
	FORMULA_VAR 	vars[] = {
		{'w', m_rcObj.Width()},
		{'h', m_rcObj.Height()},
		{0, 0}
	};

	// recalculate every UIObjects' position on back buffer one by one
	pObj->OnMeasureSizePos(vars);
	pObj->OnSize();
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
#ifdef _WIN32
	if (m_pContainer)
		m_pContainer->InvalidateUIObject(pObj);
	else
		m_pSkin->InvalidateUIObject(pObj);
#else // #ifdef _WIN32
	m_pSkin->InvalidateUIObject(pObj);
#endif // #ifdef _WIN32
}

void CSkinContainer::UpdateMemGraphicsToScreen(const RECT* lpRect, CUIObject *pObjCallUpdate)
{
	if (m_pContainer)
		m_pContainer->UpdateMemGraphicsToScreen(lpRect, this);
	else
		m_pSkin->UpdateMemGraphicsToScreen(lpRect);
}

void CSkinContainer::SwitchToPage(LPCTSTR szPageClass, bool bWaitResultOfNextPage, int nRequestCodeOfNextPage, bool bAnimation)
{
	if (m_vStackPageView.size() > 100)
	{
		DBGLOG1(_T("Stack of Page View is too large, clear it: %d"), m_vStackPageView.size());
		m_vStackPageView.empty();
	}

	PageViewItem pvToHide(NULL, false, 0);
	if (!m_vStackPageView.empty())
		pvToHide = m_vStackPageView.top();

	CSkinContainer *pToActivate = (CSkinContainer *)GetChildByClass(szPageClass);
	if (!pToActivate)
	{
		ERRLOG1(_T("Switch to an inexistant Page: %s"), szPageClass);
		return;
	}

	PageViewItem pvToActivate(pToActivate, bWaitResultOfNextPage, nRequestCodeOfNextPage);
	m_vStackPageView.push(pvToActivate);

	bool bIsNewPageAtLeft = IsPageAtLeft(pToActivate, pvToHide.pContainerPage);
	if (pvToHide.pContainerPage)

		pvToHide.pContainerPage->SetVisibleEx(false, bAnimation, bIsNewPageAtLeft ? AD_RIGHT: AD_LEFT);
	pToActivate->OnSwitchTo();
	pToActivate->SetVisibleEx(true, bAnimation, bIsNewPageAtLeft ? AD_LEFT : AD_RIGHT);
}

void CSkinContainer::SwitchToLastPage(int nResultCodeOfPage, bool bAnimation)
{
	if (m_vStackPageView.size() >= 2)
	{
		PageViewItem pvToHide = m_vStackPageView.top();
		m_vStackPageView.pop();
		PageViewItem pvToActivate = m_vStackPageView.top();

		bool bIsNewPageAtLeft = IsPageAtLeft(pvToActivate.pContainerPage, pvToHide.pContainerPage);

		// Hide page from !bIsNewPageAtLeft
		pvToHide.pContainerPage->SetVisibleEx(false, bAnimation, bIsNewPageAtLeft ? AD_RIGHT : AD_LEFT);

		if (pvToActivate.bWaitResultOfNextPage)
			pvToActivate.pContainerPage->OnPageViewResult(pvToActivate.nRequestCodeOfNextPage, nResultCodeOfPage);
		pvToActivate.pContainerPage->OnSwitchTo();
		pvToActivate.pContainerPage->SetVisibleEx(true, bAnimation, bIsNewPageAtLeft ? AD_LEFT : AD_RIGHT);
	}
}

void CSkinContainer::SetExPool(LPCTSTR szProperty, LPCTSTR szValue)
{
	m_pSkin->m_mapExchangePool[szProperty] = szValue;
}

void CSkinContainer::SetExPool(LPCTSTR szProperty, int nValue)
{
	m_pSkin->m_mapExchangePool[szProperty] = CStrPrintf(_T("%d"), nValue).c_str();
}

tstring CSkinContainer::GetExPoolStr(LPCTSTR szProperty, LPCTSTR szDefault)
{
	MapString::iterator it = m_pSkin->m_mapExchangePool.find(szProperty);
	if (it != m_pSkin->m_mapExchangePool.end())
		return (*it).second;
	else
		return szDefault;
}

int CSkinContainer::GetExPoolInt(LPCTSTR szProperty, int nDefault)
{
	MapString::iterator it = m_pSkin->m_mapExchangePool.find(szProperty);
	if (it != m_pSkin->m_mapExchangePool.end())
		return _ttoi((*it).second.c_str());
	else
		return nDefault;
}

bool CSkinContainer::GetExPoolBool(LPCTSTR szProperty, bool bDefault)
{
	MapString::iterator it = m_pSkin->m_mapExchangePool.find(szProperty);
	if (it != m_pSkin->m_mapExchangePool.end())
		return IsTRUE((*it).second.c_str());
	else
		return bDefault;
}

#ifdef _DEBUG
void CSkinContainer::DumpUIObject(int &nDeep)
{
	tstring		strDeep;
	strDeep.resize(nDeep * 2, ' ');
	nDeep++;

	for (uint i = 0; i < m_vUIObjs.size(); i++)
	{
		CUIObject	*p = m_vUIObjs[i];
		CStrPrintf str(_T("%s id=%s(%d), name=%s visible=%d rc=%d,%d,%d,%d"), + p->GetClassName(), 
			m_pSkin->GetSkinFactory()->GetStringOfID(p->GetID()).c_str(), p->GetID(),
			p->m_strName.c_str(), p->m_bVisible, p->m_rcObj.left, p->m_rcObj.top, p->m_rcObj.Width(), p->m_rcObj.Height());
		DBGLOG2(_T("%s%s"), strDeep.c_str(), str.c_str());
		if (p->IsContainer())
			p->GetContainerIf()->DumpUIObject(nDeep);
	}

	nDeep--;
}
#endif

bool CSkinContainer::IsPageAtLeft(CSkinContainer *pPage, CSkinContainer *pPage2)
{
	if (pPage2 == NULL)
		return false;

	int		nPos = -1, nPos2 = m_vUIObjs.size();

	for (int i = 0; i < (int)m_vUIObjs.size(); i++)
	{
		if (m_vUIObjs[i] == pPage)
			nPos = i;
		else if (m_vUIObjs[i] == pPage2)
			nPos2 = i;
	}

	return nPos < nPos2;
}

void CSkinContainer::OnAdd(CUIObject *pObj)
{
	m_pSkin->OnAddUIObj(pObj);

	if (!m_bCreated)
		return;

	// Only when parent is opened, OnCreate will be called.

	RecalculateUIObjSizePos(pObj);

	if (pObj->IsContainer())
		pObj->GetContainerIf()->DispatchOnCreateMsg();
	else
	{
		ASSERT(!pObj->m_bCreated);
		pObj->m_bCreated = true;
		pObj->OnCreate();
	}

	if (m_pSkin->GetFocusUIObj() == NULL && pObj->WantMsgKey() && pObj->IsEnable()
		&& pObj->IsVisible() && pObj->IsParentVisible())
		pObj->SetFocus();
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

BOOL CSkinClientArea::SetProperty(LPCTSTR szProperty, SXNode *pXmlNode)
{
	if (m_pSkin->SetProperty(szProperty, pXmlNode))
		return TRUE;

	return CSkinContainer::SetProperty(szProperty, pXmlNode);
}
