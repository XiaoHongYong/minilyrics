// SkinContainer.h: interface for the CSkinContainer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINCONTAINER_H__C8C49C75_1237_4E52_A4AC_72AFC3907E3E__INCLUDED_)
#define AFX_SKINCONTAINER_H__C8C49C75_1237_4E52_A4AC_72AFC3907E3E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMLMenu;


class CSkinContainer : public CUIObject
{
	UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	CSkinContainer();
	virtual ~CSkinContainer();

	virtual bool OnMouseMove(UINT nFlags, CPoint point);
	virtual bool OnLButtonDown(UINT nFlags, CPoint point);
	virtual bool OnLButtonDblClk(UINT nFlags, CPoint point);
	virtual bool OnLButtonUp(UINT nFlags, CPoint point);
	virtual bool OnRButtonUp(UINT nFlags, CPoint point);
	virtual bool OnRButtonDown(UINT nFlags, CPoint point);
	virtual void OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt);

	virtual bool OnMenuKey(UINT nChar, UINT nFlags);

	virtual void OnSize(int cx, int cy);

	void Draw(CRawGraph *pGraph);

	int FromXML(CSkinContainer *pContainer, SXNode *pXmlNode);

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
	void EnumProperties(CUIObjProperties &listProperties);

public:
	// functions for CSkinWnd
	void Destroy();

	CUIObject *GetUIObjectByClassName(LPCTSTR szClassName);
	CUIObject *GetUIObjectById(int nId, LPCTSTR szClassName);

	bool RemoveUIObject(CUIObject *pObj, bool bFree);

	CUIObject *RemoveUIObjectById(int nId);

	bool AddUIObject(CUIObject *pObj);
	bool InsertUIObjectAt(CUIObject *pObjWhere, CUIObject *pObj);
	bool AddUIObjectToFront(CUIObject *pObj);
	
	void RecalculateSizePos();
	void RecalculateUIObjSizePos(CUIObject *pObj);

	CMLMenu *GetMenu() { return m_pMenu; }

	virtual bool OnCommand(int nID);

	//
	// CSkinContainer API
	//
	CUIObjectIterator ChildBegin() { CUIObjectIterator it(this); it.Begin(); return it; }
	CUIObjectIterator ChildEnd() { CUIObjectIterator it(this); it.End(); return it; }

	virtual void InvalidateUIObject(CUIObject *pObj);

	virtual void UpdateMemGraphicsToScreen(const RECT* lpRect, CUIObject *pObjCallUpdate);

	int GetChildrenCount() { return m_vUIObjs.size(); }
	CUIObject *GetChildByIndex(int nIndex)
		{ if (nIndex >= 0 && nIndex < (int)m_vUIObjs.size()) return m_vUIObjs[nIndex]; else return NULL; }

	virtual bool IsContainer() const { return true; }
	virtual CSkinContainer *GetContainerIf() { return this; }

	virtual void OnLanguageChanged();

	virtual void OnAdjustHue(float hue, float saturation, float luminance);
	virtual void OnSkinFontChanged();

	virtual CRawGraph *GetMemGraph();

	void DispatchOnCreateMsg();

protected:
	virtual bool HasXMLChild();
	virtual void OnToXMLChild(CXMLWriter &xmlStream);

	friend class CSkinWnd;
	friend class CUIObjectIterator;

protected:
	// All the children UIObject
	typedef vector<CUIObject *>		V_UIOBJECT;

	V_UIOBJECT			m_vUIObjs;

	tstring				m_strContextMenu;
	CMLMenu				*m_pMenu;

};

#endif // !defined(AFX_SKINCONTAINER_H__C8C49C75_1237_4E52_A4AC_72AFC3907E3E__INCLUDED_)
