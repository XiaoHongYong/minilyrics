// SkinContainer.h: interface for the CSkinContainer class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <stack>

class CMLMenu;

class CSkinContainer : public CUIObject
{
	UIOBJECT_CLASS_NAME_DECLARE(CUIObject)
public:
	enum ResultCode
	{
		RESULT_OK = 0,
		RESULT_CANCELD = 1,
	};

	CSkinContainer();
	virtual ~CSkinContainer();

	virtual void OnInitialUpdate();
	virtual void OnDestroy();

	virtual void OnKeyUp(UINT nChar, UINT nFlags);
	virtual void OnKeyDown(UINT nChar, UINT nFlags);
	virtual bool OnMouseMove(UINT nFlags, CPoint point);
	virtual bool OnLButtonDown(UINT nFlags, CPoint point);
	virtual bool OnLButtonDblClk(UINT nFlags, CPoint point);
	virtual bool OnLButtonUp(UINT nFlags, CPoint point);
	virtual bool OnRButtonUp(UINT nFlags, CPoint point);
	virtual bool OnRButtonDown(UINT nFlags, CPoint point);
	virtual void OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt);

	virtual bool OnMenuKey(UINT nChar, UINT nFlags);

	virtual void OnSize();

	virtual bool OnCommand(int nID);
	virtual bool OnCustomCommand(int nId);

	virtual void OnSwitchTo();

	// For default result code, please see ResultCode definition.
	virtual void OnPageViewResult(int nRequestCodeOfPage, int nResultCode) { }

	virtual bool OnUserMessage(int nMessageID, LPARAM param);

	virtual bool OnClose();	// Return false, if don't want to be closed.
	virtual bool OnOK();
	virtual bool OnCancel();

	void Draw(CRawGraph *pGraph);

	int FromXML(SXNode *pXmlNode);
	void CreateChild(SXNode *pXmlNode);

	virtual BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
	virtual BOOL SetProperty(LPCTSTR szProperty, SXNode *pXmlNode);
#ifdef _SKIN_EDITOR_
	void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

	virtual void OnLanguageChanged();

public:
	// functions for CSkinWnd
	void Destroy();

	CUIObject *GetUIObjectByClassName(LPCTSTR szClassName);
	CUIObject *GetUIObjectById(int nId, LPCTSTR szClassName);
	CUIObject *GetUIObjectById(LPCTSTR szId, LPCTSTR szClassName);

	bool EnableUIObject(int nId, bool bEnable = true, bool bRedraw = true);
	bool EnableUIObject(LPCTSTR szId, bool bEnable = true, bool bRedraw = true);

	bool SetUIObjectProperty(int nId, LPCTSTR szProperty, LPCTSTR szValue);
	bool SetUIObjectProperty(LPCTSTR szId, LPCTSTR szProperty, LPCTSTR szValue);

	void SetUIObjectVisible(int nId, bool bVisible, bool bRedraw = true);
	void SetUIObjectVisible(LPCTSTR szId, bool bVisible, bool bRedraw = true);

	void InvalidateUIObject(int nId);
	void InvalidateUIObject(LPCTSTR szId);

	tstring GetUIObjectText(int nId);
	tstring GetUIObjectText(LPCTSTR szId);

	bool GetUIObjectRect(int nId, CRect &rc);
	bool GetUIObjectRect(LPCTSTR szId, CRect &rc);

	void SetUIObjectText(int nId, LPCTSTR szText, bool bRedraw = true);
	void SetUIObjectText(LPCTSTR szId, LPCTSTR szText, bool bRedraw = true);

	void CheckButton(int nId, bool bCheck);
	void CheckButton(LPCTSTR szId, bool bCheck);
	bool IsButtonChecked(int nId);
	bool IsButtonChecked(LPCTSTR szId);

	void CheckToolbarButton(int nToolbarId, int nButtonId, bool bCheck);
	void CheckToolbarButton(LPCTSTR szToolbarId, LPCTSTR szButtonId, bool bCheck);

	void SetFocusUIObject(LPCTSTR szId);
	void SetFocusUIObject(int nId);
	CUIObject *GetFocusUIObject();
	void SetFocusChild(CUIObject *pObj);
	int FocusToNext();
	int FocusToPrev();

	bool RemoveUIObject(CUIObject *pObj, bool bFree);

	CUIObject *RemoveUIObjectById(int nId);

	bool AddUIObject(CUIObject *pObj);
	bool InsertUIObjectAt(CUIObject *pObjWhere, CUIObject *pObj);

	virtual void RecalculateUIObjSizePos(CUIObject *pObj);
	BOOL RecalculateVarValue(CFormula &form, int &nRetValue);

	CMLMenu *GetMenu() { return m_pMenu; }

	//
	// CSkinContainer API
	//
	bool IsChild(CUIObject *pObj);

	virtual void InvalidateUIObject(CUIObject *pObj);

	virtual void UpdateMemGraphicsToScreen(const RECT* lpRect, CUIObject *pObjCallUpdate);

	int GetChildrenCount() { return m_vUIObjs.size(); }
	CUIObject *GetChildByIndex(int nIndex)
		{ if (nIndex >= 0 && nIndex < (int)m_vUIObjs.size()) return m_vUIObjs[nIndex]; else return NULL; }

	CUIObject *GetChildByClass(LPCTSTR szClassName);

	virtual bool IsContainer() const { return true; }
	virtual CSkinContainer *GetContainerIf() { return this; }

	virtual void OnAdjustHue(float hue, float saturation, float luminance);
	virtual void OnSkinFontChanged();

	virtual CRawGraph *GetMemGraph();

	MLColor GetBgColor() const;
	bool IsDrawBgImage() const;

	void RedrawBackground(CRawGraph *pGraph, const RECT &rc);

	void DispatchOnCreateMsg();

	// Page view APIs
	// void SwitchToPage(int nIdPage, bool bWaitResultOfNextPage, int nRequestCodeOfNextPage, bool bAnimation);
	void SwitchToPage(LPCTSTR szPageClass, bool bWaitResultOfNextPage, int nRequestCodeOfNextPage, bool bAnimation);
	void SwitchToLastPage(int nResultCodeOfPage, bool bAnimation);
	bool HasActivePage() const { return m_vStackPageView.size() > 0; }

	void SetExPool(LPCTSTR szProperty, LPCTSTR szValue);
	void SetExPool(LPCTSTR szProperty, int nValue);

	tstring GetExPoolStr(LPCTSTR szProperty, LPCTSTR szDefault = _T(""));
	int GetExPoolInt(LPCTSTR szProperty, int nDefault = 0);
	bool GetExPoolBool(LPCTSTR szProperty, bool bDefault = false);

#ifdef _DEBUG
	void DumpUIObject(int &nDeep);
#endif

protected:
	bool IsPageAtLeft(CSkinContainer *pPage, CSkinContainer *pPage2);

	void OnAdd(CUIObject *pObj);

protected:
#ifdef _SKIN_EDITOR_
	virtual bool HasXMLChild();
	virtual void OnToXMLChild(CXMLWriter &xmlStream);
#endif // _SKIN_EDITOR_

	friend class CSkinWnd;

protected:
	// All the children UIObject

	VecUIObjects		m_vUIObjs;
	int					m_nFocusUIObj;

	bool				m_bClipChildren;
	tstring				m_strContextMenu;
	CMLMenu				*m_pMenu;

	// Background color
	MLColor				m_clrBg, m_clrBgOrg;
	bool				m_bDrawBg;

	// Background images
	CScaleImagePainter	m_bgImagePainter;
	CSFImage			m_imageBg, m_imageBgMask;
	BlendPixMode		m_BgBpm;

	// Status for Page view
	struct PageViewItem
	{
		bool				bWaitResultOfNextPage;
		int					nRequestCodeOfNextPage;
		CSkinContainer		*pContainerPage;
		PageViewItem(CSkinContainer *pContainerPage, bool bWaitResultOfNextPage, int nRequestCodeOfNextPage)
		{
			this->bWaitResultOfNextPage = bWaitResultOfNextPage;
			this->nRequestCodeOfNextPage = nRequestCodeOfNextPage;
			this->pContainerPage = pContainerPage;
		}
	};
	typedef std::stack<PageViewItem>	VecStackPageView;
	VecStackPageView	m_vStackPageView;
	tstring				m_strDefaultPageClass;

};

class CSkinClientArea : public CSkinContainer
{
	UIOBJECT_CLASS_NAME_DECLARE(CSkinContainer)
public:
	virtual BOOL SetProperty(LPCTSTR szProperty, SXNode *pXmlNode);
	virtual BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue)
		{ return CSkinContainer::SetProperty(szProperty, szValue); }

};
