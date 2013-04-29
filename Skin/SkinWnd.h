/********************************************************************
	Created  :	2001/07/06
	FileName :	SkinWnd.h
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#if !defined(AFX_SKIN_H__A1FDC39A_7098_11D5_9E04_02608CAD9330__INCLUDED_)
#define AFX_SKIN_H__A1FDC39A_7098_11D5_9E04_02608CAD9330__INCLUDED_

#pragma once

class CSkinWnd;
class CSkinFactory;
class CSkinContainer;

#define UIOBJ_ID_BEGIN_ALLOC		0xFFFFF

// COMMENT:
//		窗口的位置是否合法（显示位置是否正常）
BOOL IsWndOutOfScreen(const CRect &rc);

// 0, 0, 10, 20
bool GetRectValue(LPCTSTR szValue, CSFImage &image);
bool GetRectValue(LPCTSTR szValue, long &x, long &y, long &cx, long &cy);
bool GetRectValue(LPCTSTR szValue, RECT &rc);

bool inline IsPropertyName(LPCTSTR szName, LPCTSTR szStr) { return _tcsicmp(szName, szStr) == 0; }

void PreMultiplyRGBColr(MLColor &clr, byte alpha);

tstring ColorToStr(const MLColor &clr);

// "#FFFFFF"
void GetColorValue(MLColor &clr, LPCTSTR szColor);

int GetMenuKey(LPCTSTR szText);

//
// Base interface of UIObject Notification data
//
class IUIObjNotify
{
public:
	IUIObjNotify(CUIObject *pObject)
	{
		ASSERT(pObject);
		pUIObject = pObject;
		nID = pObject->m_nId;
	}
	virtual LPCTSTR GetClassName() { if (pUIObject) return pUIObject->GetClassName(); else return _T(""); }
	virtual BOOL IsKindOf(LPCTSTR szClassName) { if (pUIObject) return pUIObject->IsKindOf(szClassName); else return false; }

	int			nID;
	CUIObject	*pUIObject;

};


//
// Base interface of handle IUIObjNotify.
//
class IUIObjNotifyHandler
{
public:
	virtual void OnUIObjNotify(IUIObjNotify *pNotify) = 0;

};


class IUICheckStatus
{
public:
	virtual bool GetChecked(UINT nID, bool &bChecked) = 0;
	virtual bool GetRadioChecked(vector<UINT> &vIDs, UINT &nIDChecked) = 0;
};

class CSkinWnd :
	public CBaseWnd,
	public ISkinWndDragHost,
	public IUICheckStatus
{
public:
	CSkinWnd();
	virtual ~CSkinWnd();

public:
	virtual int Create(SkinWndStartupInfo &skinWndStartupInfo, CSkinFactory *pSkinFactory, bool bToolWindow = true, bool bTopmost = false, bool bVisible = true);

	virtual bool Destroy();

	virtual int OpenDefaultSkin();
	virtual int OpenSkin(LPCTSTR szSkinWndName);
	virtual void CloseSkin();

	//
	// UIObject related functions
	//
	CUIObject * GetUIObjectByClassName(LPCTSTR szClassName);
	CUIObject * GetUIObjectById(int nId, LPCTSTR szClassName = NULL);
	CUIObject *RemoveUIObjectById(int nId);
	bool RemoveUIObject(CUIObject *pObj, bool bFree = false);

	void SetFocusUIObject(int nId);

	CUIObject *GetFocusUIObj();

	CSkinContainer *GetRootContainer() { return &m_rootConainter; }

	bool EnableUIObject(int nId, bool bEnable = true, bool bRedraw = true);

	bool SetUIObjectProperty(int nId, LPCTSTR szProperty, LPCTSTR szValue);
	bool SetUIObjectProperty(LPCTSTR szId, LPCTSTR szProperty, LPCTSTR szValue)
		{ return SetUIObjectProperty(m_pSkinFactory->GetIDByName(szId), szProperty, szValue); }

	void SetUIObjectVisible(int nId, bool bVisible, bool bRedraw = true);
	void SetUIObjectVisible(LPCTSTR szId, bool bVisible, bool bRedraw = true)
		{ SetUIObjectVisible(m_pSkinFactory->GetIDByName(szId), bVisible, bRedraw); }

	void InvalidateUIObject(int nId);
	void InvalidateUIObject(LPCTSTR szId)
		{ InvalidateUIObject(m_pSkinFactory->GetIDByName(szId)); }

	tstring GetUIObjectText(int nId);
	tstring GetUIObjectText(LPCTSTR szId)
		{ return GetUIObjectText(m_pSkinFactory->GetIDByName(szId)); }

	void SetUIObjectText(int nId, LPCTSTR szText, bool bRedraw = true);
	void SetUIObjectText(LPCTSTR szId, LPCTSTR szText, bool bRedraw = true)
		{ SetUIObjectText(m_pSkinFactory->GetIDByName(szId), szText, bRedraw); }

	void CheckButton(int nId, bool bCheck);
	bool IsButtonChecked(int nId);

	void CheckToolbarButton(int nToolbarId, int nButtonId, bool bCheck);
	void CheckToolbarButton(LPCTSTR szToolbarId, LPCTSTR szButtonId, bool bCheck);

	// register special messages
	void SetCaptureMouse(CUIObject *pUIObj);
	CUIObject *GetCaptureMouse();
	void ReleaseCaptureMouse(CUIObject *pUIObj);

	void UnregisterTimerObject(CUIObject *pObj);
	void UnregisterTimerObject(CUIObject *pObj, int nTimerId);
	int RegisterTimerObject(CUIObject *pObj, int nTimeDuration);

	void RegisterHandleContextMenuCmd(CUIObject *pObj) { m_pUIObjHandleContextMenuCmd = pObj; }

	int AllocAutoUniID() { return ++m_nNextAuoUniID; }

	int AllocUIObjID() { return ++m_nNextUIObjIDAlloc; }

	// update draw of UIObject
	virtual void InvalidateUIObject(CUIObject *pObj);

	void EnterInDrawUpdate();
	void LeaveInDrawUpdate();
	void CancelDrawUpdate() { m_nInRedrawUpdate--; ASSERT(m_nInRedrawUpdate >= 0); }

	CSkinFactory *GetSkinFactory() const { return m_pSkinFactory; }

	void SetMainAppWnd(bool bMainAppWnd);
	bool IsMainAppWnd() const { return m_bMainAppWnd; }
	bool IsDialogWnd() const { return m_bDialogWnd; }

	bool IsInEditorMode() const { return m_bInEditorMode; }

public:
	// ISkinWndDragHost
	virtual void GetWndDragAutoCloseTo(vector<CBaseWnd *> &vWnd);
	virtual void GetWndDragTrackMove(vector<CBaseWnd *> &vWnd);

public:
	// IUICheckStatus interface
	virtual bool GetChecked(UINT nID, bool &bChecked) { return false; }
	virtual bool GetRadioChecked(vector<UINT> &vIDs, UINT &nIDChecked) { return false; }

public:
	//
	// Messages of the window
	//
	virtual void OnActivate(bool bActived);
	virtual void OnPreCreate(bool &bTopmost, bool &bVisible) { }
	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void OnPaint(CMLGraphics *pGraph, CRect &rcClip);

	virtual void OnContexMenu(int xPos, int yPos);

	virtual void OnCommand(UINT uID, UINT nNotifyCode);

	virtual void OnOK();
	virtual void OnCancel();

	// normal input message
	virtual void OnKeyDown(UINT nChar, UINT nFlags);
	virtual void OnKeyUp(UINT nChar, UINT nFlags);
	virtual void OnChar(UINT nChar);
	virtual void OnLButtonUp(UINT nFlags, CPoint point );
	virtual void OnLButtonDown(UINT nFlags, CPoint point );
	virtual void OnLButtonDblClk(UINT nFlags, CPoint point);
	virtual void OnRButtonUp(UINT nFlags, CPoint point );
	virtual void OnRButtonDown(UINT nFlags, CPoint point );
	virtual void OnMouseDrag(UINT nFlags, CPoint point );
	virtual void OnMouseMove(CPoint point );
	virtual void OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt);

	virtual void OnMove(int x, int y);

	virtual void OnSize(int cx, int cy);
	virtual void OnSetFocus();
	virtual void OnKillFocus();

	virtual void OnTimer(UINT nIDEvent);

	virtual void OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar) { }
	virtual void OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar) { }

	// messages to process
	virtual void OnWindowSizeChanged();
	virtual void OnSkinLoaded();

	virtual BOOL MoveWindow(int X, int Y, int nWidth, int nHeight, BOOL bRepaint = TRUE);
	virtual BOOL MoveWindow(RECT &rc, BOOL bRepaint = TRUE);

	virtual bool OnCustomCommand(int nId);
	virtual void PostCustomCommandMsg(int nId);
	virtual void PostShortcutKeyCmd(int nId);

	virtual void OnUserMessage(int nMessageID, LPARAM param);

	virtual void OnUIObjNotify(IUIObjNotify *pNotify) { }

	virtual void OnRemoveUIObj(CUIObject *pObj);
	virtual void OnAddUIObj(CUIObject *pObj);

	virtual void OnLanguageChanged();

	virtual void OnAdjustHue(float hue, float saturation, float luminance);

	virtual void UpdateMemGraphicsToScreen(const RECT* lpRect);

#ifdef _WIN32
	static int ms_msgIDCustomCommand;

	virtual LRESULT WndProc(UINT message, WPARAM wParam, LPARAM lParam);
#endif

public:
	//
	// window alpha and transparent features
	//
	inline bool GetEnableTranslucencyLayered() const { return m_bTranslucencyLayered; }
	void EnableTranslucencyLayered(bool bTranslucencyLayered);
	inline int GetCurrentTranslucency() const { return m_nCurTranslucencyAlpha; }

	MLColor GetTranslucencyColor(const MLColor &clr) const;

#ifdef _WIN32_DESKTOP
	void InvalidateRectOfLayeredWindow(const RECT* lpRect);
	virtual BOOL InvalidateRect(const RECT* lpRect = NULL, BOOL bErase = FALSE);
#endif

	void StartTranslucencyFade();

	void OnTimerDynamicAlphaChange();

	virtual bool IsClickThrough() { return m_bEnableClickThrough && m_bClickThrough && !m_bActived; }

	enum TranslucencyStatus
	{
		TS_NORMAL,
		TS_ON_ACTIVE,
		TS_ON_HOVER,
	};

	int					m_nTranslucencyAlphaDefault;		// Alpha on inactivate
	DWORD				m_dwTransFadeinBegTime;				// Begin time of fade in/out translucency
	int					m_nTransAlphaBeg;					// The alpha value of begin fade in/out.
	int					m_nTranslucencyAlphaOnActive;		// Alpha on activate
	int					m_nTranslucencyAlphaOnHover;		// Alpha on mouse hover
	int					m_nCurTranslucencyAlpha;			// Current translucency alpha value

	TranslucencyStatus	m_translucencyStatus;
	bool				m_bOnMouseHover;

	bool				m_bEnableClickThrough;


public:
	void SetProperies(SXNode::LIST_PROPERTIES &listProperties);
	virtual BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
#ifdef _SKIN_EDITOR_
	virtual void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

	bool GetUnprocessedProperty(LPCTSTR szProperty, tstring &strValue);

#ifdef _SKIN_EDITOR_
	virtual void ToXML(CXMLWriter &xmlStream);
#endif // _SKIN_EDITOR_

	bool UpdateSkinProperty();

	bool IsWndActive();

	CSkinFontProperty *GetFontProperty() { return &m_fontProperty; }
	CRawBmpFont *GetRawFont() { return m_fontProperty.GetFont(); }

	LPCTSTR GetSkinWndName() { return m_strSkinWndName.c_str(); }
	LPCTSTR GetCaptionText() { return m_strCaption.c_str(); }
	void SetCaptionText(LPCTSTR szCaption, bool bRedraw = false) {
		ASSERT(szCaption); m_strCaption = szCaption;
		SetWindowText(m_strCaption.c_str());
		if (bRedraw) InvalidateRect();
	}

	LPCTSTR GetMenuName() { return m_strMenuName.c_str(); }

	bool IsWindowsAppearance() const { return m_bWindowsAppearance; }

	// after process WM_DESTORY, delete this.
	void SetFreeOnDestory(bool bFreeOnDestory = true) { m_bFreeOnDestory = bFreeOnDestory; }

	CRect &GetBoundBox() { return m_rcBoundBox; }

public:
	void GetCursorClientPos(CPoint &pt);

	void AddWndCloseto(CBaseWnd *pWnd, LPCTSTR szWndClass, LPCTSTR szWndName);

	void TrackMove(CBaseWnd *pWnd, int x, int y);

	BOOL AddTool(LPCTSTR szText = LPSTR_TEXTCALLBACK, LPCRECT lpRectTool = NULL, UINT nIDTool = 0);
	void DelTool(UINT nIDTool);


	bool RegisterUIObjNotifyHandler(int nIDUIObj, IUIObjNotifyHandler *pHandler);
	bool UnregisterUIObjNotifyHandler(IUIObjNotifyHandler *pHandler);

	void DispatchUIObjNotify(IUIObjNotify *pNotify);

protected:
	void RootContainerFromXml(SXNode *pSkinWndNode, CSkinContainer *pRootContainer);

	virtual int FromXML(SXNode *pXmlNode);

	CRawGraph *GetMemGraphics();

	bool RecreateMemGraphics();

	void RecreateBrush();

	void OnSwitchChildFocus(CUIObject *pUIObjFocusOld, CUIObject *pUIObjFocusNew);

	virtual void OnLoadWndSizePos();
	void SaveWndPos();

	void ProcessMouseMove(CPoint point);

	friend class CSkinContainer;
	friend class CUIObject;
	friend class CSjvmSkinWnd;

protected:
	//
	// Hide UIObject, if mouse is inactive in 3 seconds.
	//
	uint32			m_nTimeLatestMouseMsg;
	bool			m_bMouseActive;

	void OnMouseActiveMsg();
	void OnTimerMouseInactive();
	virtual void OnMouseActive(bool bMouseActive);

public:
	class CAutoRedrawLock {
	public:
		CAutoRedrawLock(CSkinWnd *pSkinWnd) { pSkinWnd->EnterInDrawUpdate(); m_pSkinWnd = pSkinWnd; }
		~CAutoRedrawLock() { if (m_pSkinWnd) m_pSkinWnd->LeaveInDrawUpdate(); }
		void CancelDrawUpdate() {
			if (m_pSkinWnd) {
				m_pSkinWnd->CancelDrawUpdate(); m_pSkinWnd = NULL;
			}
		}

	protected:
		CSkinWnd	*m_pSkinWnd;

	};

public:
	//
	// Public method for Hide UIObject, if mouse is inactive.
	//
	bool IsMouseActive() { return m_bMouseActive; }

public:
	CSkinWndDrag	m_WndDrag;
	CSkinWndResizer	m_wndResizer;

	// default width and height
	int				m_nWidth, m_nHeight;
	bool			m_bRememberSizePos;

protected:
	bool			m_bFreeOnDestory;
	bool			m_bManageBySkinFactory;

	bool			m_bOnDestroy;

	// flag to indicate whether the skin was opened.
	bool			m_bSkinOpened;
	bool			m_bInEditorMode;	// Is in skin editor mode?

	bool			m_bNeedRedraw;
	int				m_nInRedrawUpdate;

	CSkinFactory	*m_pSkinFactory;

	tstring			m_strSkinWndName;		// skin 名
	tstring			m_strCaption;

	tstring			m_strMenuName;

	CSkinToolTip	m_wndToolTip;

	//
	// Status
	//
	bool			m_bActived;		// 当前窗口是否为激活状态Focus?
	bool			m_bMainAppWnd;	// true: when the Window is closed, exit process.
	bool			m_bDialogWnd;	// For dialog Skin: Enter key = OK, Escape key = cancel.

	double			m_dbScale;		// 窗口是否进行缩放
	CRect			m_rcReal;		// 窗口的实际大小

	//
	// properties
	//
	CCursor			m_cursor;
	tstring			m_strCusor;		// 光标文件

	bool			m_bWindowsAppearance;
	bool			m_bAeroGlass;

 	// skin region
	bool			m_bUseRgn;		// SkinWnd 是否使用region
	tstring			m_strRgn;		// 使用的rgn文件名
	CSkinRgn		m_rgnSkin;		// 当使用region时，它代表窗口的区域
	CRect			m_rcBoundBox;	// 当使用region时，它是区域m_rgnWnd的边框
									// 当不使用region，它是skin的矩形。

	CSkinFontProperty	m_fontProperty;	// Default font property.

	// Unprocessed properties
	SXNode::LIST_PROPERTIES	m_listUnprocessedProperties;

	// draw buffer
	CRawGraph		*m_pmemGraph;
#ifndef _WIN32
	CRect			m_rcMemUpdate;
#endif

	//
	// Timer IDs
	//
	typedef struct TIMER_OBJECT
	{
		CUIObject	*pObj;
		int			nTimeDuration;
	}TIMER_OBJECT;
	int				m_nTimerIDMax;

	typedef map<int, TIMER_OBJECT>	MAP_TIMER_OBJS;
	MAP_TIMER_OBJS	m_mapTimerObjs;


	// root skin uiobject container
	CSkinClientArea	m_rootConainter;

	// only one ui object can capture the mouse at any time
	CUIObject		*m_pUIObjCapMouse;

	// The UIObject registered to handle context menu command.
	CUIObject		*m_pUIObjHandleContextMenuCmd;

	// auto allocated uni-ID
	int				m_nNextAuoUniID;

	int				m_nNextUIObjIDAlloc;

	bool			m_bIgnoreNextOnCharMsg;

	//
	// UIObject notification handler : Register, Unregister and dispatch
	//
	struct UIObjNotifyHandlerInfo
	{
		int					nIDUIObj;
		IUIObjNotifyHandler	*pHandler;
	};
	typedef vector<UIObjNotifyHandlerInfo>	VecUIObjNotifyHandlers;
	VecUIObjNotifyHandlers		m_vUIObjNotifyHandlers;

	// Exchange Pool is used to exchange data between pages.
	MapString				m_mapExchangePool;

	//
	// Skin script message delivery
	//
public:
	void SjvmSetNotifyEvent(SjvmSkinWndEvent *pNotify) { m_pSjvmNotifyEvent = pNotify; }

protected:
	SjvmSkinWndEvent	*m_pSjvmNotifyEvent;

	//
	// Skin animation object
	//
public:
	void StartAnimation(int nUIDAnimation);
	void StartAnimation(CUIObject *pObjTarget, int nAnimateId, int nDurationTime);
	void StartAnimation(CUIObject *pObjTarget, CAnimationMotion *pAnimation, int nDurationTime);
	void StopAnimation(int nIDAnimation);
	void StopAnimation(CUIObject *pObjTarget);

	AnimateType GetAnimationType() const;
	int GetAnimationDuration() const;

protected:
	typedef auto_free_list<CSkinAnimation>	ListAnimation;
	ListAnimation					m_listAnimations;

	AnimateType						m_animateType;
	int								m_animateDuration;

	void OnTimerAnimation();

};

#endif // !defined(AFX_SKIN_H__A1FDC39A_7098_11D5_9E04_02608CAD9330__INCLUDED_)
