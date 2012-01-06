#pragma once

bool GetUserDefaultLang(tstring &strLang, tstring &strLangFull);

class CLanguageFile
{
public:
	bool Init(LPCTSTR szProfile);
	void Close();
	LPCTSTR GetTranslation(LPCTSTR szString);

protected:
	MapString			m_mapString;

};

class CLanguageTool  
{
public:
	CLanguageTool();
	virtual ~CLanguageTool();

public:
	struct TransFileInfo
	{
		tstring		strFileName;
		tstring		strLanguage;
		tstring		strLanguageCode;
		tstring		strLanguageCodeFull;
	};

	typedef vector<TransFileInfo>		V_TRANSFILEINFO;

public:
	void Close();

	LPCTSTR ToLocalString(LPCTSTR szString);

	void OnLanguageChanged();

	bool IsEnglish();

	static bool GetCurrentLanguageFile(tstring &strLanguageFile);

	static bool ListTransFiles(LPCTSTR szDir, V_TRANSFILEINFO &vTransFiles);

	// Replace $ProductName$, $CompanyName$ to actual value.
	void SetMacro(LPCTSTR szName, LPCTSTR szValue);

	bool HasMacro() const { return !m_mapMacros.empty(); }

protected:
	LPCTSTR ReplaceMacroStr(LPCTSTR szString);

protected:
	bool		m_bInitialized;
	bool		m_bEnglish;

	CLanguageFile	m_LanguageFile;

	enum { MAX_CACHED_STR = 5 };
	tstring			m_vStrCached[MAX_CACHED_STR];
	int				m_nCurStr;

	typedef map<tstring, tstring>	map_string;

	map_string		m_mapMacros;

};

extern CLanguageTool			g_LangTool;

#define TOLOCAL_SECTION(szString)
#define TOLOCAL2(szString, comments)	(g_LangTool.ToLocalString(szString))
#define TOLOCAL_MARK(szString)	szString
#define TOLOCAL_MARK_T(szString)	_T(szString)
#define TOLOCAL(szString)	(g_LangTool.ToLocalString(szString))
#define TOLOCAL_T(szString)	(g_LangTool.ToLocalString(_T(szString)))

#define LOADLOCALMENU(ID)	(g_LangTool.LoadLocalMenu(ID))
#define ERROR2STR_LOCAL(nError)	(g_LangTool.ToLocalString(Error2Str(nError)))

tstring RemovePrefixOfAcckey(LPCTSTR szStr);

