// SkinFactory.h: interface for the CSkinFactory class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SKINFACTORY_H__A24C811F_2487_4750_8378_C7876023C863__INCLUDED_)
#define AFX_SKINFACTORY_H__A24C811F_2487_4750_8378_C7876023C863__INCLUDED_

class CSkinWnd;
class CMLMenu;
class CSkinFactory;

#include "SkinResMgr.h"
#include "SkinWndDrag.h"

// UIObject ID text to int
typedef struct STRING_ID_MAP
{
	const TCHAR		*szId;
	int				nId;
	const TCHAR		*szToolTip;
}STRING_ID_MAP;


// system alloc id begins from here
#define CMD_ID_CUSTOM_BASE		1000

#define _SZ_SKINWND_CLASS_NAME	_T("CMPSkin")

//
// 打开，保存XML格式的skin文件
class CSkinFileXML
{
public:
	CSkinFileXML();
	~CSkinFileXML();

public:
	int Load(LPCTSTR szFile);

	int Save(LPCTSTR szFile);

	void Close();

	SXNode *GetSkinWndNode(LPCTSTR szSkinWndName);
	int NewSkinWndNode(LPCTSTR szSkinWndName);
	int SetSkinWndNode(LPCTSTR szSkinWndName, SXNode *pWndNode);
	int RemoveSkinWnd(LPCTSTR szSkinWndName);
	int ChangeSkinWndName(LPCTSTR szNameOld, LPCTSTR szNameNew);
	bool CanUseSkinWndName(LPCTSTR szName);

	// Get the main window of the SKIN. szSkinWndNameDef specalfies the default main window.
	int GetMainSkinWnd(tstring &strSkinWndName, LPCTSTR szSkinWndNameDef);

	void EnumSkinWnd(vector<tstring> &vSkinWnd);

	SXNode *GetMenusNode();
	int SetMenusNode(SXNode *pNode);

	SXNode *GetDynamicCtrlsNode();
	int SetDynamicCtrlsNode(SXNode *pNode);

	SXNode *GetDynamicCmdsNode();
	int SetDynamicCmdsNode(SXNode *pNode);

	SXNode *GetAdjustHueNode();
	int SetAdjustHueNode(SXNode *pNode);

	// int UpdateSkinWnd(LPCTSTR szSkinWndName, CXMLWriter &xmlStream);
	// int DelSkinWnd(LPCTSTR szSkinWndName);

	LPCTSTR GetFileName() const { return m_strFile.c_str(); }

	LPCTSTR GetSkinProperty(LPCTSTR szProperty);

protected:
	int GetSkinWndNode(LPCTSTR szSkinWndName, SXNode::iterator *itWnd);

	int SetNode(LPCTSTR szName, SXNode *pNode);

protected:
	CSimpleXML			m_xml;
	tstring				m_strFile;

};

class CDynamicCtrls
{
public:
	CDynamicCtrls();
	~CDynamicCtrls();

	int FromXML(SXNode *pNode);
	void Free();
	SXNode *GetData() { return m_pNode; }

	CUIObject *CreateControl(CSkinContainer *pContainer, LPCTSTR szClassName, LPCTSTR szNameToCheck = NULL, int nIDAssign = SKIN_UIOBJ_ID_UNDEFINE, LPCTSTR szLeft = NULL, LPCTSTR szTop = NULL, LPCTSTR szWidth = NULL, LPCTSTR szHeight = NULL);

protected:
	SXNode		*m_pNode;

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
	SXNode *ToXML();

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
	CSkinFactory();
	virtual ~CSkinFactory();

	void SetStringIDMap(STRING_ID_MAP StringIdMap[]);

	virtual int GetIDByName(LPCTSTR szId);
	virtual int GetIDByNameEx(LPCTSTR szId, tstring &strToolTip);

	tstring GetStringOfID(int nID);

	virtual int OpenSkinEx(LPCTSTR szSkinName, LPCTSTR szSkinDir = _T(""), LPCTSTR szExtraDir = _T(""));

	virtual int ChangeSkinEx(LPCTSTR szSkinName, LPCTSTR szSkinDir = _T(""), LPCTSTR szExtraDir = _T(""));

	virtual int SaveSkinFile(LPCTSTR szFile);
	virtual void Close();

	CSkinWnd *FindSkinWnd(LPCTSTR szSkinWndName);

	virtual int ActiveOrCreateSkinWnd(bool bMainWnd, LPCTSTR szClassName, LPCTSTR szCaption, LPCTSTR szSkinWndName, CSkinWnd **ppSkin = NULL);
	virtual int OpenOrCloseSkinWnd(bool bMainWnd, LPCTSTR szClassName, LPCTSTR szCaption, LPCTSTR szSkinWndName, CSkinWnd **ppSkin = NULL);
	virtual int CreateSkinWnd(bool bMainWnd, LPCTSTR szClassName, LPCTSTR szCaption, LPCTSTR szSkinWndName, CSkinWnd **ppSkin = NULL);
	virtual int CreateMainSkinWnd(LPCTSTR szClassName, LPCTSTR szCaption, LPCTSTR szSkinWndNameDef, CSkinWnd **ppSkin = NULL);

	virtual void OnSkinWndDestory(CSkinWnd *pSkinWnd);

	virtual void OnMainSkinWndDestory(CSkinWnd *pSkinWnd);

	virtual CSkinWnd *NewSkinWnd(LPCTSTR szSkinWndName, bool bMainWnd);

	// methods for skin window
	SXNode *GetSkinWndNode(LPCTSTR szSkinWndName);

	// create new uiobject by class name
	virtual CUIObject * CreateUIObject(CSkinWnd *pSkin, LPCTSTR szClassName);

	// 
	virtual CUIObject *CreateDynamicCtrl(CSkinContainer *pContainer, LPCTSTR szClassName, LPCTSTR szNameToCheck = NULL, int nIDAssign = SKIN_UIOBJ_ID_UNDEFINE, LPCTSTR szLeft = NULL, LPCTSTR szTop = NULL, LPCTSTR szWidth = NULL, LPCTSTR szHeight = NULL);

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

	// resource
	virtual bool LoadMenu(CSkinWnd *pWnd, CMLMenu **ppMenu, LPCTSTR szMenu) { *ppMenu = NULL; return false; };

	virtual int OnDynamicCmd(int nCmdID, CSkinWnd *pSkinWnd);

	// hue: 0~360,		saturation: 0~1.0,		luminance: 0~1.0
	virtual void AdjustHue(float hue, float saturation = 0.5, float luminance = 0.5);
	void GetAdjustedHueResult(MLColor &clr);
	void GetAdjustedHueResult(COLORREF &clr);

public:
	// ISkinWndDragHost
	virtual void GetWndDragAutoCloseTo(vector<CBaseWnd *> &vWnd);
	virtual void GetWndDragTrackMove(vector<CBaseWnd *> &vWnd);

protected:
	virtual int OpenSkinFile(LPCTSTR szSkinFile);

	virtual int ExeDynamicCmd(const CDynamicCmds::DynamicCmd &cmd, CSkinWnd *pSkinWnd);

protected:
	tstring						m_strSkinRootDir;
	tstring						m_strSkinFileName;
	tstring						m_strSkinName;

	tstring						m_strMainSkinWndClassName;

	CSkinFileXML				m_skinFile;

	// resource manager
	CSkinResMgr					m_resourceMgr;

	//
	// skin中的控件定义
	CDynamicCtrls				m_dynamicCtrls;

	//
	// Dynamic commands define
	CDynamicCmds				m_dynamicCmds;

	// ID 的字符串与数值的映射表结构定义
	STRING_ID_MAP				*m_IdMap;
	typedef map<tstring, int>	MAP_STRID;
	MAP_STRID					m_mapCmdStrId;
	int							m_nCustomCmdIDNext;

	typedef	list<CSkinWnd*>		LIST_SKINWNDS;

	LIST_SKINWNDS				m_listSkinWnds;
	Mutex						m_mutex;


};

#endif // !defined(AFX_SKINFACTORY_H__A24C811F_2487_4750_8378_C7876023C863__INCLUDED_)
