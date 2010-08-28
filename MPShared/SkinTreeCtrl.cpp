// SkinTreeCtrl.cpp: implementation of the CSkinTreeCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SkinTreeCtrl.h"

void ISkinTreeNode::AddChildBack(ISkinTreeNode *pChild)
{
	if (m_pLastChild)
		m_pLastChild->m_pNext = pChild;
	pChild->m_pPrev = m_pLastChild;
	pChild->m_pNext = NULL;
	pChild->m_pParent = this;
	m_pLastChild = pChild;
	if (m_pFirstChild == NULL)
		m_pFirstChild = pChild;
}

void ISkinTreeNode::AddChildFront(ISkinTreeNode *pChild)
{
	if (m_pFirstChild)
		m_pFirstChild->m_pPrev = pChild;
	pChild->m_pPrev = NULL;
	pChild->m_pNext = m_pFirstChild;
	pChild->m_pParent = this;
	m_pFirstChild = pChild;
	if (m_pLastChild == NULL)
		m_pLastChild = pChild;
}

void ISkinTreeNode::RemoveLink()
{
	if (m_pPrev)
		m_pPrev->m_pNext = m_pNext;
	if (m_pNext)
		m_pNext->m_pPrev = m_pPrev;

	if (m_pParent->m_pFirstChild == this)
		m_pParent->m_pFirstChild = m_pNext;
	if (m_pParent->m_pLastChild == this)
		m_pParent->m_pLastChild = m_pPrev;
}

ISkinTreeNode *ISkinTreeNode::GetChildByIndex(int nIndex)
{
	ISkinTreeNode	*pNode = m_pFirstChild;
	for (int i = 0; i < nIndex && pNode; i++)
	{
		pNode = pNode->m_pNext;
	}

	return pNode;
}

ISkinTreeNode *ISkinTreeNode::GetChildByName(LPCTSTR szName)
{
	ISkinTreeNode	*pNode = m_pFirstChild;
	while (pNode)
	{
		if (_tcscmp(pNode->m_strName.c_str(), szName) == 0)
			return pNode;
		pNode = pNode->m_pNext;
	}

	return pNode;
}

int ISkinTreeNode::GetDepth()
{
	int				i = -1;
	ISkinTreeNode	*pParent = m_pParent;
	while (pParent)
	{
		i++;
		pParent = pParent->m_pParent;
	}
	return i;
}

bool ISkinTreeNode::IsBigBrother()
{
	if (!m_pParent)
		return true;

	return m_pParent->m_pFirstChild == this;
}

void ISkinTreeNode::GetPath(SkinTreePath_t &path)
{
	ISkinTreeNode	*pNode, *pNodeSearch;
	int		nIndex;

	path.clear();

	pNodeSearch = this;
	while (pNodeSearch->m_pParent)
	{
		pNode = pNodeSearch->m_pParent->FirstChild();
		for (nIndex = 0; pNode != NULL; pNode = pNode->NextSibling(), nIndex++)
		{
			if (pNode == pNodeSearch)
				break;
		}
		path.insert(path.begin(), nIndex);
		pNodeSearch = pNodeSearch->m_pParent;
	}
}

void ISkinTreeNode::GetPath(SkinTreeStrPath_t &path)
{
	ISkinTreeNode	*pNodeSearch;

	path.clear();

	pNodeSearch = this;
	while (pNodeSearch->m_pParent)
	{
		path.push_back(pNodeSearch->m_strName);
		pNodeSearch = pNodeSearch->m_pParent;
	}
}

int ISkinTreeNode::GetExpandedNodeCount()
{
	int		n = 1;
	ISkinTreeNode	*pNode = m_pFirstChild;
	while (pNode)
	{
		if (pNode->HasChild() && pNode->IsExpanded())
			n += pNode->GetExpandedNodeCount();
		else
			n++;

		pNode = pNode->m_pNext;
	}

	return n;
}

int ISkinTreeNode::GetExpandedNodeCountTill(ISkinTreeNode *pNodeTill, bool &bFound)
{
	int		n = 1;
	ISkinTreeNode	*pNode = m_pFirstChild;
	while (pNode)
	{
		if (pNodeTill == pNode)
		{
			bFound = true;
			break;
		}
		if (pNode->HasChild() && pNode->IsExpanded())
			n += pNode->GetExpandedNodeCountTill(pNodeTill, bFound);
		else
			n++;

		if (bFound)
			break;
		pNode = pNode->m_pNext;
	}

	return n;
}

ISkinTreeNode *ISkinTreeNode::GetExpandedNodeByPos(int &nPos)
{
	ISkinTreeNode	*pNodeFound, *pNode;

	if (nPos == 0)
		return this;
	nPos--;
	if (IsExpanded() && m_pFirstChild)
	{
		pNode = m_pFirstChild;
		while (pNode)
		{
			pNodeFound = pNode->GetExpandedNodeByPos(nPos);
			if (pNodeFound)
				return pNodeFound;
			pNode = pNode->m_pNext;
		}
	}

/*	pNode = this->NextSibling();
	while (pNode)
	{
		pNodeFound = pNode->GetExpandedNodeByPos(nPos);
		if (pNodeFound)
			return pNodeFound;
		pNode = pNode->NextSibling();
	}*/

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
ISkinTree::ISkinTree()
{
	m_root.SetExpanded(true);
}

ISkinTree::~ISkinTree()
{
	m_root.Free();
}

void ISkinTree::GetCurPath(SkinTreePath_t &path)
{
	path = m_pathSel;
}

void ISkinTree::GetCurPath(SkinTreeStrPath_t &path)
{
	ISkinTreeNode	*pNode;

	path.clear();
	pNode = GetSelNode();
	if (pNode)
		pNode->GetPath(path);
}

ISkinTreeNode *ISkinTree::GetSelNode()
{
	return GetNode(m_pathSel);
}

ISkinTreeNode *ISkinTree::GetFirstVisibleNode()
{
	ISkinTreeNode	*pNode = GetNode(m_pathFirstVisible);
	if (!pNode)
	{
		pNode = m_root.FirstChild();
	}

	return pNode;
}

int ISkinTree::GetAllExpandedNodeCount()
{
	return m_root.GetExpandedNodeCount() - 1;
}

int ISkinTree::GetFirstVisibleNodePos()
{
	ISkinTreeNode	*pNodeFirstVisible = GetFirstVisibleNode();
	bool			bFound = false;
	return m_root.GetExpandedNodeCountTill(pNodeFirstVisible, bFound) - 1;
}

int ISkinTree::GetExpandedNodePos(ISkinTreeNode *pNode)
{
	bool			bFound = false;
	return m_root.GetExpandedNodeCountTill(pNode, bFound) - 1;
}

ISkinTreeNode *ISkinTree::GetExpandedNodeByPos(int nPos)
{
	nPos++;
	return m_root.GetExpandedNodeByPos(nPos);
}

bool ISkinTree::SetSelNode(ISkinTreeNode *pNode)
{
	ISkinTreeNode	*pNodeSelOld = GetSelNode();

	if (pNodeSelOld == pNode)
		return false;

	if (pNodeSelOld)
		pNodeSelOld->SetSelected(false);

	m_pathSel.clear();

	if (pNode)
	{
		pNode->SetSelected(true);
		pNode->GetPath(m_pathSel);
	}

	return true;
}

void ISkinTree::SetFirstVisibleNode(ISkinTreeNode *pNode)
{
	m_pathFirstVisible.clear();

	if (pNode)
		pNode->GetPath(m_pathFirstVisible);
}

ISkinTreeNode *ISkinTree::GetNode(SkinTreePath_t &path)
{
	ISkinTreeNode	*pNode, *pNodeNext;

	pNodeNext = NULL;
	pNode = &m_root;
	for (int i = 0; i < (int)path.size(); i++)
	{
		pNodeNext = pNode->GetChildByIndex(path[i]);
		if (!pNodeNext)
		{
			pNodeNext = pNode->LastChild();
			if (!pNodeNext)
			{
				pNodeNext = pNode;
				break;
			}
		}
		pNode = pNodeNext;
	}

	return pNodeNext;
}

ISkinTreeNode *ISkinTree::GetNode(SkinTreeStrPath_t &path)
{
	ISkinTreeNode	*pNode, *pNodeNext;

	pNodeNext = NULL;
	pNode = &m_root;
	for (int i = 0; i < (int)path.size(); i++)
	{
		pNodeNext = pNode->GetChildByName(path[i].c_str());
		if (!pNodeNext)
		{
			pNodeNext = pNode->LastChild();
			if (!pNodeNext)
			{
				pNodeNext = pNode;
				break;
			}
		}
		pNode = pNodeNext;
	}

	return pNodeNext;
}

// ÖÐÐò±éÀú
ISkinTreeNode *ISkinTree::CenterOrderNextExpandedOnly(ISkinTreeNode *pNode)
{
	if (pNode->FirstChild() && pNode->IsExpanded())
		return pNode->FirstChild();		// child
	else if (pNode->NextSibling())
		return pNode->NextSibling();	// next sibling
	else
	{
		// parent's next sibling
		while (pNode->m_pParent->m_pParent)
		{
			pNode = pNode->m_pParent;
			if (pNode->NextSibling())
				return pNode->NextSibling();
		}
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////

UIOBJECT_CLASS_NAME_IMP(CSkinTreeCtrl, _T("TreeCtrl"))

CSkinTreeCtrl::CSkinTreeCtrl()
{
	m_pTreeData = NULL;

	m_nXMargin = 2;
	m_nLineIndent = 18;

	m_nMsgWant = UO_MSG_WANT_LBUTTON | UO_MSG_WANT_MOUSEMOVE | UO_MSG_WANT_KEY | UO_MSG_WANT_MOUSEWHEEL;

	// SetBkColor(MLColor(GetSysColor(COLOR_WINDOW)));
	SetBkColor(MLColor(RGB(192, 192, 192)));
	SetSelRowBkColor(MLColor(RGB(128, 128, 128)));
	m_clrSelText.set(RGB(255, 255, 255));

	m_nLineHeight = 14;

	m_pObjScrollBar = NULL;
	m_pScrollBar = NULL;

/*	// test code
	ISkinTree		g_treeData;
	ISkinTreeNode *pRoot = g_treeData.GetRoot();
	ISkinTreeNode	*p, *p2;

	for (int i = 0; i < 20; i++)
	{
		p = new ISkinTreeNode;
		p->m_strName = CStrPrintf(_T("Node %d"), i).c_str();
		p->SetExpanded(true);
		pRoot->AddChildBack(p);

		for (int k = 0; k < 2; k++)
		{
			p2 = new ISkinTreeNode;
			p2->m_strName = CStrPrintf(_T("Node %d-%d"), i, k).c_str();
			p->SetExpanded(true);
			p->AddChildBack(p2);
		}
	}
	SetDataSrc(&g_treeData);

	for ( i = 0; i < 40; i++)
	{
		p = m_pTreeData->GetExpandedNodeByPos(i);
		int	nRet = m_pTreeData->GetExpandedNodePos(p);
		if (i != nRet)
		{
			ERRLOG0(_T("not equal."));
		}
	}*/
}

CSkinTreeCtrl::~CSkinTreeCtrl()
{

}

void CSkinTreeCtrl::Draw(CRawGraph *pGraph)
{
	ISkinTreeNode		*pNodeFirstVisible, *pNode, *pNodeNext;
	int			nDepth;
	bool		bBigBrother;
	int			x, y, xMax, yMax;
	bool		bVScrollBarVisible = true;
	CRect		rcNodeText;

	pGraph->FillRect(m_rcObj.left, m_rcObj.top, m_rcObj.Width(), m_rcObj.Height(), m_clrBk);

	if (!m_pTreeData)
		return;

	pNodeFirstVisible = m_pTreeData->GetFirstVisibleNode();
	if (!pNodeFirstVisible)
		return;

	CRawGraph::CClipBoxAutoRecovery	autoCBR(pGraph);
	pGraph->SetClipBoundBox(m_rcObj);

	xMax = m_rcObj.right - m_nXMargin;
	//if (bVScrollBarVisible && m_pObjScrollBar)
	//	xMax -= m_pObjScrollBar->m_rcObj.Width();

	y = m_rcObj.top;
	yMax = m_rcObj.bottom;

	// Draw Tree from first visible node
	nDepth = pNodeFirstVisible->GetDepth();
	bBigBrother = pNodeFirstVisible->IsBigBrother();
	pNode = pNodeFirstVisible;

	if (m_font.IsGood())
		pGraph->SetFont(m_font.GetFont());

	while (y < yMax && pNode)
	{
		if (pNode->IsSelected())
		{
			// draw background
			x = m_rcObj.left;
			pGraph->FillRect(x, y, m_rcObj.right - x, m_nLineHeight, m_clrSelRowBk);
		}

		// draw image
		x = m_rcObj.left + m_nXMargin + nDepth * m_nLineIndent;
		if (pNode->IsExpanded() && pNode->HasChild() && pNode->m_nExpandedImageIndex != SKIN_TREE_INVALID_IMG_INDEX)
			m_imageList.Draw(pGraph, pNode->m_nExpandedImageIndex, x, y);
		else
			m_imageList.Draw(pGraph, pNode->m_nImageIndex, x, y);

		// draw text
		x += m_imageList.GetIconCx() + 6;
		rcNodeText.left = x;
		rcNodeText.top = y;
		rcNodeText.right = xMax;
		rcNodeText.bottom = y + m_nLineHeight;

		if (m_font.IsOutlined())
		{
			pGraph->DrawTextOutlined(pNode->m_strName.c_str(), pNode->m_strName.size(), &rcNodeText, 
				pNode->IsSelected() ? m_clrSelText : m_font.GetTextColor(m_bEnable), m_font.GetColorOutlined());
		}
		else
		{
			if (pNode->IsSelected())
				pGraph->SetTextColor(m_clrSelText);
			else
				pGraph->SetTextColor(m_font.GetTextColor(m_bEnable));
			pGraph->DrawText(pNode->m_strName.c_str(), pNode->m_strName.size(), &rcNodeText);
		}

		// to next item
		if (pNode->HasChild() && pNode->IsExpanded())
		{
			// to it's child
			pNodeNext = pNode->FirstChild();
			if (pNodeNext)
				nDepth++;
		}
		else
			pNodeNext = NULL;

		if (!pNodeNext)
		{
			pNodeNext = pNode->NextSibling();
			if (!pNodeNext)
			{
				while (!pNodeNext && pNode)
				{
					pNode = pNode->m_pParent;
					nDepth--;
					if (pNode)
						pNodeNext = pNode->NextSibling();
					else
						pNodeNext = NULL;
				}
			}
		}
		pNode = pNodeNext;
		y += m_nLineHeight;
	}
}

CSkinTreeCtrl::TreeArea CSkinTreeCtrl::HitTest(POINT point, ISkinTreeNode **ppNode)
{
	if (!m_pTreeData)
		return TA_BLANK;

	ISkinTreeNode		*pNodeFirstVisible, *pNode, *pNodeNext;
	int			nDepth;
	bool		bBigBrother;
	int			x, y, xMax, yMax;
	bool		bVScrollBarVisible = true;

	pNodeFirstVisible = m_pTreeData->GetFirstVisibleNode();
	if (!pNodeFirstVisible)
		return TA_BLANK;

	xMax = m_rcObj.right - m_nXMargin;
	if (bVScrollBarVisible && m_pObjScrollBar)
		xMax -= m_pObjScrollBar->m_rcObj.Width();

	y = m_rcObj.top;
	yMax = m_rcObj.bottom;

	// Draw Tree from first visible node
	nDepth = pNodeFirstVisible->GetDepth();
	bBigBrother = pNodeFirstVisible->IsBigBrother();
	pNode = pNodeFirstVisible;

	if (point.y < y || point.x > xMax || point.x < m_rcObj.left + m_nXMargin)
		return TA_BLANK;

	while (y < yMax && pNode)
	{
		if (point.y < y + m_nLineHeight)
		{
			// hit on this line
			if (ppNode)
				*ppNode = pNode;

			// image area?
			x = m_rcObj.left + m_nXMargin + nDepth * m_nLineIndent;
			if (point.x >= x && point.x <= x + m_imageList.GetIconCx())
				return TA_EXPAND;
			else
				return TA_LINE;
		}

		// to next item
		if (pNode->HasChild() && pNode->IsExpanded())
		{
			// to it's child
			pNodeNext = pNode->FirstChild();
			if (pNodeNext)
				nDepth++;
		}
		else
			pNodeNext = NULL;

		if (!pNodeNext)
		{
			pNodeNext = pNode->NextSibling();
			if (!pNodeNext)
			{
				while (!pNodeNext && pNode)
				{
					pNode = pNode->m_pParent;
					nDepth--;
					if (pNode)
						pNodeNext = pNode->NextSibling();
					else
						pNodeNext = NULL;
				}
			}
		}
		pNode = pNodeNext;
		y += m_nLineHeight;
	}

	return TA_BLANK;
}

bool CSkinTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!m_pTreeData)
		return false;

	ISkinTreeNode	*pNode = NULL;

	TreeArea	ta = HitTest(point, &pNode);
	if (ta != TA_BLANK && pNode)
	{
		SetSelNode(pNode);

		if (ta == TA_EXPAND && pNode->HasChild())
		{
			pNode->SetExpanded(!pNode->IsExpanded());
			UpdateVScrollBar();
		}

		Invalidate();
	}

	return true;
}

bool CSkinTreeCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (!m_pTreeData)
		return false;

	ISkinTreeNode	*pNode = NULL;

	TreeArea	ta = HitTest(point, &pNode);
	if (ta != TA_BLANK && pNode)
	{
		SetSelNode(pNode);

		if (!pNode->NoChild())
		{
			pNode->SetExpanded(!pNode->IsExpanded());
			UpdateVScrollBar();
		}

		Invalidate();

		NotifyEvent(CSkinTreeCtrlEventNotify::C_DBL_CLICK);
	}

	return true;
}

void CSkinTreeCtrl::OnKeyDown(UINT nChar, UINT nFlags)
{
	if (!m_pTreeData)
		return;

	switch (nChar)
	{
	case VK_LEFT:
	case VK_RIGHT:
	case VK_UP:
	case VK_DOWN:
		{
			ISkinTreeNode	*pNodeSel;

			pNodeSel = m_pTreeData->GetSelNode();
			if (!pNodeSel)
			{
				pNodeSel = m_pTreeData->GetFirstVisibleNode();
				if (pNodeSel)
					SetSelNode(pNodeSel);
				return;
			}

			if (nChar == VK_LEFT)
			{
				if (pNodeSel->IsExpanded() && pNodeSel->HasChild())
				{
					pNodeSel->SetExpanded(false);
					UpdateVScrollBar();
				}
				else if (pNodeSel->m_pParent != m_pTreeData->GetRoot())
				{
					pNodeSel = pNodeSel->m_pParent;
					SetSelNode(pNodeSel);
				}
			}
			else if (nChar == VK_RIGHT)
			{
				if (!pNodeSel->IsExpanded() && pNodeSel->HasChild())
				{
					pNodeSel->SetExpanded(true);
					UpdateVScrollBar();
				}
				else if (pNodeSel->HasChild())
				{
					pNodeSel = pNodeSel->FirstChild();
					SetSelNode(pNodeSel);
				}
			}
			else if (nChar == VK_UP)
			{
				// move to previous visible node
				if (!pNodeSel->PrevSibling())
				{
					// to its parent
					if (pNodeSel->m_pParent != m_pTreeData->GetRoot())
					{
						pNodeSel = pNodeSel->m_pParent;
						SetSelNode(pNodeSel);
						Invalidate();
					}
					return;
				}
				else
					pNodeSel = pNodeSel->PrevSibling();
				while (pNodeSel->IsExpanded() && pNodeSel->HasChild())
					pNodeSel = pNodeSel->m_pLastChild;
				SetSelNode(pNodeSel);
			}
			else if (nChar == VK_DOWN)
			{
				// move to next visible node
				if (pNodeSel->IsExpanded() && pNodeSel->FirstChild())
				{
					pNodeSel = pNodeSel->FirstChild();
					SetSelNode(pNodeSel);
					Invalidate();
					return;
				}

				if (pNodeSel->NextSibling())
				{
					pNodeSel = pNodeSel->NextSibling();
					SetSelNode(pNodeSel);
					Invalidate();
					return;
				}

				// to its parent's next sibling
				while (pNodeSel->m_pParent->m_pParent)
				{
					pNodeSel = pNodeSel->m_pParent;
					if (pNodeSel->NextSibling())
					{
						pNodeSel = pNodeSel->NextSibling();
						SetSelNode(pNodeSel);
						break;
					}
				}
			}

			Invalidate();
		}
		break;
	case VK_RETURN:
		{
			NotifyEvent(CSkinTreeCtrlEventNotify::C_ENTER);
			break;
		}
	case VK_DELETE:
		{
			NotifyEvent(CSkinTreeCtrlEventNotify::C_KEY_DELETE);
			break;
		}
	}
}

bool CSkinTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
	NotifyEvent(CSkinTreeCtrlEventNotify::C_CLICK);

	return true;
}

bool CSkinTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
{

	return true;
}

void CSkinTreeCtrl::OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt)
{
	if (m_pObjScrollBar)
		m_pObjScrollBar->OnMouseWheel(nWheelDistance, nMkeys, pt);
}

void CSkinTreeCtrl::OnCreate()
{
	m_font.OnCreate(m_pSkin);

	CUIObject::OnCreate();

	m_nLineHeight = m_font.GetHeight() + 2;

	// if in Editor mode, don't create scrollbar
	if (m_pSkin->IsInEditorMode())
		return;

	// Create Scrollbar
	m_pObjScrollBar = m_pSkin->GetSkinFactory()->CreateDynamicCtrl(m_pContainer, CSkinVScrollBar::ClassName(), SKIN_UIOBJ_ID_UNDEFINE, NULL, NULL, NULL, NULL);
	ASSERT(m_pObjScrollBar);
	if (!m_pObjScrollBar)
		return;

	TCHAR		szTemp[MAX_PATH];

	// set left
	_stprintf_s(szTemp, CountOf(szTemp), _T("%s+%s-%s"), m_formLeft.GetFormula(), m_formWidth.GetFormula(),
		m_pObjScrollBar->m_formWidth.GetFormula());
	m_pObjScrollBar->SetProperty(SZ_PN_LEFT, szTemp);

	// top
	m_pObjScrollBar->SetProperty(SZ_PN_TOP, m_formTop.GetFormula());

	// width of this
	_stprintf_s(szTemp, CountOf(szTemp), _T("%s-%s"), m_formWidth.GetFormula(), m_pObjScrollBar->m_formWidth.GetFormula());
	m_formWidth.SetFormula(szTemp);

	// height.
	m_pObjScrollBar->SetProperty(SZ_PN_HEIGHT, m_formHeight.GetFormula());

	m_pContainer->RecalculateUIObjSizePos(m_pObjScrollBar);

	m_pScrollBar = (CSkinVScrollBar*)m_pObjScrollBar;
	m_pScrollBar->SetScrollNotify(this);
}

void CSkinTreeCtrl::OnSize(int cx, int cy)
{
	CUIObject::OnSize(cx, cy);

	UpdateVScrollBar();
}

void CSkinTreeCtrl::OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar)
{
	if (!m_pTreeData)
		return;

	ISkinTreeNode	*pNode;
	pNode = m_pTreeData->GetExpandedNodeByPos(nPos);
	if (pNode)
	{
		m_pTreeData->SetFirstVisibleNode(pNode);
		Invalidate();
	}
	else if (m_pScrollBar)
		m_pScrollBar->SetScrollPos(m_pTreeData->GetFirstVisibleNodePos());
}

void CSkinTreeCtrl::OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar)
{

}

BOOL CSkinTreeCtrl::SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
{
	if (CUIObject::SetProperty(szProperty, szValue))
		return TRUE;

	if (_tcsicmp(szProperty, _T("BgColor")) == 0 ||
		_tcsicmp(szProperty, _T("BkColor")) == 0)
	{
		GetColorValue(m_clrBk, szValue);
	}
	else if (_tcsicmp(szProperty, _T("SelBgColor")) == 0 || 
		_tcsicmp(szProperty, _T("SelBkColor")) == 0)
	{
		GetColorValue(m_clrSelRowBk, szValue);
	}
	else if (_tcsicmp(szProperty, _T("SelTextColor")) == 0)
	{
		GetColorValue(m_clrSelText, szValue);
	}
	else if (m_font.SetProperty(szProperty, szValue))
		return true;
	else if (_tcsicmp(szProperty, _T("ImageList")) == 0)
	{
		m_strImageList = szValue;
		m_imageList.Destroy();
		m_imageList.Load(m_pSkin->GetSkinFactory(), szValue, 16);
	}
	else
		return FALSE;

	return TRUE;
}

#ifdef _SKIN_EDITOR_
void CSkinTreeCtrl::EnumProperties(CUIObjProperties &listProperties)
{
	CUIObject::EnumProperties(listProperties);

	listProperties.AddPropColor(_T("BgColor"), m_clrBk);
	listProperties.AddPropColor(_T("SelBgColor"), m_clrSelRowBk);
	listProperties.AddPropColor(_T("SelTextColor"), m_clrSelText);
	m_font.EnumProperties(listProperties);
	listProperties.AddPropImageFile(_T("ImageList"), m_strImageList.c_str());
}
#endif // _SKIN_EDITOR_

void CSkinTreeCtrl::SetBkColor(const MLColor &clrBk)
{
	m_clrBk = clrBk;
}

void CSkinTreeCtrl::SetSelRowBkColor(const MLColor &clrBk)
{
	m_clrSelRowBk = clrBk;
}

void CSkinTreeCtrl::SetFont(const CFontInfo &font)
{
	m_font.Create(font);
	m_nLineHeight = m_font.GetHeight() + 2;
}

void CSkinTreeCtrl::SetTextColor(const MLColor &clrText)
{
	tstring		strColor;

	ColorToStr(clrText, strColor);
	m_font.SetProperty(_T("TextColor"), strColor.c_str());
}

void CSkinTreeCtrl::NotifyEvent(CSkinTreeCtrlEventNotify::Command cmd)
{
	CSkinTreeCtrlEventNotify	event(this);

	event.cmd = cmd;
	m_pSkin->DispatchUIObjNotify(&event);
}

void CSkinTreeCtrl::SetSelNode(ISkinTreeNode *pNode)
{
	if (!m_pTreeData)
		return;

	if (!pNode->IsUpdated())
	{
		pNode->SetUpdated(true);
		pNode->OnUpdate();
	}

	if (m_pTreeData->SetSelNode(pNode))
	{
		MakeSureSelNodeVisible();
		NotifyEvent(CSkinTreeCtrlEventNotify::C_SEL_CHANGED);
	}
}

void CSkinTreeCtrl::SetFirstVisibleNode(ISkinTreeNode *pNode)
{
	if (!m_pTreeData || !m_pScrollBar)
		return;

	m_pTreeData->SetFirstVisibleNode(pNode);
	m_pScrollBar->SetScrollPos(m_pTreeData->GetFirstVisibleNodePos());
}

void CSkinTreeCtrl::UpdateVScrollBar()
{
	if (!m_pTreeData || !m_pScrollBar)
		return;

	int		nCount = m_pTreeData->GetAllExpandedNodeCount();
	int		nLinePerPage = m_rcObj.Height() / m_nLineHeight;
	if (nLinePerPage == 0)
		nLinePerPage = 1;

	m_pScrollBar->SetScrollInfo(0, nCount, nLinePerPage, m_pTreeData->GetFirstVisibleNodePos(), 1);
}

void CSkinTreeCtrl::MakeSureSelNodeVisible()
{
	if (!m_pTreeData)
		return;

	ISkinTreeNode *pNodeSel = m_pTreeData->GetSelNode();
	int		nLinePerPage = m_rcObj.Height() / m_nLineHeight;
	int		nPosFirstVisibleOld, nPosFirstVisible, nPosCurSel;

	if (!pNodeSel)
		return;

	if (nLinePerPage == 0)
		nLinePerPage = 1;

	nPosFirstVisibleOld = nPosFirstVisible = m_pTreeData->GetFirstVisibleNodePos();
	nPosCurSel = m_pTreeData->GetExpandedNodePos(pNodeSel);

	if (nPosCurSel < nPosFirstVisible)
	{
		if (nPosCurSel == nPosFirstVisible - 1)
			nPosFirstVisible = nPosCurSel;
		else
		{
			while (nPosCurSel < nPosFirstVisible)
				nPosFirstVisible -= nLinePerPage;
			nPosFirstVisible += nLinePerPage;
		}
		if (nPosFirstVisible < 0)
			nPosFirstVisible = 0;
	}
	else if (nPosCurSel >= nPosFirstVisible + nLinePerPage)
	{
		if (nPosCurSel == nPosFirstVisible + nLinePerPage)
			nPosFirstVisible++;
		else
		{
			while (nPosCurSel > nPosFirstVisible)
				nPosFirstVisible += nLinePerPage;
			nPosFirstVisible -= nLinePerPage;

			// nPosFirstVisible = nPosCurSel - nPosCurSel % nLinePerPage + nPosFirstVisible % nLinePerPage;
		}
		if (nPosFirstVisible < 0)
			nPosFirstVisible = 0;
	}
	else
		return;

	if (nPosFirstVisibleOld != nPosFirstVisible)
	{
		ISkinTreeNode *pNode = m_pTreeData->GetExpandedNodeByPos(nPosFirstVisible);
		if (pNode)
		{
			SetFirstVisibleNode(pNode);
			Invalidate();
		}
	}

}
