// SkinTreeCtrl.h: interface for the CSkinTreeCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINTREECTRL_H__03A1FDA7_659B_415F_949B_A3F1549926F3__INCLUDED_)
#define AFX_SKINTREECTRL_H__03A1FDA7_659B_415F_949B_A3F1549926F3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSkinTreeCtrlEventNotify : public IUIObjNotify
{
public:
	CSkinTreeCtrlEventNotify (CUIObject *pObject) : IUIObjNotify(pObject) { cmd = C_CLICK; }
	enum Command
	{
		C_CLICK,
		C_DBL_CLICK,
		C_SEL_CHANGED,
		C_ENTER,
		C_KEY_DELETE,
	};

	Command				cmd;

};

class ISkinTreeNode;

typedef vector<ISkinTreeNode*>		V_SkinTreeNode;
typedef vector<int>					SkinTreePath_t;
typedef vector<tstring>				SkinTreeStrPath_t;

#define SKIN_TREE_INVALID_IMG_INDEX		0xFF

class ISkinTreeNode
{
public:
	enum
	{
		STNF_EXPANDED		= 1,
		STNF_UPDATED		= 1 << 1,
		STNF_SELECTED		= 1 << 2,
	};

	ISkinTreeNode()
	{
		m_nImageIndex = 0;
		m_nExpandedImageIndex = SKIN_TREE_INVALID_IMG_INDEX;
		m_nFlags = STNF_UPDATED;
// 		m_bExpanded = false;
// 		m_bUpdated = true;
// 		m_bSelected = false;
		m_pParent = NULL;
		m_pPrev = NULL;
		m_pNext = NULL;
		m_pFirstChild = NULL;
		m_pLastChild = NULL;
	}
	virtual ~ISkinTreeNode()
	{
		Free();
	}

	virtual void Free()
	{
		ISkinTreeNode		*p, *pNext = NULL;
		for (p = m_pFirstChild; p != NULL; p = pNext)
		{
			pNext = p->m_pNext;
			delete p;
		}
		m_pFirstChild = NULL;
		m_pLastChild = NULL;
	}

	void AddChildBack(ISkinTreeNode *pChild);
	void AddChildFront(ISkinTreeNode *pChild);
	void RemoveLink();

	ISkinTreeNode *FirstChild() { return m_pFirstChild; }
	ISkinTreeNode *LastChild() { return m_pLastChild; }
	ISkinTreeNode *PrevSibling() { return m_pPrev; }
	ISkinTreeNode *NextSibling() { return m_pNext; }

	ISkinTreeNode *GetChildByIndex(int nIndex);
	ISkinTreeNode *GetChildByName(LPCTSTR szName);
	int GetDepth();
	bool IsBigBrother();
	bool NoChild() { return m_pFirstChild == NULL; }
	bool HasChild() { return m_pFirstChild != NULL; }

	void SetExpanded(bool bExpanded) { if (bExpanded) m_nFlags |= STNF_EXPANDED; else m_nFlags &= ~STNF_EXPANDED; }
	bool IsExpanded() { return (m_nFlags & STNF_EXPANDED) != 0; }

	void SetUpdated(bool bUpdated) { if (bUpdated) m_nFlags |= STNF_UPDATED; else m_nFlags &= ~STNF_UPDATED; }
	bool IsUpdated() { return (m_nFlags & STNF_UPDATED) != 0; }

	void SetSelected(bool bSelcted) { if (bSelcted) m_nFlags |= STNF_SELECTED; else m_nFlags &= ~STNF_SELECTED; }
	bool IsSelected() { return (m_nFlags & STNF_SELECTED) != 0; }

	void GetPath(SkinTreePath_t &path);
	void GetPath(SkinTreeStrPath_t &path);

	// visible == expanded node
	int GetExpandedNodeCount();
	int GetExpandedNodeCountTill(ISkinTreeNode *pNodeTill, bool &bFound);
	ISkinTreeNode *GetExpandedNodeByPos(int &nPos);

	// ISkinTreeNode *LastVisibleNode();
	// Tree notify messages...
	virtual void OnUpdate() { }

public:
	tstring					m_strName;
	BYTE					m_nImageIndex;
	BYTE					m_nExpandedImageIndex;
	WORD					m_nFlags;
// 	bool					m_bExpanded;
// 	bool					m_bSelected;
// 	bool					m_bUpdated;
	ISkinTreeNode			*m_pParent, *m_pPrev, *m_pNext, *m_pFirstChild, *m_pLastChild;

};

class ISkinTree
{
public:
	ISkinTree();
	virtual ~ISkinTree();

	virtual void GetCurPath(SkinTreePath_t &path);
	virtual void GetCurPath(SkinTreeStrPath_t &path);
	virtual ISkinTreeNode *GetSelNode();
	virtual ISkinTreeNode *GetFirstVisibleNode();

	int GetAllExpandedNodeCount();
	int GetFirstVisibleNodePos();

	int GetExpandedNodePos(ISkinTreeNode *pNode);
	ISkinTreeNode *GetExpandedNodeByPos(int nPos);

	// if selected node changed, return true.
	bool SetSelNode(ISkinTreeNode *pNode);
	void SetFirstVisibleNode(ISkinTreeNode *pNode);

	ISkinTreeNode *GetRoot() { return &m_root; }

	ISkinTreeNode *GetNode(SkinTreePath_t &path);
	ISkinTreeNode *GetNode(SkinTreeStrPath_t &path);

public:
	// Tree node visitor
	static ISkinTreeNode *CenterOrderNextExpandedOnly(ISkinTreeNode *pNode);

protected:
	ISkinTreeNode			m_root;	// Root Node isn't Visible
	SkinTreePath_t			m_pathSel;
	SkinTreePath_t			m_pathFirstVisible;

};

class CSkinTreeCtrl : public CUIObject, public IScrollNotify
{
	UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	enum TreeArea
	{
		TA_BLANK,
		TA_LINE,
		TA_EXPAND,
	};

	CSkinTreeCtrl();
	~CSkinTreeCtrl();

	virtual void SetDataSrc(ISkinTree *pTree) { m_pTreeData = pTree; }

	void Draw(CRawGraph *pGraph);

	TreeArea HitTest(POINT point, ISkinTreeNode **ppNode);

	void OnKeyDown(UINT nChar, UINT nFlags);

	bool OnLButtonUp(UINT nFlags, CPoint point);
	bool OnLButtonDown(UINT nFlags, CPoint point);
	bool OnLButtonDblClk(UINT nFlags, CPoint point);
	bool OnMouseMove(UINT nFlags, CPoint point);
	void OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt);

	void OnCreate();
	void OnSize(int cx, int cy);

	virtual void OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar);
	virtual void OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar);


	virtual BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

	void EnumProperties(CUIObjProperties &listProperties);

protected:
	void SetBkColor(const MLColor &clrBk);
	void SetSelRowBkColor(const MLColor &clrBk);
	void SetFont(const CMLFont &font);
	void SetTextColor(const MLColor &clrText);

	void NotifyEvent(CSkinTreeCtrlEventNotify::Command cmd);

	void SetSelNode(ISkinTreeNode *pNode);
	void SetFirstVisibleNode(ISkinTreeNode *pNode);

	void UpdateVScrollBar();

	void MakeSureSelNodeVisible();

protected:
	ISkinTree				*m_pTreeData;

	IScrollBar				*m_pScrollBar;
	CUIObject				*m_pObjScrollBar;

	CImgList				m_imageList;
	tstring					m_strImageList;

	MLColor					m_clrBk, m_clrSelRowBk;
	MLColor					m_clrSelText;
	CMLBrush				m_brBk, m_brSelBk;
	CSkinFontProperty		m_font;
	int						m_nLineHeight;
	int						m_nXMargin;
	int						m_nLineIndent;

};

#endif // !defined(AFX_SKINTREECTRL_H__03A1FDA7_659B_415F_949B_A3F1549926F3__INCLUDED_)
