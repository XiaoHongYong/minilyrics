// ThemesFile.h: interface for the CThemesFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_THEMESFILE_H__F2A7834E_B1FD_496B_9B2D_A3D302DF3B74__INCLUDED_)
#define AFX_THEMESFILE_H__F2A7834E_B1FD_496B_9B2D_A3D302DF3B74__INCLUDED_

#pragma once


enum ThemeItemID
{
	TII_LYR_COLORS,
	TII_LYR_FONT,
	TII_LYR_STYLE,
	TII_LYR_BG_PIC,
	TII_COUNT,
};

struct ThemeItem
{
	ThemeItemID	nTII;
	LPCTSTR		szName;
	LPCTSTR		*vProperties;
	int			nPropertiesCount;
	int			nIDCtrl;
	bool		bValid;
};

extern ThemeItem	g_themeItems[];
extern const int	g_themeItemsCount;

class CThemesXML : public CSimpleXML
{
public:
	CThemesXML();

	bool Delete(LPCTSTR szThemeName);
	bool SetCurTheme(LPCTSTR szThemeName);
	bool UseCurSettingAsTheme(LPCTSTR szThemeName);

	bool IsThemeExists(LPCTSTR szThemeName);

	void EnumAllThemes(vector<tstring> &vThemes);

protected:
	SXNode::iterator GetTheme(LPCTSTR szThemeName);

	void LyrSettingsToTheme(bool bFloatingLyr, SXNode *pNode);
	void ThemeToLyrSettings(bool bFloatingLyr, SXNode *pNode);

};


class CThemesFile
{
public:
	CThemesFile()
	{
	}

public:
	bool Open();
	bool Save();

	bool Delete(LPCTSTR szThemeName);
	bool SetCurTheme(LPCTSTR szThemeName);
	bool UseCurSettingAsTheme(LPCTSTR szThemeName);

	void EnumAllThemes(vector<tstring> &vThemes);

protected:

protected:
	CThemesXML		m_xmlThemes;
	CThemesXML		m_xmlThemesCustomized;

};

#endif // !defined(AFX_THEMESFILE_H__F2A7834E_B1FD_496B_9B2D_A3D302DF3B74__INCLUDED_)
