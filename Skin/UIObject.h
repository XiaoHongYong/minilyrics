#pragma once

class CFormula;
class CSFImage;
class CSkinContainer;
class CUIObject;
class CSkinAnimation;

typedef vector<CUIObject *>		VecUIObjects;
typedef list<CUIObject *>		ListUIObjects;

enum LayoutParams
{
	// For both
	LAYOUT_WIDTH_WRAP_CONENT		= 1 << 0,
	LAYOUT_HEIGHT_WRAP_CONENT		= 1 << 1,

	// Only for vertical linear container
	LAYOUT_WIDTH_MATCH_PARENT		= 1 << 2,
	LAYOUT_ALIN_HORZ_CENTER			= 1 << 3,
	LAYOUT_ALIN_RIGHT				= 1 << 4,

	// Only for horizontal linear container
	LAYOUT_HEIGHT_MATCH_PARENT		= 1 << 5,
	LAYOUT_ALIN_VERT_CENTER			= 1 << 6,
	LAYOUT_ALIN_BOTTOM				= 1 << 7,
};

enum BLT_MODE
{
	BLT_COPY,			// 直接拷贝
	BLT_STRETCH,		// 进行伸缩复制
	BLT_TILE			// 平铺复制，自动判断是X方向还是Y方向
};

BLT_MODE BltModeFromStr(LPCTSTR szBltMode);

BlendPixMode BlendPixModeFromStr(LPCTSTR szBpm);

enum AlginText
{
	AT_LEFT		= DT_LEFT,
	AT_CENTER	= DT_CENTER,
	AT_RIGHT	= DT_RIGHT,
	AT_TOP		= DT_TOP,
	AT_VCENTER	= DT_VCENTER,
	AT_BOTTOM	= DT_BOTTOM,
};

DWORD AlignTextFromStr(LPCTSTR szAlignText);
void AlignTextToStr(DWORD dwAlignTextFlags, tstring &str);

class CUIObjProperty
{
public:
	CUIObjProperty() { bToXmlAttrib = true; valueType = VT_INT; }
	enum VALUE_TYPE
	{
		VT_INT,				// 整型数值
		VT_ID,				// ID of controls
		VT_STR,				// 字符串值
		VT_FILE,			// vOptions 是文件扩展名列表
		VT_IMG_FILE,
		VT_IMAGE,			// Image file(strValue) and rect(vOptions[0, 1])
		VT_VAR_INT,
		VT_RECT,			// %d, %d, %d, %d 型的值
		VT_BOOL_STR,		// BOOL 值
		VT_COMB_STR,		// vOptions 是可以选择的字符串列表
		VT_COLOR,			// color
		VT_FONT_NAME,		// font
	};
	tstring		strName;
	tstring		strDesc;		// 描述
	tstring		strValue;
	VALUE_TYPE	valueType;
	bool		bToXmlAttrib;

	vector<tstring>	vOptions;

	// 子属性
	vector<CUIObjProperty>		vChildProp;

	void SetValue(int nValue)
	{
		strValue = StringFromInt(nValue);
	}

	void ToUIObjProperties(vector<tstring> &vProperties);

};

class CUIObjProperties : public list<CUIObjProperty>
{
public:
	void AddPropStr(LPCTSTR szName, LPCTSTR szValue, bool bToXmlAttrib = true);
	void AddPropID(LPCTSTR szName, LPCTSTR szValue, bool bToXmlAttrib = true);
	void AddPropInt(LPCTSTR szName, int nValue, bool bToXmlAttrib = true);
	void AddPropVar(LPCTSTR szName, CFormula &formular, bool bToXmlAttrib = true);
	void AddPropBoolStr(LPCTSTR szName, BOOL bValue, bool bToXmlAttrib = true);
	void AddPropImage(LPCTSTR szName, LPCTSTR szRectName, LPCTSTR szImage, CSFImage &image, bool bToXmlAttrib = true);
	void AddPropImageEx(LPCTSTR szName, LPCTSTR szRectName, LPCTSTR szImage, CSFImage &image, vector<tstring> &vExtraProperies, bool bToXmlAttrib = true);
	void AddPropImageFile(LPCTSTR szName, LPCTSTR szImageFile, bool bToXmlAttrib = true);
	void AddPropFile(LPCTSTR szName, LPCTSTR szFile, LPCTSTR szExtFilter, bool bToXmlAttrib = true);
	void AddPropColor(LPCTSTR szName, MLColor &clrValue, bool bToXmlAttrib = true);
	void AddPropFontName(LPCTSTR szName, LPCTSTR szFont, bool bToXmlAttrib = true);
	void AddPropBltMode(LPCTSTR szName, BLT_MODE bltMode, bool bToXmlAttrib = true);
	void AddPropBlendPixMode(LPCTSTR szName, BlendPixMode bpm, bool bToXmlAttrib = true);

	void DelProp(LPCTSTR szName);

	void ToXMLCategoryAttrib(CXMLWriter &xmlStream);

	void ToUIObjProperties(vector<tstring> &vProperties);

	bool IsPropExist(LPCTSTR szName);

};

enum
{
	UO_MSG_WANT_MOUSEMOVE	= 1,
	UO_MSG_WANT_LBUTTON		= 1 << 1,
	UO_MSG_WANT_RBUTTON		= 1 << 2,
	UO_MSG_WANT_KEY			= 1 << 3,
	UO_MSG_WANT_ENTER_KEY	= 1 << 4,
	UO_MSG_WANT_ALL_KEYS	= 1 << 5,
	UO_MSG_WANT_MOUSEWHEEL	= 1 << 6,
	UO_MSG_WANT_MENU_KEY	= 1 << 7,
	UO_MSG_WANT_CUSTOM_CMD	= 1 << 8,
	UO_MSG_WANT_MENU_CMD	= 1 << 9,
	UO_MSG_WANT_ALL			= 0xFFFFFFFF
};

bool IsTRUE(LPCTSTR szValue);

#define BOOLTOSTR(bValue)		(bValue ? _T("TRUE") : _T("FALSE"))

#define GET_ID_BY_NAME(_id)		_id = GetIDByName(_T(#_id))
#define GET_ID_BY_NAME2(_id1, _id2)		{ _id1 = GetIDByName(_T(#_id1)); _id2 = GetIDByName(_T(#_id2)); }
#define GET_ID_BY_NAME3(_id1, _id2, _id3)		{ _id1 = GetIDByName(_T(#_id1)); _id2 = GetIDByName(_T(#_id2)); _id3 = GetIDByName(_T(#_id3)); }
#define GET_ID_BY_NAME4(_id1, _id2, _id3, _id4)		{ _id1 = GetIDByName(_T(#_id1)); _id2 = GetIDByName(_T(#_id2)); _id3 = GetIDByName(_T(#_id3)); _id4 = GetIDByName(_T(#_id4)); }

#define SZ_TRUE				_T("TRUE")
#define SZ_FALSE			_T("FALSE")

// PN = Property Name
#define SZ_PN_NAME			_T("Name")
#define SZ_PN_TEXT			_T("Text")
#define SZ_PN_ID			_T("ID")
#define SZ_PN_LEFT			_T("Left")
#define SZ_PN_TOP			_T("Top")
#define SZ_PN_WIDTH			_T("Width")
#define SZ_PN_HEIGHT		_T("Height")
#define SZ_PN_TOOLTIP		_T("ToolTip")
#define SZ_PN_EXTENDS		_T("Extends")

#define SZ_PN_PROPERTY		_T("Property")

#define SZ_PN_IMAGE			_T("Image")
#define SZ_PN_IMAGE_SIZE	_T("ImageSize")
#define SZ_PN_IMAGE_POS		_T("ImagePos")
#define SZ_PN_IMAGERECT		_T("ImageRect")

#define SZ_PN_RECT			_T("Rect")
#define SZ_PN_MARGIN		_T("Margin")
#define SZ_PN_PADDING		_T("Padding")
#define SZ_PN_LAYOUT_PARAMS	_T("LayoutParams")
#define SZ_PN_WEIGHT		_T("Weight")
#define SZ_PN_MIN_WIDTH		_T("MinWidth")
#define SZ_PN_MIN_HEIGHT	_T("MinHeight")
#define SZ_PN_FIXED_WIDTH	_T("FixedWidth")
#define SZ_PN_FIXED_HEIGHT	_T("FixedHeight")

#define SZ_PN_LINK			_T("Link")

//
// declare GetClassName, IsKindOf, ClassName of classes derived from CUIObject
//
#define UIOBJECT_CLASS_NAME_DECLARE(BaseClass)	\
public:\
	LPCTSTR GetClassName() { return ms_szClassName; }\
	BOOL IsKindOf(LPCTSTR szClassName)\
	{\
			if (BaseClass::IsKindOf(szClassName))\
				return TRUE;\
			return _tcsicmp(szClassName, ms_szClassName) == 0;\
	}\
	static LPCTSTR ClassName() { return ms_szClassName; }\
protected:\
	static LPCTSTR		ms_szClassName;\
public:


#define UIOBJECT_CLASS_NAME_IMP(_ClassName, szClassName)	\
	LPCTSTR _ClassName::ms_szClassName = szClassName;


class CUIObject  
{
public:
	CUIObject();
	virtual ~CUIObject();

public:
	void SetParent(CSkinWnd *pSkinWnd, CSkinContainer *pContainer)
		{ m_pSkin = pSkinWnd; m_pContainer = pContainer; }

	// modify CUIObject's property
	virtual BOOL SetProperty(LPCTSTR szProperty, LPCTSTR szValue);
	virtual BOOL SetPropertyInt(LPCTSTR szProperty, int nValue);
	virtual void SetProperties(vector<tstring> &vProperties);

	uint32 GetLayoutParams() const { return m_layoutParams; }
	uint GetWeight() const { return m_nWeight; }
	uint GetMinHeight() const;
	uint GetMinWidth() const;
	bool IsFixedWidth() const { return m_bFixedWidth; }
	bool IsFixedHeight() const { return m_bFixedHeight; }

	void OnMeasureSizePos(FORMULA_VAR vars[]);
	virtual void OnMeasureSizeByContent() { }

	const CRect &GetMargin() const { return m_rcMargin; }
	const CRect &GetPadding() const { return m_rcPadding; }

	// when CSkinWnd's size is changed, call every CUIObject's RecalclulatePos to change pos
	virtual void Invalidate();

	CSkinContainer *GetParent() const { return m_pContainer; }
	CSkinWnd *GetSkinWnd() const { return m_pSkin; }
	CSkinFactory *GetSkinFactory() const;


	// UIObject messages
	virtual void OnInitialUpdate() { }
	virtual void OnCreate() { }
	virtual void OnDestroy() { }
	virtual void OnKeyUp(UINT nChar, UINT nFlags) { }
	virtual void OnKeyDown(UINT nChar, UINT nFlags) { }
	virtual bool OnMenuKey(UINT nChar, UINT nFlags) { return false; }
	virtual void OnChar(UINT nChar) { }
	virtual bool OnRButtonUp(UINT nFlags, CPoint point) { return false; }
	virtual bool OnRButtonDown(UINT nFlags, CPoint point) { return false; }
	virtual bool OnLButtonUp(UINT nFlags, CPoint point) { return false; }
	virtual bool OnLButtonDown(UINT nFlags, CPoint point) { return false; }
	virtual bool OnLButtonDblClk(UINT nFlags, CPoint point) { return false; }
	virtual bool OnMouseDrag(CPoint point) { return false; }
	virtual bool OnMouseMove(CPoint point) { return false; }
	virtual void OnMouseWheel(int nWheelDistance, int nMkeys, POINT pt) { }

	virtual void OnVScroll(UINT nSBCode, int nPos, IScrollBar *pScrollBar) { }

	virtual bool OnCommand(int nId) { return false; }
	virtual bool OnCustomCommand(int nId) { return false; }

	virtual void OnAnimate(CSkinAnimation *pAnimation) { }

	virtual void Draw(CRawGraph *pGraph) { }

	int GetOpacity();

	virtual void OnKillFocus() { }
	virtual void OnSetFocus() { }

	virtual void OnTimer(int nId) { }
	virtual void OnSize();

	virtual void OnAdjustHue(float hue, float saturation, float luminance) { }
	virtual void OnSkinFontChanged() { }

	inline bool WantMsgMouseMove()	{ return (m_nMsgWant & UO_MSG_WANT_MOUSEMOVE) != 0; }
	inline bool WantMsgMouseWheel()	{ return (m_nMsgWant & UO_MSG_WANT_MOUSEWHEEL) != 0; }
	inline bool WantMsgLButton()	{ return (m_nMsgWant & UO_MSG_WANT_LBUTTON) != 0; }
	inline bool WantMsgRButton()	{ return (m_nMsgWant & UO_MSG_WANT_RBUTTON) != 0; }
	inline bool WantMsgKey()	{ return (m_nMsgWant & (UO_MSG_WANT_KEY | UO_MSG_WANT_ALL_KEYS)) != 0; }
	inline bool WantMsgAllKeys()	{ return (m_nMsgWant & UO_MSG_WANT_ALL_KEYS) != 0; }
	inline bool WantMsgEnterKey()	{ return (m_nMsgWant & UO_MSG_WANT_ENTER_KEY) != 0; }
	inline bool WantMsgMenuKey()	{ return (m_nMsgWant & UO_MSG_WANT_MENU_KEY) != 0; }
	inline bool WantMsgMenuCmd()	{ return (m_nMsgWant & UO_MSG_WANT_MENU_CMD) != 0; }
	inline bool WantMsgCustomCmd()	{ return (m_nMsgWant & UO_MSG_WANT_CUSTOM_CMD) != 0; }

	bool IsVisible();
	virtual void SetVisible(bool bVisible, bool bRedraw = false);
	virtual void SetVisibleEx(bool bVisible, bool bAnimation, AnimateDirection animateDirection);
	bool IsParentVisible();

	inline bool IsEnable() { return m_bEnable; }
	inline void SetEnable(bool bEnable) { m_bEnable = bEnable; } 

	inline bool IsUseParentBg() { return m_bUseParentBg; }
	inline void SetUseParentBg(bool bUseSkinBg) { m_bUseParentBg = bUseSkinBg; }

	virtual void SetText(LPCTSTR szText);
	virtual tstring &GetText() { return m_strText; }

	void SetFocus();
	bool IsOnFocus();

	inline int GetID() const { return m_nId; }

	virtual bool IsPtIn(POINT pt);

#ifdef _SKIN_EDITOR_
	virtual void EnumProperties(CUIObjProperties &listProperties);
#endif // _SKIN_EDITOR_

	virtual int FromXML(SXNode *pXmlNode);
#ifdef _SKIN_EDITOR_
	virtual void ToXML(CXMLWriter &xmlStream);
#endif // _SKIN_EDITOR_

	AnimateType GetAnimationType() const;
	int GetAnimationDuration() const;

	virtual LPCTSTR GetClassName() { return ms_szClassName; }
	virtual BOOL IsKindOf(LPCTSTR szClassName) { return _tcsicmp(szClassName, ms_szClassName) == 0; }
	static LPCTSTR ClassName() { return ms_szClassName; }

	// profile api
	UINT GetInt(LPCTSTR szKeyName, int nDefault);
	bool GetBool(LPCTSTR szKeyName, bool bDefault) { return GetInt(szKeyName, bDefault) != 0; }
	tstring GetString(LPCTSTR szKeyName, LPCTSTR szDefault);
	void WriteInt(LPCTSTR szKeyName, int nValue);
	void WriteString(LPCTSTR szKeyName, LPCTSTR szValue);
	tstring GetProfileKeyName(LPCTSTR szSubKeyName);

	virtual void OnLanguageChanged();

	int GetIDByName(LPCTSTR szID);

public:
	//
	// CSkinContainer API
	//
	virtual bool IsContainer() const { return false; }
	virtual CSkinContainer *GetContainerIf() { return NULL; }

	void TryToCallInitialUpdate()
	{ if (!m_bInitialUpdated) { m_bInitialUpdated = true; OnInitialUpdate(); } }

protected:
#ifdef _SKIN_EDITOR_
	// For Skin Editor to port child.
	virtual bool HasXMLChild() { return false; }
	virtual void OnToXMLChild(CXMLWriter &xmlStream) {}
#endif // _SKIN_EDITOR_

public:
	CFormula	m_formTop;
	CFormula	m_formLeft;
	CFormula	m_formWidth;
	CFormula	m_formHeight;

	uint32		m_layoutParams;
	uint16		m_nWeight;
	uint16		m_nMinWidth, m_nMinHeight;	// The minimum width and height
	bool		m_bFixedWidth, m_bFixedHeight;

	CRect		m_rcObj;		// The whole area of the UIObject, including its border.
	CRect		m_rcContent;	// The content area of the UIObject, for example: for TextButton, it's the text area.

	// For differences between margin and padding: http://stackoverflow.com/questions/4619899/difference-between-a-views-padding-and-margin
	CRect		m_rcMargin;		// The margin of UIObject (left, top, right, bottom)
	CRect		m_rcPadding;	// The padding of UIObject (left, top, right, bottom)

	// auto allocated uni-ID by CSkinWnd.
	int			m_nAutoUniID;

	int			m_nId;
	tstring		m_strName;

	int			m_nMsgWant;		// message mask is used to specify what messages ui objects want to process
	tstring		m_strText, m_strTextEnglish;
	bool		m_bEnable;		// Is control being enabled?
	bool		m_bVisible;
	bool		m_bHideIfWndInactive;
	bool		m_bHideIfMouseInactive;
	bool		m_bUseParentBg;	// Invalidate call will update background, if m_bUseParentBg is true

	bool		m_bTranslucencyWithSkin;	// Set the alpha of UIObject to the skin, if this flag is set.
	int16		m_nOpacity;

	tstring		m_strMaskImage;
	CSFImage	m_imgMask;

protected:
	static LPCTSTR		ms_szClassName;

	tstring			m_strTooltip;
	bool			m_bTempTooltip;

	AnimateType		m_animateType;
	int				m_animateDuration;

	CSkinWnd		*m_pSkin;
	CSkinContainer	*m_pContainer;

protected:
	bool			m_bCreated;
	bool			m_bInitialUpdated;

	friend class CSkinWnd;
	friend class CSkinContainer;

};
