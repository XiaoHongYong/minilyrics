// ThemesFile.cpp: implementation of the CThemesFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MPlayerApp.h"
#include "ThemesFile.h"
#include "Helper.h"

LPCTSTR		__vLyrClrProperties[] = {
	_T("Ed_HighColor"),
	_T("Ed_LowColor"),
	_T("Ed_TagColor"),
	_T("Ed_BgColor"),
	_T("Ed_FocusLineBgColor"),
	_T("BgColor"),
	_T("OutlineLyrText"),

	_T("HilightOBM"),
	_T("FgColor"),
	_T("HilightGradient1"), _T("HilightGradient2"), _T("HilightGradient3"),
	_T("HilightBorderColor"),
	_T("HilightPattern"),

	_T("LowlightOBM"),
	_T("FgLowColor"),
	_T("LowlightGradient1"), _T("LowlightGradient2"), _T("LowlightGradient3"),
	_T("LowlightBorderColor"),
	_T("LowlightPattern"),
};

LPCTSTR		__LyrFontProperties[] = {
	_T("Font"),
	_T("LineSpacing"),
};

LPCTSTR		__LyrStyleProperties[] = {
	_T("LyrAlign"),
	_T("LyrDrawOpt"),
	_T("Karaoke"),
	_T("LyrDisplayStyle"),
};

LPCTSTR		__LyrBgPicProperties[] = {
	_T("UseBgImage"),
	_T("BgPicFolder"),
	_T("UseAlbumArtAsBg"),
	_T("DarkenLyrBg"),
	_T("SlideDelayTime"),
};

ThemeItem	g_themeItems[] = 
{
	{
		TII_LYR_COLORS,
		_T("LyrColors"),
		__vLyrClrProperties,
		CountOf(__vLyrClrProperties),
		IDC_C_LYR_COLORS,
		true,
	},
	{
		TII_LYR_FONT,
		_T("LyrFont"),
		__LyrFontProperties,
		CountOf(__LyrFontProperties),
		IDC_C_LYR_FONT,
		true,
	},
	{
		TII_LYR_STYLE,
		_T("LyrStyle"),
		__LyrStyleProperties,
		CountOf(__LyrStyleProperties),
		IDC_C_LYR_STYLE,
		true,
	},
	{
		TII_LYR_BG_PIC,
		_T("LyrBgPic"),
		__LyrBgPicProperties,
		CountOf(__LyrBgPicProperties),
		IDC_C_LYR_BG_PIC,
		false,
	},
};

const int	g_themeItemsCount = CountOf(g_themeItems);


#define SZ_THEMES_FILE		_T("Themes.xml")
#define SZ_CUST_THEMES_FILE	_T("ThemesCust.xml")
#define SZ_NAME				_T("Name")
#define SZ_THEMES			_T("Themes")

CThemesXML::CThemesXML()
{
	New(SZ_THEMES);
}

bool CThemesXML::Delete(LPCTSTR szThemeName)
{
	if (!m_pRoot)
		return false;

	SXNode::iterator	it = GetTheme(szThemeName);
	if (it == m_pRoot->listChildren.end())
		return FALSE;

	SXNode		*pNode = *it;
	m_pRoot->listChildren.erase(it);
	delete pNode;

	return TRUE;
}

bool CThemesXML::SetCurTheme(LPCTSTR szThemeName)
{
	if (!m_pRoot)
		return false;

	SXNode::iterator	it = GetTheme(szThemeName);
	if (it == m_pRoot->listChildren.end())
		return FALSE;

	SXNode		*pNode = *it;

	SXNode		*pNodeLyr = pNode->GetChild(_T("NormalLyr"));
	if (pNodeLyr)
		ThemeToLyrSettings(false, pNodeLyr);

	SXNode		*pNodeFloatingLyr = pNode->GetChild(_T("FloatingLyr"));
	if (pNodeFloatingLyr)
		ThemeToLyrSettings(true, pNodeFloatingLyr);

	return TRUE;
}

bool CThemesXML::UseCurSettingAsTheme(LPCTSTR szThemeName)
{
	if (!m_pRoot)
		New(SZ_THEMES);

	Delete(szThemeName);

	SXNode		*pNode = new SXNode;

	pNode->strName = _T("Theme");
	pNode->AddProperty(SZ_NAME, szThemeName);

	SXNode		*pNodeLyr = new SXNode;
	pNodeLyr->strName = _T("NormalLyr");
	LyrSettingsToTheme(false, pNodeLyr);
	pNode->listChildren.push_back(pNodeLyr);

	SXNode		*pNodeFloatingLyr = new SXNode;
	pNodeFloatingLyr->strName = _T("FloatingLyr");
	LyrSettingsToTheme(true, pNodeFloatingLyr);
	pNode->listChildren.push_back(pNodeFloatingLyr);

	m_pRoot->listChildren.push_back(pNode);

	return TRUE;
}

bool CThemesXML::IsThemeExists(LPCTSTR szThemeName)
{
	if (!m_pRoot)
		return false;

	return GetTheme(szThemeName) != m_pRoot->listChildren.end();
}

void CThemesXML::EnumAllThemes(vector<tstring> &vThemes)
{
	if (!m_pRoot)
		return;

	for (SXNode::iterator it = m_pRoot->listChildren.begin(); it != m_pRoot->listChildren.end(); ++it)
	{
		SXNode		*pNode = *it;
		LPCTSTR		szName = pNode->GetProperty(SZ_NAME);
		if (szName)
			vThemes.push_back(szName);
	}
}

SXNode::iterator CThemesXML::GetTheme(LPCTSTR szThemeName)
{
	ASSERT(m_pRoot);

	for (SXNode::iterator it = m_pRoot->listChildren.begin(); it != m_pRoot->listChildren.end(); ++it)
	{
		SXNode		*pNode = *it;
		if (_tcsicmp(pNode->GetPropertySafe(SZ_NAME), szThemeName) == 0)
			return it;
	}

	return m_pRoot->listChildren.end();
}

static void AddSubPropNode(SXNode *pNodeParent, LPCTSTR szSect, LPCTSTR szSubPropName, LPCTSTR szNames[], int nCount)
{
	LPCTSTR		szContent;
	SXNode		*pNodeChild = new SXNode;
	pNodeChild->strName = szSubPropName;

	for (int i = 0; i < nCount; i++)
	{
		szContent = g_Profile.GetProfileString(szSect, szNames[i], _T(""));
		if (!szContent || IsEmptyString(szContent))
			continue;

		SXNode		*pNode = new SXNode;

		pNode->strName = szNames[i];
		pNode->strContent = szContent;

		pNodeChild->listChildren.push_back(pNode);
	}

	pNodeParent->listChildren.push_back(pNodeChild);
}

void CThemesXML::LyrSettingsToTheme(bool bFloatingLyr, SXNode *pNode)
{
	ASSERT(m_pRoot);

	tstring strSectName;
	EventType etDispSettings;
	CMPlayerApp::GetInstance()->GetCurLyrDisplaySettingName(bFloatingLyr, strSectName, etDispSettings);

	for (int i = 0; i < g_themeItemsCount; i++)
	{
		AddSubPropNode(pNode, strSectName.c_str(), g_themeItems[i].szName, 
			g_themeItems[i].vProperties, g_themeItems[i].nPropertiesCount);
	}
}

void CThemesXML::ThemeToLyrSettings(bool bFloatingLyr, SXNode *pNode)
{
	ASSERT(m_pRoot);

	tstring strSectName;
	EventType etDispSettings;
	CMPlayerApp::GetInstance()->GetCurLyrDisplaySettingName(bFloatingLyr, strSectName, etDispSettings);

	for (int i = 0; i < g_themeItemsCount; i++)
	{
		if (!g_themeItems[i].bValid)
			continue;

		SXNode *pNodeChild = pNode->GetChild(g_themeItems[i].szName);
		if (!pNodeChild)
			continue;

		for (SXNode::iterator it = pNodeChild->listChildren.begin(); it != pNodeChild->listChildren.end(); ++it)
		{
			SXNode	*pProperty = *it;
			CMPlayerSettings::SetSettings(etDispSettings, strSectName.c_str(), 
				pProperty->strName.c_str(), pProperty->strContent.c_str());
		}
	}
}

//////////////////////////////////////////////////////////////////////////

bool CThemesFile::Open()
{
	tstring		strFile;

	strFile = GetAppResourceDir();
	strFile += SZ_THEMES_FILE;
	m_xmlThemes.ParseFile(strFile.c_str());

	strFile = GetAppDataDir();
	strFile += SZ_CUST_THEMES_FILE;
	m_xmlThemesCustomized.ParseFile(strFile.c_str());

	return true;
}

bool CThemesFile::Save()
{
	tstring		strFile;

	strFile = GetAppResourceDir();
	strFile += SZ_THEMES_FILE;
	m_xmlThemes.SaveFile(strFile.c_str());

	strFile = GetAppDataDir();
	strFile += SZ_CUST_THEMES_FILE;
	return Tobool(m_xmlThemesCustomized.SaveFile(strFile.c_str()));
}

bool CThemesFile::Delete(LPCTSTR szThemeName)
{
	m_xmlThemes.Delete(szThemeName);
	m_xmlThemesCustomized.Delete(szThemeName);

	return TRUE;
}

bool CThemesFile::SetCurTheme(LPCTSTR szThemeName)
{
	if (m_xmlThemesCustomized.SetCurTheme(szThemeName))
		return TRUE;

	return m_xmlThemes.SetCurTheme(szThemeName);
}

bool CThemesFile::UseCurSettingAsTheme(LPCTSTR szThemeName)
{
	m_xmlThemes.Delete(szThemeName);

	return m_xmlThemesCustomized.UseCurSettingAsTheme(szThemeName);
}

void CThemesFile::EnumAllThemes(vector<tstring> &vThemes)
{
	m_xmlThemes.EnumAllThemes(vThemes);
	m_xmlThemesCustomized.EnumAllThemes(vThemes);
	std::sort(vThemes.begin(), vThemes.end());
}
