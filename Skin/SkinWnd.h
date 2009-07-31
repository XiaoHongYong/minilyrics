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

enum CommonCtrlClassName
{
	CCCN_VSCROLLBAR,
	CCCN_HSCROLLBAR,
};


// COMMENT:
//		窗口的位置是否合法（显示位置是否正常）
BOOL IsWndOutOfScreen(const CRect &rc);

// 0, 0, 10, 20
bool GetRectValue(LPCTSTR szValue, CSFImage &image);
bool GetRectValue(LPCTSTR szValue, long &x, long &y, long &cx, long &cy);

bool inline IsPropertyName(LPCTSTR szName, LPCTSTR szStr) { return _tcsicmp(szName, szStr) == 0; }

void PreMultiplyRGBColr(MLColor &clr, byte alpha);

void ColorToStr(const MLColor &clr, tstring &strColor);

// "#FFFFFF"
UINT GetColorValue(LPCSTR szColor, UINT nDefault);
UINT GetColorValue(LPCWSTR szColor, UINT nDefault);
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
	virtual int Create(LPCTSTR szClassName, LPCTSTR szCaption, CSkinFactory *pSkinFactory, LPCTSTR szSkinWndName);

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
// 	BOOL AddUIObject(CUIObject *pObj);
// 	BOOL AddUIObjectToFront(CUIObject *pObj);

	void FocusPrevUIObject();
	void FocusNextUIObject();

	void VerifyFocusUIObject();

	void SetFocusUIObject(CUIObject *pObj);

	CUIObject *GetFocusUIObj() { return m_itFocusUIObject.Get(); }

	CSkinContainer *GetRootContainer() { return &m_rootConainter; }

	bool EnableSkinObject(int nId, bool bEnable = true, bool bRedraw = true);

	LPCTSTR GetCommonCtrlClassName(CommonCtrlClassName ctrlClassName);

	// register special messages
	void SetCaptureMouse(CUIObject *pUIObj);
	CUIObject *GetCaptureMouse();
	void ReleaseCaptureMouse(CUIObject *pUIObj);

	void UnregisterTimerObject(CUIObject *pObj);
	void UnregisterTimerObject(CUIObject *pObj, int nTimerId);
	int RegisterTimerObject(CUIObject *pObj, int nTimeDuration);

	int AllocAutoUniID() { return ++m_nNextAuoUniID; }

	int AllocUIObjID() { return ++m_nNextUIObjIDAlloc; }

	// update draw of UIObject
	virtual void InvalidateUIObject(CUIObject *pObj);

	void RedrawBackground(RECT *prc);

	CSkinFactory *GetSkinFactory() { return m_pSkinFactory; }

	void SetMainAppWnd(bool bMainAppWnd);
	bool IsMainAppWnd() const { return m_bMainAppWnd; }

	bool IsInEditorMode() const { return m_bInEditorMode; }

	bool IsIndividualProperties() const { return m_bIndividualProperties; }

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
	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void OnPaint(IMLGraphicsBase *pGraph);

	// normal input message
	virtual void OnKeyDown(UINT nChar, UINT nFlags);
	virtual void OnKeyUp(UINT nChar, UINT nFlags);
	virtual void OnChar(UINT nChar);
	virtual void OnLButtonUp(UINT nFlags, CPoint point );
	virtual void OnLButtonDown(UINT nFlags, CPoint point );
	virtual void OnLButtonDblClk(UINT nFlags, CPoint point);
	virtual void OnRButtonUp(UINT nFlags, CPoint point );
	virtual void OnRButtonDown(UINT nFlags, CPoint point );
	virtual void OnMouseMove(UINT nFlags, CPoint point );
	virtual void OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt);

	virtual void OnMove(int x, int y);

	virtual void OnSize(int cx, int cy);
	virtual void OnSetFocus();
	virtual void OnKillFocus();

	virtual void OnTimer (UINT nIDEvent);

	virtual void OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar) { }
	virtual void OnHScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar) { }

	// messages to process
	virtual void OnWindowSizeChanged();
	virtual void OnSkinLoaded();
	virtual void OnLoadWndSizePos();

	virtual BOOL MoveWindow(int X, int Y, int nWidth, int nHeight, BOOL bRepaint = TRUE);
	virtual BOOL MoveWindow(RECT &rc, BOOL bRepaint = TRUE);

	virtual bool OnCustomCommand(int nId);
	virtual void PostCustomCommandMsg(int nId);

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
	void SetTransparent(BYTE nAlpha, bool bTransparentClr, COLORREF clrTransparent, bool bClickThrough);
	bool GetEnableTranslucencyLayered() const { return m_bTranslucencyLayered; }
	void EnableTranslucencyLayered(bool bTranslucencyLayered);

#ifdef _WIN32_DESKTOP
	virtual BOOL InvalidateRect(const RECT* lpRect = NULL, BOOL bErase = FALSE);

	BOOL UpdateLayeredWindowUsingMemGraph(CRawGraph *pGraph);

	void OnTimerDynamicAlphaChange();
#endif

	enum TranslucencyStatus
	{
		TS_NORMAL,
		TS_ON_ACTIVE,
		TS_ON_HOVER,
	};

	bool				m_bTranslucencyLayered;				// Is following alpha setting enabled?
	int					m_nTranslucencyAlpha;				// Alpha on inactivate
	float				m_nTranslucencyAlphaDynamic;		// Current alpha value
	float				m_nTranslucencyAlphaDynamicStep;
	int					m_nTranslucencyAlphaOnActive;		// Alpha on activate
	int					m_nTranslucencyAlphaOnHover;		// Alpha on mouse hover

	TranslucencyStatus	m_translucencyStatus;
	bool				m_bOnMouseHover;
	DWORD				m_dwLastSetHoverAlphaTime;			// Used to check when to stop hover alpha timer

	int					m_nAlpha;
	bool				m_bClickThrough;
	bool				m_bEnableTransparentFeature;


public:
	virtual BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
	virtual void EnumProperties(CUIObjProperties &listProperties);

	bool GetUnprocessedProperty(LPCTSTR szProperty, tstring &strValue);

	virtual void ToXML(CXMLWriter &xmlStream);

	bool UpdateSkinProperty();

	bool IsWndActive();

	MLColor &GetFgColor() { return m_clrFg; }
	MLColor &GetBgColor() { return m_clrBg; }

	CSkinFontProperty *GetFontProperty() { return &m_fontProperty; }
	CRawBmpFont *GetRawFont() { return m_fontProperty.GetFont(); }

	LPCTSTR GetSkinWndName() { return m_strSkinWndName.c_str(); }

	bool IsWindowsAppearance() const { return m_bWindowsAppearance; }

	// after process WM_DESTORY, delete this.
	void SetFreeOnDestory(bool bFreeOnDestory = true) { m_bFreeOnDestory = bFreeOnDestory; }

public:
	void GetCursorClientPos(CPoint &pt);

	void AddWndCloseto(CBaseWnd *pWnd, LPCTSTR szWndClass, LPCTSTR szWndName);

	void TrackMove(CBaseWnd *pWnd, int x, int y);

	void RecalculateSizePos();

	BOOL AddTool(LPCTSTR szText = LPSTR_TEXTCALLBACK, LPCRECT lpRectTool = NULL, UINT nIDTool = 0);
	void DelTool(UINT nIDTool);


	bool RegisterUIObjNotifyHandler(int nIDUIObj, IUIObjNotifyHandler *pHandler);
	bool UnregisterUIObjNotifyHandler(IUIObjNotifyHandler *pHandler);

	void DispatchUIObjNotify(IUIObjNotify *pNotify);

protected:
	virtual int FromXML(SXNode *pXmlNode);

	CRawGraph *GetMemGraphics();

	bool RecreateMemGraphics();

	void RecreateBrush();

	void OnSwitchChildFocus(CUIObject *pUIObjFocusOld, CUIObject *pUIObjFocusNew);

	friend class CSkinContainer;

protected:
	//
	// Hide UIObject, if mouse is inactive in 3 seconds.
	//
	uint32			m_nTimeLatestMouseMsg;
	bool			m_bMouseInactive;

	void OnMouseActiveMsg();
	void OnTimerMouseInactive();

public:
	//
	// Public method for Hide UIObject, if mouse is inactive.
	//
	bool			m_bHasUIObjHideOnMouseInactive;

	bool IsMouseInactive() { return m_bMouseInactive; }

public:
	CSkinWndDrag	m_WndDrag;
	CSkinWndResizer	m_wndResizer;

	// default width and height
	int				m_nWidth, m_nHeight;

protected:
	bool			m_bFreeOnDestory;

	// flag to indicate whether the skin was opened.
	bool			m_bSkinOpened;
	bool			m_bInEditorMode;	// Is in skin editor mode?

	CSkinFactory	*m_pSkinFactory;

	tstring			m_strSkinWndName;		// skin 名
	tstring			m_strCaption;

	CSkinToolTip	m_wndToolTip;

	//
	// Status
	//
	bool			m_bActived;		// 当前窗口是否为激活状态Focus?
	bool			m_bMainAppWnd;	// true: when the Window is closed, exit process.

	double			m_dbScale;		// 窗口是否进行缩放
	CRect			m_rcReal;		// 窗口的实际大小

	//
	// properties
	//
	CCursor			m_cursor;
	tstring			m_strCusor;		// 光标文件

	bool			m_bWindowsAppearance;

 	// skin region
	bool			m_bUseRgn;		// SkinWnd 是否使用region
	tstring			m_strRgn;		// 使用的rgn文件名
	CSkinRgn		m_rgnSkin;		// 当使用region时，它代表窗口的区域
	CRect			m_rcBoundBox;	// 当使用region时，它是区域m_rgnWnd的边框
									// 当不使用region，它是skin的矩形。

	CSkinFontProperty	m_fontProperty;	// Default font property.

	MLColor			m_clrFg;		// foreground color
	MLColor			m_clrBg;		// background color
	MLColor			m_clrBgOrg;
	bool			m_bPaintBg;		// Should Paint background?

	// Window properties
	bool			m_bIndividualProperties;

	// Unprocessed properties
	SXNode::LIST_PROPERTIES	m_listUnprocessedProperties;

	// draw buffer
	CRawGraph		*m_pmemGraph;

	//
	// Timer IDs
	//
	typedef struct TIMER_OBJECT
	{
		CUIObject	*pObj;
		int			nTimerId;
		int			nTimeDuration;
	}TIMER_OBJECT;
	int				m_nTimerIDMax;

	typedef map<int, TIMER_OBJECT>	MAP_TIMER_OBJS;
	MAP_TIMER_OBJS	m_mapTimerObjs;


	// root skin uiobject container
	CSkinContainer	m_rootConainter;

	CUIObjectIterator	m_itFocusUIObject;

	// only one ui object can capture the mouse at any time
	CUIObject		*m_pUIObjCapMouse;

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

};

#endif // !defined(AFX_SKIN_H__A1FDC39A_7098_11D5_9E04_02608CAD9330__INCLUDED_)
