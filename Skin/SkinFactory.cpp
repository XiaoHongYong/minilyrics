// SkinFactory.cpp: implementation of the CSkinFactory class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Skin.h"
#include "SkinFactory.h"
#include "SkinWndResizeObj.h"
#include "SkinFadeArea.h"
#include "SkinDecorativeContainer.h"
#include "SkinToggleCtrlContainer.h"
#include "SkinMenuBar.h"
#include "SkinFrameCtrl.h"
#include "SkinEditCtrl.h"
#include "SkinLinearContainer.h"
#include "SkinResizableLinearContainer.h"

#define _SZ_SKINWND				_T("skinwnd")
#define _SZ_MENUS				_T("Menus")
#define _SZ_DYNMICCTRLS			_T("DynControls")
#define _SZ_DYNMICCMDS			_T("DynCmds")
#define _SZ_CONTROLDEF			_T("controldef")
#define _SZ_SKIN_FILE_NAME		_T("ZikiPlayer.xml")
#define _SZ_ADJUST_HUE			_T("AdjustHue")

#define MENU_ID_CUSTOM_BASE		46000


SkinWndStartupInfo::SkinWndStartupInfo(LPCTSTR szClassName, LPCTSTR szCaptionText, 
									   LPCTSTR szSkinWnd, CBaseWnd *pWndParent, bool bMainWnd)
{
	this->strClassName = szClassName;
	this->strCaptionText = szCaptionText;
	this->strSkinWnd = szSkinWnd;
	this->pWndParent = pWndParent;
	this->bMainWnd = bMainWnd;
	this->pWndParent = pWndParent;
	this->pSkinWnd = NULL;
}

UIObjectIDDefinition g_uidBaseDefinition[] = 
{
	{ _T("ID_MINIMIZE"), CMD_MINIMIZE, 0, _T("Minimize") },
	{ _T("ID_MAXIMIZE"), CMD_MAXIMIZE, 0, _T("Maximize") },
	{ _T("ID_CLOSE"), CMD_CLOSE, IDCLOSE, _T("Close") },
	{ _T("ID_QUIT"), CMD_QUIT, 0, _T("Quit") },
	{ _T("ID_MENU"), CMD_MENU, 0, _T("Menu") },
	{ _T("ID_OK"), CMD_OK, IDOK, NULL },
	{ _T("ID_CANCEL"), CMD_CANCEL, IDCANCEL, NULL },

	{ NULL, 0, 0, NULL },
};

/*
   <DynControls>
     <VScrollBar width="14" PushBtHeight="14" ThumbHeight="28" TrackHeight="28" ScrollBarWidth="14" Image="vscrollbar.bmp"/>
     <ListCtrl BkColor="#4F586E" SelBkColor="#D9D9D9" NowPlayBkColor="#5478CA" TextColor="#FFFFFF" SelTextColor="#4F586E" NowPlayTextColor="#FFFFFF" FontHeight="13"/>
     <Toolbar name="normal" image="toolbar-small.bmp" enablehover="TRUE" height="18" units_x="18" blank_x="216" blank_cx="18" seperator_x="216" seperator_cx="10">
	 </Toolbar>
   </DynControls>
*/

void InsertNodeAbove(SXNode *pNodeParent, LPCTSTR szNodeAbove, SXNode *pNodeChild)
{
	SXNode::iterator	it, itEnd;

	itEnd = pNodeParent->listChildren.end();
	for (it = pNodeParent->listChildren.begin(); it != itEnd; ++it)
	{
		SXNode		*pNode = *it;

		if (_tcsicmp(pNode->strName.c_str(), szNodeAbove) == 0)
		{
			pNodeParent->listChildren.insert(it, pNodeChild);
			return;
		}
	}

	pNodeParent->listChildren.push_back(pNodeChild);
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Class CSkinFileXML
CSkinFileXML::CSkinFileXML()
{
}

CSkinFileXML::~CSkinFileXML()
{
}

int CSkinFileXML::Load(LPCTSTR szFile)
{
	m_strFile = szFile;

	if (!m_xml.ParseFile(szFile))
		return ERR_PARSE_XML;

	if (_tcsicmp(m_xml.m_pRoot->strName.c_str(), _T("skin")) == 0)
		return ERR_OK;
	else
		return ERR_BAD_FILE_FORMAT;
}

int CSkinFileXML::Save(LPCTSTR szFile)
{
	CXMLWriter	xml;

#ifdef _SKIN_EDITOR_
	m_xml.m_pRoot->ToXML(xml);
#endif // _SKIN_EDITOR_

	if (xml.SaveAsFile(szFile))
		return ERR_OK;
	else
		return ERR_WRITE_FILE;
}

void CSkinFileXML::Close()
{
	m_xml.Free();
	m_strFile.resize(0);
}

int CSkinFileXML::GetSkinWndNode(LPCTSTR szSkinWndName, SXNode::iterator *itWnd)
{
	ASSERT(IsOpened());
	if (!IsOpened())
		return ERR_NOT_FOUND;

	SXNode		*pNodeRoot;
	pNodeRoot = m_xml.m_pRoot;
	SXNode::iterator	it, itEnd;

	itEnd = pNodeRoot->listChildren.end();
	for (it = pNodeRoot->listChildren.begin(); it != itEnd; ++it)
	{
		SXNode		*pNode = *it;
		if (_tcsicmp(pNode->strName.c_str(), _SZ_SKINWND) == 0)
		{
			LPCTSTR	szName = pNode->GetProperty(SZ_PN_NAME);
			if (szName && _tcsicmp(szName, szSkinWndName) == 0)
			{
				*itWnd = it;
				return ERR_OK;
			}
		}
	}

	return ERR_NOT_FOUND;
}

SXNode *CSkinFileXML::GetSkinWndNode(LPCTSTR szSkinWndName)
{
	SXNode::iterator	itWnd;
	int		nRet;

	nRet = GetSkinWndNode(szSkinWndName, &itWnd);
	if (nRet == ERR_OK)
		return *itWnd;

	return NULL;
}

int CSkinFileXML::NewSkinWndNode(LPCTSTR szSkinWndName)
{
	ASSERT(IsOpened());
	if (!IsOpened())
		return ERR_NOT_FOUND;

	SXNode::iterator	itWnd;
	int		nRet;
	SXNode	*pWndNode;

	nRet = GetSkinWndNode(szSkinWndName, &itWnd);
	if (nRet == ERR_OK)
		return ERR_EXIST;

	pWndNode = new SXNode;
	pWndNode->strName = _T("skinwnd");
	pWndNode->SetProperty(SZ_PN_NAME, szSkinWndName);
	m_xml.m_pRoot->listChildren.push_back(pWndNode);

	return ERR_OK;
}

int CSkinFileXML::SetSkinWndNode(LPCTSTR szSkinWndName, SXNode *pWndNode)
{
	SXNode::iterator	itWnd;
	int		nRet;

	nRet = GetSkinWndNode(szSkinWndName, &itWnd);
	if (nRet == ERR_OK)
	{
		SXNode *pNode = *itWnd;
		delete pNode;
		*itWnd = pWndNode;
		return ERR_OK;
	}

	ASSERT(0 && _T("Not fond skin wnd node"));

	return nRet;
}

int CSkinFileXML::RemoveSkinWnd(LPCTSTR szSkinWndName)
{
	SXNode::iterator	itWnd;
	int		nRet;

	nRet = GetSkinWndNode(szSkinWndName, &itWnd);
	if (nRet == ERR_OK)
		m_xml.m_pRoot->listChildren.erase(itWnd);

	return nRet;
}

int CSkinFileXML::ChangeSkinWndName(LPCTSTR szNameOld, LPCTSTR szNameNew)
{
	SXNode::iterator	itWnd;
	int		nRet;

	if (_tcsicmp(szNameNew, szNameOld) == 0)
		return ERR_OK;

	if (!CanUseSkinWndName(szNameNew))
		return ERR_EXIST;

	nRet = GetSkinWndNode(szNameOld, &itWnd);
	if (nRet == ERR_OK)
	{
		SXNode	*pNode;
		pNode = *itWnd;
		pNode->SetProperty(SZ_PN_NAME, szNameNew);
	}

	return nRet;
}

bool CSkinFileXML::CanUseSkinWndName(LPCTSTR szName)
{
	SXNode::iterator	itWnd;
	int		nRet;

	nRet = GetSkinWndNode(szName, &itWnd);
	if (nRet == ERR_NOT_FOUND)
		return true;

	return false;
}

SXNode *CSkinFileXML::GetMenusNode()
{
	return m_xml.m_pRoot->GetChild(_SZ_MENUS);
}

int CSkinFileXML::SetMenusNode(SXNode *pNode)
{
	return SetNode(_SZ_MENUS, pNode);
}

SXNode *CSkinFileXML::GetDynamicCmdsNode()
{
	return m_xml.m_pRoot->GetChild(_SZ_DYNMICCMDS);
}

int CSkinFileXML::SetDynamicCmdsNode(SXNode *pNode)
{
	return SetNode(_SZ_DYNMICCMDS, pNode);
}

LPCTSTR CSkinFileXML::GetSkinProperty(LPCTSTR szProperty)
{
	return m_xml.m_pRoot->GetProperty(szProperty);
}

int CSkinFileXML::GetMainSkinWnd(tstring &strSkinWndName, LPCTSTR szSkinWndNameDef)
{
	LPCTSTR		szMainWnds, szDefMainWnd;

	szDefMainWnd = m_xml.m_pRoot->GetPropertySafe(_T("defmainwnd"));
	szMainWnds = m_xml.m_pRoot->GetPropertySafe(_T("mainwnds"));

	strSkinWndName.resize(0);

	//
	// Is szSkinWndName exists in "mainwnds"?
	//
	if (szSkinWndNameDef == NULL || IsEmptyString(szSkinWndNameDef))
	{
		if (!IsEmptyString(szDefMainWnd))
		{
			strSkinWndName = szDefMainWnd;
			return ERR_OK;
		}
	}
	else if (szMainWnds)
	{
		vector<tstring>		vWnds;
		StrSplit(szMainWnds, ',', vWnds);
		for (int i = 0; i < (int)vWnds.size(); i++)
		{
			TrimStr(vWnds[i]);
			if (_tcsicmp(szSkinWndNameDef, vWnds[i].c_str()) == 0)
			{
				strSkinWndName = vWnds[i];
				return ERR_OK;
			}
		}
	}

	//
	// Use defined "defmainwnd"
	//
	if (!IsEmptyString(szDefMainWnd))
	{
		strSkinWndName = szDefMainWnd;
		return ERR_OK;
	}

	vector<tstring>		vWnds;
	if (szMainWnds)
		StrSplit(szMainWnds, ',', vWnds);
	if (vWnds.size())
	{
		TrimStr(vWnds[0]);
		strSkinWndName = vWnds[0];
		return ERR_OK;
	}
	else
	{
		ERRLOG1(_T("No mainwnds is defined: %s"), m_strFile.c_str());
		return ERR_NOT_FOUND;
	}
}

void CSkinFileXML::EnumSkinWnd(vector<tstring> &vSkinWnd)
{
	SXNode		*pNodeRoot;
	pNodeRoot = m_xml.m_pRoot;

	for (SXNode::iterator it = pNodeRoot->listChildren.begin(); 
		it != pNodeRoot->listChildren.end(); ++it)
	{
		SXNode		*pNode = *it;

		if (_tcsicmp(pNode->strName.c_str(), _SZ_SKINWND) == 0)
		{
			LPCTSTR	szName = pNode->GetProperty(SZ_PN_NAME);
			if (szName)
				vSkinWnd.push_back(szName);
		}
	}
}

int CSkinFileXML::SetNode(LPCTSTR szName, SXNode *pNode)
{
	SXNode		*pNodeOld;
	SXNode		*pNodeRoot;

	pNodeRoot = m_xml.m_pRoot;
	for (SXNode::iterator it = pNodeRoot->listChildren.begin(); 
		it != pNodeRoot->listChildren.end(); ++it)
	{
		pNodeOld = *it;
		if (_tcsicmp(pNodeOld->strName.c_str(), szName) == 0)
		{
			delete pNodeOld;
			*it = pNode;
			return ERR_OK;
		}
	}

	InsertNodeAbove(pNodeRoot, szName, pNode);

	return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
//
CSkinStyles::CSkinStyles()
{
}

CSkinStyles::~CSkinStyles()
{
	m_listStyleNodes.clear();
}

int CSkinStyles::FromXML(SXNode *pNodeSkin)
{
	m_listStyleNodes.clear();

	for (SXNode::iterator it = pNodeSkin->listChildren.begin(); 
		it != pNodeSkin->listChildren.end(); ++it)
	{
		SXNode		*pNode = *it;

		if (IsPropertyName(pNode->strName.c_str(), _T("styles")))
		{
			m_listStyleNodes.insert(m_listStyleNodes.end(), pNode->listChildren.begin(), pNode->listChildren.end());
		}
		else if (IsPropertyName(pNode->strName.c_str(), _T("style")))
		{
			m_listStyleNodes.push_back(pNode);
		}
	}

	return ERR_OK;
}

void CSkinStyles::Free()
{
	m_listStyleNodes.clear();
}

SXNode *CSkinStyles::GetClassNode(LPCTSTR szClassName) const
{
	for (SXNode::LIST_CHILDREN::const_iterator itStyle = m_listStyleNodes.begin(); 
		itStyle != m_listStyleNodes.end(); ++itStyle)
	{
		SXNode		*pNodeStyle = *itStyle;

		for (SXNode::iterator it = pNodeStyle->listChildren.begin(); 
			it != pNodeStyle->listChildren.end(); ++it)
		{
			SXNode		*pNode = *it;

			if (IsPropertyName(pNode->strName.c_str(), szClassName))
			{
				return pNode;
			}
		}
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
// CDynamicCmds class

int CDynamicCmds::FromXML(SXNode *pNodeDynCmds)
{
	m_listDynCmd.clear();

	ASSERT(m_pSkinFactory);
	ASSERT(_tcsicmp(pNodeDynCmds->strName.c_str(), _SZ_DYNMICCMDS) == 0);
	for (SXNode::iterator it = pNodeDynCmds->listChildren.begin(); 
		it != pNodeDynCmds->listChildren.end(); ++it)
	{
		SXNode		*pNode = *it;

		if (_tcsicmp(pNode->strName.c_str(), _T("cmd")) == 0)
		{
			DynamicCmd		cmd;
			cmd.nID = m_pSkinFactory->GetIDByName(pNode->GetPropertySafe(SZ_PN_ID));
			cmd.strFunction = pNode->GetPropertySafe(_T("func"));
			cmd.strParam = pNode->GetPropertySafe(_T("param"));
			m_listDynCmd.push_back(cmd);
		}
	}

	return ERR_OK;
}

#ifdef _SKIN_EDITOR_
SXNode *CDynamicCmds::ToXML()
{
	ASSERT(m_pSkinFactory);

	SXNode		*pNode;
	SXNode		*pNodeRoot;
	
	pNodeRoot = new SXNode;
	pNodeRoot->strName = _SZ_DYNMICCMDS;

	LIST_DYNCMD::iterator	it, itEnd;
	itEnd = m_listDynCmd.end();
	for (it = m_listDynCmd.begin(); it != itEnd; ++it)
	{
		DynamicCmd	&dc = *it;

		pNode = new SXNode;
		pNode->strName = _T("cmd");
		pNode->AddProperty(SZ_PN_ID, m_pSkinFactory->GetStringOfID(dc.nID).c_str());
		pNode->AddProperty(_T("func"), dc.strFunction.c_str());
		pNode->AddProperty(_T("param"), dc.strParam.c_str());
		pNodeRoot->listChildren.push_back(pNode);
	}

	return pNodeRoot;
}
#endif // _SKIN_EDITOR_

void CDynamicCmds::Free()
{
	m_listDynCmd.clear();
}

bool CDynamicCmds::GetCmd(int nCmdID, DynamicCmd &cmd)
{
	LIST_DYNCMD::iterator	it, itEnd;

	itEnd = m_listDynCmd.end();
	for (it = m_listDynCmd.begin(); it != itEnd; ++it)
	{
		DynamicCmd	&dc = *it;
		if (dc.nID == nCmdID)
		{
			cmd = dc;
			return true;
		}
	}

	return false;
}

void CDynamicCmds::EnumAllCmds(V_Str &vstrCmds)
{
	ASSERT(m_pSkinFactory);

	LIST_DYNCMD::iterator	it, itEnd;

	itEnd = m_listDynCmd.end();
	for (it = m_listDynCmd.begin(); it != itEnd; ++it)
	{
		DynamicCmd	&dc = *it;
		vstrCmds.push_back(m_pSkinFactory->GetStringOfID(dc.nID));
	}	
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkinFactory::CSkinFactory(CSkinApp *pApp, UIObjectIDDefinition uidDefinition[])
{
	m_pApp = pApp;

	m_strSkinFileName = _SZ_SKIN_FILE_NAME;
	m_nCustomCmdIDNext = CMD_ID_CUSTOM_BASE;
	m_nMenuIDNext = MENU_ID_CUSTOM_BASE;
	m_dynamicCmds.Init(this);

	assert(uidDefinition);
	for (int i = 0; uidDefinition[i].szId != 0; i++)
	{
		ASSERT(m_setUIDDefinition.find(uidDefinition + i) == m_setUIDDefinition.end());
		m_setUIDDefinition.insert(uidDefinition + i);
	}

	for (int i = 0; g_uidBaseDefinition[i].szId != 0; i++)
	{
		ASSERT(m_setUIDDefinition.find(g_uidBaseDefinition + i) == m_setUIDDefinition.end());
		m_setUIDDefinition.insert(g_uidBaseDefinition + i);
	}
}

CSkinFactory::~CSkinFactory()
{

}

int CSkinFactory::Init()
{
	AddUIObjNewer(CSkinButton);
	AddUIObjNewer(CSkinActiveButton);
	AddUIObjNewer(CSkinImageButton);
	AddUIObjNewer(CSkinImage);
	AddUIObjNewer(CSkinNStatusImage);
	AddUIObjNewer(CSkinActiveImage);
	AddUIObjNewer(CSkinXScaleImage);
	m_mapUIObjNewer[_T("XScaleImage")] = new CUIObjectNewer<CSkinXScaleImage>();
	AddUIObjNewer(CSkinYScaleImage);
	m_mapUIObjNewer[_T("YScaleImage")] = new CUIObjectNewer<CSkinYScaleImage>();
	AddUIObjNewer(CSkinVScrollBar);
	AddUIObjNewer(CSkinHScrollBar);
	m_mapUIObjNewer[_T("VScrollBarOSStyle")] = new CUIObjectNewer<CSkinVScrollBar>();
	m_mapUIObjNewer[_T("HScrollBarOSStyle")] = new CUIObjectNewer<CSkinHScrollBar>();
	AddUIObjNewer(CSkinCaption);
	AddUIObjNewer(CSkinNStatusButton);
	AddUIObjNewer(CSkinContainer);
	AddUIObjNewer(CSkinDecorativeContainer);
	AddUIObjNewer(CSkinMenuBar);
	AddUIObjNewer(CSkinWndResizeObj);
	AddUIObjNewer(CSkinEditCtrl);
	AddUIObjNewer(CSkinSquare);
	AddUIObjNewer(CSkinFadeArea);
	AddUIObjNewer(CSkinToggleCtrlContainer);
	AddUIObjNewer(CSkinClientArea);
	AddUIObjNewer(CSkinAnimationUIObj);
	AddUIObjNewer(CSkinFrameCtrl);
	AddUIObjNewer(CSkinHoverActionButton);
	AddUIObjNewer(CSkinComboBox);
	AddUIObjNewer(CSkinListCtrl);
	AddUIObjNewer(CSkinSeekCtrl);
	AddUIObjNewer(CSkinStaticText);
	AddUIObjNewer(CSkinTabButton);
	AddUIObjNewer(CSkinTextButton);
	AddUIObjNewer(CSkinToolbar);
	AddUIObjNewer(CSkinTxtLink);
	AddUIObjNewer(CSkinLinearContainer);
	AddUIObjNewer(CSkinResizableLinearContainer);

	// Init SimpleJVM environment
	CSjvmSkinSystem::Init(this);

	return ERR_OK;
}

void CSkinFactory::Quit()
{
	for (MapUIObjNewer::iterator it = m_mapUIObjNewer.begin();
		it != m_mapUIObjNewer.end(); ++it)
		delete (*it).second;
	m_mapUIObjNewer.clear();

	Close(true);
}

int CSkinFactory::GetIDByNameEx(LPCTSTR szId, tstring *pstrToolTip)
{
	assert(szId);
	if (IsEmptyString(szId))
		return UID_INVALID;

	if (pstrToolTip)
		pstrToolTip->resize(0);

	UIObjectIDDefinition uidDef;
	uidDef.szId = szId;

	SetUIObjectIDDefinition::iterator it = m_setUIDDefinition.find(&uidDef);
	if (it != m_setUIDDefinition.end())
	{
		UIObjectIDDefinition *p = *it;
		if (pstrToolTip && p->szToolTip)
			*pstrToolTip = p->szToolTip;
		return p->nId;
	}

	m_nCustomCmdIDNext++;

	UIObjectIDDefinition *p = (UIObjectIDDefinition *)m_allocator.allocate(sizeof(UIObjectIDDefinition));
	p->szId = m_allocator.allocate(szId);
	p->nIdMenu = 0;
	p->szToolTip = NULL;
	p->nId = m_nCustomCmdIDNext;
	m_setUIDDefinition.insert(p);

	return m_nCustomCmdIDNext;
}

void CSkinFactory::GetTooltip(int nUID, tstring &strToolTip)
{
	UIObjectIDDefinition *p = GetUidDef(nUID);
	if (p && p->szToolTip)
		strToolTip = p->szToolTip;
	else
		strToolTip.resize(0);
}

int CSkinFactory::GetMenuIDByUID(int nUID, bool bAllocIfInexistence)
{
	UIObjectIDDefinition *p = GetUidDef(nUID);
	if (!p)
		return 0;

	if (p->nIdMenu == 0)
		p->nIdMenu = ++m_nMenuIDNext;
	return p->nIdMenu;
}

bool CSkinFactory::SetMenuIDByUID(int nUID, int nMenuId)
{
	UIObjectIDDefinition *p = GetUidDef(nUID);
	if (!p)
		return false;

	ASSERT(p->nIdMenu == 0 || p->nIdMenu == nMenuId);
	p->nIdMenu = nMenuId;
	return true;
}

int CSkinFactory::GetUIDByMenuID(int nMenuID)
{
	if (nMenuID == 0)
		return UID_INVALID;

	for (SetUIObjectIDDefinition::iterator it = m_setUIDDefinition.begin();
		it != m_setUIDDefinition.end(); ++it)
	{
		UIObjectIDDefinition *p = *it;
		if (p->nIdMenu == nMenuID)
			return p->nId;
	}

	return UID_INVALID;
}

#ifdef _DEBUG
void CSkinFactory::DumpUID()
{
	typedef map<int, tstring>	MapString;

	MapString mapUID;

	for (SetUIObjectIDDefinition::iterator it = m_setUIDDefinition.begin();
		it != m_setUIDDefinition.end(); ++it)
	{
		UIObjectIDDefinition *p = *it;
		MapString::iterator i = mapUID.find(p->nId);
		// ASSERT(i == mapUID.end());
		mapUID[p->nId] = tstring(p->szId);
	}

	for (MapString::iterator it = mapUID.begin(); it != mapUID.end(); ++it)
		DBGLOG2(_T("UID: %04X, %s"), (*it).first, (*it).second.c_str());
}
#endif

UIObjectIDDefinition *CSkinFactory::GetUidDef(int nUID)
{
	if (nUID == UID_INVALID)
		return NULL;

	for (SetUIObjectIDDefinition::iterator it = m_setUIDDefinition.begin();
		it != m_setUIDDefinition.end(); ++it)
	{
		UIObjectIDDefinition *p = *it;
		if (p->nId == nUID)
			return p;
	}

	return NULL;
}

int CSkinFactory::AllocUID()
{
	m_nCustomCmdIDNext++;
	ASSERT(m_nCustomCmdIDNext <= uint16(-1));
	if (m_nCustomCmdIDNext >= uint16(-1))
		m_nCustomCmdIDNext = CMD_ID_CUSTOM_BASE + 1000;

	return m_nCustomCmdIDNext;
}

tstring CSkinFactory::GetStringOfID(int nID)
{
	for (SetUIObjectIDDefinition::iterator it = m_setUIDDefinition.begin();
		it != m_setUIDDefinition.end(); ++it)
	{
		UIObjectIDDefinition *p = *it;
		if (p->nId == nID)
			return p->szId;
	}

	return _T("");
}

int CSkinFactory::OpenSkin(LPCTSTR szSkinName, LPCTSTR szSkinDir, LPCTSTR szExtraDir)
{
	TCHAR		szSkinFile[MAX_PATH];
	tstring		strSkinDir;

	ASSERT(szSkinDir && szExtraDir);

	Close();

	// If szSkinDir is NULL, set to default position: %SkinRoot%\\szSkinName.
	if (szSkinDir == NULL || IsEmptyString(szSkinDir))
	{
		strSkinDir = GetSkinRootDir();
		strSkinDir += szSkinName;
		szSkinDir = strSkinDir.c_str();
	}

	m_strSkinName = szSkinName;

	m_resourceMgr.ClearRessourceDir();
	m_resourceMgr.AddRessourceDir(szSkinDir);
	if (szExtraDir && !IsEmptyString(szExtraDir))
		m_resourceMgr.AddRessourceDir(szExtraDir);
	m_resourceMgr.AddRessourceDir(GetSkinRootDir());

	if (!m_resourceMgr.GetResourcePathName(szSkinFile, CountOf(szSkinFile), m_strSkinFileName.c_str()))
	{
		ERRLOG2(_T("Can't find skin file in dir: %s, %s"), szSkinDir, szExtraDir);
		return ERR_OPEN_FILE;
	}

	return OpenSkinFile(szSkinFile);
}

int CSkinFactory::ChangeSkin(LPCTSTR szSkinName, LPCTSTR szSkinDir, LPCTSTR szExtraDir, bool bLoadSkinColorTheme, bool bCreateSkinWnd)
{
	int nRet = OpenSkin(szSkinName, szSkinDir, szExtraDir);
	if (nRet != ERR_OK)
		return nRet;

	if (bLoadSkinColorTheme)
	{
		// Load the default theme settings of skin
		g_Profile.WriteProfileInt(szSkinName, _T("Hue"), 0);

		ApplyDefaultThemeOfSkin();
	}

	float hue = (float)g_Profile.GetProfileInt(szSkinName, _T("Hue"), 0);
	if (hue < 0)
		hue = 0;
	else if (hue > 360)
		hue = 360;
	m_resourceMgr.AdjustHue((float)hue, 0.5, 0.5);

	if (!bCreateSkinWnd)
		return ERR_OK;

	// change the skins of existed windows.
	tstring		strSkinWndDef;
	tstring		strSkinWndName;
	tstring		strOpenedSkinWnds;
	SetTStrNCase	setOpenedSkinWnds;

	strOpenedSkinWnds = g_Profile.GetProfileString(GetSkinFileName(), _T("LatestOpenedSkinWnds"), _T(""));
	strSkinWndDef = g_Profile.GetProfileString(GetSkinFileName(), _T("DefaultSkinWnd"), _T(""));

	StrSplit(strOpenedSkinWnds.c_str(), ',', setOpenedSkinWnds);

	nRet = m_skinFile.GetMainSkinWnd(strSkinWndName, strSkinWndDef.c_str());
	if (nRet != ERR_OK)
	{
		ERRLOG1(_T("Can't get main window name: %s"), m_skinFile.GetFileName());
		return nRet;
	}

	{
		// Remove main window from other windows.
		SetTStrNCase::iterator it = setOpenedSkinWnds.find(strSkinWndName.c_str());
		if (it != setOpenedSkinWnds.end())
			setOpenedSkinWnds.erase(it);
	}

	// Create or Open Main Window
	CSkinWnd *pMainWnd = NULL;
	if (m_listSkinWnds.empty())
	{
		pMainWnd = NewSkinWnd(_T("MiniLyrics"), true);

		SkinWndStartupInfo skinWndStartupInfo(_T("MiniLyrics"), _T("MiniLyrics"), strSkinWndName.c_str(), NULL, true);
		int nRet = pMainWnd->Create(skinWndStartupInfo, this, false);
		ASSERT(nRet == ERR_OK);
		if (nRet == ERR_OK)
			pMainWnd->SetMainAppWnd(true);
	}
	else
	{
		pMainWnd = m_listSkinWnds.front();
		nRet = pMainWnd->OpenSkin(strSkinWndName.c_str());
		if (nRet != ERR_OK)
			return nRet;
		pMainWnd->ShowWindow(SW_SHOW);
	}

	LIST_SKINWNDS		listToBeProcessed;

	// Destroy other skin windows.
	if (m_listSkinWnds.size() > 1)
	{
		listToBeProcessed.insert(listToBeProcessed.begin(), ++(m_listSkinWnds.begin()), m_listSkinWnds.end());

		LIST_SKINWNDS::iterator	it;
		for (it = listToBeProcessed.begin(); it != listToBeProcessed.end(); ++it)
		{
			CSkinWnd	*pSkinWnd = *it;
			nRet = pSkinWnd->OpenDefaultSkin();
			if (nRet != ERR_OK)
				pSkinWnd->Destroy();
			else
				setOpenedSkinWnds.erase(pSkinWnd->GetSkinWndName());
		}
	}

	for (SetTStrNCase::iterator itSet = setOpenedSkinWnds.begin();
		itSet != setOpenedSkinWnds.end(); ++itSet)
	{
		if (!IsAppResSkinWnd((*itSet).c_str()))
		{
			SkinWndStartupInfo skinWndStartupInfo(_SZ_SKINWND_CLASS_NAME, (*itSet).c_str(), (*itSet).c_str(), NULL);
			nRet = CreateSkinWnd(skinWndStartupInfo);
		}
	}

	return ERR_OK;
}

void CSkinFactory::GetWndDragAutoCloseTo(vector<CBaseWnd *> &vWnd)
{
	LIST_SKINWNDS::iterator	it, itEnd;
	itEnd = m_listSkinWnds.end();
	for (it = m_listSkinWnds.begin(); it != itEnd; ++it)
	{
		CSkinWnd	*pSkinWnd = *it;
		vWnd.push_back(pSkinWnd);
	}
}

void CSkinFactory::GetWndDragTrackMove(vector<CBaseWnd *> &vWnd)
{
	LIST_SKINWNDS::iterator	it, itEnd;
	itEnd = m_listSkinWnds.end();
	for (it = m_listSkinWnds.begin(); it != itEnd; ++it)
	{
		CSkinWnd	*pSkinWnd = *it;
		vWnd.push_back(pSkinWnd);
	}
}

int CSkinFactory::OpenSkinFile(LPCTSTR szSkinFile)
{
	int				nRet;
	SXNode			*pNode;

	nRet = m_skinFile.Load(szSkinFile);
	if (nRet != ERR_OK)
	{
		ERRLOG1(_T("Failed to parse skin file(xml): %s"), szSkinFile);
		return nRet;
	}

	// Extra resource folder can be muliple folder, seperated with ','.
	LPCTSTR szExtraResouceFolder = m_skinFile.GetSkinProperty(_T("ExtraResouceFolder"));
	if (szExtraResouceFolder && !IsEmptyString(szExtraResouceFolder))
	{
		V_Str		vExtraResDirs;

		StrSplit(szExtraResouceFolder, ',', vExtraResDirs);

		for (uint i = 0; i < vExtraResDirs.size(); ++i)
		{
			TrimStr(vExtraResDirs[i]);
			if (vExtraResDirs[i].empty())
				continue;
#ifdef _WIN32
			strrep(vExtraResDirs[i], _T("/"), _T(SZ_DIR_SLASH));
#else
			strrep(vExtraResDirs[i], _T("\\"), _T(SZ_DIR_SLASH));
#endif
			tstring		strExtraDirAbs;
			strExtraDirAbs = GetSkinRootDir();
			if (vExtraResDirs[i][0] == DIR_SLASH)
				strExtraDirAbs += vExtraResDirs[i].c_str() + 1;
			else
				strExtraDirAbs += vExtraResDirs[i];
			m_resourceMgr.AddRessourceDir(strExtraDirAbs.c_str(), 1);
		}
	}

	ExpandIncludeNode(m_skinFile.GetRootNode());

	m_skinStyle.FromXML(m_skinFile.GetRootNode());

	pNode = m_skinFile.GetDynamicCmdsNode();
	if (pNode)
		m_dynamicCmds.FromXML(pNode);

	// Load and compile SJVM script files.
	LPCTSTR szScript = m_skinFile.GetSkinProperty(_T("Script"));
	if (szScript && !IsEmptyString(szScript))
		CSjvmSkinSystem::LoadScript(szScript);

	return ERR_OK;
}

void CSkinFactory::ExpandIncludeNode(SXNode *pNodeRoot)
{
	for (SXNode::LIST_CHILDREN::iterator it = pNodeRoot->listChildren.begin();
		it != pNodeRoot->listChildren.end(); ++it)
	{
		SXNode	*p = *it;
		if (IsPropertyName(p->strName.c_str(), _T("include")))
		{
			LPCTSTR szName = p->GetProperty(SZ_PN_NAME);
			if (!szName || IsEmptyString(szName))
			{
				ERRLOG0(_T("Invalid include node"));
				continue;
			}

			CSimpleXML	xml;
			TCHAR		szIncludeFile[MAX_PATH];
			if (!m_resourceMgr.GetResourcePathName(szIncludeFile, CountOf(szIncludeFile), szName))
			{
				ERRLOG1(_T("Can't get resource file: %s"), szName);
				continue;
			}

			if (!xml.ParseFile(szIncludeFile))
			{
				ERRLOG1(_T("Failed to parse xml file: %s"), szIncludeFile);
				continue;
			}
			ExpandIncludeNode(xml.m_pRoot);

			delete p;
			*it = xml.m_pRoot;
			xml.m_pRoot = NULL;
		}
		else
			ExpandIncludeNode(p);
	}
}

int CSkinFactory::SaveSkinFile(LPCTSTR szFile)
{
/*
	CXMLWriter		xmlStream;

	xmlStream.pushCategory(_T("skin"));
	ToXML(xmlStream);
	xmlStream.popCategory();

	if (xmlStream.SaveAsFile(szFile))
		return ERR_OK;

*/
	return ERR_FAILED;
}

void CSkinFactory::Close(bool bQuit)
{
	m_skinStyle.Free();
	m_dynamicCmds.Free();
	m_skinFile.Close();

	// Save opened window name, and close it.
	tstring		strOpenedSkinWnds;
	LIST_SKINWNDS	listSkinWnds = m_listSkinWnds;
	for (LIST_SKINWNDS::iterator it = listSkinWnds.begin(); it != listSkinWnds.end(); ++it)
	{
		CSkinWnd	*pSkin = *it;

		// If the skin window has been destroyed, ignore it.
		if (find(m_listSkinWnds.begin(), m_listSkinWnds.end(), *it) == m_listSkinWnds.end())
			continue;

		if (pSkin->IsMainAppWnd())
			g_Profile.WriteProfileString(GetSkinFileName(), _T("DefaultSkinWnd"), pSkin->GetSkinWndName());
		else
		{
			if (!strOpenedSkinWnds.empty())
				strOpenedSkinWnds += _T(",");
			strOpenedSkinWnds += pSkin->GetSkinWndName();
		}

		if (bQuit)
			pSkin->Destroy();
		else
			pSkin->CloseSkin();
	}
	if (listSkinWnds.size() > 0)
		g_Profile.WriteProfileString(GetSkinFileName(), _T("LatestOpenedSkinWnds"), strOpenedSkinWnds.c_str());

	// ASSERT(m_listSkinWnds.size() <= 1);

	m_resourceMgr.OnClose();

	// Recover the original UIDs.
	m_nCustomCmdIDNext = CMD_ID_CUSTOM_BASE;
	m_nMenuIDNext = MENU_ID_CUSTOM_BASE;
	for (SetUIObjectIDDefinition::iterator it = m_setUIDDefinition.begin();
		it != m_setUIDDefinition.end();)
	{
		UIObjectIDDefinition *p = *it;
		if (p->nId >= CMD_ID_CUSTOM_BASE)
			it = m_setUIDDefinition.erase(it);
		else
			++it;
	}

	CSjvmSkinSystem::CloseScript();
}

CSkinWnd *CSkinFactory::NewSkinWnd(LPCTSTR szSkinWndName, bool bMainWnd)
{
	return new CSkinWnd;
}

CSkinWnd *CSkinFactory::FindSkinWnd(LPCTSTR szSkinWndName)
{
	LIST_SKINWNDS::iterator	it, itEnd;
	itEnd = m_listSkinWnds.end();
	for (it = m_listSkinWnds.begin(); it != itEnd; ++it)
	{
		CSkinWnd	*pSkinWnd = *it;
		if (_tcsicmp(szSkinWndName, pSkinWnd->GetSkinWndName()) == 0)
		{
			return pSkinWnd;
		}
	}

	return NULL;
}

bool CSkinFactory::IsValidSkinWnd(CSkinWnd *pWnd)
{
	LIST_SKINWNDS::iterator	it;
	for (it = m_listSkinWnds.begin(); it != m_listSkinWnds.end(); ++it)
	{
		if (*it == pWnd)
			return true;
	}

	return false;
}

int CSkinFactory::ActiveOrCreateSkinWnd(SkinWndStartupInfo &skinWndStartupInfo)
{
	CSkinWnd	*pSkinWnd = FindSkinWnd(skinWndStartupInfo.strSkinWnd.c_str());
	if (pSkinWnd)
	{
		if (!pSkinWnd->IsVisible())
			pSkinWnd->ShowWindow(SW_SHOW);
		if (pSkinWnd->IsIconic())
			pSkinWnd->ShowWindow(SW_RESTORE);
		pSkinWnd->SetForeground();
		skinWndStartupInfo.pSkinWnd = pSkinWnd;
		return ERR_OK;
	}

	return CreateSkinWnd(skinWndStartupInfo);
}

int CSkinFactory::OpenOrCloseSkinWnd(SkinWndStartupInfo &skinWndStartupInfo)
{
	CSkinWnd	*pSkinWnd = FindSkinWnd(skinWndStartupInfo.strSkinWnd.c_str());
	if (pSkinWnd)
	{
		pSkinWnd->Destroy();
		return ERR_OK;
	}

	return CreateSkinWnd(skinWndStartupInfo);
}

int CSkinFactory::CreateSkinWnd(SkinWndStartupInfo &skinWndStartupInfo)
{
	// If the skinwnd doesn't exist, return ERR_NOT_FOUND
	if (GetSkinWndNode(skinWndStartupInfo.strSkinWnd.c_str()) == NULL)
	{
		if (!IsAppResSkinWnd(skinWndStartupInfo.strSkinWnd.c_str()))
			return ERR_NOT_FOUND;
	}

	// If the pSkinWnd is newed already, use it.
	if (skinWndStartupInfo.pSkinWnd == NULL)
		skinWndStartupInfo.pSkinWnd = NewSkinWnd(skinWndStartupInfo.strSkinWnd.c_str(), skinWndStartupInfo.bMainWnd);
	ASSERT(skinWndStartupInfo.pSkinWnd);
	skinWndStartupInfo.pSkinWnd->SetFreeOnDestory(true);
	int nRet = skinWndStartupInfo.pSkinWnd->Create(skinWndStartupInfo, this);
	if (nRet == ERR_OK)
	{
#ifdef _WIN32
		if (!skinWndStartupInfo.bMainWnd && skinWndStartupInfo.pSkinWnd->IsWindowsAppearance())
			ShowAsAppWindow(skinWndStartupInfo.pSkinWnd->GetHandle());
#endif
	}
	else
		delete skinWndStartupInfo.pSkinWnd;

	return nRet;
}

void CSkinFactory::OnSkinWndCreate(CSkinWnd *pSkinWnd)
{
	LIST_SKINWNDS::iterator	it;

#ifdef _DEBUG
	for (it = m_listSkinWnds.begin(); it != m_listSkinWnds.end(); ++it)
	{
		CSkinWnd	*pSkin = *it;
		ASSERT(pSkin != pSkinWnd);
	}
#endif

	m_listSkinWnds.push_back(pSkinWnd);
}

void CSkinFactory::OnSkinWndDestory(CSkinWnd *pSkinWnd)
{
	LIST_SKINWNDS::iterator	it, itEnd;

	itEnd = m_listSkinWnds.end();
	for (it = m_listSkinWnds.begin(); it != itEnd; ++it)
	{
		CSkinWnd	*pSkin = *it;
		if (pSkin == pSkinWnd)
		{
			m_listSkinWnds.erase(it);
			return;
		}
	}
}

void CSkinFactory::OnMainSkinWndDestory(CSkinWnd *pSkinWnd)
{
	int nMainAppWnd = 0;
	for (LIST_SKINWNDS::iterator it = m_listSkinWnds.begin();
		it != m_listSkinWnds.end(); ++it)
	{
		CSkinWnd *p = *it;
		if (p->IsMainAppWnd())
			nMainAppWnd++;
	}
	if (nMainAppWnd <= 1)
		m_pApp->PostQuitMessage();
}

// methods for skin window
SXNode *CSkinFactory::GetSkinWndNode(LPCTSTR szSkinWndName)
{
	return m_skinFile.GetSkinWndNode(szSkinWndName);
}

int CSkinFactory::LoadAppResSkinWndXml(LPCTSTR szSkinWndName, CSimpleXML &xml)
{
	TCHAR	szFile[MAX_PATH];

	if (!m_resourceMgr.GetResourcePathName(szFile, CountOf(szFile), szSkinWndName))
		return ERR_NOT_FOUND;

	if (!xml.ParseFile(szFile))
		return ERR_PARSE_XML;

	ExpandIncludeNode(xml.m_pRoot);

	return ERR_OK;
}

bool CSkinFactory::IsAppResSkinWnd(LPCTSTR szSkinWndName)
{
	TCHAR	szFile[MAX_PATH];

	if (m_resourceMgr.GetResourcePathName(szFile, CountOf(szFile), szSkinWndName))
		return true;

	return false;
}

void CSkinFactory::RegisterUIObjNewer(LPCTSTR szClassName, IUIObjNewer *pObjNewer)
{
	ASSERT(m_mapUIObjNewer.find(szClassName) == m_mapUIObjNewer.end());
	m_mapUIObjNewer[szClassName] = pObjNewer;
}

// create new uiobject by class name
CUIObject * CSkinFactory::CreateUIObject(CSkinWnd *pSkin, LPCTSTR szClassName, CSkinContainer *pContainer)
{
	SXNode *pNode = m_skinStyle.GetClassNode(szClassName);
	if (!pNode)
		return NewUIObject(pSkin, pContainer, szClassName);

	LPCTSTR szExtends = pNode->GetProperty(SZ_PN_EXTENDS);
	if (szExtends)
		szClassName = szExtends;

	CUIObject *pObj = NewUIObject(pSkin, pContainer, szClassName);

	// Set styled properties of Extends from
	if (pObj)
		pObj->FromXML(pNode);

	return pObj;
}

CUIObject *CSkinFactory::NewUIObject(CSkinWnd *pSkin, CSkinContainer *pContainer, LPCTSTR szClassName)
{
	MapUIObjNewer::iterator it = m_mapUIObjNewer.find(szClassName);
	if (it == m_mapUIObjNewer.end())
		return NULL;

	CUIObject *pObj = (*it).second->New();
	ASSERT(pObj);
	pObj->SetParent(pSkin, pContainer);

	return pObj;
}

CUIObject *CSkinFactory::CreateDynamicCtrl(CSkinContainer *pContainer, LPCTSTR szClassName, int nIDAssign, LPCTSTR szLeft, LPCTSTR szTop, LPCTSTR szWidth, LPCTSTR szHeight)
{
	CUIObject			*pObj = NULL;

	// Create the control
	pObj = CreateUIObject(pContainer->GetSkinWnd(), szClassName, pContainer);
	assert(pObj);
	if (pObj)
	{
		if (nIDAssign == UID_INVALID)
			nIDAssign = pContainer->GetSkinWnd()->AllocUIObjID();
		pObj->m_nId =  nIDAssign;

		//
		// 先使用参数来设置高度和宽度
		if (szWidth && !IsEmptyString(szWidth))
			pObj->SetProperty(SZ_PN_WIDTH, szWidth);
		if (szHeight && !IsEmptyString(szHeight))
			pObj->SetProperty(SZ_PN_HEIGHT, szHeight);
		if (szLeft && !IsEmptyString(szLeft))
			pObj->SetProperty(SZ_PN_LEFT, szLeft);
		if (szLeft && !IsEmptyString(szTop))
			pObj->SetProperty(SZ_PN_TOP, szTop);

		// 添加到数组中
		pContainer->AddUIObject(pObj);
	}

	return pObj;
}

void CSkinFactory::SetSkinsRootDir(LPCTSTR szSkinsRootDir)
{
	m_strSkinRootDir = szSkinsRootDir;
	if (!m_strSkinRootDir.empty())
	{
		DirStringFormat(m_strSkinRootDir);
		g_Profile.WriteProfileString(_T("SkinRootDir"), m_strSkinRootDir.c_str());
	}
}

LPCTSTR CSkinFactory::GetSkinRootDir()
{
	if (m_strSkinRootDir.empty())
	{
		m_strSkinRootDir = g_Profile.GetProfileString(_T("SkinRootDir"), _T(""));
		if (!m_strSkinRootDir.empty())
		{
			DirStringFormat(m_strSkinRootDir);
			if (IsDirExist(m_strSkinRootDir.c_str()))
				return m_strSkinRootDir.c_str();
		}

		m_strSkinRootDir = GetWorkingFolder();
		m_strSkinRootDir += _T("skins");
		m_strSkinRootDir += _T(SZ_DIR_SLASH);

#if defined (_DEBUG) && defined (_WIN32)
		if (!IsDirExist(m_strSkinRootDir.c_str()))
		{
			m_strSkinRootDir = GetInstallShareDir();

			m_strSkinRootDir += _T("skins\\");
		}
#endif
	}

	return m_strSkinRootDir.c_str();
}

void CSkinFactory::SetSkinFileName(LPCTSTR szSkinFileName)
{
	m_strSkinFileName = szSkinFileName;

#if defined (_DEBUG) && defined (_WIN32)
// 	if (_tcsicmp(m_strSkinFileName.c_str(), _T("iPodLyricsDownloader.xml")) == 0)
// 	{
// 		m_strSkinRootDir = GetWorkingFolder();
// 		m_strSkinRootDir += _T("..\\Install_iPodLyricsDownloader\\Share\\skins\\");
// 	}
#endif
}

// 枚举 skin
// COMMENT:
//		查找Skin目录下的第一个Skin
// RETURN:
//		The handle of find skin
//		INVALID_HANDLE_VALUE - 没有找到
bool CSkinFactory::FindFirstSkin(tstring &strSkin)
{
	CMLFileFind		find;
	tstring			strFile;

	if (!find.OpenDir(GetSkinRootDir()))
		return false;

	while (find.FindNext())
	{
		if (find.IsCurDir())
		{
			strFile = GetSkinRootDir();
			strFile += find.GetCurName();
			strFile += _T(SZ_DIR_SLASH);
			strFile += m_strSkinFileName;

			if (IsFileExist(strFile.c_str()))
			{
				strSkin = find.GetCurName();
				return true;
			}
		}
	}

	return false;
}

bool CSkinFactory::EnumAllSkins(vector<tstring> &vSkins)
{
	CMLFileFind		find;
	tstring			strFile;

	if (!find.OpenDir(GetSkinRootDir()))
		return false;

	while (find.FindNext())
	{
		if (find.IsCurDir())
		{
			strFile = GetSkinRootDir();
			strFile += find.GetCurName();
			strFile += _T(SZ_DIR_SLASH);
			strFile += m_strSkinFileName;

			if (IsFileExist(strFile.c_str()))
				vSkins.push_back(find.GetCurName());
		}
	}

	// sort(vSkins.begin(), vSkins.end());

	return true;
}

CSkinWnd *CSkinFactory::GetMainWnd()
{
	for (LIST_SKINWNDS::iterator it = m_listSkinWnds.begin();
		it != m_listSkinWnds.end(); ++it)
	{
		CSkinWnd *p = *it;
		if (p->IsMainAppWnd())
			return p;
	}

	return NULL;
}

SXNode *CSkinFactory::GetExtendSkinWndStyle(LPCTSTR szExtendsSkinWnd) const
{
	return m_skinStyle.GetClassNode(szExtendsSkinWnd);
}

static SXNode *GetMenuOfMenus(SXNode *pNodeMenus, LPCTSTR szMenuName)
{
	SXNode::iterator it, itEnd;
	itEnd = pNodeMenus->listChildren.end();
	for (it = pNodeMenus->listChildren.begin(); it != itEnd; ++it)
	{
		SXNode		*pNode = *it;

		// OK, found the contorl
		if (_tcsicmp(pNode->strName.c_str(), _T("Menu")) == 0)
		{
			if (_tcsicmp(pNode->GetPropertySafe(_T("Name")), szMenuName) == 0)
				return pNode;
		}
	}

	return NULL;
}

bool CSkinFactory::LoadMenu(CSkinWnd *pWnd, CMLMenu **ppMenu, LPCTSTR szMenu)
{
	CSkinMenu	*pSkinMenu = NULL;
	SXNode		*pNodeMenus = NULL, *pNode = NULL;

	*ppMenu = NULL;

	// Load menu from xml
	pNodeMenus = m_skinFile.GetMenusNode();
	if (pNodeMenus)
	{
		pNode = GetMenuOfMenus(pNodeMenus, szMenu);
		if (pNode)
		{
			LPCTSTR szFromMenu = pNode->GetProperty(_T("From"));
			if (szFromMenu)
				pSkinMenu = LoadPresetMenu(pWnd, szFromMenu);

			if (!pSkinMenu)
				pSkinMenu = new CSkinMenu(this);

			pSkinMenu->FromXML(pNode, pSkinMenu->GetOrgAppendPos());
		}
	}

	if (!pSkinMenu)
	{
		// is this a preset menu?
		pSkinMenu = LoadPresetMenu(pWnd, szMenu);
		if (pSkinMenu)
		{
			*ppMenu = pSkinMenu;
			return true;
		}
	}

	*ppMenu = pSkinMenu;

	return pSkinMenu != NULL;
}

int CSkinFactory::OnDynamicCmd(int nCmdID, CSkinWnd *pSkinWnd)
{
	int			nRet = ERR_NOT_FOUND;

	CDynamicCmds::DynamicCmd	cmd;
	if (m_dynamicCmds.GetCmd(nCmdID, cmd))
	{
		nRet = ExeDynamicCmd(cmd, pSkinWnd);
	}
	else
		nRet = ERR_NOT_FOUND;

	return nRet;
}

int CSkinFactory::ExeDynamicCmd(const CDynamicCmds::DynamicCmd &cmd, CSkinWnd *pSkinWnd)
{
	if (cmd.strFunction == _T("change_skin_wnd"))
		return pSkinWnd->OpenSkin(cmd.strParam.c_str());
	else if (cmd.strFunction == _T("create_skin_wnd"))
	{
		SkinWndStartupInfo skinWndStartupInfo(_SZ_SKINWND_CLASS_NAME, cmd.strParam.c_str(), cmd.strParam.c_str(), NULL);
		return ActiveOrCreateSkinWnd(skinWndStartupInfo);
	}
	else if (cmd.strFunction == _T("openclose_skin_wnd"))
	{
		CSkinWnd	*pSkinWnd = FindSkinWnd(cmd.strParam.c_str());
		if (pSkinWnd)
		{
			if (pSkinWnd->IsIconic())
				pSkinWnd->ActivateWindow();
			else
				pSkinWnd->PostDestroy();
			return ERR_OK;
		}

		SkinWndStartupInfo skinWndStartupInfo(_SZ_SKINWND_CLASS_NAME, cmd.strParam.c_str(), cmd.strParam.c_str(), NULL);
		return CreateSkinWnd(skinWndStartupInfo);
	}
	else if (cmd.strFunction == _T("is_skin_wnd_visible"))
	{
		CSkinWnd	*pSkinWnd = FindSkinWnd(cmd.strParam.c_str());
		if (pSkinWnd)
			return ERR_OK;
		else
			return ERR_FALSE;
	}
	else if (cmd.strFunction == _T("load_ctrl_visible_state"))
	{
		V_Str		vStrParam;
		CUIObject	*pObj;

		StrSplit(cmd.strParam.c_str(), ',', vStrParam);
		if (vStrParam.size() > 2 || vStrParam.size() == 0)
			return ERR_FALSE;
		for (int i = 0; i < (int)vStrParam.size(); i++)
		{
			TrimStr(vStrParam[i]);

			pObj = pSkinWnd->GetUIObjectById(GetIDByName(vStrParam[0].c_str()));
			if (pObj)
			{
				// Proceed only if visible state is set.
				tstring strValue = pObj->GetProfileString(_T("VisibleState"), _T(""));
				if (!strValue.empty())
					pObj->SetVisible(IsTRUE(strValue.c_str()));
			}
		}

		pSkinWnd->InvalidateRect();

		return ERR_OK;
	}
	else if (cmd.strFunction == _T("save_ctrl_visible_state"))
	{
		V_Str		vStrParam;
		CUIObject	*pObj;

		StrSplit(cmd.strParam.c_str(), ',', vStrParam);
		for (int i = 0; i < (int)vStrParam.size(); i++)
		{
			TrimStr(vStrParam[i]);

			pObj = pSkinWnd->GetUIObjectById(GetIDByName(vStrParam[i].c_str()));
			if (pObj)
				pObj->WriteProfileInt(_T("VisibleState"), pObj->IsVisible());
		}

		return ERR_OK;
	}
	else if (cmd.strFunction == _T("toggle_ctrl_visible"))
	{
		V_Str		vStrParam;
		CUIObject	*pObj;

		StrSplit(cmd.strParam.c_str(), ',', vStrParam);
		for (int i = 0; i < (int)vStrParam.size(); i++)
			TrimStr(vStrParam[i]);

		pObj = pSkinWnd->GetUIObjectById(GetIDByName(vStrParam[0].c_str()));
		if (pObj)
		{
			bool	bVisible;
			if (vStrParam.size() > 1)
				bVisible = IsTRUE(vStrParam[1].c_str());
			else
				bVisible = !pObj->IsVisible();
			pObj->SetVisible(bVisible);
			pSkinWnd->InvalidateRect();
			return ERR_OK;
		}
		else
			return ERR_FALSE;
	}
	else if (cmd.strFunction == _T("show_hide_ctrl"))
	{
		V_Str		vStrParam;
		CUIObject	*pObj;

		StrSplit(cmd.strParam.c_str(), ',', vStrParam);
		if (vStrParam.size() != 2)
			return ERR_FALSE;
		for (int i = 0; i < (int)vStrParam.size(); i++)
			TrimStr(vStrParam[i]);

		// show first
		pObj = pSkinWnd->GetUIObjectById(GetIDByName(vStrParam[0].c_str()));
		if (pObj)
			pObj->SetVisible(true);

		// hide second
		pObj = pSkinWnd->GetUIObjectById(GetIDByName(vStrParam[1].c_str()));
		if (pObj)
			pObj->SetVisible(false);
		pSkinWnd->InvalidateRect();

		return ERR_OK;
	}
	else if (cmd.strFunction == _T("is_ctrl_visible"))
	{
		CUIObject *pObj = pSkinWnd->GetUIObjectById(GetIDByName(cmd.strParam.c_str()));
		if (pObj && pObj->IsVisible())
				return ERR_OK;

		return ERR_FALSE;
	}
	else if (cmd.strFunction == _T("set_property"))
	{
		// p1: property name, p2: value
		V_Str		vStrParam;

		StrSplit(cmd.strParam.c_str(), ',', vStrParam);
		if (vStrParam.size() != 2)
			return ERR_FALSE;
		for (int i = 0; i < (int)vStrParam.size(); i++)
			TrimStr(vStrParam[i]);

		pSkinWnd->SetProperty(vStrParam[0].c_str(), vStrParam[1].c_str());

		pSkinWnd->InvalidateRect();

		return ERR_OK;
	}
	else if (cmd.strFunction == _T("set_ctrl_property"))
	{
		V_Str		vStrParam;
		CUIObject	*pObj;

		StrSplit(cmd.strParam.c_str(), ',', vStrParam);
		if (vStrParam.size() != 3)
			return ERR_FALSE;
		for (int i = 0; i < (int)vStrParam.size(); i++)
			TrimStr(vStrParam[i]);

		pObj = pSkinWnd->GetUIObjectById(GetIDByName(vStrParam[0].c_str()));
		if (pObj)
			pObj->SetProperty(vStrParam[1].c_str(), vStrParam[2].c_str());

		return ERR_OK;
	}
	else if (cmd.strFunction == _T("start_animation"))
	{
		pSkinWnd->StartAnimation(GetIDByName(cmd.strParam.c_str()));

		return ERR_OK;
	}
	else if (cmd.strFunction == _T("multi_cmd"))
	{
		V_Str		vStrParam;
		int			nRet = ERR_NOT_FOUND;

		StrSplit(cmd.strParam.c_str(), ',', vStrParam);
		for (int i = 0; i < (int)vStrParam.size(); i++)
		{
			TrimStr(vStrParam[i]);
			nRet = OnDynamicCmd(GetIDByName(vStrParam[i].c_str()), pSkinWnd);
			if (nRet != ERR_OK)
				ERRLOG2(_T("Execute subcommand failed: %s, %s"), vStrParam[i].c_str(), (LPCTSTR)Error2Str(nRet));
		}
		return nRet;
	}
	else if (cmd.strFunction == _T("get_profile_bool"))
	{
		V_Str		vStrParam;
		StrSplit(cmd.strParam.c_str(), ',', vStrParam);
		TrimStr(vStrParam);

		if (vStrParam.size() == 0)
			return ERR_FALSE;

		bool bDefault = true;
		if (vStrParam.size() > 1)
			bDefault = IsTRUE(vStrParam[1].c_str());
		if (g_Profile.GetProfileBool(GetSkinName(), vStrParam[0].c_str(), bDefault))
			return ERR_OK;

		return ERR_FALSE;
	}
//	else if (cmd.strFunction == _T("open_skin_wnd"))
//		return ActiveOrCreateSkinWnd(false, _SZ_SKINWND_CLASS_NAME, cmd.strParam.c_str(), cmd.strParam.c_str());

	return ERR_NOT_FOUND;
}

void CSkinFactory::AdjustHue(float hue, float saturation, float luminance)
{
	LIST_SKINWNDS::iterator	it, itEnd;

	// reload the resource which is set to adjust hue
	m_resourceMgr.AdjustHue(hue, saturation, luminance);

	// Notify each child ctrl of this message.
	itEnd = m_listSkinWnds.end();
	for (it = m_listSkinWnds.begin(); it != itEnd; ++it)
	{
		CSkinWnd	*pSkin = *it;
		pSkin->OnAdjustHue(hue, saturation, luminance);
		pSkin->InvalidateRect();
	}
}

void CSkinFactory::GetAdjustedHueResult(MLColor &clr)
{
	float		H, S, L;
	COLORREF	c;

	m_resourceMgr.GetAdjustHueParam(H, S, L);
	if (H != 0)
	{
		c = clr.get();
		AdjustColorHue(c, H);
		clr.set(c);
	}
}

void CSkinFactory::GetAdjustedHueResult(COLORREF &clr)
{
	float		H, S, L;

	m_resourceMgr.GetAdjustHueParam(H, S, L);
	if (H != 0)
		AdjustColorHue(clr, H);
}

void CSkinFactory::OnLanguageChanged()
{
	LIST_SKINWNDS::iterator	it, itEnd;
	itEnd = m_listSkinWnds.end();
	for (it = m_listSkinWnds.begin(); it != itEnd; ++it)
	{
		CSkinWnd	*pWnd = *it;
		pWnd->OnLanguageChanged();
	}
}

void CSkinFactory::ApplyDefaultThemeOfSkin()
{
	TCHAR	szThemeFile[MAX_PATH];

	tstring strThemeName;
	FileGetTitle(GetSkinFileName(), strThemeName);
	strThemeName += _T("_theme.xml");
	if (!m_resourceMgr.GetResourcePathName(szThemeFile, CountOf(szThemeFile), strThemeName.c_str()))
		return;

	CSimpleXML xml;
	if (!xml.ParseFile(szThemeFile))
	{
		ERRLOG1(_T("Failed to parse theme file: %s"), szThemeFile);
		return;
	}

	if (_tcscmp(xml.m_pRoot->strName.c_str(), _T("Theme")) != 0)
		return;

	for (SXNode::iterator itSection = xml.m_pRoot->listChildren.begin();
		itSection != xml.m_pRoot->listChildren.end(); ++itSection)
	{
		SXNode *pNodeSection = *itSection;
		for (SXNode::iterator itValue = pNodeSection->listChildren.begin();
			itValue != pNodeSection->listChildren.end(); ++itValue)
		{
			SXNode *pNodeValue = *itValue;
			g_Profile.WriteProfileString(pNodeSection->strName.c_str(),
				pNodeValue->strName.c_str(), pNodeValue->strContent.c_str());
		}
	}
}

//
// Bring all the skin windows to the top
//
void CSkinFactory::OnSkinWndActivate(CSkinWnd *pWndActive)
{
#ifdef _WIN32
	LIST_SKINWNDS::iterator	it, itEnd;

	HWND		hWndPrev, hWndNext, hWndActive;
	HDWP		hWindowPosInfo;
	vector<HWND>	vToBring;

	if (pWndActive)
		hWndActive = pWndActive->GetHandle();
	else
		hWndActive = GetForegroundWindow();

	if (IsTopmostWindow(hWndActive))
		vToBring.push_back(HWND_TOP);
	else
		vToBring.push_back(hWndActive);

	hWndPrev = hWndNext = hWndActive;
	hWndNext = GetWindow(hWndNext, GW_HWNDNEXT);
	while (hWndNext)
	{
		itEnd = m_listSkinWnds.end();
		for (it = m_listSkinWnds.begin(); it != itEnd; ++it)
		{
			if ((*it)->GetHandle() == hWndNext && hWndNext != hWndActive)
			{
				if (!IsTopmostWindow(hWndNext) && !::IsIconic(hWndNext))
				{
					vToBring.push_back(hWndNext);
					hWndPrev = hWndNext;
				}
				break;
			}
		}
		hWndNext = GetWindow(hWndNext, GW_HWNDNEXT);
	}
	hWindowPosInfo = BeginDeferWindowPos(vToBring.size());
	for (int i = 1; i < (int)vToBring.size(); i++)
		DeferWindowPos(hWindowPosInfo, vToBring[i], vToBring[i - 1], 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	EndDeferWindowPos(hWindowPosInfo);

	// DBGLOG1(_T("OnSkinWndActivate: %s"), pWndActive->GetSkinWndName());
#endif
}

void CSkinFactory::EnterInDrawUpdate()
{
	for (LIST_SKINWNDS::iterator it = m_listSkinWnds.begin(); it != m_listSkinWnds.end(); ++it)
	{
		CSkinWnd	*pWnd = *it;
		pWnd->EnterInDrawUpdate();
	}
}

void CSkinFactory::LeaveInDrawUpdate()
{
	for (LIST_SKINWNDS::iterator it = m_listSkinWnds.begin(); it != m_listSkinWnds.end(); ++it)
	{
		CSkinWnd	*pWnd = *it;
		pWnd->LeaveInDrawUpdate();
	}
}
