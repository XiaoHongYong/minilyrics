// SkinDroplistCtrl.h: interface for the CSkinDroplistCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINDROPLISTCTRL_H__D9E4286C_5FAD_4DC4_A1BE_E6A36626E899__INCLUDED_)
#define AFX_SKINDROPLISTCTRL_H__D9E4286C_5FAD_4DC4_A1BE_E6A36626E899__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "SkinTextButton.h"

class IDropListCtrlNotify
{
public:
	virtual void OnSelChanged() = 0;
};

template<class _DataType>
class CTree
{
public:
	struct Node
	{
		vector<Node *>		vChildren;
		_DataType			dataNode;
	};

	typedef vector<Node *>	V_CHILDREN;

public:
	CTree()
	{
		m_pRootNode = new Node;
		m_pCurNode = m_pRootNode;
		m_nCurNodeIndex = 0;
	}
	~CTree()
	{
		FreeNode(m_pRootNode);
	}

	void Clear()
	{
		m_vPath.clear();
		m_pCurNode = m_pRootNode;
		m_nCurNodeIndex = 0;
		EraseAllChildren();
	}

	void ChToRoot()
	{
		m_nCurNodeIndex = 0;
		m_pCurNode = m_pRootNode;
		m_vPath.clear();
	}

	bool ChToParent()
	{
		ASSERT(!m_vPath.empty());
		if (m_vPath.empty())
			return false;

		m_nCurNodeIndex = m_vPath.back();
		m_vPath.pop_back();

		if (!ChToPath(m_vPath))
			return false;

		return true;
	}

	bool ChToChild(int nIndex)
	{
		if (nIndex < 0 || nIndex >= (int)m_pCurNode->vChildren.size())
			return false;
		m_pCurNode = m_pCurNode->vChildren[nIndex];
		m_nCurNodeIndex = nIndex;
		m_vPath.push_back(nIndex);

		return true;
	}

	_DataType &GetCurNodeData()
	{
		return m_pCurNode->dataNode;
	}

	_DataType &GetParentNodeData()
	{
		Node		*pNode = m_pRootNode;
		for (int i = 0; i < (int)m_vPath.size() - 1; i++)
		{
			if ((size_t)m_vPath[i] < pNode->vChildren.size())
			{
				pNode = pNode->vChildren[m_vPath[i]];
			}
		}

		return pNode->dataNode;
	}

	int GetChildrenCount()
	{
		return m_pCurNode->vChildren.size();
	}

	_DataType &GetChild(int nIndex)
	{
		ASSERT(nIndex >= 0 && nIndex < (int)m_pCurNode->vChildren.size());

		if (nIndex >= 0 && nIndex < (int)m_pCurNode->vChildren.size())
			return m_pCurNode->vChildren[nIndex]->dataNode;
		else
			return m_pRootNode->dataNode;
	}

	void InsertChild(int nIndex, _DataType data)
	{
		Node	*pNode = new Node;
		pNode->dataNode = data;
		m_pCurNode->vChildren.insert(m_pCurNode->vChildren.begin() + nIndex, pNode);
	}

	void AddChild(_DataType data)
	{
		Node	*pNode = new Node;
		pNode->dataNode = data;
		m_pCurNode->vChildren.push_back(pNode);
	}

	void EraseChild(int nIndex)
	{
		Node	*pNode;

		if (nIndex >= 0 && nIndex < (int)m_pCurNode->vChildren.size())
		{
			pNode = m_pCurNode->vChildren[nIndex];
			m_pCurNode->vChildren.erase(m_pCurNode->vChildren.begin() + nIndex);
			FreeNode(pNode);
		}
	}

	void EraseAllChildren()
	{
		for (int i = 0; i < (int)m_pCurNode->vChildren.size(); i++)
		{
			FreeNode(m_pCurNode->vChildren[i]);
		}
		m_pCurNode->vChildren.clear();
	}

	void GetPath(vector<_DataType> &vPath)
	{
		Node		*pNode = m_pRootNode;
		vPath.clear();
		for (int i = 0; i < (int)m_vPath.size(); i++)
		{
			if (m_vPath[i] < (int)pNode->vChildren.size())
			{
				pNode = pNode->vChildren[m_vPath[i]];
				vPath.push_back(pNode->dataNode);
			}
		}
	}

protected:
	bool ChToPath(vector<int> &vPath)
	{
		if (vPath.empty())
		{
			m_pCurNode = NULL;
			m_nCurNodeIndex = 0;
			return true;
		}

		if (vPath[0] >= (int)m_pRootNode->vChildren.size())
			return false;
		m_pCurNode = m_pRootNode->vChildren[vPath[0]];
		m_nCurNodeIndex = vPath[0];

		for (int i = 1; i < (int)vPath.size(); i++)
		{
			if (vPath[i] >= (int)m_pCurNode->vChildren.size())
				return false;
			m_pCurNode = m_pCurNode->vChildren[vPath[i]];
			m_nCurNodeIndex = vPath[i];
		}

		return true;
	}

	void FreeNode(Node *pNode)
	{
		for (int i = 0; i < (int)pNode->vChildren.size(); i++)
		{
			FreeNode(pNode->vChildren[i]);
		}

		delete pNode;
	}

protected:
	// V_ITEMS					m_root;
	Node					*m_pRootNode;
	vector<int>				m_vPath;
	int						m_nCurNodeIndex;
	Node					*m_pCurNode;

};

class CDropListCtrl : public CBaseWnd
{
public:
	struct Item
	{
		int			nImageIndex;
		tstring		strText;
	};
	CDropListCtrl();
	~CDropListCtrl();

	void SetStyle(LPCTSTR szFontName, int nFontHeight, MLColor &clrText, MLColor &clrBg, MLColor &clrSelBg);

	bool DropList(int x, int y, CBaseWnd *pWndParent, int nID, CImgList *pImageList, IDropListCtrlNotify *pNotify);

	void Clear();
	void Append(int nImageIndex, LPCTSTR szText);

	void GetSelPath(vector<tstring> &vPath);

	void OnKillFocus();

	void OnKeyDown(UINT nChar, UINT nFlags);
	void OnKeyUp(UINT nChar, UINT nFlags);
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnLButtonDown(UINT nFlags, CPoint point);

	void OnPaint(IMLGraphicsBase *pGraph);

	void OnDestroy();

#ifdef _WIN32
	LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);
#endif

protected:
	void Paint(IMLGraphicsBase *pGraph, bool bResize = false);

	void NotifySelChanged();

protected:
	vector<Item>		m_vItems;
	int					m_nID;
	CBaseWnd			*m_pWndParent;
	CImgList			*m_pImageList;

	CFontInfo			m_font;
	CMLBrush			m_brBg, m_brSelBg;
	MLColor				m_clrText, m_clrBg, m_clrSelBg;

	int					m_nLineHeight;
	int					m_nRightMargin;
	int					m_nSel;

	int					m_x, m_y;

	IDropListCtrlNotify	*m_pNotify;

};

class CSkinDroplistCtrlEventNotify : public IUIObjNotify
{
public:
	CSkinDroplistCtrlEventNotify(CUIObject *pObject) : IUIObjNotify(pObject) { cmd = C_SEL_CHANGED; }
	enum Command
	{
		C_SEL_CHANGED,
	};

	Command				cmd;

};


class CSkinDroplistCtrl : public CSkinTextButton, public IDropListCtrlNotify
{
public:
	CSkinDroplistCtrl();
	virtual ~CSkinDroplistCtrl();

	LPCTSTR GetClassName();

	void Draw(CRawGraph *pGraph);

	bool OnLButtonUp(UINT nFlags, CPoint point);

	virtual void OnSelChanged();

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

	void Clear()
	{
		m_droplist.Clear();
	}

	void Append(int nImageIndex, LPCTSTR szText)
	{
		m_nImageIndex = nImageIndex;
		m_strName = szText;
		m_droplist.Append(nImageIndex, szText);
	}

	void GetSelPath(vector<tstring> &vPath)
	{
		m_droplist.GetSelPath(vPath);
	}

public:
	static LPCTSTR ClassName() { return ms_szClassName; }
	BOOL IsKindOf(LPCTSTR szClassName);

protected:
	static LPCTSTR		ms_szClassName;

	CImgList			m_imageList;
	CDropListCtrl		m_droplist;

	int					m_nImageIndex;

	tstring				m_strImgListFile;
	MLColor				m_clrSelBg;

};

#endif // !defined(AFX_SKINDROPLISTCTRL_H__D9E4286C_5FAD_4DC4_A1BE_E6A36626E899__INCLUDED_)
