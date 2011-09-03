// SkinFactory.h: interface for the CSkinFactory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINFACTORY_H__A24C811F_2487_4750_8378_C7876023C863__INCLUDED_)
#define AFX_SKINFACTORY_H__A24C811F_2487_4750_8378_C7876023C863__INCLUDED_

class CSkinWnd;
class CMLMenu;
class CSkinFactory;
class CSkinApp;
class CSkinMenu;

#include "SkinResMgr.h"
#include "SkinWndDrag.h"

typedef map<tstring, tstring> MapExchangePool;

struct SkinWndStartupInfo
{
	tstring			strClassName, strCaptionText;
	tstring			strSkinWnd;
	bool			bMainWnd;
	CSkinWnd		*pSkinWnd;
	CBaseWnd		*pWndParent;
	MapExchangePool	mapExchangePool;

	SkinWndStartupInfo(LPCTSTR szClassName, LPCTSTR szCaptionText, LPCTSTR szSkinWnd,
		CBaseWnd *pWndParent, bool bMainWnd = false);
};

// UIObject ID text to int
struct UIObjectIDDefinition
{
	const TCHAR		*szId;
	uint16			nId;
	uint16			nIdMenu;
	const TCHAR		*szToolTip;
};

class UIObjectIDDefinitionLessCmp
{
public:
	bool operator()(UIObjectIDDefinition *l1, UIObjectIDDefinition *l2) const
	{
		return _tcscmp(l1->szId, l2->szId) < 0;
	}
};

typedef set<UIObjectIDDefinition *, UIObjectIDDefinitionLessCmp>	SetUIObjectIDDefinition;

enum UIObjectId
{
	UID_INVALID = 0,
	CMD_MINIMIZE = 1,
	CMD_MAXIMIZE,
	CMD_CLOSE,
	CMD_QUIT,
	CMD_MENU,

	// Skin Dialog
	CMD_OK,
	CMD_CANCEL,

	CMD_BASE_END,
	CMD_ID_CUSTOM_BASE = 1000,		// system alloc id begins from here

};

//////////////////////////////////////////////////////////////////////////////////////////////

//
// Define the map of new UIObject.
//
class IUIObjNewer
{
public:
	virtual CUIObject *New() = 0;

};

template<class _CUIObj_t>
class CUIObjectNewer : public IUIObjNewer
{
public:
	virtual ~CUIObjectNewer() { }
	virtual CUIObject *New() { return new _CUIObj_t(); }

};

class CUIObjectNewerLessCmp
{
public:
	bool operator()(LPCTSTR s1, LPCTSTR s2) const
		{ return _tcsicmp(s1, s2) < 0; }

};

typedef map<LPCTSTR, IUIObjNewer *, CUIObjectNewerLessCmp>		MapUIObjNewer;

#define AddUIObjNewer(_uiobj_class)		m_mapUIObjNewer[_uiobj_class::ClassName()] = new CUIObjectNewer< _uiobj_class >()
#define AddUIObjNewer2(_skinFactory, _uiobj_class)		_skinFactory->RegisterUIObjNewer(_uiobj_class::ClassName(), new CUIObjectNewer< _uiobj_class >())

/////////////////////////////////////////////////////////////////////////////////////////////////////

#define _SZ_SKINWND_CLASS_NAME	_T("CMPSkin")

//
// 打开，保存XML格式的skin文件
class CSkinFileXML
{
public:
	CSkinFileXML();
	~CSkinFileXML();

public:
	bool IsOpened() { return m_xml.m_pRoot != NULL; }

	int Load(LPCTSTR szFile);

	int Save(LPCTSTR szFile);

	void Close();

	SXNode *GetSkinWndNode(LPCTSTR szSkinWndName);
	int NewSkinWndNode(LPCTSTR szSkinWndName);
	int SetSkinWndNode(LPCTSTR szSkinWndName, SXNode *pWndNode);
	int RemoveSkinWnd(LPCTSTR szSkinWndName);
	int ChangeSkinWndName(LPCTSTR szNameOld, LPCTSTR szNameNew);
	bool CanUseSkinWndName(LPCTSTR szName);

	// Get the main window of the SKIN. szSkinWndNameDef specifies the default main window.
	int GetMainSkinWnd(tstring &strSkinWndName, LPCTSTR szSkinWndNameDef);

	void EnumSkinWnd(vector<tstring> &vSkinWnd);

	SXNode *GetRootNode() { return m_xml.m_pRoot; }

	SXNode *GetMenusNode();
	int SetMenusNode(SXNode *pNode);

	SXNode *GetDynamicCmdsNode();
	int SetDynamicCmdsNode(SXNode *pNode);

	LPCTSTR GetFileName() const { return m_strFile.c_str(); }

	LPCTSTR GetSkinProperty(LPCTSTR szProperty);

protected:
	int GetSkinWndNode(LPCTSTR szSkinWndName, SXNode::iterator *itWnd);

	int SetNode(LPCTSTR szName, SXNode *pNode);

protected:
	CSimpleXML			m_xml;
	tstring				m_strFile;

};

class CSkinStyles
{
public:
	CSkinStyles();
	~CSkinStyles();

	int FromXML(SXNode *pNodeSkin);
	void Free();

	SXNode *GetClassNode(LPCTSTR szClassName) const;

protected:
	SXNode::LIST_CHILDREN	m_listStyleNodes;

};

class CDynamicCmds
{
public:
	struct DynamicCmd
	{
		int			nID;
		tstring		strFunction;
		tstring		strParam;
	};
	typedef list<DynamicCmd>		LIST_DYNCMD;

	CDynamicCmds() { m_pSkinFactory = NULL; }
	virtual ~CDynamicCmds() { Free(); }

	void Init(CSkinFactory *pSkinFactory) { m_pSkinFactory = pSkinFactory; }

	int FromXML(SXNode *pNodeDynCmds);
#ifdef _SKIN_EDITOR_
	SXNode *ToXML();
#endif // _SKIN_EDITOR_

	void Free();

	bool GetCmd(int nCmdID, DynamicCmd &cmd);

	void EnumAllCmds(V_Str &vstrCmds);

	LIST_DYNCMD &GetDataList() { return m_listDynCmd; }

protected:
	CSkinFactory	*m_pSkinFactory;
	LIST_DYNCMD		m_listDynCmd;

};

class CSkinFactory : public ISkinWndDragHost
{
public:
	CSkinFactory(CSkinApp *pApp, UIObjectIDDefinition uidDefinition[]);
	virtual ~CSkinFactory();

	virtual int Init();
	virtual void Quit();

	//
	// UID = UIObject ID
	//
	int GetIDByName(LPCTSTR szId) { return GetIDByNameEx(szId, NULL); }
	int GetIDByNameEx(LPCTSTR szId, tstring *pstrToolTip);
	virtual int GetIDByNameEx(LPCTSTR szId, tstring &strToolTip)
		{ return GetIDByNameEx(szId, &strToolTip); }
	virtual void GetTooltip(int nUID, tstring &strToolTip);

	int GetMenuIDByUID(int nUID, bool bAllocIfInexistence = false);
	bool SetMenuIDByUID(int nUID, int nMenuId);
	int GetUIDByMenuID(int nMenuID);

	UIObjectIDDefinition *GetUidDef(int nUID);

	int AllocUID();
	tstring GetStringOfID(int nID);

	bool IsOpened() { return m_skinFile.IsOpened(); }

	virtual int OpenSkin(LPCTSTR szSkinName, LPCTSTR szSkinDir = _T(""), LPCTSTR szExtraDir = _T(""));

	virtual int ChangeSkin(LPCTSTR szSkinName, LPCTSTR szSkinDir = _T(""), LPCTSTR szExtraDir = _T(""), bool bLoadSkinColorTheme = false, bool bCreateSkinWnd = true);

	virtual int SaveSkinFile(LPCTSTR szFile);
	virtual void Close(bool bQuit = false);

	CSkinWnd *FindSkinWnd(LPCTSTR szSkinWndName);
	bool IsValidSkinWnd(CSkinWnd *pWnd);

	virtual int ActiveOrCreateSkinWnd(SkinWndStartupInfo &skinWndStartupInfo);
	virtual int OpenOrCloseSkinWnd(SkinWndStartupInfo &skinWndStartupInfo);
	virtual int CreateSkinWnd(SkinWndStartupInfo &skinWndStartupInfo);

	virtual void OnSkinWndCreate(CSkinWnd *pSkinWnd);
	virtual void OnSkinWndDestory(CSkinWnd *pSkinWnd);

	virtual void OnMainSkinWndDestory(CSkinWnd *pSkinWnd);

	virtual CSkinWnd *NewSkinWnd(LPCTSTR szSkinWndName, bool bMainWnd);

	// methods for skin window
	SXNode *GetSkinWndNode(LPCTSTR szSkinWndName);

	int LoadAppResSkinWndXml(LPCTSTR szSkinWndName, CSimpleXML &xml);
	bool IsAppResSkinWnd(LPCTSTR szSkinWndName);

	void RegisterUIObjNewer(LPCTSTR szClassName, IUIObjNewer *pObjNewer);

	// create new uiobject by class name
	virtual CUIObject * CreateUIObject(CSkinWnd *pSkin, LPCTSTR szClassName, CSkinContainer *pContainer);

	// 
	virtual CUIObject *CreateDynamicCtrl(CSkinContainer *pContainer, LPCTSTR szClassName, int nIDAssign = UID_INVALID, LPCTSTR szLeft = NULL, LPCTSTR szTop = NULL, LPCTSTR szWidth = NULL, LPCTSTR szHeight = NULL);

	//
	// Skin root dir, and extra dir. resource files search order: SkinDir > SkinRootDir > Extra Dir
	//
	void SetSkinDir(LPCTSTR szSkinDir);
	void SetResourceExtraDir(LPCTSTR szExtraDir);
	void SetSkinsRootDir(LPCTSTR szSkinsRootDir);
	LPCTSTR GetSkinRootDir();
	void SetSkinFileName(LPCTSTR szSkinFileName);

	bool FindFirstSkin(tstring &strSkin);

	bool EnumAllSkins(vector<tstring> &vSkins);

	// resource manage APIs
	CSkinResMgr *GetResourceMgr() { return &m_resourceMgr; }

	CSkinWnd *GetMainWnd();

	CSkinFileXML *GetSkinFile() { return &m_skinFile; }

	LPCTSTR GetSkinName() const { return m_strSkinName.c_str(); }
	LPCTSTR GetSkinFileName() const { return m_strSkinFileName.c_str(); }

	SXNode *GetExtendSkinWndStyle(LPCTSTR szExtendsSkinWnd) const;

	// resource
	virtual bool LoadMenu(CSkinWnd *pWnd, CMLMenu **ppMenu, LPCTSTR szMenu);

	virtual int OnDynamicCmd(int nCmdID, CSkinWnd *pSkinWnd);

	// hue: 0~360,		saturation: 0~1.0,		luminance: 0~1.0
	virtual void AdjustHue(float hue, float saturation = 0.5, float luminance = 0.5);
	void GetAdjustedHueResult(MLColor &clr);
	void GetAdjustedHueResult(COLORREF &clr);

	virtual void OnLanguageChanged();

	virtual void OnSkinWndActivate(CSkinWnd *pWnd);

public:
	// ISkinWndDragHost
	virtual void GetWndDragAutoCloseTo(vector<CBaseWnd *> &vWnd);
	virtual void GetWndDragTrackMove(vector<CBaseWnd *> &vWnd);

protected:
	virtual int OpenSkinFile(LPCTSTR szSkinFile);

	virtual CSkinMenu *LoadPresetMenu(CSkinWnd *pWnd, LPCTSTR szMenu)
		{ return NULL; }

	void ExpandIncludeNode(SXNode *pNodeRoot);

	virtual int ExeDynamicCmd(const CDynamicCmds::DynamicCmd &cmd, CSkinWnd *pSkinWnd);

	virtual CUIObject *NewUIObject(CSkinWnd *pSkin, CSkinContainer *pContainer, LPCTSTR szClassName);

	void ApplyDefaultThemeOfSkin();

protected:
	tstring						m_strSkinRootDir;
	tstring						m_strSkinFileName;
	tstring						m_strSkinName;

	CSkinFileXML				m_skinFile;

	// resource manager
	CSkinResMgr					m_resourceMgr;

	// Skin styles
	CSkinStyles					m_skinStyle;

	//
	// Dynamic commands define
	CDynamicCmds				m_dynamicCmds;

	SetUIObjectIDDefinition		m_setUIDDefinition;
	COnlyAllocator				m_allocator;
	int							m_nCustomCmdIDNext;
	int							m_nMenuIDNext;

	MapUIObjNewer				m_mapUIObjNewer;

	typedef	list<CSkinWnd*>		LIST_SKINWNDS;

	LIST_SKINWNDS				m_listSkinWnds;
	Mutex						m_mutex;

	CSkinApp					*m_pApp;

};

#endif // !defined(AFX_SKINFACTORY_H__A24C811F_2487_4750_8378_C7876023C863__INCLUDED_)
