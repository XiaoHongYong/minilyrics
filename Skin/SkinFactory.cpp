// SkinFactory.cpp: implementation of the CSkinFactory class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "MLSkin.h"
#include "SkinFactory.h"
#include "SkinWndResizeObj.h"
#include "SkinFadeArea.h"
#include "SkinDecorativeContainer.h"
#include "SkinToggleCtrlContainer.h"
#include "SkinMenuBar.h"
#include "SkinAnimationObj.h"

#define _SZ_SKINWND				_T("skinwnd")
#define _SZ_MENUS				_T("Menus")
#define _SZ_DYNMICCTRLS			_T("DynControls")
#define _SZ_DYNMICCMDS			_T("DynCmds")
#define _SZ_CONTROLDEF			_T("controldef")
#define _SZ_SKIN_FILE_NAME		_T("ZikiPlayer.xml")
#define _SZ_ADJUST_HUE			_T("AdjustHue")


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

SXNode *CSkinFileXML::GetAdjustHueNode()
{
	return m_xml.m_pRoot->GetChild(_SZ_ADJUST_HUE);
}

int CSkinFileXML::SetAdjustHueNode(SXNode *pNode)
{
	return SetNode(_SZ_ADJUST_HUE, pNode);
}

LPCTSTR CSkinFileXML::GetSkinProperty(LPCTSTR szProperty)
{
	return m_xml.m_pRoot->GetProperty(szProperty);
}

int CSkinFileXML::GetMainSkinWnd(tstring &strSkinWndName, LPCTSTR szSkinWndNameDef)
{
	LPCTSTR		szMainWnds, szDefMainWnd;

	szDefMainWnd = m_xml.m_pRoot->GetProperty(_T("defmainwnd"));
	szMainWnds = m_xml.m_pRoot->GetProperty(_T("mainwnds"));

	strSkinWndName.resize(0);

	//
	// Is szSkinWndName exists in "mainwnds"?
	//
	if (szSkinWndNameDef == NULL || IsEmptyString(szSkinWndNameDef))
	{
		if (szDefMainWnd && !IsEmptyString(szDefMainWnd))
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
	if (szDefMainWnd && !IsEmptyString(szDefMainWnd))
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
	m_pCurStyle = NULL;
}

CSkinStyles::~CSkinStyles()
{
	m_listStyleNodes.clear();
}

int CSkinStyles::FromXML(SXNode *pNodeSkin)
{
	m_listStyleNodes.clear();
	m_pCurStyle = NULL;

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

	if (m_listStyleNodes.size())
		m_pCurStyle = m_listStyleNodes.front();

	return ERR_OK;
}

void CSkinStyles::Free()
{
	m_listStyleNodes.clear();
	m_pCurStyle = NULL;
}

bool CSkinStyles::SetCurStyle(LPCTSTR szStyleName)
{
	for (SXNode::iterator it = m_listStyleNodes.begin(); 
		it != m_listStyleNodes.end(); ++it)
	{
		SXNode		*pNode = *it;

		ASSERT(IsPropertyName(pNode->strName.c_str(), _T("style")));
		if (IsPropertyName(pNode->GetPropertySafe(SZ_PN_NAME), szStyleName))
		{
			m_pCurStyle = pNode;
			return true;
		}
	}

	m_pCurStyle = NULL;

	return false;
}

SXNode *CSkinStyles::GetClassNode(LPCTSTR szClassName) const
{
	if (!m_pCurStyle)
		return NULL;

	for (SXNode::iterator it = m_pCurStyle->listChildren.begin(); 
		it != m_pCurStyle->listChildren.end(); ++it)
	{
		SXNode		*pNode = *it;

		if (IsPropertyName(pNode->strName.c_str(), szClassName))
		{
			return pNode;
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

CSkinFactory::CSkinFactory()
{
	m_strMainSkinWndClassName = _SZ_SKINWND_CLASS_NAME;
	m_IdMap = NULL;
	m_strSkinFileName = _SZ_SKIN_FILE_NAME;
	m_nCustomCmdIDNext = CMD_ID_CUSTOM_BASE;
	m_dynamicCmds.Init(this);
}

CSkinFactory::~CSkinFactory()
{

}


void CSkinFactory::SetStringIDMap(STRING_ID_MAP StringIdMap[])
{
	assert(StringIdMap);

	m_IdMap = StringIdMap;
}

// COMMENT:
//		根据控件 ID 字符串来取得对应的控件 ID 数值
//		改进方法: 对所有的ID字符串值进行排序,然后使用二分法进行搜索
int CSkinFactory::GetIDByName(LPCTSTR szId)
{
	tstring		strTemp;

	return GetIDByNameEx(szId, strTemp);
}

int CSkinFactory::GetIDByNameEx(LPCTSTR szId, tstring &strToolTip)
{
	assert(szId);
	assert(m_IdMap);
	if (!m_IdMap || IsEmptyString(szId))
		return SKIN_UIOBJ_ID_UNDEFINE;

	strToolTip.resize(0);

	for (int i = 0; m_IdMap[i].szId != 0; i++)
	{
		if (_tcsicmp(szId, m_IdMap[i].szId) == 0)
		{
			if (m_IdMap[i].szToolTip)
				strToolTip = TOLOCAL(m_IdMap[i].szToolTip);

			return m_IdMap[i].nId;
		}
	}

	MAP_STRID::iterator it = m_mapCmdStrId.find(szId);
	if (it != m_mapCmdStrId.end())
	{
		return (*it).second;
	}

	m_nCustomCmdIDNext++;

	m_mapCmdStrId[szId] = m_nCustomCmdIDNext;
	return m_nCustomCmdIDNext;
}

tstring CSkinFactory::GetStringOfID(int nID)
{
	assert(m_IdMap);
	if (!m_IdMap)
		return _T("");

	for (int i = 0; m_IdMap[i].szId != 0; i++)
	{
		if (nID == m_IdMap[i].nId)
			return m_IdMap[i].szId;
	}

	MAP_STRID::iterator it;
	for (it = m_mapCmdStrId.begin(); it != m_mapCmdStrId.end(); ++it)
	{
		if ((*it).second == nID)
			return (*it).first;
	}

	return _T("");
}

int CSkinFactory::OpenSkinEx(LPCTSTR szSkinName, LPCTSTR szSkinDir, LPCTSTR szExtraDir)
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

int CSkinFactory::ChangeSkinEx(LPCTSTR szSkinName, LPCTSTR szSkinDir, LPCTSTR szExtraDir)
{
	int		nRet;

	nRet = OpenSkinEx(szSkinName, szSkinDir, szExtraDir);
	if (nRet != ERR_OK)
		return nRet;

	float hue = (float)g_Profile.GetProfileInt(szSkinName, _T("Hue"), 0);
	if (hue < 0)
		hue = 0;
	else if (hue > 360)
		hue = 360;
	m_resourceMgr.AdjustHue((float)hue, 0.5, 0.5);

	// change the skins of existed windows.
	tstring		strSkinWndDef;
	tstring		strSkinWndName;
	tstring		strOpenedSkinWnds;
	SetTStrNCase	setOpenedSkinWnds;

	strOpenedSkinWnds = g_Profile.GetProfileString(_T("LatestOpenedSkinWnds"), _T(""));
	strSkinWndDef = g_Profile.GetProfileString(_T("DefaultSkinWnd"), _T(""));

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
	if (m_listSkinWnds.empty())
	{
		nRet = CreateMainSkinWnd(_T("MiniLyrics"), strSkinWndName.c_str(), strSkinWndName.c_str());
		if (nRet != ERR_OK)
			return nRet;
	}
	else
	{
		CSkinWnd *pSkin = m_listSkinWnds.front();
		nRet = pSkin->OpenSkin(strSkinWndName.c_str());
		if (nRet != ERR_OK)
			return nRet;
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
			nRet = CreateSkinWnd(false, _SZ_SKINWND_CLASS_NAME, (*itSet).c_str(), (*itSet).c_str());
		// delete pSkinWnd;
	}

	// Open other skin windows.
/*	LIST_SKINWNDS::iterator	it;
	SetTStrNCase::iterator itSet;
	for (it = listToBeProcessed.begin(), itSet = setOpenedSkinWnds.begin();
		it != listToBeProcessed.end() && itSet != setOpenedSkinWnds.end(); ++it, ++itSet)
	{
		CSkinWnd	*pSkinWnd = *it;
		pSkinWnd->OpenSkin((*itSet).c_str());
	}

	// Destroy other skin window
	for (; it != listToBeProcessed.end(); ++it)
	{
		CSkinWnd	*pSkinWnd = *it;
		pSkinWnd->Destroy();
		// delete pSkinWnd;
	}

	// Create other skin windows.
	for (; itSet != setOpenedSkinWnds.end(); ++itSet)
	{
		nRet = CreateSkinWnd(false, _SZ_SKINWND_CLASS_NAME, (*itSet).c_str(), (*itSet).c_str());
		// delete pSkinWnd;
	}*/

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
		return nRet;

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
	LPCTSTR szDefaultStyle = m_skinFile.GetSkinProperty(_T("DefaultStyle"));
	if (szDefaultStyle && !IsEmptyString(szDefaultStyle))
		m_skinStyle.SetCurStyle(szDefaultStyle);

	pNode = m_skinFile.GetDynamicCmdsNode();
	if (pNode)
		m_dynamicCmds.FromXML(pNode);

	// 
	pNode = m_skinFile.GetAdjustHueNode();
	if (pNode)
		m_resourceMgr.AdjustHueInfoFromXML(pNode);

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
			delete p;
			*it = xml.m_pRoot;
			xml.m_pRoot = NULL;
		}
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

void CSkinFactory::Close()
{
	CSjvmSkinSystem::CloseScript();

	m_skinStyle.Free();
	m_dynamicCmds.Free();
	m_skinFile.Close();

	LIST_SKINWNDS::iterator	it;

	// Save opened window name, and close it.
	tstring		strOpenedSkinWnds;
	bool		bSave = false;
	for (it = m_listSkinWnds.begin(); it != m_listSkinWnds.end(); ++it)
	{
		CSkinWnd	*pSkin = *it;

		if (pSkin->IsMainAppWnd())
		{
			bSave = true;
			g_Profile.WriteProfileString(_T("DefaultSkinWnd"), pSkin->GetSkinWndName());
		}
		else
		{
			if (!strOpenedSkinWnds.empty())
				strOpenedSkinWnds += _T(",");
			strOpenedSkinWnds += pSkin->GetSkinWndName();
		}

		// pSkin->CloseSkin();
	}
	if (bSave || strOpenedSkinWnds.size())
		g_Profile.WriteProfileString(_T("LatestOpenedSkinWnds"), strOpenedSkinWnds.c_str());

	m_resourceMgr.OnClose();

	m_mapCmdStrId.clear();
	m_nCustomCmdIDNext = CMD_ID_CUSTOM_BASE;
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

int CSkinFactory::ActiveOrCreateSkinWnd(bool bMainWnd, LPCTSTR szClassName, LPCTSTR szCaption, LPCTSTR szSkinWndName, CSkinWnd **ppSkin)
{
	CSkinWnd	*pSkinWnd = FindSkinWnd(szSkinWndName);
	if (pSkinWnd)
	{
		if (pSkinWnd->IsVisible())
			pSkinWnd->ShowWindow(SW_SHOW);
		if (pSkinWnd->IsIconic())
			pSkinWnd->ShowWindow(SW_RESTORE);
		pSkinWnd->SetForeground();
		if (ppSkin)
			*ppSkin = pSkinWnd;
		return ERR_OK;
	}

	return CreateSkinWnd(bMainWnd, szClassName, szCaption, szSkinWndName, ppSkin);
}

int CSkinFactory::OpenOrCloseSkinWnd(bool bMainWnd, LPCTSTR szClassName, LPCTSTR szCaption, LPCTSTR szSkinWndName, CSkinWnd **ppSkin)
{
	CSkinWnd	*pSkinWnd = FindSkinWnd(szSkinWndName);
	if (pSkinWnd)
	{
		pSkinWnd->Destroy();
		return ERR_OK;
	}

	return CreateSkinWnd(bMainWnd, szClassName, szCaption, szSkinWndName, ppSkin);
}

int CSkinFactory::CreateSkinWnd(bool bMainWnd, LPCTSTR szClassName, LPCTSTR szCaption, LPCTSTR szSkinWndName, CSkinWnd **ppSkin)
{
	int		nRet;
	CSkinWnd	*pSkin;

	// If the skinwnd doesn't exist, return ERR_NOT_FOUND
	if (GetSkinWndNode(szSkinWndName) == NULL)
	{
		if (!IsAppResSkinWnd(szSkinWndName))
			return ERR_NOT_FOUND;
	}

	if (ppSkin)
		*ppSkin = NULL;

	pSkin = NewSkinWnd(szSkinWndName, bMainWnd);
	ASSERT(pSkin);
	pSkin->SetFreeOnDestory(true);
	nRet = pSkin->Create(szClassName, szCaption, this, szSkinWndName, NULL, !bMainWnd);
	if (nRet == ERR_OK)
	{
		if (ppSkin)
			*ppSkin = pSkin;
#ifdef _WIN32
		if (!bMainWnd && pSkin->IsWindowsAppearance())
			ShowAsAppWindow(pSkin->GetHandle());
#endif
	}
	else
		delete pSkin;

	return nRet;
}

int CSkinFactory::CreateMainSkinWnd(LPCTSTR szClassName, LPCTSTR szCaption, LPCTSTR szSkinWndNameDef, CSkinWnd **ppSkin)
{
	int			nRet;
	tstring		strSkinWndName;
	CSkinWnd		*pSkinWnd;

	if (ppSkin)
		*ppSkin = NULL;

	nRet = m_skinFile.GetMainSkinWnd(strSkinWndName, szSkinWndNameDef);
	if (nRet != ERR_OK)
	{
		ERRLOG1(_T("Can't get main window name: %s"), m_skinFile.GetFileName());
		return nRet;
	}

	nRet = CreateSkinWnd(true, szClassName, szCaption, strSkinWndName.c_str(), &pSkinWnd);
	if (nRet == ERR_OK)
	{
		pSkinWnd->SetMainAppWnd(true);
		if (ppSkin)
			*ppSkin = pSkinWnd;
	}

	return nRet;
}

int CSkinFactory::OpenOrCloseSkinWnd(LPCTSTR szClassName, LPCTSTR szCaption, LPCTSTR szSkinWndName, CBaseWnd *pWndParent, CSkinWnd **ppSkinWnd)
{
	CSkinWnd *pWndOld = FindSkinWnd(szSkinWndName);
	if (pWndOld)
	{
		pWndOld->ActivateWindow();
		delete *ppSkinWnd;
		*ppSkinWnd = pWndOld;
		return ERR_OK;
	}

	int nRet = (*ppSkinWnd)->Create(szClassName, szCaption, this, szSkinWndName, pWndParent, true);
	if (nRet != ERR_OK)
	{
		delete *ppSkinWnd;
		*ppSkinWnd = NULL;
		return nRet;
	}

#ifdef _WIN32
	if (!(*ppSkinWnd)->IsWindowsAppearance())
		ShowAsToolWindow((*ppSkinWnd)->GetHandle());
#endif

	return ERR_OK;
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
#ifdef _WIN32
	PostQuitMessage(0);
#endif
}

// methods for skin window
SXNode *CSkinFactory::GetSkinWndNode(LPCTSTR szSkinWndName)
{
	return m_skinFile.GetSkinWndNode(szSkinWndName);
}

static tstring	GetAppResSkinWndFile(CSkinFactory *pFactory, LPCTSTR szSkinWndName)
{
	tstring		strFile = pFactory->GetSkinRootDir();
	strFile += szSkinWndName;

	return strFile;
}

int CSkinFactory::LoadAppResSkinWndXml(LPCTSTR szSkinWndName, CSimpleXML &xml)
{
	tstring		strFile = GetAppResSkinWndFile(this, szSkinWndName);

	if (!IsFileExist(strFile.c_str()))
		return ERR_NOT_FOUND;

	if (!xml.ParseFile(strFile.c_str()))
		return ERR_PARSE_XML;

	ExpandIncludeNode(xml.m_pRoot);

	return ERR_OK;
}

bool CSkinFactory::IsAppResSkinWnd(LPCTSTR szSkinWndName)
{
	tstring		strFile = GetAppResSkinWndFile(this, szSkinWndName);

	return IsFileExist(strFile.c_str());
}

// create new uiobject by class name
CUIObject * CSkinFactory::CreateUIObject(CSkinWnd *pSkin, LPCTSTR szClassName)
{
	SXNode *pNode = m_skinStyle.GetClassNode(szClassName);
	if (!pNode)
		return NewUIObject(pSkin, szClassName);

	LPCTSTR szExtends = pNode->GetProperty(SZ_PN_EXTENDS);
	if (szExtends)
		szClassName = szExtends;

	CUIObject *pObj = NewUIObject(pSkin, szClassName);

	// Set styled properties
	if (pObj)
		pObj->FromXML(NULL, pNode);

	return pObj;
}

CUIObject *CSkinFactory::NewUIObject(CSkinWnd *pSkin, LPCTSTR szClassName)
{
	CUIObject *pObj = NULL;

	if (_tcsicmp(szClassName, CSkinButton::ClassName()) == 0)
	{
		pObj = new CSkinButton;
	}
	if (_tcsicmp(szClassName, CSkinActiveButton::ClassName()) == 0)
	{
		pObj = new CSkinActiveButton;
	}
	else if (_tcsicmp(szClassName, CSkinImage::ClassName()) == 0)
	{
		pObj = new CSkinImage;
	}
	else if (_tcsicmp(szClassName, CSkinNStatusImage::ClassName()) == 0)
	{
		pObj = new CSkinNStatusImage;
	}
	else if (_tcsicmp(szClassName, CSkinActiveImage::ClassName()) == 0)
	{
		pObj = new CSkinActiveImage;
	}
	else if (_tcsicmp(szClassName, CSkinXScaleImage::ClassName()) == 0 ||
		_tcsicmp(szClassName, _T("XScaleImage")) == 0)
	{
		pObj = new CSkinXScaleImage;
	}
	else if (_tcsicmp(szClassName, CSkinYScaleImage::ClassName()) == 0 ||
		_tcsicmp(szClassName, _T("YScaleImage")) == 0)
	{
		pObj = new CSkinYScaleImage;
	}
	else if (_tcsicmp(szClassName, CSkinVScrollBar::ClassName()) == 0)
	{
		pObj = new CSkinVScrollBar;
	}
	else if (_tcsicmp(szClassName, CSkinHScrollBar::ClassName()) == 0)
		pObj = new CSkinHScrollBar;
	else if (_tcsicmp(szClassName, CSkinVScrollBarOSStyle::ClassName()) == 0)
	{
		pObj = new CSkinVScrollBarOSStyle;
	}
	else if (_tcsicmp(szClassName, CSkinHScrollBarOSStyle::ClassName()) == 0)
	{
		pObj = new CSkinHScrollBarOSStyle;
	}
	else if (_tcsicmp(szClassName, CSkinCaption::ClassName()) == 0)
	{
		pObj = new CSkinCaption;
	}
	else if (_tcsicmp(szClassName, CSkinNStatusButton::ClassName()) == 0)
		pObj = new CSkinNStatusButton;
	else if (_tcsicmp(szClassName, CSkinContainer::ClassName()) == 0)
		pObj = new CSkinContainer;
	else if (_tcsicmp(szClassName, CSkinDecorativeContainer::ClassName()) == 0)
		pObj = new CSkinDecorativeContainer;
	else if (_tcsicmp(szClassName, CSkinMenuBar::ClassName()) == 0)
		pObj = new CSkinMenuBar;
	else if (_tcsicmp(szClassName, CSkinWndResizeObj::ClassName()) == 0)
		pObj = new CSkinWndResizeObj;
	else if (_tcsicmp(szClassName, CSkinSquare::ClassName()) == 0)
		pObj = new CSkinSquare;
	else if (_tcsicmp(szClassName, CSkinFadeArea::ClassName()) == 0)
		pObj = new CSkinFadeArea;
	else if (_tcsicmp(szClassName, CSkinToggleCtrlContainer::ClassName()) == 0)
		pObj = new CSkinToggleCtrlContainer;
	else if (_tcsicmp(szClassName, CSkinClientArea::ClassName()) == 0)
		pObj = new CSkinClientArea;
	else if (_tcsicmp(szClassName, CSkinAnimationObj::ClassName()) == 0)
		pObj = new CSkinAnimationObj;

	if (pObj)
	{
		pObj->m_pSkin = pSkin;
		pObj->m_pContainer = pSkin->GetRootContainer();
	}

	return pObj;
}

CUIObject *CSkinFactory::CreateDynamicCtrl(CSkinContainer *pContainer, LPCTSTR szClassName, int nIDAssign, LPCTSTR szLeft, LPCTSTR szTop, LPCTSTR szWidth, LPCTSTR szHeight)
{
	CUIObject			*pObj = NULL;

	// Create the control
	pObj = pContainer->m_pSkin->GetSkinFactory()->CreateUIObject(pContainer->m_pSkin, szClassName);
	assert(pObj);
	if (pObj)
	{
		pObj->m_pContainer = pContainer;

		if (nIDAssign == SKIN_UIOBJ_ID_UNDEFINE)
			nIDAssign = pContainer->m_pSkin->AllocUIObjID();
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

#ifdef _DEBUG
#ifdef _WIN32
		if (!IsDirExist(m_strSkinRootDir.c_str()))
		{
			m_strSkinRootDir = GetWorkingFolder();

			TCHAR		szFile[MAX_PATH];
			if (GetModuleFileName(GetAppInstance(), szFile, CountOf(szFile)))
			{
				if (FileIsExtSame(szFile, _T(".exe")))
					m_strSkinRootDir += _T("..\\Install_ZP\\Share\\skins\\");
				else
					m_strSkinRootDir += _T("..\\Install_ML\\Share\\skins\\");
			}
		}
#endif
#endif
	}

	return m_strSkinRootDir.c_str();
}

void CSkinFactory::SetSkinFileName(LPCTSTR szSkinFileName)
{
	m_strSkinFileName = szSkinFileName;
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
	if (!m_listSkinWnds.empty())
		return m_listSkinWnds.front();
	else
		return NULL;
}

SXNode *CSkinFactory::GetExtendSkinWndStyle(LPCTSTR szExtendsSkinWnd) const
{
	return m_skinStyle.GetClassNode(szExtendsSkinWnd);
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
		return ActiveOrCreateSkinWnd(false, _SZ_SKINWND_CLASS_NAME, cmd.strParam.c_str(), cmd.strParam.c_str());
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
		return CreateSkinWnd(false, _SZ_SKINWND_CLASS_NAME, cmd.strParam.c_str(), cmd.strParam.c_str());
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

			pObj = pSkinWnd->GetUIObjectById(GetIDByName(vStrParam[0].c_str()));
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
