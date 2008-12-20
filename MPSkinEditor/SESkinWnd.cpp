// SESkinWnd.cpp: implementation of the CSESkinWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPSkinEditor.h"
#include "SESkinWnd.h"
#include "UIEditObject.h"
#include "SEPropertyListCtrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSESkinWnd::CSESkinWnd()
{
	m_WndDrag.EnableDrag(FALSE);

	m_hMenuContext = LoadMenu(GetAppInstance(), MAKEINTRESOURCE(IDR_CONTEXT));

	m_dbScale = 1.0;

	m_pNotify = NULL;

	m_bEnableVolumeSwitch = false;
	m_bIndividualProperties = false;

	m_bInEditorMode = true;

	m_bEnableTransparentFeatureEditing = true;
	m_bTranslucencyLayeredEditing = false;
}

CSESkinWnd::~CSESkinWnd()
{

}

int CSESkinWnd::Create(LPCTSTR szClassName, LPCTSTR szCaption, CSkinFactory *pSkinFactory, LPCTSTR szSkinWndName, CBaseWnd *pWndParent)
{
	CRect		rcWnd;

	m_pSkinFactory = pSkinFactory;
	m_strSkinWndName = szSkinWndName;

	if (!CreateEx(szClassName, szCaption,
		rcWnd.left, rcWnd.top, rcWnd.Width(), rcWnd.Height(),
		pWndParent,
		WS_CHILD | DS_NOIDLEMSG | WS_VISIBLE | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_THICKFRAME | WS_MAXIMIZEBOX))
		return ERR_FALSE;

	return ERR_OK;
}

void CSESkinWnd::NewSkin(LPCTSTR szSkinWndName)
{
	CloseSkin();

	m_strSkinWndName = szSkinWndName;
}

int CSESkinWnd::OpenSkin(LPCTSTR szSkinWndName)
{
	return CSkin::OpenSkin(szSkinWndName);
}

int CSESkinWnd::CopyFrom(LPCTSTR szSkinWndName, CSESkinWnd *pSkinWndSrc)
{
	CUIObjProperties	properties;
	V_Str				vStr;
	int					i;

	CloseSkin();

	// copy properties
	pSkinWndSrc->EnumProperties(properties);
	properties.ToUIObjProperties(vStr);
	for (i = 0; i < vStr.size(); i += 2)
		SetProperty(vStr[i].c_str(), vStr[i + 1].c_str());

	// copy uiobject
	for (i = 0; i < pSkinWndSrc->m_vUIObjs.size(); i++)
	{
		CUIObject		*pObj = pSkinWndSrc->m_vUIObjs[i];
		CUIObject		*pObjDup;
		pObjDup = g_skinFactory.CreateUIObject(this, pObj->GetClassName());
		properties.clear();
		vStr.clear();
		pObj->EnumProperties(properties);
		properties.ToUIObjProperties(vStr);
		pObjDup->SetProperties(vStr);

		m_rootConainter.AddUIObject(pObjDup);
	}

	m_strSkinWndName = szSkinWndName;

	RecalculateSizePos();

	InvalidateRect();

	return ERR_OK;
}

// COMMENT:
//		当加载完SKIN后，调用此函数进行一些设置工作
//		1)	设置子控件的焦点
void CSESkinWnd::OnSkinLoaded()
{
	CSkin::OnSkinLoaded();

	::MoveWindow(m_hWnd, 5, 5, int(m_rcReal.Width()), int(m_rcReal.Height()), true);
// 
// 	if (m_SizeObj.GetEnableResize())
// 	{
// 		MoveWindow(m_rcReal);
// 	}
}

bool ContainerIsUIObjExist(CSkinContainer *pContainer, CUIObject *pObj)
{
	CUIObject	*p;
	int			n;

	n = pContainer->GetChildrenCount();
	for (int i = 0; i < n; i++)
	{
		p = pContainer->GetChildByIndex(i);
		if (p == pObj)
			return true;
		else if (p->IsContainer())
		{
			if (ContainerIsUIObjExist(p->GetContainerIf(), pObj))
				return true;
		}
	}

	return false;
}

bool CSESkinWnd::IsUIObjectExist(CUIObject *pObj)
{
	return ContainerIsUIObjExist(&m_rootConainter, pObj);
}

int CSESkinWnd::InsertUIObject(CUIObject *pObj)
{
	CSkinContainer		*pContainer;
	CUIObject			*pObjFocus;

	if (!pObj)
		return ERR_FALSE;

	pObjFocus = GetFocusUIObj();
	if (pObjFocus)
		pContainer = pObjFocus->m_pContainer;
	else
		pContainer = &m_rootConainter;

	if (IsEmptyString(pObj->m_formLeft.GetFormula()))
	{
		pObj->SetProperty(SZ_PN_LEFT, _T("10"));
		pObj->SetProperty(SZ_PN_TOP, _T("10"));
		pObj->SetProperty(SZ_PN_WIDTH, _T("30"));
		pObj->SetProperty(SZ_PN_HEIGHT, _T("15"));
		pObj->m_nId = 0;
	}
	pObj->m_pContainer = pContainer;
	pObj->m_pSkin = this;

	pContainer->AddUIObject(pObj);

	RecalculateSizePos();

	pObj->SetFocus();

	InvalidateRect(NULL, TRUE);

	m_pNotify->OnUIObjectListChanged();
	m_pNotify->OnUIObjFocusChanged();

	return ERR_OK;
}


void CSESkinWnd::SetNOFocusUIObj()
{
	m_itFocusUIObject.End();
}


bool CSESkinWnd::RemoveFocusUIObject()
{
	CUIObject	*pObj = GetFocusUIObj();
	if (!pObj)
		return false;

	pObj->m_pContainer->RemoveUIObject(pObj, true);

	m_pNotify->OnUIObjectListChanged();

	InvalidateRect(NULL, TRUE);
	return true;
}

void CSESkinWnd::OnKeyDown(UINT nChar, UINT nFlags)
{
	BOOL		bUpdate = FALSE;
	tstring		strName;
	TCHAR		szValue[64];
	CUIObject	*pObjFocus;
	pObjFocus = GetFocusUIObj();

	if (nChar == VK_DELETE)
	{
		RemoveFocusUIObject();

		m_pNotify->OnUIObjFocusChanged();
		return;
	}
	else if (nChar == VK_LEFT)
	{
		// 左移焦点控件
		if (pObjFocus)
		{
			_stprintf(szValue, _T("%s-1"), pObjFocus->m_formLeft.GetFormula());
			strName = _T("left");
			bUpdate = TRUE;
		}
	}
	else if (nChar == VK_RIGHT)
	{
		// 右移焦点控件
		CUIObject	*pObj;
		pObj = GetFocusUIObj();
		if (pObj)
		{
			_stprintf(szValue, _T("%s+1"), pObj->m_formLeft.GetFormula());
			strName = _T("left");
			bUpdate = TRUE;
		}
	}
	else if (nChar == VK_UP)
	{
		// 上移焦点控件
		if (pObjFocus)
		{
			_stprintf(szValue, _T("%s-1"), pObjFocus->m_formTop.GetFormula());
			strName = _T("top");
			bUpdate = TRUE;
		}
	}
	else if (nChar == VK_DOWN)
	{
		// 上移焦点控件
		if (pObjFocus)
		{
			strName = _T("top");
			_stprintf(szValue, _T("%s+1"), pObjFocus->m_formTop.GetFormula());
			bUpdate = TRUE;
		}
	}
	if (bUpdate)
	{
		if (pObjFocus)
		{
			pObjFocus->SetProperty(strName.c_str(), szValue);
			RecalculateSizePos();
			InvalidateRect(NULL, TRUE);
			// 更新
			CFormula	form;
			form.SetFormula(szValue);
			m_pNotify->OnFocusUIObjPropertyChanged(strName.c_str(), form.GetFormula());
		}
	}

	CSkin::OnKeyDown(nChar, nFlags);

	if (pObjFocus != GetFocusUIObj())
	{
		// 更新
		m_pNotify->OnUIObjFocusChanged();
	}
}

void CSESkinWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	CUIObject	*pObjFocus, *pObjFocusNew;

	pObjFocus = GetFocusUIObj();

	CSkin::OnLButtonDown(nFlags, point);

	pObjFocusNew = GetFocusUIObj();

	point.x = long(point.x / m_dbScale);
	point.y = long(point.y / m_dbScale);

	// 判断鼠标是否击中某个object?
	BOOL	bHit = FALSE;
	if (pObjFocusNew && pObjFocusNew->IsPtIn(point))
	{
		bHit = true;
	}

	if (!bHit && pObjFocus)
	{
		// 未击中，在skin编辑模式下，不设置任何Object为焦点状态
		SetNOFocusUIObj();
	}

	if (pObjFocus != pObjFocusNew)
	{
		// 更新
		m_pNotify->OnUIObjFocusChanged();
	}
}

void CSESkinWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	CSkin::OnMouseMove(nFlags, point);

	g_seUIAdapter.SetMousePointerInfo(int(point.x / m_dbScale), int(point.y / m_dbScale));
}

void CSESkinWnd::OnPaint(IMLGraphicsBase *pGraph)
{
	IMLGraphicsBase		*pMemGraph;

	pMemGraph = GetMemGraphics();

	// 刷背景色
	if (m_brBg.IsValid())
		pMemGraph->FillRect(0, 0, m_rcReal.Width(), m_rcReal.Height(), &m_brBg);

	CSkin::OnPaint(pGraph);
}

void CSESkinWnd::OnContexMenu(int xPos, int yPos)
{
	HMENU	hMenuSub = GetSubMenu(m_hMenuContext, 0);
	g_seSkinPropertyAdapter.TrackPopupMenu(hMenuSub, xPos, yPos);
}

void CSESkinWnd::OnRButtonUp(UINT nFlags, CPoint point)
{
	POINT	pt;
	MLWidget::GetCursorPos(&pt);
	OnContexMenu(pt.x, pt.y);
}

LRESULT CSESkinWnd::WndProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	return CSkin::WndProc(message, wParam, lParam);
}

BOOL CSESkinWnd::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	BOOL	bRet;

	if (_tcsicmp(szProperty, SZ_PN_NAME) == 0)
	{
		if (_tcscmp(szValue, m_strSkinWndName.c_str()) != 0)
		{
			if (m_pSkinFactory->GetSkinFile()->ChangeSkinWndName(m_strSkinWndName.c_str(), szValue) == ERR_OK)
			{
				m_strSkinWndName = szValue;
				m_pNotify->OnSkinWndNameChanged();
			}
			else
				m_pNotify->OnUIObjFocusChanged();
		}
		return TRUE;
	}
	else if (IsPropertyName(szProperty, _T("EnableTransparentFeature")))
	{
		m_bEnableTransparentFeatureEditing = IsTRUE(szValue);
		return TRUE;
	}
	else if (IsPropertyName(szProperty, _T("EnableTranslucency")))
	{
		m_bTranslucencyLayeredEditing = IsTRUE(szValue);
		return TRUE;
	}

	bRet = CSkin::SetProperty(szProperty, szValue);

	if (_tcsicmp(szProperty, SZ_PN_WIDTH) == 0 ||
		_tcsicmp(szProperty, SZ_PN_HEIGHT) == 0)
		::MoveWindow(m_hWnd, 5, 5, int(m_nWidth * m_dbScale), int(m_nHeight * m_dbScale), true);
	else if (_tcsicmp(szProperty, _T("PaintBg")) == 0 ||
		_tcsicmp(szProperty, _T("BgColor")) == 0)
		InvalidateRect();
	else if (_tcsicmp(szProperty, _T("Cursor")) == 0)
		MLWidget::SetCursor(m_cursor);
	else if (_tcsicmp(szProperty, _T("Region")) == 0)
		UpdateSkinProperty();

	if (bRet)
		return TRUE;

	if (_tcsicmp(szProperty, _T("CmdHandler")) == 0)
		m_strCmdHandler = szValue;
	else if (_tcsicmp(szProperty, _T("ContextMenu")) == 0)
		m_strContextMenu = szValue;
	else if (_tcsicmp(szProperty, _T("EnableVolumeSwitch")) == 0)
		m_bEnableVolumeSwitch = IsTRUE(szValue);
	else if (_tcsicmp(szProperty, _T("IndividualProperties")) == 0)
		m_bIndividualProperties = IsTRUE(szValue);
	else
		return FALSE;

	return TRUE;
}

void CSESkinWnd::EnumProperties(CUIObjProperties &listProperties)
{
	CSkin::EnumProperties(listProperties);

	listProperties.DelProp(_T("EnableTransparentFeature"));
	listProperties.DelProp(_T("EnableTranslucency"));

	listProperties.AddPropBoolStr(_T("EnableTransparentFeature"), m_bEnableTransparentFeatureEditing, !m_bEnableTransparentFeatureEditing);
	listProperties.AddPropBoolStr(_T("EnableTranslucency"), m_bTranslucencyLayeredEditing, m_bTranslucencyLayeredEditing);

	listProperties.AddPropStr(_T("CmdHandler"), m_strCmdHandler.c_str(), !m_strCmdHandler.empty());
	listProperties.AddPropBoolStr(_T("EnableVolumeSwitch"), m_bEnableVolumeSwitch, m_bEnableVolumeSwitch);
}

void CSESkinWnd::ZoomIn()
{
	if (m_dbScale < 4.0)
	{
		m_dbScale += 1.0;
		::MoveWindow(m_hWnd, 5, 5, int(m_rcBoundBox.Width() * m_dbScale), int(m_rcBoundBox.Height() * m_dbScale), true);
		InvalidateRect();
	}
}

void CSESkinWnd::ZoomOut()
{
	if (m_dbScale > 1.0)
	{
		m_dbScale -= 1.0;
		::MoveWindow(m_hWnd, 5, 5, int(m_rcBoundBox.Width() * m_dbScale), int(m_rcBoundBox.Height() * m_dbScale), true);
		InvalidateRect();
	}
}

void CSESkinWnd::EnumAllUIObj(vector<tstring> &vObj)
{
	V_UIOBJECT::iterator	it, itEnd;

	itEnd = m_vUIObjs.end();
	for (it = m_vUIObjs.begin(); it != itEnd; ++it)
	{
		vObj.push_back((*it)->m_strName);
		vObj.push_back((*it)->GetClassName());
	}
}
