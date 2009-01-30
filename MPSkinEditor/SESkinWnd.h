// SESkinWnd.h: interface for the CSESkinWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SESKINWND_H__9054D134_A62D_4BB6_955C_6BA510528E1F__INCLUDED_)
#define AFX_SESKINWND_H__9054D134_A62D_4BB6_955C_6BA510528E1F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum SkinNotifyMsg
{
	SNM_UPDATEPROPERTYITEM		= WM_USER + 1,		// wparam: propname, lparam: value

};

interface ISESkinNotify
{
	virtual void OnSkinWndNameChanged() = 0;
	virtual void OnUIObjNameChanged(CUIObject *pObj) = 0;
	virtual void OnUIObjectListChanged() = 0;
	virtual void OnUIObjFocusChanged() = 0;
	virtual void OnFocusUIObjPropertyChanged(LPCTSTR szPropertyName, LPCTSTR szNewValue) = 0;
	virtual void TrackPopupMenu(HMENU hMenu, int x, int y) = 0;
};

class CSESkinWnd : public CSkinWnd  
{
public:
	CSESkinWnd();
	virtual ~CSESkinWnd();

	int Create(LPCTSTR szClassName, LPCTSTR szCaption, CSkinFactory *pSkinFactory, LPCTSTR szSkinWndName, CBaseWnd *pWndParent);

	virtual void NewSkin(LPCTSTR szSkinWndName);

	virtual int OpenSkin(LPCTSTR szSkinWndName);

	virtual int CopyFrom(LPCTSTR szSkinWndName, CSESkinWnd *pSkinWndSrc);

	void OnSkinLoaded();

	bool IsUIObjectExist(CUIObject *pObj);

	int InsertUIObject(CUIObject *pObj);


	void SetNOFocusUIObj();

	bool RemoveFocusUIObject();

	void OnKeyDown(UINT nChar, UINT nFlags);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnMouseMove(UINT nFlags, CPoint point);

	void OnPaint(IMLGraphicsBase *pGraph);
	void OnContexMenu(int xPos, int yPos);
	void OnRButtonUp(UINT nFlags, CPoint point);

	LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);

	BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);

	void EnumProperties(CUIObjProperties &listProperties);

	void ZoomIn();
	void ZoomOut();

	void SetNotify(ISESkinNotify *pNotify) { m_pNotify = pNotify; }
	ISESkinNotify *GetNotify() { return m_pNotify; }

protected:
	HMENU		m_hMenuContext;
	ISESkinNotify	*m_pNotify;

	// other properties...
	tstring		m_strCmdHandler;
	tstring		m_strContextMenu;
	bool		m_bEnableVolumeSwitch;
	bool		m_bIndividualProperties;

	bool		m_bEnableTransparentFeatureEditing;
	bool		m_bTranslucencyLayeredEditing;
	
};

#endif // !defined(AFX_SESKINWND_H__9054D134_A62D_4BB6_955C_6BA510528E1F__INCLUDED_)
